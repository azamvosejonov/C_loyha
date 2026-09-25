/* =============================================================================
 *  mm/pmm.h - fizik xotira menejeri: BUDDY allocator
 * ============================================================================= */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "mm/page.h"

/* gfp ("get free pages") bayroqlari */
#define GFP_ZERO   (1u << 0)            /* nollangan sahifa */
#define GFP_DMA32  (1u << 1)            /* 4 GB dan past (32-bitli DMA qurilmalar uchun) */

#define ZONE_DMA32  0                   /* [1 MB, 4 GB) */
#define ZONE_NORMAL 1                   /* [4 GB, ...) */
#define NR_ZONES    2

void pmm_init(void);                    /* memblock'dan keyin chaqiriladi */

/* 2^order ta ketma-ket sahifa. Xotira yo'q bo'lsa NULL. refcount = 1. */
struct page *alloc_pages(unsigned order, unsigned gfp);
void free_pages(struct page *p, unsigned order);

/* refcount bilan ishlash (bir sahifani bir necha joy ishlatganda - COW). */
static inline void get_page(struct page *p)
{
    __atomic_add_fetch(&p->refcount, 1, __ATOMIC_RELAXED);
}
/* refcount 0 ga tushsa, sahifa qaytariladi. */
void put_page(struct page *p);

/* ---- Qulay qisqartmalar (fizik manzil bilan) ---- */
uint64_t pmm_alloc_page(unsigned gfp);          /* 0 = xotira yo'q */
void pmm_free_page(uint64_t phys);
/* n ta ketma-ket sahifa (n 2 ning darajasigacha yaxlitlanadi). */
uint64_t pmm_alloc_pages(size_t n, unsigned gfp);
void pmm_free_pages(uint64_t phys, size_t n);

/* n sahifa uchun kerakli tartib: 2^order >= n */
unsigned pages_to_order(size_t n);

size_t pmm_total_pages(void);
size_t pmm_free_pages_count(void);
void pmm_dump(void);                    /* har bir tartib bo'yicha bo'sh bloklar */
