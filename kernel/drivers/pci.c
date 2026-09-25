/* =============================================================================
 *  drivers/pci.c - PCI SHINASINI SKANERLASH va drayverlarni ulash
 * =============================================================================
 *
 *  PCI - zamonaviy kompyuterdagi deyarli BARCHA qurilmalar ulangan shina:
 *  disk kontrollerlari (AHCI, NVMe), USB (xHCI), tarmoq kartalari, videokarta...
 *
 *  Har bir qurilma (bus, device, function) manzili bilan topiladi va 256
 *  baytlik "KONFIGURATSIYA MAYDONI"ga ega:
 *     0x00  vendor ID (ishlab chiqaruvchi, masalan 0x8086 = Intel)
 *     0x02  device ID
 *     0x08  revision, prog_if, subclass, class  <- qurilma TURI (01:06:01 = AHCI)
 *     0x0E  header type (7-bit = ko'p funksiyali qurilma)
 *     0x10  BAR0..BAR5 - qurilma registrlari qayerda (xotira yoki port)
 *     0x3C  IRQ liniyasi
 *
 *  KONFIGURATSIYAGA KIRISH: klassik usul - 0xCF8 portiga manzil yozib, 0xCFC
 *  dan o'qish (barcha x86 kompyuterlarda ishlaydi). PCIe da yana ECAM
 *  (xotira orqali, 4 KB maydon) bor - mashq sifatida qoldirildi.
 *
 *  BAR HAJMINI ANIQLASH: BAR ga 0xFFFFFFFF yozamiz, qayta o'qiymiz - qurilma
 *  "o'zgarmas" nollarni qaytaradi. ~(qiymat) + 1 = hajm. Keyin asl qiymatni
 *  qaytarib yozamiz. (Klassik hiyla - barcha OS'lar shunday qiladi.)
 *
 *  DRAYVER MODELI: drayver "men 01:06:01 sinfidagi qurilmalarni boshqaraman"
 *  deb ro'yxatdan o'tadi (pci_register_driver). Skanerlashdan keyin har bir
 *  qurilmaga mos drayverning probe() funksiyasi chaqiriladi. Linux ham aynan
 *  shunday ishlaydi (struct pci_driver, .id_table, .probe).
 * ============================================================================= */
#include "drivers/pci.h"

#include "arch/io.h"
#include "lib/kprintf.h"
#include "lib/spinlock.h"
#include "mm/slab.h"

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

static LIST_HEAD(devices);
static LIST_HEAD(drivers);
static int device_count;
static spinlock_t pci_lock = SPINLOCK_INIT("pci-config");

static uint32_t cfg_addr(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off)
{
    /* bit31: yoqish | bus | device | function | registr (4 ga tekislangan) */
    return (1u << 31) | ((uint32_t)bus << 16) | ((uint32_t)dev << 11) |
           ((uint32_t)func << 8) | (off & 0xFC);
}

static uint32_t raw_read32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off)
{
    spin_lock(&pci_lock);               /* manzil+ma'lumot juftligi atomar bo'lishi shart */
    outl(PCI_CONFIG_ADDR, cfg_addr(bus, dev, func, off));
    uint32_t v = inl(PCI_CONFIG_DATA);
    spin_unlock(&pci_lock);
    return v;
}

static void raw_write32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off, uint32_t val)
{
    spin_lock(&pci_lock);
    outl(PCI_CONFIG_ADDR, cfg_addr(bus, dev, func, off));
    outl(PCI_CONFIG_DATA, val);
    spin_unlock(&pci_lock);
}

uint32_t pci_read32(const struct pci_device *d, uint8_t off)
{
    return raw_read32(d->bus, d->dev, d->func, off);
}

uint16_t pci_read16(const struct pci_device *d, uint8_t off)
{
    return (uint16_t)(pci_read32(d, off) >> ((off & 2) * 8));
}

uint8_t pci_read8(const struct pci_device *d, uint8_t off)
{
    return (uint8_t)(pci_read32(d, off) >> ((off & 3) * 8));
}

void pci_write32(const struct pci_device *d, uint8_t off, uint32_t val)
{
    raw_write32(d->bus, d->dev, d->func, off, val);
}

