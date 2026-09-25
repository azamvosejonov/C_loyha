/* =============================================================================
 *  arch/gdt.h - Global Descriptor Table va Task State Segment (har bir CPU uchun)
 * ============================================================================= */
#pragma once

#include <stdint.h>

/* Selektorlar = GDT ichidagi bayt siljishi (har bir yozuv 8 bayt).
 * Pastki 2 bit - RPL (so'ralgan imtiyoz darajasi): user selektorlarida 3.
 * TARTIB syscall/sysret talabiga mos (arch/syscall.c ga qarang). */
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_DATA   (0x18 | 3)
#define GDT_USER_CODE   (0x20 | 3)
#define GDT_TSS         0x28
#define GDT_ENTRIES     7               /* null, kcode, kdata, udata, ucode, TSS (2 ta) */

/* 64-bitli TSS (Intel SDM 3A, 8.7). */
struct tss {
    uint32_t reserved0;
    uint64_t rsp0;                      /* ring 3 -> ring 0 o'tishda ishlatiladigan stek */
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];                    /* maxsus uzilish steklari */
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} __attribute__((packed));

struct cpu;

/* Joriy CPU uchun GDT va TSS ni o'rnatish (har bir CPU o'zinikini oladi). */
void gdt_init_cpu(struct cpu *c);

/* User rejimidan uzilish kelganda JORIY CPU qaysi yadro stekiga o'tishi. */
void tss_set_kernel_stack(uint64_t rsp0);
