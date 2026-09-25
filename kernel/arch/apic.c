/* =============================================================================
 *  arch/apic.c - LOCAL APIC va IO APIC: zamonaviy uzilishlar tizimi
 * =============================================================================
 *
 *  8259 PIC (1976!) faqat bitta CPU'ga uzilish yubora oladi. Ko'p yadroli
 *  tizimda har bir CPU'ning o'z "LOCAL APIC"i bor:
 *    * boshqa manbalardan kelgan uzilishlarni qabul qiladi
 *    * o'z TAYMERi bor (har bir CPU mustaqil scheduler tikiga ega)
 *    * boshqa CPU'larga IPI (Inter-Processor Interrupt) yubora oladi:
 *        - AP larni uyg'otish (INIT, SIPI)
 *        - TLB tozalash so'rovi, "to'xta" buyrug'i (panic)
 *
 *  IO APIC - qurilma uzilishlarini (klaviatura, disk...) istalgan CPU'ga
 *  yo'naltiruvchi "marshrutizator". Har bir kirish (GSI) uchun jadval yozuvi:
 *  qaysi vektor, qaysi CPU, qanday signal (front/daraja, yuqori/past).
 *
 *  xAPIC vs x2APIC:
 *    xAPIC  - registrlar XOTIRA orqali (MMIO, 0xFEE00000), 8 bitli ID (256 CPU)
 *    x2APIC - registrlar MSR orqali (0x800 + reg/16), 32 bitli ID, tezroq.
 *    Zamonaviy UEFI ba'zan x2APIC ni YOQIB qoldiradi - shunda MMIO ishlamaydi!
 *    Shuning uchun ikkalasini ham qo'llab-quvvatlaymiz.
 * ============================================================================= */
#include "arch/apic.h"

#include "acpi/acpi.h"
#include "arch/cpu.h"
#include "arch/io.h"
#include "arch/percpu.h"
#include "arch/tsc.h"
#include "drivers/pic.h"
#include "drivers/pit.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "mm/vmalloc.h"

#define LAPIC_ID        0x020
#define LAPIC_VERSION   0x030
#define LAPIC_TPR       0x080           /* Task Priority: 0 = hamma uzilishlarni qabul qil */
#define LAPIC_EOI       0x0B0
#define LAPIC_SVR       0x0F0           /* Spurious Vector + yoqish biti (8) */
#define LAPIC_ESR       0x280           /* xato holati */
#define LAPIC_ICR_LOW   0x300           /* IPI buyrug'i */
#define LAPIC_ICR_HIGH  0x310           /* IPI manzili (xAPIC) */
#define LAPIC_LVT_TIMER 0x320
#define LAPIC_LVT_LINT0 0x350
#define LAPIC_LVT_LINT1 0x360
#define LAPIC_LVT_ERROR 0x370
#define LAPIC_TIMER_INIT 0x380
#define LAPIC_TIMER_CUR  0x390
#define LAPIC_TIMER_DIV  0x3E0

#define LVT_MASKED      (1u << 16)
#define LVT_PERIODIC    (1u << 17)
#define ICR_PENDING     (1u << 12)

bool apic_active;
static bool x2apic;
static volatile uint32_t *lapic_mmio;
static uint32_t lapic_ticks_per_ms;

static uint32_t lapic_read(uint32_t reg)
{
    if (x2apic)
        return (uint32_t)rdmsr(0x800 + (reg >> 4));
    return lapic_mmio[reg / 4];
}

static void lapic_write(uint32_t reg, uint32_t val)
{
    if (x2apic)
        wrmsr(0x800 + (reg >> 4), val);
    else
        lapic_mmio[reg / 4] = val;
}

uint32_t lapic_id(void)
{
    uint32_t id = lapic_read(LAPIC_ID);
    return x2apic ? id : id >> 24;
}

void lapic_eoi(void)
{
    lapic_write(LAPIC_EOI, 0);
}

