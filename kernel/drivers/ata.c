/* =============================================================================
 *  drivers/ata.c - IDE/ATA disk drayveri (PIO rejimi)
 * =============================================================================
 *
 *  ATA (IDE, PATA) - 1986 yildan beri mavjud disk interfeysi. Zamonaviy SATA
 *  disklar ham "IDE moslik rejimida" shu interfeysni beradi (BIOS'da "IDE/
 *  Legacy" sozlamasi), QEMU'ning sukut mashinasi (pc/i440fx) ham shu.
 *
 *  Kontroller - bir nechta I/O PORT (xotira emas, `in`/`out` instruksiyalari):
 *
 *   port  (primary 0x1F0)   o'qilganda           yozilganda
 *   +0    DATA              sektor ma'lumoti (16 bit)
 *   +1    ERROR / FEATURES  xato kodi            qo'shimcha parametr
 *   +2    SECCOUNT          -                    nechta sektor
 *   +3..5 LBA_LO/MID/HI     -                    sektor raqami (24 bit bo'laklab)
 *   +6    DRIVE             -                    master/slave + LBA rejimi
 *   +7    STATUS / COMMAND  holat bitlari        buyruq
 *   ctrl (0x3F6)            ALT STATUS           DEVICE CONTROL (nIEN, SRST)
 *
 *  PIO (Programmed I/O): har bir 2 baytni CPU o'zi `in` bilan o'qiydi. Sekin
 *  (~ 3-16 MB/s) va CPU band, lekin eng sodda va HAMMA joyda ishlaydi.
 *  Tez usul - DMA (disk RAM ga o'zi yozadi): uni AHCI drayverida ko'ramiz.
 *
 *  BUYRUQ PROTOKOLI (o'qish):
 *    1. BSY=0 ni kutish                     (disk band emas)
 *    2. DRIVE, SECCOUNT, LBA registrlarini yozish
 *    3. COMMAND = READ SECTORS EXT (0x24)
 *    4. har bir sektor uchun: BSY=0 va DRQ=1 ni kutish, 256 ta so'z o'qish
 *
 *  Uzilishlar o'chirilgan (nIEN=1): holatni POLLING bilan tekshiramiz. Bu
 *  oddiyroq; kutish paytida taymer baribir boshqa jarayonlarga o'tadi
 *  (yadro preemptible), shuning uchun tizim "qotib" qolmaydi.
 * ============================================================================= */
#include "drivers/ata.h"

#include "arch/cpu.h"
#include "arch/io.h"
#include "arch/tsc.h"
#include "drivers/pci.h"
#include "fs/block.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/slab.h"

/* Registrlar (base ga nisbatan) */
#define REG_DATA     0
#define REG_ERROR    1
#define REG_SECCOUNT 2
#define REG_LBA_LO   3
#define REG_LBA_MID  4
#define REG_LBA_HI   5
#define REG_DRIVE    6
#define REG_STATUS   7
#define REG_COMMAND  7

/* STATUS bitlari */
#define ST_ERR  0x01                    /* xato - ERROR registriga qarang */
#define ST_DRQ  0x08                    /* ma'lumot uzatishga tayyor */
#define ST_DF   0x20                    /* qurilma nosozligi */
#define ST_BSY  0x80                    /* band - boshqa registrlar ishonchsiz */

/* DEVICE CONTROL bitlari */
#define CTL_NIEN 0x02                   /* uzilishlarni o'chirish */
#define CTL_SRST 0x04                   /* dasturiy reset */

/* Buyruqlar */
#define CMD_READ_PIO       0x20         /* LBA28 */
#define CMD_READ_PIO_EXT   0x24         /* LBA48 */
#define CMD_WRITE_PIO      0x30
#define CMD_WRITE_PIO_EXT  0x34
#define CMD_FLUSH          0xE7
#define CMD_FLUSH_EXT      0xEA
#define CMD_IDENTIFY       0xEC

#define TIMEOUT_US 5000000              /* 5 soniya - disk aylanib ulgurishi uchun */

struct ata_drive {
    uint16_t base, ctrl;
    bool slave;
    bool lba48;
    struct block_device bd;
};

/* ALT STATUS ni o'qish INTERRUPT holatini tozalamaydi va ~100 ns oladi.
 * 4 marta o'qish = ~400 ns: drayv tanlangandan keyin kutish kerak bo'lgan vaqt. */
static void delay400(const struct ata_drive *d)
{
    for (int i = 0; i < 4; i++)
        inb(d->ctrl);
}

/* BSY=0 bo'lishini kutish. Qaytaradi: oxirgi status yoki -1 (vaqt tugadi). */
static int wait_not_busy(const struct ata_drive *d)
{
    uint64_t start = rdtsc(), limit = tsc_khz * (TIMEOUT_US / 1000);
    for (;;) {
        uint8_t st = inb(d->base + REG_STATUS);
        if (!(st & ST_BSY))
            return st;
        if (rdtsc() - start > limit)
            return -1;
    }
}

