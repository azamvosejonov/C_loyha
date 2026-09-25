/* =============================================================================
 *  drivers/pit.c - 8253/8254 PIT taymeri
 * =============================================================================
 *
 *  PIT ichida 1 193 182 Hz chastotali kvarts generatori bor. Biz unga "bo'luvchi"
 *  beramiz: har N tebranishda IRQ0 yuboradi. N = 1193182 / 100 = 11931 -> 100 Hz.
 *
 *  NEGA TAYMER - OS'NING YURAGI:
 *    Taymer bo'lmasa, cheksiz tsiklga tushgan dastur CPU'ni abadiy egallaydi.
 *    Taymer har 10 ms da yadroga boshqaruvni QAYTARIB beradi va scheduler
 *    "keyingi jarayon navbati" deb qaror qiladi. Bu PREEMPTIVE (majburiy)
 *    multitasking deyiladi.
 * ============================================================================= */
#include "drivers/pit.h"

#include "arch/interrupts.h"
#include "arch/io.h"

#define PIT_CHANNEL0 0x40               /* 0-kanal ma'lumot porti (IRQ0 ga ulangan) */
#define PIT_COMMAND  0x43               /* buyruq porti */
#define PIT_BASE_HZ  1193182

/* volatile - uzilish handleri o'zgartiradi, oddiy kod o'qiydi. Kompilyator
 * qiymatni registrda keshlab qo'ymasligi kerak. */
static volatile uint64_t ticks;
static void (*tick_callback)(void);

static void timer_irq(struct interrupt_frame *frame)
{
    (void)frame;
    ticks++;
    if (tick_callback)
        tick_callback();                /* scheduler shu yerda jarayonni almashtirishi mumkin */
}

void pit_init(void)
{
    uint16_t divisor = PIT_BASE_HZ / TIMER_HZ;
    /* 0x36 = 00 11 011 0:
     *   00  - 0-kanal
     *   11  - avval past bayt, keyin yuqori bayt yuboriladi
     *   011 - 3-rejim (kvadrat to'lqin - davriy uzilish)
     *   0   - ikkilik sanoq (BCD emas) */
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, divisor >> 8);
    irq_register(IRQ_TIMER, timer_irq);
}

uint64_t timer_ticks(void)
{
    return ticks;
}

void timer_set_tick_callback(void (*callback)(void))
{
    tick_callback = callback;
}