static void icr_send(uint32_t dest, uint32_t low)
{
    if (x2apic) {
        wrmsr(0x830, ((uint64_t)dest << 32) | low);     /* x2APIC: bitta 64-bitli MSR */
        return;
    }
    lapic_write(LAPIC_ICR_HIGH, dest << 24);
    lapic_write(LAPIC_ICR_LOW, low);
    while (lapic_read(LAPIC_ICR_LOW) & ICR_PENDING)     /* yetkazilishini kutamiz */
        cpu_pause();
}

void lapic_send_ipi(uint32_t dest, uint8_t vector)
{
    icr_send(dest, vector);             /* fixed delivery, physical destination */
}

void lapic_send_ipi_all_but_self(uint8_t vector)
{
    icr_send(0, vector | (3u << 18));   /* shorthand 11 = "o'zimdan boshqa hammaga" */
}

void lapic_send_init(uint32_t dest)
{
    icr_send(dest, 0x4500);             /* INIT, level assert */
}

void lapic_send_sipi(uint32_t dest, uint8_t page)
{
    icr_send(dest, 0x4600 | page);      /* Startup IPI: AP 0xPP000 manzilidan real mode'da boshlaydi */
}

/* Har bir CPU'da bir xil: APIC ni yoqish va LVT larni sozlash. */
static void lapic_setup_common(void)
{
    uint64_t base = rdmsr(MSR_APIC_BASE);
    base |= 1u << 11;                   /* global yoqish */
    if (x2apic)
        base |= 1u << 10;               /* x2APIC rejimi */
    wrmsr(MSR_APIC_BASE, base);

    lapic_write(LAPIC_TPR, 0);
    lapic_write(LAPIC_LVT_ERROR, VECTOR_LAPIC_ERROR);
    lapic_write(LAPIC_LVT_LINT0, LVT_MASKED);           /* eski PIC chizig'i - kerak emas */
    lapic_write(LAPIC_LVT_LINT1, 0x400);                /* NMI */
    lapic_write(LAPIC_ESR, 0);
    lapic_write(LAPIC_SVR, 0x100 | VECTOR_SPURIOUS);    /* yoqish + soxta uzilish vektori */
    lapic_eoi();
}

void lapic_init_bsp(void)
{
    if (!cpu_features.apic || !acpi.present || acpi.ioapic_count == 0) {
        kprintf("[apic] APIC/ACPI yo'q - eski 8259 PIC + PIT rejimi\n");
        return;
    }
    /* Firmware x2APIC ni yoqib qo'ygan bo'lsa - majburan x2APIC. Aks holda CPU
     * qo'llasa ham x2APIC ni tanlaymiz (tezroq, MMIO keshlash muammolari yo'q). */
    x2apic = cpu_features.x2apic;
    if (!x2apic) {
        lapic_mmio = ioremap(acpi.lapic_phys ? acpi.lapic_phys
                                             : (rdmsr(MSR_APIC_BASE) & ~0xFFFUL), 4096);
        if (!lapic_mmio)
            panic("LAPIC ni xaritalab bo'lmadi");
    }
    lapic_setup_common();
    apic_active = true;
    this_cpu()->apic_id = lapic_id();
    kprintf("[apic] Local APIC: %s rejimi, BSP APIC ID %u, versiya %x\n",
            x2apic ? "x2APIC (MSR)" : "xAPIC (MMIO)", this_cpu()->apic_id,
            lapic_read(LAPIC_VERSION) & 0xFF);
}

void lapic_init_ap(void)
{
    lapic_setup_common();
}

void lapic_timer_calibrate(void)
{
    if (!apic_active)
        return;
    lapic_write(LAPIC_TIMER_DIV, 0x3);                  /* 16 ga bo'lish */
    lapic_write(LAPIC_LVT_TIMER, LVT_MASKED);
    uint32_t best = UINT32_MAX;
    for (int i = 0; i < 3; i++) {
        lapic_write(LAPIC_TIMER_INIT, 0xFFFFFFFF);
        pit_wait_ms(10);
        uint32_t elapsed = 0xFFFFFFFF - lapic_read(LAPIC_TIMER_CUR);
        if (elapsed < best)
            best = elapsed;
    }
    lapic_write(LAPIC_TIMER_INIT, 0);
    lapic_ticks_per_ms = best / 10;
    kprintf("[apic] LAPIC taymeri: %u tik/ms (bo'luvchi 16)\n", lapic_ticks_per_ms);
}

