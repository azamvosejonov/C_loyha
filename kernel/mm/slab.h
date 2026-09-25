/* =============================================================================
 *  mm/slab.h - obyekt keshlari (kmem_cache) va kmalloc
 * ============================================================================= */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "lib/list.h"
#include "lib/spinlock.h"

struct kmem_cache {
    const char *name;
    size_t size;                        /* foydalanuvchi so'ragan hajm */
    size_t stride;                      /* obyekt + redzone, tekislangan: slab ichidagi qadam */
    unsigned order;                     /* har bir slab 2^order sahifa */
    unsigned objs_per_slab;
    void (*ctor)(void *obj);            /* ixtiyoriy konstruktor */
    struct list_head partial;           /* qisman bo'sh slab'lar */
    struct list_head full;              /* to'la slab'lar (debug va statistika uchun) */
    struct list_head empty;             /* bo'sh slab'lar (1 tagacha zaxira) */
    size_t nr_slabs;
    size_t active_objs;
    uint64_t allocs, frees;
    spinlock_t lock;
    struct list_head cache_list;        /* barcha keshlar ro'yxati (slabinfo) */
};

void slab_init(void);

struct kmem_cache *kmem_cache_create(const char *name, size_t size, size_t align,
                                     void (*ctor)(void *));
/* Keshni yo'q qilish. Unda faol obyekt qolmagan bo'lishi SHART. */
void kmem_cache_destroy(struct kmem_cache *c);
void *kmem_cache_alloc(struct kmem_cache *c);
void kmem_cache_free(struct kmem_cache *c, void *obj);

void *kmalloc(size_t size);
void *kzalloc(size_t size);
void kfree(void *ptr);
/* ptr uchun haqiqatda ajratilgan hajm. */
size_t ksize(const void *ptr);

struct heap_stats {
    uint64_t alloc_count, free_count;
    uint64_t bytes_in_use;
    uint64_t slab_pages, large_pages;
};
void heap_get_stats(struct heap_stats *out);
void slab_dump(void);                   /* "slabinfo" */
