/* =============================================================================
 *  acpi/acpi.c - ACPI: firmware'ning "kompyuter tavsifi"ni o'qish
 * =============================================================================
 *
 *  ACPI (Advanced Configuration and Power Interface) - firmware (BIOS/UEFI)
 *  RAM'ga qo'yib ketadigan JADVALLAR to'plami. Ular kompyuter haqida OS bilishi
 *  kerak bo'lgan narsalarni aytadi:
 *
 *    RSDP  "Root System Description Pointer" - boshlang'ich nuqta. GRUB uni
 *          bizga beradi (Multiboot2 tegi), bo'lmasa BIOS xotirasida qidiramiz.
 *     └─► XSDT (64-bit) yoki RSDT (32-bit) - boshqa jadvallar manzillari ro'yxati
 *          ├─► MADT ("APIC")  - nechta CPU bor, ularning APIC ID lari, IOAPIC lar,
 *          │                    IRQ qayta yo'naltirishlar  -> SMP va uzilishlar uchun
 *          ├─► FADT ("FACP")  - quvvat boshqaruvi portlari  -> o'chirish/qayta yuklash
 *          │    └─► DSDT      - AML bayt-kodi (qurilmalar tavsifi). Undan faqat
 *          │                    \_S5 (o'chirish holati) qiymatini olamiz
 *          ├─► MCFG           - PCIe konfiguratsiya xotirasi
 *          └─► HPET, ...
 *
 *  Har bir jadval 36 baytlik umumiy sarlavha bilan boshlanadi va CHECKSUM ga
 *  ega: barcha baytlar yig'indisi (mod 256) = 0. Buzilgan jadvalga ishonmaymiz.
 * ============================================================================= */
#include "acpi/acpi.h"

#include "arch/io.h"
#include "boot/bootinfo.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/layout.h"
#include "mm/vmalloc.h"

struct acpi_info acpi;

struct rsdp {
    char signature[8];                  /* "RSD PTR " */
    uint8_t checksum;                   /* birinchi 20 bayt uchun */
    char oem[6];
    uint8_t revision;                   /* 0 = ACPI 1.0 (RSDT), 2+ = XSDT bor */
    uint32_t rsdt;
    uint32_t length;
    uint64_t xsdt;
    uint8_t ext_checksum;               /* butun struktura uchun */
    uint8_t reserved[3];
} __attribute__((packed));

