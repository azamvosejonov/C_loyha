/* =============================================================================
 *  arch/smp.h - ko'p yadroli (SMP) qo'llab-quvvatlash
 * ============================================================================= */
#pragma once

#include <stdint.h>

/* AP larni uyg'otish (ACPI MADT dagi barcha CPU'lar). */
void smp_init(void);
/* Yadro sahifalarini BARCHA CPU'larda TLB dan o'chirish. */
void tlb_shootdown(uint64_t virt, uint64_t pages);
/* Panic: boshqa CPU'larni to'xtatish. */
void smp_stop_others(void);
