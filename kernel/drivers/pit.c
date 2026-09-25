/* =============================================================================
 *  drivers/pit.c - tizim taymeri: LAPIC taymeri (asosiy) yoki PIT (zaxira)
 * =============================================================================
 *
 *  NEGA TAYMER - OS'NING YURAGI:
 *    Taymer bo'lmasa, cheksiz tsiklga tushgan dastur CPU'ni abadiy egallaydi.
 *    Taymer har 10 ms da yadroga boshqaruvni QAYTARIB beradi va scheduler
 *    "keyingi jarayon navbati" deb qaror qiladi (PREEMPTIVE multitasking).
 *
 *  IKKI MANBA:
 *    * LAPIC taymeri - HAR BIR CPU'da o'zining taymeri bor. Ko'p yadroli
 *      tizimda har bir CPU mustaqil ravishda o'z jarayonlarini almashtiradi.
 *      Uning chastotasi CPU'ga bog'liq, shuning uchun avval PIT bilan
 *      kalibrlanadi (apic.c).
 *    * PIT (8253/8254) - 1 193 182 Hz, faqat bitta. ACPI/APIC bo'lmagan juda
 *      eski kompyuterlar uchun zaxira.
 *
 *  Global "tik" hisoblagichi (timer_ticks) faqat CPU0 da oshadi.
 * ============================================================================= */
#include "drivers/pit.h"

#include "arch/apic.h"
#include "arch/interrupts.h"
#include "arch/io.h"
#include "arch/percpu.h"
#include "proc/process.h"

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_BASE_HZ  1193182

static volatile uint64_t ticks;

static void timer_tick(struct interrupt_frame *frame)
{
    (void)frame;
    if (cpu_id() == 0)
        __atomic_add_fetch(&ticks, 1, __ATOMIC_RELAXED);
    sched_tick();                       /* shu CPU'da jarayonni almashtirish mumkin */
}

void timer_init(void)
{
    if (apic_active) {
        lapic_timer_calibrate();
        interrupt_register(VECTOR_LAPIC_TIMER, timer_tick);
        lapic_timer_start();            /* BSP; AP lar o'zlarinikini smp.c da */
        return;
    }
    /* Zaxira: PIT 0-kanali, 3-rejim (kvadrat to'lqin), ikkilik sanoq. */
    uint16_t divisor = PIT_BASE_HZ / TIMER_HZ;
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, divisor >> 8);
    irq_register(IRQ_TIMER, timer_tick);
}

uint64_t timer_ticks(void)
{
    return __atomic_load_n(&ticks, __ATOMIC_RELAXED);
}
