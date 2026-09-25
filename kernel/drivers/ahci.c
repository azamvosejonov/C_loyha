/* =============================================================================
 *  drivers/ahci.c - SATA disklar: AHCI kontroller drayveri (DMA bilan)
 * =============================================================================
 *
 *  AHCI (Advanced Host Controller Interface) - 2004 yildan beri deyarli har
 *  bir kompyuterdagi SATA kontroller standarti. ATA PIO'dan asosiy farqi:
 *
 *    PIO:  CPU har bir 2 baytni `in` bilan o'zi tashiydi.
 *    AHCI: CPU xotirada "buyruq" tayyorlaydi va kontrollerga "bajar" deydi.
 *          Kontroller ma'lumotni RAM ga O'ZI yozadi (DMA - Direct Memory
 *          Access). CPU bu vaqtda boshqa ish qilishi mumkin.
 *
 *  Registrlar portlarda emas, XOTIRADA (MMIO): PCI BAR5 ("ABAR") ko'rsatgan
 *  fizik manzilga ioremap qilamiz va oddiy o'zgaruvchi kabi o'qiymiz/yozamiz.
 *
 *  XOTIRADAGI TUZILMALAR (har bir port uchun, biz ajratamiz):
 *
 *    CLB -> Command List: 32 ta "slot" (sarlavha), har biri 32 bayt
 *             slot 0: [FIS uzunligi, W (yozish), PRDT soni] [CTBA ─┐]
 *                                                                 │
 *    Command Table <──────────────────────────────────────────────┘
 *       +0x00  CFIS: buyruq FIS (ATA registrlarining "paketi": buyruq, LBA, soni)
 *       +0x80  PRDT: [fizik manzil, bayt soni] ro'yxati - ma'lumot QAYERGA/QAYERDAN
 *
 *    FB  -> Received FIS: qurilma javoblari (status) shu yerga tushadi
 *
 *  BUYRUQ BERISH:  CFIS + PRDT ni to'ldirish -> PxCI registrida slot bitini
 *  yoqish -> kontroller bitni o'zi o'chirganda buyruq tugagan bo'ladi.
 *
 *  BIZNING SODDALASHTIRISHLAR (Linux'dan farq):
 *    * Faqat 0-slot (bir vaqtda bitta buyruq). NCQ bilan 32 tagacha parallel.
 *    * Polling (uzilish o'rniga). Kutish paytida CPU'ni boshqalarga beramiz.
 *    * "Bounce" bufer: ma'lumot avval fizik jihatdan uzluksiz 64 KB buferga,
 *      keyin nusxalanadi. Chaqiruvchi buferi vmalloc xotira bo'lishi mumkin
 *      (fizik sahifalari tarqoq) - PRDT ni har bir sahifa uchun tuzish mashq.
 * ============================================================================= */
#include "drivers/ahci.h"

#include "arch/cpu.h"
#include "arch/tsc.h"
#include "drivers/pci.h"
#include "fs/block.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/layout.h"
#include "mm/pmm.h"
#include "mm/slab.h"
#include "mm/vmalloc.h"
#include "proc/process.h"

/* ---- HBA (butun kontroller) registrlari ---- */
#define HBA_CAP   0x00
#define HBA_GHC   0x04
#define HBA_PI    0x0C                  /* qaysi portlar mavjud (bitmask) */
#define HBA_VS    0x10
#define HBA_CAP2  0x24
#define HBA_BOHC  0x28                  /* BIOS/OS "egalik" almashinuvi */

#define CAP_S64A  (1u << 31)            /* 64-bitli manzillar */
#define GHC_AE    (1u << 31)            /* AHCI rejimini yoqish */
#define CAP2_BOH  (1u << 0)
#define BOHC_OOS  (1u << 1)             /* OS egalik so'raydi */
#define BOHC_BOS  (1u << 0)             /* BIOS hali egalik qilyapti */

/* ---- Port registrlari (0x100 + port * 0x80) ---- */
#define PX_CLB    0x00
#define PX_CLBU   0x04
#define PX_FB     0x08
#define PX_FBU    0x0C
#define PX_IS     0x10
#define PX_IE     0x14
#define PX_CMD    0x18
#define PX_TFD    0x20                  /* task file: status (0-7) va error (8-15) */
#define PX_SIG    0x24
#define PX_SSTS   0x28                  /* SATA holati: DET (0-3) - ulanganmi */
#define PX_SERR   0x30
#define PX_CI     0x38                  /* command issue: bajarilayotgan slotlar */

#define CMD_ST    (1u << 0)             /* buyruqlarni bajarishni boshlash */
#define CMD_FRE   (1u << 4)             /* FIS qabul qilishni yoqish */
#define CMD_FR    (1u << 14)            /* FIS qabul qilish ishlayapti */
#define CMD_CR    (1u << 15)            /* buyruq ro'yxati ishlayapti */