void lapic_timer_start(void)
{
    if (!apic_active)
        return;
    lapic_write(LAPIC_TIMER_DIV, 0x3);
    lapic_write(LAPIC_LVT_TIMER, VECTOR_LAPIC_TIMER | LVT_PERIODIC);
    lapic_write(LAPIC_TIMER_INIT, lapic_ticks_per_ms * (1000 / TIMER_HZ));
}

/* ---- IO APIC ---- */

#define IOAPIC_REGSEL 0x00
#define IOAPIC_WIN    0x10
#define IOAPIC_VER    0x01
#define IOAPIC_REDTBL 0x10

struct ioapic_rt {
    volatile uint32_t *mmio;
    uint32_t gsi_base;
    uint32_t count;                     /* kirishlar soni */
};

static struct ioapic_rt ioapics[ACPI_MAX_IOAPICS];
static int ioapic_count;

static uint32_t ioapic_read(struct ioapic_rt *io, uint32_t reg)
{
    io->mmio[IOAPIC_REGSEL / 4] = reg;  /* avval registr raqami, keyin qiymat oynasi */
    return io->mmio[IOAPIC_WIN / 4];
}

static void ioapic_write(struct ioapic_rt *io, uint32_t reg, uint32_t val)
{
    io->mmio[IOAPIC_REGSEL / 4] = reg;
    io->mmio[IOAPIC_WIN / 4] = val;
}

void ioapic_init(void)
{
    if (!apic_active)
        return;
    for (int i = 0; i < acpi.ioapic_count; i++) {
        struct ioapic_rt *io = &ioapics[ioapic_count];
        io->mmio = ioremap(acpi.ioapics[i].phys, 4096);
        if (!io->mmio)
            continue;
        io->gsi_base = acpi.ioapics[i].gsi_base;
        io->count = ((ioapic_read(io, IOAPIC_VER) >> 16) & 0xFF) + 1;
        /* Hamma kirishlarni niqoblaymiz - drayverlar keraklisini yoqadi. */
        for (uint32_t n = 0; n < io->count; n++) {
            ioapic_write(io, IOAPIC_REDTBL + 2 * n, 1u << 16);
            ioapic_write(io, IOAPIC_REDTBL + 2 * n + 1, 0);
        }
        kprintf("[apic] IOAPIC %u: GSI %u..%u\n", acpi.ioapics[i].id, io->gsi_base,
                io->gsi_base + io->count - 1);
        ioapic_count++;
    }
    /* Eski PIC ni butunlay o'chiramiz (hamma IRQ niqoblangan) - aks holda u ham
     * uzilish yuborib, "soxta" uzilishlar paydo bo'ladi. */
    pic_disable();
}

void ioapic_route_isa(uint8_t irq, uint8_t vector, uint32_t dest)
{
    const struct acpi_irq_override *ov = &acpi.isa[irq];
    for (int i = 0; i < ioapic_count; i++) {
        struct ioapic_rt *io = &ioapics[i];
        if (ov->gsi < io->gsi_base || ov->gsi >= io->gsi_base + io->count)
            continue;
        uint32_t pin = ov->gsi - io->gsi_base;
        uint32_t low = vector;                          /* fixed delivery, physical */
        if (ov->active_low)
            low |= 1u << 13;
        if (ov->level_triggered)
            low |= 1u << 15;
        ioapic_write(io, IOAPIC_REDTBL + 2 * pin + 1, dest << 24);
        ioapic_write(io, IOAPIC_REDTBL + 2 * pin, low); /* niqob biti 0 - yoqilgan */
        return;
    }
    kprintf("[apic] ogohlantirish: IRQ %u (GSI %u) uchun IOAPIC topilmadi\n", irq, ov->gsi);
}
