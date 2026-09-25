/* =============================================================================
 *  drivers/pic.h - 8259A Programmable Interrupt Controller
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stdint.h>

/* IRQ 0..15 IDT dagi 32..47 vektorlariga ko'chiriladi. */
#define PIC_IRQ_BASE 32

void pic_init(void);
void pic_send_eoi(uint8_t irq);
void pic_unmask(uint8_t irq);
void pic_mask(uint8_t irq);
/* IRQ 7 yoki 15 "soxta" (spurious) uzilishmi? */
bool pic_is_spurious(uint8_t irq);
