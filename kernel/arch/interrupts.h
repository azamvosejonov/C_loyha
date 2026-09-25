/* =============================================================================
 *  arch/interrupts.h - uzilishlarni qayta ishlash interfeysi
 * ============================================================================= */
#pragma once

#include <stdint.h>

/* isr.asm stekka qo'ygan registrlar. TARTIB isr.asm bilan AYNAN mos! */
struct interrupt_frame {
    /* isr_common saqlagan umumiy registrlar (oxirgi push = eng past manzil) */
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    /* stub qo'ygan */
    uint64_t vector;
    uint64_t error_code;
    /* CPU avtomatik qo'ygan */
    uint64_t rip, cs, rflags, rsp, ss;
};

typedef void (*interrupt_handler_t)(struct interrupt_frame *frame);

#define IRQ_TIMER    0
#define IRQ_KEYBOARD 1
#define IRQ_COM1     4

#define VECTOR_SYSCALL 0x80

/* GDT, IDT va PIC ni sozlaydi. Uzilishlar hali O'CHIQ qoladi. */
void interrupts_init(void);

/* Istalgan vektor uchun handler o'rnatish. */
void interrupt_register(uint8_t vector, interrupt_handler_t handler);

/* Apparat IRQ uchun handler o'rnatish va uni PIC'da yoqish. */
void irq_register(uint8_t irq, interrupt_handler_t handler);

/* Uzilish user rejimida (ring 3) sodir bo'lganmi? CS ning pastki 2 biti = daraja. */
static inline int frame_from_user(const struct interrupt_frame *frame)
{
    return (frame->cs & 3) == 3;
}

/* Registrlarni chiroyli qilib chiqarish (debug uchun). */
void dump_frame(const struct interrupt_frame *frame);