/* Keyingi sektorni uzatishga tayyorlikni kutish: BSY=0 va (DRQ=1 yoki xato). */
static int wait_drq(const struct ata_drive *d)
{
    int st = wait_not_busy(d);
    if (st < 0)
        return -1;
    if (st & (ST_ERR | ST_DF)) {
        kprintf("[ata]  %s: xato, status=0x%x error=0x%x\n", d->bd.name, st,
                inb(d->base + REG_ERROR));
        return -1;
    }
    return (st & ST_DRQ) ? 0 : -1;
}

/* Buyruq registrlarini to'ldirish (LBA28 yoki LBA48). */
static void setup_lba(const struct ata_drive *d, uint64_t lba, uint32_t count)
{
    uint16_t b = d->base;
    if (d->lba48) {
        /* LBA48: har bir registrga IKKI marta yoziladi - avval yuqori bayt
         * ("oldingi" qiymat), keyin quyi bayt. Kontroller ikkalasini eslaydi. */
        outb(b + REG_DRIVE, 0x40 | (d->slave << 4));
        delay400(d);
        outb(b + REG_SECCOUNT, (uint8_t)(count >> 8));
        outb(b + REG_LBA_LO, (uint8_t)(lba >> 24));
        outb(b + REG_LBA_MID, (uint8_t)(lba >> 32));
        outb(b + REG_LBA_HI, (uint8_t)(lba >> 40));
        outb(b + REG_SECCOUNT, (uint8_t)count);
        outb(b + REG_LBA_LO, (uint8_t)lba);
        outb(b + REG_LBA_MID, (uint8_t)(lba >> 8));
        outb(b + REG_LBA_HI, (uint8_t)(lba >> 16));
    } else {
        /* LBA28: 0xE0 = LBA rejimi (6-bit) + 1 ga majburiy bitlar; 24-27 bitlar DRIVE da. */
        outb(b + REG_DRIVE, 0xE0 | (d->slave << 4) | ((lba >> 24) & 0x0F));
        delay400(d);
        outb(b + REG_SECCOUNT, (uint8_t)count);    /* 0 = 256 sektor */
        outb(b + REG_LBA_LO, (uint8_t)lba);
        outb(b + REG_LBA_MID, (uint8_t)(lba >> 8));
        outb(b + REG_LBA_HI, (uint8_t)(lba >> 16));
    }
}

static int ata_rw(struct block_device *bd, uint64_t lba, uint32_t count, void *buf, bool write)
{
    struct ata_drive *d = bd->priv;
    uint8_t *p = buf;
    while (count) {
        /* LBA28 bitta buyruqda 256 tagacha, LBA48 - 65536 tagacha sektor. */
        uint32_t n = count > 256 ? 256 : count;
        if (wait_not_busy(d) < 0)
            return -1;
        setup_lba(d, lba, n);
        outb(d->base + REG_COMMAND, write ? (d->lba48 ? CMD_WRITE_PIO_EXT : CMD_WRITE_PIO)
                                          : (d->lba48 ? CMD_READ_PIO_EXT : CMD_READ_PIO));
        for (uint32_t i = 0; i < n; i++, p += 512) {
            delay400(d);
            if (wait_drq(d) < 0)
                return -1;
            if (write)
                outsw(d->base + REG_DATA, p, 256);
            else
                insw(d->base + REG_DATA, p, 256);
        }
        if (write) {
            /* Diskning ichki keshi (8-256 MB) ham bor! FLUSH bo'lmasa, tok o'chganda
             * "yozildi" deb o'ylagan ma'lumot yo'qolishi mumkin. */
            outb(d->base + REG_COMMAND, d->lba48 ? CMD_FLUSH_EXT : CMD_FLUSH);
            int st = wait_not_busy(d);
            if (st < 0 || (st & (ST_ERR | ST_DF)))
                return -1;
        }
        lba += n;
        count -= n;
    }
    return 0;
}

static int ata_read(struct block_device *bd, uint64_t lba, uint32_t count, void *buf)
{
    return ata_rw(bd, lba, count, buf, false);
}

static int ata_write(struct block_device *bd, uint64_t lba, uint32_t count, const void *buf)
{
    return ata_rw(bd, lba, count, (void *)buf, true);
}

static const struct block_ops ata_ops = {
    .read = ata_read,
    .write = ata_write,
};

/* ATA satrlari (model nomi) so'z ichida baytlari almashgan holda saqlanadi:
 * "QEMU HARDDISK" -> "EQUMH RADDSI K". Har bir juftlikni qaytarib, oxiridagi
 * bo'shliqlarni olib tashlaymiz. */
