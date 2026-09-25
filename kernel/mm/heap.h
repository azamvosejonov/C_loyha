/* =============================================================================
 *  mm/heap.h - yadro heap'i: kmalloc / kfree
 * ============================================================================= */
#pragma once

#include <stddef.h>
#include <stdint.h>

/* size baytli xotira ajratish. 16 baytga tekislangan. Xotira yetmasa NULL. */
void *kmalloc(size_t size);
/* kmalloc + nollash. */
void *kzalloc(size_t size);
/* kmalloc() qaytargan ko'rsatkichni qaytarish. NULL ham mumkin (hech narsa qilmaydi). */
void kfree(void *ptr);

struct heap_stats {
    uint64_t alloc_count;               /* jami kmalloc chaqiruvlari */
    uint64_t free_count;                /* jami kfree chaqiruvlari */
    uint64_t bytes_in_use;              /* hozir band (obyekt hajmlari bo'yicha) */
    uint64_t slab_pages;                /* slab'lar uchun olingan sahifalar */
    uint64_t large_pages;               /* katta ajratmalar uchun sahifalar */
};

void heap_get_stats(struct heap_stats *out);
void heap_dump(void);                   /* har bir o'lcham sinfi bo'yicha holat */