struct sdt_header {
    char signature[4];
    uint32_t length;                    /* sarlavha bilan birga */
    uint8_t revision;
    uint8_t checksum;
    char oem[6];
    char oem_table[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct madt {
    struct sdt_header h;
    uint32_t lapic_addr;
    uint32_t flags;                     /* bit0: 8259 PIC ham bor */
    uint8_t entries[];
} __attribute__((packed));

struct gas {                            /* Generic Address Structure */
    uint8_t space;                      /* 0 = xotira, 1 = I/O port */
    uint8_t bit_width, bit_offset, access_size;
    uint64_t address;
} __attribute__((packed));

static bool checksum_ok(const void *p, size_t len)
{
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++)
        sum += ((const uint8_t *)p)[i];
    return sum == 0;
}

/* Jadvalni xotiraga xaritalash: avval sarlavhani (uzunlikni bilish uchun), keyin
 * to'liq. memremap: direct map'da bo'lsa - o'shani, aks holda yangi xaritalash. */
static const struct sdt_header *map_table(uint64_t phys)
{
    const struct sdt_header *h = memremap(phys, sizeof(*h));
    if (!h)
        return NULL;
    uint32_t len = h->length;
    if (len < sizeof(*h) || len > 16 * 1024 * 1024)
        return NULL;
    const struct sdt_header *full = memremap(phys, len);
    if (!full || !checksum_ok(full, len)) {
        kprintf("[acpi] %.4s jadvali checksum xato - e'tiborsiz qoldirildi\n", h->signature);
        return NULL;
    }
    return full;
}

/* BIOS xotirasida RSDP ni qidirish (GRUB bermagan bo'lsa): EBDA ning birinchi
 * 1 KB i va 0xE0000..0xFFFFF, 16 bayt qadam bilan. */
static const struct rsdp *find_rsdp_bios(void)
{
    uint16_t ebda_seg = *(const uint16_t *)phys_to_virt(0x40E);
    uint64_t ranges[2][2] = { { (uint64_t)ebda_seg << 4, ((uint64_t)ebda_seg << 4) + 1024 },
                              { 0xE0000, 0x100000 } };
    for (int r = 0; r < 2; r++) {
        for (uint64_t a = ranges[r][0]; a + 20 <= ranges[r][1]; a += 16) {
            const struct rsdp *p = phys_to_virt(a);
            if (memcmp(p->signature, "RSD PTR ", 8) == 0 && checksum_ok(p, 20))
                return p;
        }
    }
    return NULL;
}

static void parse_madt(const struct madt *m)
{
    acpi.lapic_phys = m->lapic_addr;
    acpi.legacy_pic = m->flags & 1;
    for (int i = 0; i < 16; i++)
        acpi.isa[i] = (struct acpi_irq_override){ .gsi = (uint32_t)i };

    const uint8_t *p = m->entries;
    const uint8_t *end = (const uint8_t *)m + m->h.length;
    while (p + 2 <= end && p[1] >= 2 && p + p[1] <= end) {
        uint8_t type = p[0];
        switch (type) {
        case 0: {                       /* Processor Local APIC */
            uint32_t flags = *(const uint32_t *)(p + 4);
            /* bit0: yoqilgan, bit1: "online capable" (keyin yoqilishi mumkin) */
            if ((flags & 3) && acpi.cpu_count < ACPI_MAX_CPUS)
                acpi.cpus[acpi.cpu_count++] = (struct acpi_cpu){ .apic_id = p[3], .acpi_id = p[2] };
            break;
        }
        case 1:                         /* I/O APIC */
            if (acpi.ioapic_count < ACPI_MAX_IOAPICS)
                acpi.ioapics[acpi.ioapic_count++] = (struct acpi_ioapic){
                    .id = p[2],
                    .phys = *(const uint32_t *)(p + 4),
                    .gsi_base = *(const uint32_t *)(p + 8),
                };
            break;
        case 2: {                       /* Interrupt Source Override */
            uint8_t irq = p[3];
            uint32_t gsi = *(const uint32_t *)(p + 4);
            uint16_t flags = *(const uint16_t *)(p + 8);
            if (irq < 16) {
                acpi.isa[irq].gsi = gsi;
                acpi.isa[irq].active_low = (flags & 3) == 3;        /* polarity: 11 = past */
                acpi.isa[irq].level_triggered = ((flags >> 2) & 3) == 3;  /* trigger: 11 = daraja */
            }
            break;
        }
        case 5:                         /* 64-bitli Local APIC manzili */
            acpi.lapic_phys = *(const uint64_t *)(p + 4);
            break;
        case 9: {                       /* x2APIC (ID > 255 bo'lgan CPU'lar) */
            uint32_t flags = *(const uint32_t *)(p + 8);
            if ((flags & 3) && acpi.cpu_count < ACPI_MAX_CPUS)
                acpi.cpus[acpi.cpu_count++] = (struct acpi_cpu){
                    .apic_id = *(const uint32_t *)(p + 4), .acpi_id = *(const uint32_t *)(p + 12) };
            break;
        }
        default:
            break;
        }
        p += p[1];
    }
}

/* DSDT AML kodida \_S5 paketini topib, SLP_TYP qiymatlarini olish.
 * To'liq AML interpretatori (ACPICA ~100 000 qator!) o'rniga keng tarqalgan
 * "qidiruv" usuli: `_S5_` nomini topamiz, keyin Package tuzilishini o'qiymiz:
 *   08 5F 53 35 5F 12 <PkgLength> <NumElements> 0A <SLP_TYPa> 0A <SLP_TYPb> ... */
static void parse_s5(const struct sdt_header *dsdt)
{
    const uint8_t *p = (const uint8_t *)dsdt + sizeof(*dsdt);
    const uint8_t *end = (const uint8_t *)dsdt + dsdt->length;
    for (; p + 8 < end; p++) {
        if (memcmp(p, "_S5_", 4) != 0)
            continue;
        /* Oldida NameOp (0x08) yoki "\" (0x5C 0x08) bo'lishi kerak. */
        if (!(p[-1] == 0x08 || (p[-2] == 0x08 && p[-1] == '\\')))
            continue;
        p += 4;
        if (*p++ != 0x12)               /* PackageOp */
            continue;
        p += ((*p & 0xC0) >> 6) + 1;    /* PkgLength (1..4 bayt) */
        p++;                            /* NumElements */
        uint16_t vals[2];
        for (int i = 0; i < 2; i++) {
            if (*p == 0x0A)             /* BytePrefix */
                p++;
            vals[i] = *p++;
        }
        acpi.slp_typa = vals[0];
        acpi.slp_typb = vals[1];
        acpi.s5_found = true;
        return;
    }
}

static void parse_fadt(const struct sdt_header *f)
{
    const uint8_t *b = (const uint8_t *)f;
    uint64_t dsdt = *(const uint32_t *)(b + 40);
    acpi.smi_cmd = *(const uint32_t *)(b + 48);
    acpi.acpi_enable = b[52];
    acpi.pm1a_cnt = (uint16_t)*(const uint32_t *)(b + 64);
    acpi.pm1b_cnt = (uint16_t)*(const uint32_t *)(b + 68);
    if (f->length >= 129) {
        uint32_t flags = *(const uint32_t *)(b + 112);
        const struct gas *reset = (const void *)(b + 116);
        acpi.reset_supported = flags & (1u << 10);
        acpi.reset_addr = reset->address;
        acpi.reset_space = reset->space;
        acpi.reset_value = b[128];
    }
    if (f->length >= 148 && *(const uint64_t *)(b + 140))
        dsdt = *(const uint64_t *)(b + 140);        /* X_DSDT (64-bit) */
    if (dsdt) {
        const struct sdt_header *d = map_table(dsdt);
        if (d && memcmp(d->signature, "DSDT", 4) == 0)
            parse_s5(d);
    }
}

void acpi_init(void)
{
    const struct rsdp *rsdp = NULL;
    if (boot_info.rsdp_present)
        rsdp = (const struct rsdp *)boot_info.rsdp;
    else
        rsdp = find_rsdp_bios();
    if (!rsdp || memcmp(rsdp->signature, "RSD PTR ", 8) != 0) {
        kprintf("[acpi] RSDP topilmadi - ACPI yo'q (bitta CPU, eski PIC rejimi)\n");
        return;
    }
    acpi.revision = rsdp->revision;
    memcpy(acpi.oem, rsdp->oem, 6);

    bool use_xsdt = rsdp->revision >= 2 && rsdp->xsdt;
    const struct sdt_header *root = map_table(use_xsdt ? rsdp->xsdt : rsdp->rsdt);
    if (!root) {
        kprintf("[acpi] RSDT/XSDT o'qib bo'lmadi\n");
        return;
    }
    acpi.present = true;
    size_t entry_size = use_xsdt ? 8 : 4;
    size_t count = (root->length - sizeof(*root)) / entry_size;
    const uint8_t *entries = (const uint8_t *)root + sizeof(*root);

    kprintf("[acpi] ACPI %s, OEM \"%s\", %s: %zu ta jadval:", rsdp->revision >= 2 ? "2.0+" : "1.0",
            acpi.oem, use_xsdt ? "XSDT" : "RSDT", count);
    for (size_t i = 0; i < count; i++) {
        uint64_t phys = use_xsdt ? *(const uint64_t *)(entries + i * 8)
                                 : *(const uint32_t *)(entries + i * 4);
        const struct sdt_header *t = map_table(phys);
        if (!t)
            continue;
        kprintf(" %.4s", t->signature);
        if (memcmp(t->signature, "APIC", 4) == 0)
            parse_madt((const struct madt *)t);
        else if (memcmp(t->signature, "FACP", 4) == 0)
            parse_fadt(t);
        else if (memcmp(t->signature, "MCFG", 4) == 0 && t->length >= 44 + 16)
            acpi.mcfg_base = *(const uint64_t *)((const uint8_t *)t + 44);
    }
    kprintf("\n");
    kprintf("[acpi] CPU'lar: %d, IOAPIC: %d, LAPIC @ %p, S5: %s\n", acpi.cpu_count,
            acpi.ioapic_count, (void *)acpi.lapic_phys, acpi.s5_found ? "topildi" : "yo'q");
}

void acpi_poweroff(void)
{
    if (!acpi.present || !acpi.s5_found || !acpi.pm1a_cnt)
        return;
    /* Ba'zi kompyuterlarda avval "ACPI rejimi"ni yoqish kerak (SCI_EN = 1). */
    if (acpi.smi_cmd && acpi.acpi_enable && !(inw(acpi.pm1a_cnt) & 1)) {
        outb((uint16_t)acpi.smi_cmd, acpi.acpi_enable);
        for (int i = 0; i < 1000000 && !(inw(acpi.pm1a_cnt) & 1); i++)
            io_wait();
    }
    /* SLP_TYP (10..12 bitlar) + SLP_EN (13-bit) = "shu holatga o't" */
    outw(acpi.pm1a_cnt, (uint16_t)((acpi.slp_typa << 10) | (1 << 13)));
    if (acpi.pm1b_cnt)
        outw(acpi.pm1b_cnt, (uint16_t)((acpi.slp_typb << 10) | (1 << 13)));
}

void acpi_reboot(void)
{
    if (acpi.reset_supported && acpi.reset_space == 1)          /* I/O port */
        outb((uint16_t)acpi.reset_addr, acpi.reset_value);
    /* Zaxira: klaviatura kontrolleri orqali CPU'ni qayta yuklash (1980-yillardan beri). */
    for (int i = 0; i < 100000 && (inb(0x64) & 2); i++)
        ;
    outb(0x64, 0xFE);
}
