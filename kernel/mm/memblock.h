/* =============================================================================
 *  mm/memblock.h - ilk (boot vaqtidagi) xotira allocatori
 * ============================================================================= */
#pragma once

#include <stdint.h>

#include "boot/bootinfo.h"

#define MEMBLOCK_MAX 64

struct memblock_region {
    uint64_t base;
    uint64_t end;                       /* [base, end) */
};

void memblock_init(const struct boot_info *bi);
/* size baytni align ga tekislab ajratadi (fizik manzil, NOLLANGAN). limit dan
 * past manzildan. Xotira yo'q bo'lsa panic (boot vaqtida davom etib bo'lmaydi). */
uint64_t memblock_alloc(uint64_t size, uint64_t align, uint64_t limit);
/* Hozirgacha bo'sh qolgan hududlar (buddy allocator ularni oladi). */
const struct memblock_region *memblock_free_regions(int *count);
/* Eng yuqori RAM manzili (struct page massivi hajmi uchun). */
uint64_t memblock_max_ram(void);
/* memblock ishini tugatdi: bundan keyin memblock_alloc panic qiladi. */
void memblock_retire(void);