#define IS_TFES   (1u << 30)            /* task file xatosi */

#define TFD_BSY   0x80
#define TFD_DRQ   0x08
#define TFD_ERR   0x01

#define SIG_ATA   0x00000101            /* oddiy SATA disk */
#define SIG_ATAPI 0xEB140101            /* CD/DVD */

#define FIS_TYPE_REG_H2D 0x27           /* host -> qurilma: registrlar */

#define ATA_READ_DMA_EXT   0x25
#define ATA_WRITE_DMA_EXT  0x35
#define ATA_FLUSH_EXT      0xEA
#define ATA_IDENTIFY       0xEC

#define BOUNCE_SIZE  (64 * 1024)        /* bitta buyruqda 128 sektor */
#define TIMEOUT_MS   5000

struct cmd_header {
    uint16_t flags;                     /* 0-4: CFIS uzunligi (dword), 6: W, ... */
    uint16_t prdtl;                     /* PRDT yozuvlari soni */
    volatile uint32_t prdbc;            /* uzatilgan baytlar (kontroller yozadi) */
    uint32_t ctba, ctbau;               /* command table manzili (128 ga tekis) */
    uint32_t reserved[4];
};

struct prd {
    uint32_t dba, dbau;                 /* ma'lumot bufferining fizik manzili */
    uint32_t reserved;
    uint32_t dbc;                       /* bayt soni - 1 (0-21 bitlar), 31: uzilish */
};

struct cmd_table {
    uint8_t cfis[64];
    uint8_t acmd[16];                   /* ATAPI buyruqlari uchun */
    uint8_t reserved[48];
    struct prd prdt[1];
};

struct ahci_port {
    volatile uint8_t *regs;             /* shu portning MMIO registrlari */
    struct cmd_header *clb;
    struct cmd_table *ct;
    uint64_t ct_phys;
    uint8_t *bounce;
    uint64_t bounce_phys;
    struct block_device bd;
};

static inline uint32_t rd(volatile uint8_t *base, uint32_t off)
{
    return *(volatile uint32_t *)(base + off);
}

static inline void wr(volatile uint8_t *base, uint32_t off, uint32_t v)
{
    *(volatile uint32_t *)(base + off) = v;
}

/* (reg & mask) == want bo'lishini kutish. false - vaqt tugadi. */
static bool wait_reg(volatile uint8_t *base, uint32_t off, uint32_t mask, uint32_t want,
                     uint32_t ms)
{
    uint64_t start = rdtsc(), limit = tsc_khz * ms;
    while ((rd(base, off) & mask) != want) {
        if (rdtsc() - start > limit)
            return false;
        cpu_pause();
    }
    return true;
}

/* Portni to'xtatish: ST va FRE ni o'chirib, CR va FR o'chishini kutamiz.
 * CLB/FB manzillarini faqat to'xtagan portda o'zgartirish mumkin. */
static bool port_stop(volatile uint8_t *p)
{
    wr(p, PX_CMD, rd(p, PX_CMD) & ~CMD_ST);
    if (!wait_reg(p, PX_CMD, CMD_CR, 0, 500))
        return false;
    wr(p, PX_CMD, rd(p, PX_CMD) & ~CMD_FRE);
    return wait_reg(p, PX_CMD, CMD_FR, 0, 500);
}