void pci_write16(const struct pci_device *d, uint8_t off, uint16_t val)
{
    uint32_t v = pci_read32(d, off & 0xFC);
    int shift = (off & 2) * 8;
    v = (v & ~(0xFFFFu << shift)) | ((uint32_t)val << shift);
    pci_write32(d, off & 0xFC, v);
}

void pci_enable(struct pci_device *d)
{
    uint16_t cmd = pci_read16(d, 0x04);
    cmd |= (1u << 0) | (1u << 1) | (1u << 2);   /* I/O, xotira, bus master (DMA) */
    cmd &= ~(1u << 10);                         /* INTx uzilishlarini o'chirmaymiz */
    pci_write16(d, 0x04, cmd);
}

uint8_t pci_find_capability(const struct pci_device *d, uint8_t id)
{
    if (!(pci_read16(d, 0x06) & (1u << 4)))     /* status: capability ro'yxati bormi? */
        return 0;
    uint8_t ptr = pci_read8(d, 0x34) & 0xFC;
    for (int guard = 0; ptr && guard < 48; guard++) {
        if (pci_read8(d, ptr) == id)
            return ptr;
        ptr = pci_read8(d, ptr + 1) & 0xFC;
    }
    return 0;
}

static void read_bars(struct pci_device *d)
{
    if ((d->header_type & 0x7F) != 0)   /* faqat oddiy qurilmalar (ko'priklar emas) */
        return;
    /* Hajmni aniqlash paytida qurilma bu manzillarga javob bermasin. */
    uint16_t cmd = pci_read16(d, 0x04);
    pci_write16(d, 0x04, cmd & ~3u);
    for (int i = 0; i < 6; i++) {
        uint8_t off = (uint8_t)(0x10 + i * 4);
        uint32_t orig = pci_read32(d, off);
        pci_write32(d, off, 0xFFFFFFFF);
        uint32_t mask = pci_read32(d, off);
        pci_write32(d, off, orig);
        struct pci_bar *b = &d->bars[i];
        if (orig & 1) {                 /* I/O port BAR */
            b->is_io = true;
            b->base = orig & ~3u;
            b->size = (~(mask & ~3u) + 1) & 0xFFFF;
            continue;
        }
        b->prefetchable = orig & 8;
        uint64_t base = orig & ~0xFu;
        uint64_t size_mask = mask & ~0xFu;
        if (((orig >> 1) & 3) == 2 && i < 5) {    /* 64-bitli BAR: keyingisi yuqori qism */
            uint8_t off2 = (uint8_t)(off + 4);
            uint32_t orig_hi = pci_read32(d, off2);
            pci_write32(d, off2, 0xFFFFFFFF);
            uint32_t mask_hi = pci_read32(d, off2);
            pci_write32(d, off2, orig_hi);
            base |= (uint64_t)orig_hi << 32;
            size_mask |= (uint64_t)mask_hi << 32;
            b->is_64 = true;
            b->base = base;
            b->size = size_mask ? ~size_mask + 1 : 0;
            i++;                        /* keyingi BAR ishlatildi */
            continue;
        }
        b->base = base;
        b->size = size_mask ? (uint32_t)(~(uint32_t)size_mask + 1) : 0;
    }
    pci_write16(d, 0x04, cmd);
}

static void scan_bus(uint8_t bus);

static void scan_function(uint8_t bus, uint8_t dev, uint8_t func)
{
    uint32_t id = raw_read32(bus, dev, func, 0);
    if ((id & 0xFFFF) == 0xFFFF)
        return;
    struct pci_device *d = kzalloc(sizeof(*d));
    if (!d)
        return;
    d->bus = bus, d->dev = dev, d->func = func;
    d->vendor = id & 0xFFFF;
    d->device = id >> 16;
    uint32_t cls = pci_read32(d, 0x08);
    d->revision = cls & 0xFF;
    d->prog_if = (cls >> 8) & 0xFF;
    d->subclass = (cls >> 16) & 0xFF;
    d->class_code = cls >> 24;
    d->header_type = pci_read8(d, 0x0E);
    d->irq_line = pci_read8(d, 0x3C);
    d->irq_pin = pci_read8(d, 0x3D);
    read_bars(d);
    list_add_tail(&d->node, &devices);
    device_count++;

    /* PCI-PCI ko'prik (06:04): uning orqasida boshqa shina bor - rekursiya. */
    if (d->class_code == 0x06 && d->subclass == 0x04) {
        uint8_t secondary = pci_read8(d, 0x19);
        if (secondary > bus)
            scan_bus(secondary);
    }
}

