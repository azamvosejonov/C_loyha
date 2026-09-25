/* =============================================================================
 *  mm/page.h - "struct page": har bir fizik sahifa haqida ma'lumot
 * =============================================================================
 *
 *  Linux'dagi eng muhim strukturalardan biri. RAM'dagi HAR BIR 4 KB sahifa
 *  uchun bitta struct page bor, ular bitta massivda (mem_map) turadi:
 *
 *      mem_map[pfn]  <->  fizik manzil pfn * 4096
 *
 *  (pfn = page frame number = fizik manzil / 4096). Shu tufayli fizik manzildan
 *  sahifa ma'lumotiga va aksincha O(1) da o'tamiz.
 *
 *  Sahifa ma'lumoti nima uchun kerak:
 *    * buddy allocator: sahifa bo'shmi, qaysi tartib (order) blokining boshi
 *    * slab: sahifa qaysi keshga tegishli (kfree hajmni so'ramasligi uchun)
 *    * refcount: nechta joy bu sahifadan foydalanyapti (fork + copy-on-write!)
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lib/list.h"
#include "mm/layout.h"

#define MAX_ORDER 11                    /* 0..10 tartib: 4 KB .. 4 MB bloklar */

/* flags */
#define PG_RESERVED (1u << 0)           /* umuman boshqarilmaydi (BIOS, yadro, teshik) */
#define PG_BUDDY    (1u << 1)           /* buddy'ning bo'sh ro'yxatida turibdi */
#define PG_SLAB     (1u << 2)           /* slab allocator ishlatyapti */
#define PG_HEAD     (1u << 3)           /* ajratilgan blokning birinchi sahifasi */

struct kmem_cache;

struct page {
    uint32_t flags;
    int32_t refcount;
    uint8_t order;                      /* blok tartibi (bo'sh yoki ajratilgan blok boshida) */
    uint8_t zone;
    uint16_t _pad;
    uint32_t mapcount;                  /* nechta sahifa jadvalida xaritalangan (COW uchun) */
    struct list_head list;              /* buddy bo'sh ro'yxati yoki slab ro'yxati */
    /* Slab uchun maydonlar */
    struct kmem_cache *cache;
    void *freelist;
    uint32_t inuse;
    uint32_t _pad2;
    struct page *head;                  /* ko'p sahifali slab: birinchi sahifaga ko'rsatkich */
};

/* Struktura aniq 64 bayt: bitta kesh qatori. 128 MB RAM uchun ~2 MB. */
_Static_assert(sizeof(struct page) == 64, "struct page 64 bayt bo'lishi kerak");

extern struct page *mem_map;
extern uint64_t max_pfn;

static inline uint64_t page_to_pfn(const struct page *p)
{
    return (uint64_t)(p - mem_map);
}

static inline struct page *pfn_to_page(uint64_t pfn)
{
    return &mem_map[pfn];
}

static inline uint64_t page_to_phys(const struct page *p)
{
    return page_to_pfn(p) << PAGE_SHIFT;
}

static inline struct page *phys_to_page(uint64_t phys)
{
    return pfn_to_page(phys >> PAGE_SHIFT);
}

static inline void *page_to_virt(const struct page *p)
{
    return phys_to_virt(page_to_phys(p));
}

static inline struct page *virt_to_page(const void *v)
{
    return phys_to_page(virt_to_phys(v));
}