/* Bitta ATA buyrug'ini 0-slotda bajarish. bytes > 0 bo'lsa bounce bufer ishlatiladi. */
static int port_exec(struct ahci_port *ap, uint8_t command, uint64_t lba, uint16_t count,
                     uint32_t bytes, bool write)
{
    volatile uint8_t *p = ap->regs;
    /* Oldingi buyruq tugaganmi va qurilma band emasmi? */
    if (!wait_reg(p, PX_TFD, TFD_BSY | TFD_DRQ, 0, TIMEOUT_MS))
        return -1;

    struct cmd_header *h = &ap->clb[0];
    h->flags = (uint16_t)((sizeof(uint32_t) * 5 / 4) | (write ? (1u << 6) : 0));  /* CFIS 5 dword */
    h->prdtl = bytes ? 1 : 0;
    h->prdbc = 0;

    struct cmd_table *t = ap->ct;
    memset(t, 0, sizeof(*t));
    uint8_t *f = t->cfis;
    f[0] = FIS_TYPE_REG_H2D;
    f[1] = 0x80;                        /* C=1: bu buyruq (control emas) */
    f[2] = command;
    f[4] = (uint8_t)lba;
    f[5] = (uint8_t)(lba >> 8);
    f[6] = (uint8_t)(lba >> 16);
    f[7] = 1u << 6;                     /* LBA rejimi */
    f[8] = (uint8_t)(lba >> 24);
    f[9] = (uint8_t)(lba >> 32);
    f[10] = (uint8_t)(lba >> 40);
    f[12] = (uint8_t)count;
    f[13] = (uint8_t)(count >> 8);
    if (bytes) {
        t->prdt[0].dba = (uint32_t)ap->bounce_phys;
        t->prdt[0].dbau = (uint32_t)(ap->bounce_phys >> 32);
        t->prdt[0].dbc = bytes - 1;
    }

    wr(p, PX_IS, 0xFFFFFFFF);           /* eski holat bitlarini tozalash (1 yozib) */
    /* Xotiraga yozganlarimiz kontroller o'qishidan OLDIN ko'rinishi kerak.
     * x86 da yozishlar tartibi saqlanadi, kompilyator to'sig'i yetarli. */
    __asm__ volatile("" ::: "memory");
    wr(p, PX_CI, 1);                    /* 0-slot: BAJAR! */

    uint64_t start = rdtsc(), limit = tsc_khz * TIMEOUT_MS;
    uint32_t spins = 0;
    while (rd(p, PX_CI) & 1) {
        if (rd(p, PX_IS) & IS_TFES)
            break;
        if (rdtsc() - start > limit) {
            kprintf("[ahci] %s: buyruq 0x%x vaqt tugadi\n", ap->bd.name, command);
            return -1;
        }
        if (++spins % 1024 == 0 && current && !current->is_idle)
            proc_yield();               /* uzoq kutish - boshqalar ishlasin */
        cpu_pause();
    }
    uint32_t tfd = rd(p, PX_TFD);
    if ((rd(p, PX_IS) & IS_TFES) || (tfd & TFD_ERR)) {
        kprintf("[ahci] %s: buyruq 0x%x xato, TFD=0x%x\n", ap->bd.name, command, tfd);
        return -1;
    }
    return 0;
}

static int ahci_rw(struct block_device *bd, uint64_t lba, uint32_t count, void *buf, bool write)
{
    struct ahci_port *ap = bd->priv;
    uint8_t *ptr = buf;
    const uint32_t max_sectors = BOUNCE_SIZE / 512;
    while (count) {
        uint32_t n = count > max_sectors ? max_sectors : count;
        if (write)
            memcpy(ap->bounce, ptr, n * 512);
        if (port_exec(ap, write ? ATA_WRITE_DMA_EXT : ATA_READ_DMA_EXT, lba, (uint16_t)n,
                      n * 512, write))
            return -1;
        if (!write)
            memcpy(ptr, ap->bounce, n * 512);
        ptr += n * 512;
        lba += n;
        count -= n;
    }
    if (write && port_exec(ap, ATA_FLUSH_EXT, 0, 0, 0, false))  /* disk keshi -> plastina */
        return -1;
    return 0;
}

static int ahci_read(struct block_device *bd, uint64_t lba, uint32_t count, void *buf)
{
    return ahci_rw(bd, lba, count, buf, false);
}

static int ahci_write(struct block_device *bd, uint64_t lba, uint32_t count, const void *buf)
{
    return ahci_rw(bd, lba, count, (void *)buf, true);
}

static const struct block_ops ahci_ops = {
    .read = ahci_read,
    .write = ahci_write,
};

static void ata_model(char *out, const uint16_t *id)
{
    for (int i = 0; i < 20; i++) {
        out[2 * i] = (char)(id[27 + i] >> 8);
        out[2 * i + 1] = (char)id[27 + i];
    }
    int len = 40;
    out[len] = '\0';
    while (len > 0 && out[len - 1] == ' ')
        out[--len] = '\0';
}