static void scan_bus(uint8_t bus)
{
    for (uint8_t dev = 0; dev < 32; dev++) {
        if ((raw_read32(bus, dev, 0, 0) & 0xFFFF) == 0xFFFF)
            continue;
        scan_function(bus, dev, 0);
        uint8_t ht = (uint8_t)(raw_read32(bus, dev, 0, 0x0C) >> 16);
        if (ht & 0x80)                  /* ko'p funksiyali qurilma */
            for (uint8_t f = 1; f < 8; f++)
                scan_function(bus, dev, f);
    }
}

const char *pci_class_name(uint8_t c, uint8_t s)
{
    switch (c) {
    case 0x01:
        switch (s) {
        case 0x01: return "IDE kontroller";
        case 0x06: return "SATA (AHCI) kontroller";
        case 0x08: return "NVMe kontroller";
        default:   return "Disk kontroller";
        }
    case 0x02: return s == 0x00 ? "Ethernet kontroller" : "Tarmoq kontroller";
    case 0x03: return "Video kontroller";
    case 0x04: return "Multimedia (audio)";
    case 0x06:
        switch (s) {
        case 0x00: return "Host ko'prik";
        case 0x01: return "ISA ko'prik";
        case 0x04: return "PCI-PCI ko'prik";
        default:   return "Ko'prik";
        }
    case 0x0C:
        switch (s) {
        case 0x03: return "USB kontroller";
        case 0x05: return "SMBus";
        default:   return "Seriya shina kontroller";
        }
    default:   return "Boshqa qurilma";
    }
}

static bool driver_matches(const struct pci_driver *drv, const struct pci_device *d)
{
    return (drv->vendor == PCI_ANY_ID || drv->vendor == d->vendor) &&
           (drv->device == PCI_ANY_ID || drv->device == d->device) &&
           (drv->class_code == PCI_ANY_CLASS || drv->class_code == d->class_code) &&
           (drv->subclass == PCI_ANY_CLASS || drv->subclass == d->subclass) &&
           (drv->prog_if == PCI_ANY_CLASS || drv->prog_if == d->prog_if);
}

static void try_bind(struct pci_device *d)
{
    struct pci_driver *drv;
    list_for_each_entry(drv, &drivers, node) {
        if (d->driver || !driver_matches(drv, d))
            continue;
        d->driver = drv;
        if (drv->probe(d) != 0)
            d->driver = NULL;
    }
}

void pci_register_driver(struct pci_driver *drv)
{
    list_add_tail(&drv->node, &drivers);
    struct pci_device *d;
    list_for_each_entry(d, &devices, node)
        if (!d->driver && driver_matches(drv, d)) {
            d->driver = drv;
            if (drv->probe(d) != 0)
                d->driver = NULL;
        }
}

void pci_init(void)
{
    scan_bus(0);
    /* Ko'p host ko'priklari bo'lgan tizimlar: 0:0.x funksiyalari alohida shinalar. */
    uint8_t ht = (uint8_t)(raw_read32(0, 0, 0, 0x0C) >> 16);
    if (ht & 0x80)
        for (uint8_t f = 1; f < 8; f++)
            if ((raw_read32(0, 0, f, 0) & 0xFFFF) != 0xFFFF)
                scan_bus(f);

    kprintf("[pci]  %d ta qurilma:\n", device_count);
    struct pci_device *d;
    list_for_each_entry(d, &devices, node) {
        kprintf("         %02x:%02x.%u  %04x:%04x  [%02x%02x]  %s\n", d->bus, d->dev, d->func,
                d->vendor, d->device, d->class_code, d->subclass,
                pci_class_name(d->class_code, d->subclass));
        try_bind(d);
    }
}

struct pci_device *pci_get(int index)
{
    struct pci_device *d;
    int i = 0;
    list_for_each_entry(d, &devices, node)
        if (i++ == index)
            return d;
    return NULL;
}

int pci_count(void)
{
    return device_count;
}
