/* =============================================================================
 *  arch/gdt.h - Global Descriptor Table va Task State Segment
 * ============================================================================= */
#pragma once

#include <stdint.h>

/* Selektorlar = GDT ichidagi bayt siljishi (har bir yozuv 8 bayt).
 * Pastki 2 bit - RPL (so'ralgan imtiyoz darajasi): user selektorlarida 3. */
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_DATA   (0x18 | 3)
#define GDT_USER_CODE   (0x20 | 3)
#define GDT_TSS         0x28

void gdt_init(void);

/* User rejimidan uzilish kelganda CPU qaysi yadro stekiga o'tishini belgilash.
 * Har bir jarayonga o'tganda scheduler shu funksiyani chaqiradi. */
void tss_set_kernel_stack(uint64_t rsp0);