static void ata_string(char *out, const uint16_t *words, int nwords)
{
    for (int i = 0; i < nwords; i++) {
        out[2 * i] = (char)(words[i] >> 8);
        out[2 * i + 1] = (char)words[i];
    }
    int len = nwords * 2;
    out[len] = '\0';
    while (len > 0 && out[len - 1] == ' ')
        out[--len] = '\0';
}

/* Kanaldagi bitta drayvni aniqlash. */
static void probe_drive(uint16_t base, uint16_t ctrl, bool slave)
{
    struct ata_drive tmp = { .base = base, .ctrl = ctrl, .slave = slave };
    outb(ctrl, CTL_NIEN);               /* polling: uzilishlar kerak emas */
    outb(base + REG_DRIVE, 0xA0 | (slave << 4));
    delay400(&tmp);
    if (inb(base + REG_STATUS) == 0xFF) /* "suzuvchi shina" - kanalda hech narsa yo'q */
        return;
    outb(base + REG_SECCOUNT, 0);
    outb(base + REG_LBA_LO, 0);
    outb(base + REG_LBA_MID, 0);
    outb(base + REG_LBA_HI, 0);
    outb(base + REG_COMMAND, CMD_IDENTIFY);
    delay400(&tmp);
    if (inb(base + REG_STATUS) == 0)    /* drayv yo'q */
        return;
    if (wait_not_busy(&tmp) < 0)
        return;
    /* ATAPI (CD-ROM) va SATA qurilmalar IDENTIFY ni rad etadi va LBA_MID/HI ga
     * o'z "imzosini" yozadi: ATAPI = 0x14/0xEB. Bizga faqat disklar kerak. */
    uint8_t mid = inb(base + REG_LBA_MID), hi = inb(base + REG_LBA_HI);
    if (mid || hi) {
        if (mid == 0x14 && hi == 0xEB)
            kprintf("[ata]  0x%x/%s: ATAPI qurilma (CD/DVD) - o'tkazib yuborildi\n", base,
                    slave ? "slave" : "master");
        return;
    }
    if (wait_drq(&tmp) < 0)
        return;
    uint16_t id[256];
    insw(base + REG_DATA, id, 256);

    struct ata_drive *d = kzalloc(sizeof(*d));
    if (!d)
        return;
    *d = tmp;
    /* IDENTIFY javobi (ATA-8 standarti):
     *   so'z 27-46  - model nomi (40 belgi)
     *   so'z 60-61  - LBA28 sektorlar soni
     *   so'z 83     - 10-bit: LBA48 qo'llanadi
     *   so'z 100-103 - LBA48 sektorlar soni */
    d->lba48 = (id[83] >> 10) & 1;
    uint64_t sectors = d->lba48 ? (uint64_t)id[100] | ((uint64_t)id[101] << 16) |
                                      ((uint64_t)id[102] << 32) | ((uint64_t)id[103] << 48)
                                : (uint64_t)id[60] | ((uint64_t)id[61] << 16);
    ata_string(d->bd.model, &id[27], 20);
    d->bd.sectors = sectors;
    d->bd.sector_size = 512;
    d->bd.ops = &ata_ops;
    d->bd.priv = d;
    blkdev_assign_sd(&d->bd);
    kprintf("[ata]  0x%x/%s: %s, LBA%d\n", base, slave ? "slave" : "master", d->bd.name,
            d->lba48 ? 48 : 28);
    blkdev_register(&d->bd);
}

static int ata_probe(struct pci_device *pdev)
{
    pci_enable(pdev);
    /* prog_if 0-bit: primary kanal "native" rejimda (portlar BAR0/BAR1 da),
     * 2-bit: secondary (BAR2/BAR3). Aks holda - eski qat'iy portlar. */
    uint16_t pbase = 0x1F0, pctrl = 0x3F6, sbase = 0x170, sctrl = 0x376;
    if ((pdev->prog_if & 0x01) && pdev->bars[0].is_io) {
        pbase = (uint16_t)pdev->bars[0].base;
        pctrl = (uint16_t)pdev->bars[1].base + 2;
    }
    if ((pdev->prog_if & 0x04) && pdev->bars[2].is_io) {
        sbase = (uint16_t)pdev->bars[2].base;
        sctrl = (uint16_t)pdev->bars[3].base + 2;
    }
    probe_drive(pbase, pctrl, false);
    probe_drive(pbase, pctrl, true);
    probe_drive(sbase, sctrl, false);
    probe_drive(sbase, sctrl, true);
    return 0;
}

static struct pci_driver ata_driver = {
    .name = "ata-pio",
    .vendor = PCI_ANY_ID,
    .device = PCI_ANY_ID,
    .class_code = 0x01,                 /* ommaviy xotira */
    .subclass = 0x01,                   /* IDE kontroller */
    .prog_if = PCI_ANY_CLASS,
    .probe = ata_probe,
};

void ata_init(void)
{
    pci_register_driver(&ata_driver);
}