static void port_init(volatile uint8_t *abar, int num, bool dma64)
{
    volatile uint8_t *p = abar + 0x100 + num * 0x80;
    uint32_t ssts = rd(p, PX_SSTS);
    if ((ssts & 0xF) != 3)              /* DET=3: qurilma bor va aloqa o'rnatilgan */
        return;
    uint32_t sig = rd(p, PX_SIG);
    if (sig == SIG_ATAPI) {
        kprintf("[ahci] port %d: ATAPI (CD/DVD) - o'tkazib yuborildi\n", num);
        return;
    }
    if (sig != SIG_ATA) {
        kprintf("[ahci] port %d: noma'lum qurilma (imzo 0x%x)\n", num, sig);
        return;
    }
    if (!port_stop(p)) {
        kprintf("[ahci] port %d: to'xtamadi\n", num);
        return;
    }

    struct ahci_port *ap = kzalloc(sizeof(*ap));
    if (!ap)
        return;
    ap->regs = p;
    /* 64-bit manzillarni qo'llamaydigan kontroller uchun - 4 GB dan past xotira. */
    unsigned gfp = GFP_ZERO | (dma64 ? 0 : GFP_DMA32);
    uint64_t clb_phys = pmm_alloc_page(gfp);   /* 1 KB command list + 256 B FIS */
    ap->ct_phys = pmm_alloc_page(gfp);
    ap->bounce_phys = pmm_alloc_pages(BOUNCE_SIZE / 4096, gfp);
    if (!clb_phys || !ap->ct_phys || !ap->bounce_phys) {
        kprintf("[ahci] port %d: xotira yetmadi\n", num);
        kfree(ap);
        return;
    }
    ap->clb = phys_to_virt(clb_phys);
    ap->ct = phys_to_virt(ap->ct_phys);
    ap->bounce = phys_to_virt(ap->bounce_phys);
    uint64_t fb_phys = clb_phys + 1024;
    ap->clb[0].ctba = (uint32_t)ap->ct_phys;
    ap->clb[0].ctbau = (uint32_t)(ap->ct_phys >> 32);

    wr(p, PX_CLB, (uint32_t)clb_phys);
    wr(p, PX_CLBU, (uint32_t)(clb_phys >> 32));
    wr(p, PX_FB, (uint32_t)fb_phys);
    wr(p, PX_FBU, (uint32_t)(fb_phys >> 32));
    wr(p, PX_SERR, 0xFFFFFFFF);         /* eski xatolar */
    wr(p, PX_IS, 0xFFFFFFFF);
    wr(p, PX_IE, 0);                    /* polling - uzilishlar kerak emas */
    wr(p, PX_CMD, rd(p, PX_CMD) | CMD_FRE);
    wr(p, PX_CMD, rd(p, PX_CMD) | CMD_ST);

    /* IDENTIFY DEVICE: 512 bayt ma'lumot - model, hajm, imkoniyatlar. */
    ksnprintf(ap->bd.name, sizeof(ap->bd.name), "ahci%d", num);   /* vaqtincha, xabarlar uchun */
    if (port_exec(ap, ATA_IDENTIFY, 0, 0, 512, false)) {
        kprintf("[ahci] port %d: IDENTIFY muvaffaqiyatsiz\n", num);
        return;                         /* xotira atayin qaytarilmaydi - port ishlashda davom etishi mumkin */
    }
    const uint16_t *id = (const uint16_t *)ap->bounce;
    ata_model(ap->bd.model, id);
    bool lba48 = (id[83] >> 10) & 1;
    ap->bd.sectors = lba48 ? (uint64_t)id[100] | ((uint64_t)id[101] << 16) |
                                 ((uint64_t)id[102] << 32) | ((uint64_t)id[103] << 48)
                           : (uint64_t)id[60] | ((uint64_t)id[61] << 16);
    ap->bd.sector_size = 512;
    ap->bd.ops = &ahci_ops;
    ap->bd.priv = ap;
    blkdev_assign_sd(&ap->bd);
    kprintf("[ahci] port %d: %s\n", num, ap->bd.name);
    blkdev_register(&ap->bd);
}

static int ahci_probe(struct pci_device *pdev)
{
    struct pci_bar *bar = &pdev->bars[5];
    if (bar->is_io || !bar->base) {
        kprintf("[ahci] ABAR (BAR5) topilmadi\n");
        return -1;
    }
    pci_enable(pdev);                   /* MMIO + bus master (DMA uchun SHART) */
    size_t size = bar->size ? bar->size : 0x1100;
    volatile uint8_t *abar = ioremap(bar->base, size);
    if (!abar)
        return -1;

    /* BIOS/UEFI kontrollerni hali "ushlab" turgan bo'lishi mumkin - so'raymiz. */
    if (rd(abar, HBA_CAP2) & CAP2_BOH) {
        wr(abar, HBA_BOHC, rd(abar, HBA_BOHC) | BOHC_OOS);
        wait_reg(abar, HBA_BOHC, BOHC_BOS, 0, 2000);
    }
    wr(abar, HBA_GHC, rd(abar, HBA_GHC) | GHC_AE);
    uint32_t cap = rd(abar, HBA_CAP), pi = rd(abar, HBA_PI), vs = rd(abar, HBA_VS);
    kprintf("[ahci] AHCI %u.%u, %u port, 64-bit DMA: %s\n", vs >> 16, (vs >> 8) & 0xFF,
            (cap & 0x1F) + 1, (cap & CAP_S64A) ? "ha" : "yo'q");
    for (int i = 0; i < 32; i++)
        if (pi & (1u << i))
            port_init(abar, i, (cap & CAP_S64A) != 0);
    return 0;
}

static struct pci_driver ahci_driver = {
    .name = "ahci",
    .vendor = PCI_ANY_ID,
    .device = PCI_ANY_ID,
    .class_code = 0x01,                 /* ommaviy xotira */
    .subclass = 0x06,                   /* SATA */
    .prog_if = 0x01,                    /* AHCI 1.0 */
    .probe = ahci_probe,
};

void ahci_init(void)
{
    pci_register_driver(&ahci_driver);
}
