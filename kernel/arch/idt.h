/* =============================================================================
 *  arch/idt.h - Interrupt Descriptor Table
 * ============================================================================= */
#pragma once

#include <stdint.h>

void idt_init(void);
/* Tayyor IDT ni joriy CPU'ga yuklash (AP lar uchun). */
void idt_load(void);
