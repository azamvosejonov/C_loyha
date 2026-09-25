/* =============================================================================
 *  mm/slab.c - SLAB ALLOCATOR (Linux'dagi SLUB g'oyasi asosida)
 * =============================================================================
 *
 *  OBYEKT KESHI (kmem_cache): bir xil o'lchamdagi obyektlar ombori. Masalan,
 *  "process" keshi faqat struct process larni saqlaydi. Har bir kesh buddy'dan
 *  sahifa(lar) oladi ("slab") va ularni teng katakchalarga bo'ladi.
 *
 *     slab (1 sahifa), kesh "kmalloc-256":
 *     ┌──────────┬──────────┬──────────┬─────┬──────────┐
 *     │ obj + RZ │ obj + RZ │ obj + RZ │ ... │ obj + RZ │    RZ = redzone (0xBB)
 *     └──────────┴──────────┴──────────┴─────┴──────────┘
 *     Slab haqidagi ma'lumot sahifaning ICHIDA emas, uning struct page'ida
 *     (cache, freelist, inuse). Shuning uchun butun sahifa obyektlar uchun.
 *
 *  kfree(p) HAJMNI QAYERDAN BILADI? virt_to_page(p)->cache - O(1).
 *
 *  kmalloc - umumiy maqsadli keshlar: 16, 32, 64, 96, 128, 192, 256, 512,
 *  1024, 2048, 4096, 8192 bayt. Kattaroq so'rovlar - to'g'ridan-to'g'ri buddy.
 *
 *  DEBUG (har doim yoqilgan - o'rganish loyihasi uchun xavfsizlik tezlikdan muhim):
 *    * REDZONE: har bir obyekt oxiridan 8 bayt 0xBB. kfree va kmalloc da
 *      tekshiriladi -> BUFFER OVERFLOW (chegaradan chiqib yozish) ushlanadi.
 *    * ZAHAR: bo'sh obyekt 0x6B bilan to'ladi, qayta berishda tekshiriladi ->
 *      USE-AFTER-FREE ushlanadi.
 *    * FREE_MAGIC: bo'sh obyektning 8..15 baytlari -> DOUBLE FREE ushlanadi.
 *    * cache/sahifa tekshiruvi -> noto'g'ri kfree ushlanadi.
 * ============================================================================= */
#include "mm/slab.h"

#include <stdbool.h>

#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/string.h"
#include "mm/page.h"
#include "mm/pmm.h"

#define REDZONE       8
#define RZ_BYTE       0xBB
#define POISON_FREE   0x6B
#define FREE_MAGIC    0xF4EEF4EEF4EEF4EEULL
#define MIN_OBJ       16                /* next + FREE_MAGIC sig'ishi uchun */
#define KMALLOC_MAX   8192

static LIST_HEAD(cache_list);
static spinlock_t cache_list_lock = SPINLOCK_INIT("slab-caches");
static struct kmem_cache cache_cache;   /* kmem_cache larning o'zini saqlovchi kesh */

static const size_t kmalloc_sizes[] = { 16, 32, 64, 96, 128, 192, 256, 512,
                                        1024, 2048, 4096, 8192 };
static struct kmem_cache *kmalloc_caches[ARRAY_SIZE(kmalloc_sizes)];

static uint64_t large_pages;            /* kmalloc > 8 KB uchun buddy sahifalari */
static uint64_t large_bytes;
static uint64_t large_allocs, large_frees;

/* ---- Kesh yaratish ---- */

static void cache_setup(struct kmem_cache *c, const char *name, size_t size, size_t align,
                        void (*ctor)(void *))
{
    if (align < 8)
        align = 8;
    size_t obj = MAX(size, (size_t)MIN_OBJ);
    c->name = name;
    c->size = size;
    c->stride = ALIGN_UP(obj + REDZONE, align);
    /* Slab hajmini tanlaymiz: kamida 8 ta obyekt sig'sin, isrof 1/8 dan oshmasin. */
    c->order = 0;
    while (c->order < 3) {
        size_t bytes = PAGE_SIZE << c->order;
        size_t n = bytes / c->stride;
        if (n >= 8 && (bytes - n * c->stride) * 8 <= bytes)
            break;
        c->order++;
    }
    c->objs_per_slab = (unsigned)((PAGE_SIZE << c->order) / c->stride);
    if (c->objs_per_slab == 0)
        panic("kmem_cache %s: obyekt juda katta (%zu)", name, size);
    c->ctor = ctor;
    list_init(&c->partial);
    list_init(&c->full);
    list_init(&c->empty);
    c->nr_slabs = c->active_objs = c->allocs = c->frees = 0;
    c->lock = (spinlock_t)SPINLOCK_INIT("kmem_cache");
    uint64_t f = spin_lock_irqsave(&cache_list_lock);
    list_add_tail(&c->cache_list, &cache_list);
    spin_unlock_irqrestore(&cache_list_lock, f);
}

struct kmem_cache *kmem_cache_create(const char *name, size_t size, size_t align,
                                     void (*ctor)(void *))
{
    struct kmem_cache *c = kmem_cache_alloc(&cache_cache);
    if (c)
        cache_setup(c, name, size, align, ctor);
    return c;
}

static void free_slab_pages(struct kmem_cache *c, struct page *slab)
{
    for (unsigned i = 0; i < (1u << c->order); i++) {
        slab[i].flags &= ~PG_SLAB;
        slab[i].cache = NULL;
        slab[i].head = NULL;
    }
    c->nr_slabs--;
    free_pages(slab, c->order);
}

void kmem_cache_destroy(struct kmem_cache *c)
{
    if (c->active_objs)
        panic("kmem_cache_destroy(%s): %zu ta obyekt hali ishlatilmoqda (xotira oqishi!)",
              c->name, c->active_objs);
    uint64_t f = spin_lock_irqsave(&cache_list_lock);
    list_del(&c->cache_list);
    spin_unlock_irqrestore(&cache_list_lock, f);
    struct page *s, *tmp;
    list_for_each_entry_safe(s, tmp, &c->empty, list) {
        list_del(&s->list);
        free_slab_pages(c, s);
    }
    kmem_cache_free(&cache_cache, c);
}

/* ---- Obyekt holatlari ---- */

static void make_free(struct kmem_cache *c, uint8_t *obj, void *next)
{
    memset(obj, POISON_FREE, c->stride - REDZONE);
    memset(obj + c->stride - REDZONE, RZ_BYTE, REDZONE);
    ((void **)obj)[0] = next;
    ((uint64_t *)obj)[1] = FREE_MAGIC;
}

static void check_redzone(struct kmem_cache *c, const uint8_t *obj, const char *when)
{
    const uint8_t *rz = obj + c->stride - REDZONE;
    for (int i = 0; i < REDZONE; i++)
        if (rz[i] != RZ_BYTE)
            panic("slab %s: BUFFER OVERFLOW! obyekt %p chegarasidan tashqariga yozilgan "
                  "(redzone buzilgan, %s)", c->name, obj, when);
}

static void check_free_object(struct kmem_cache *c, const uint8_t *obj)
{
    if (((const uint64_t *)obj)[1] != FREE_MAGIC)
        panic("slab %s: bo'sh obyekt %p belgisi buzilgan (use-after-free)", c->name, obj);
    for (size_t i = 16; i < c->stride - REDZONE; i++)
        if (obj[i] != POISON_FREE)
            panic("slab %s: USE-AFTER-FREE! bo'shatilgan obyekt %p ning %zu-baytiga yozilgan",
                  c->name, obj, i);
    check_redzone(c, obj, "bo'sh turganida");
}

/* ---- Slab (sahifa guruhi) yaratish ---- */

static struct page *new_slab(struct kmem_cache *c)
{
    struct page *head = alloc_pages(c->order, 0);
    if (!head)
        return NULL;
    for (unsigned i = 0; i < (1u << c->order); i++) {
        head[i].flags |= PG_SLAB;
        head[i].cache = c;
        head[i].head = head;            /* kfree istalgan sahifadan boshni topsin */
    }
    uint8_t *base = page_to_virt(head);
    void *next = NULL;
    for (int i = (int)c->objs_per_slab - 1; i >= 0; i--) {
        uint8_t *obj = base + (size_t)i * c->stride;
        make_free(c, obj, next);
        next = obj;
    }
    head->freelist = next;
    head->inuse = 0;
    c->nr_slabs++;
    return head;
}

void *kmem_cache_alloc(struct kmem_cache *c)
{
    uint64_t flags = spin_lock_irqsave(&c->lock);
    struct page *slab;
    if (!list_empty(&c->partial)) {
        slab = list_first_entry(&c->partial, struct page, list);
    } else if (!list_empty(&c->empty)) {
        slab = list_first_entry(&c->empty, struct page, list);
        list_del(&slab->list);
        list_add(&slab->list, &c->partial);
    } else {
        slab = new_slab(c);
        if (!slab) {
            spin_unlock_irqrestore(&c->lock, flags);
            return NULL;
        }
        list_add(&slab->list, &c->partial);
    }

    uint8_t *obj = slab->freelist;
    check_free_object(c, obj);
    slab->freelist = ((void **)obj)[0];
    ((uint64_t *)obj)[1] = 0;
    slab->inuse++;
    if (slab->inuse == c->objs_per_slab) {
        list_del(&slab->list);
        list_add(&slab->list, &c->full);
    }
    c->active_objs++;
    c->allocs++;
    spin_unlock_irqrestore(&c->lock, flags);

    if (c->ctor)
        c->ctor(obj);
    return obj;
}

void kmem_cache_free(struct kmem_cache *c, void *ptr)
{
    struct page *pg = virt_to_page(ptr);
    if (!(pg->flags & PG_SLAB) || pg->cache != c)
        panic("kmem_cache_free(%s): %p bu keshga tegishli emas", c->name, ptr);
    struct page *slab = pg->head;
    uint8_t *base = page_to_virt(slab);
    uint8_t *obj = ptr;
    if ((size_t)(obj - base) % c->stride != 0)
        panic("kfree: %p obyekt boshiga ko'rsatmaydi (kesh %s)", ptr, c->name);
    if (((uint64_t *)obj)[1] == FREE_MAGIC)
        panic("kfree: DOUBLE FREE! %p allaqachon bo'shatilgan (kesh %s)", ptr, c->name);
    check_redzone(c, obj, "kfree paytida");

    uint64_t flags = spin_lock_irqsave(&c->lock);
    bool was_full = slab->inuse == c->objs_per_slab;
    make_free(c, obj, slab->freelist);
    slab->freelist = obj;
    slab->inuse--;
    c->active_objs--;
    c->frees++;
    if (was_full) {
        list_del(&slab->list);
        list_add(&slab->list, &c->partial);
    }
    if (slab->inuse == 0) {
        list_del(&slab->list);
        if (list_empty(&c->empty)) {    /* bittasini zaxirada saqlaymiz */
            list_add(&slab->list, &c->empty);
        } else {
            free_slab_pages(c, slab);
        }
    }
    spin_unlock_irqrestore(&c->lock, flags);
}

/* ---- kmalloc ---- */

void *kmalloc(size_t size)
{
    if (size == 0)
        return NULL;
    if (size <= KMALLOC_MAX) {
        for (size_t i = 0; i < ARRAY_SIZE(kmalloc_sizes); i++)
            if (size <= kmalloc_sizes[i])
                return kmem_cache_alloc(kmalloc_caches[i]);
    }
    /* Katta so'rov: to'g'ridan-to'g'ri buddy. Hajmni struct page'da saqlaymiz. */
    unsigned order = pages_to_order((size + PAGE_SIZE - 1) / PAGE_SIZE);
    struct page *p = alloc_pages(order, 0);
    if (!p)
        return NULL;
    p->inuse = (uint32_t)MIN(size, (size_t)UINT32_MAX);
    __atomic_add_fetch(&large_pages, 1UL << order, __ATOMIC_RELAXED);
    __atomic_add_fetch(&large_bytes, PAGE_SIZE << order, __ATOMIC_RELAXED);
    __atomic_add_fetch(&large_allocs, 1, __ATOMIC_RELAXED);
    return page_to_virt(p);
}

void *kzalloc(size_t size)
{
    void *p = kmalloc(size);
    if (p)
        memset(p, 0, size);
    return p;
}

void kfree(void *ptr)
{
    if (!ptr)
        return;
    uint64_t v = (uint64_t)ptr;
    if (v < HHDM_BASE || v >= VMALLOC_START)
        panic("kfree: %p kmalloc() dan olinmagan (direct map tashqarisida)", ptr);
    struct page *pg = virt_to_page(ptr);
    if (pg->flags & PG_SLAB) {
        kmem_cache_free(pg->cache, ptr);
        return;
    }
    if (!(pg->flags & PG_HEAD) || ((uint64_t)ptr & (PAGE_SIZE - 1)))
        panic("kfree: %p kmalloc() dan olinmagan yoki allaqachon bo'shatilgan", ptr);
    unsigned order = pg->order;
    __atomic_sub_fetch(&large_pages, 1UL << order, __ATOMIC_RELAXED);
    __atomic_sub_fetch(&large_bytes, PAGE_SIZE << order, __ATOMIC_RELAXED);
    __atomic_add_fetch(&large_frees, 1, __ATOMIC_RELAXED);
    free_pages(pg, order);
}

size_t ksize(const void *ptr)
{
    struct page *pg = virt_to_page(ptr);
    if (pg->flags & PG_SLAB)
        return pg->cache->size;
    return PAGE_SIZE << pg->order;
}

/* ---- Ishga tushirish va statistika ---- */

static char kmalloc_names[ARRAY_SIZE(kmalloc_sizes)][16];

void slab_init(void)
{
    cache_setup(&cache_cache, "kmem_cache", sizeof(struct kmem_cache), 8, NULL);
    for (size_t i = 0; i < ARRAY_SIZE(kmalloc_sizes); i++) {
        ksnprintf(kmalloc_names[i], sizeof(kmalloc_names[i]), "kmalloc-%zu", kmalloc_sizes[i]);
        kmalloc_caches[i] = kmem_cache_create(kmalloc_names[i], kmalloc_sizes[i], 8, NULL);
        if (!kmalloc_caches[i])
            panic("slab_init: %s yaratilmadi", kmalloc_names[i]);
    }
    kprintf("[slab] %zu ta kmalloc keshi (16..8192 bayt), redzone + zahar + double-free tekshiruvi\n",
            ARRAY_SIZE(kmalloc_sizes));
}

void heap_get_stats(struct heap_stats *out)
{
    memset(out, 0, sizeof(*out));
    uint64_t f = spin_lock_irqsave(&cache_list_lock);
    struct kmem_cache *c;
    list_for_each_entry(c, &cache_list, cache_list) {
        out->alloc_count += c->allocs;
        out->free_count += c->frees;
        out->bytes_in_use += c->active_objs * c->size;
        out->slab_pages += c->nr_slabs << c->order;
    }
    spin_unlock_irqrestore(&cache_list_lock, f);
    out->alloc_count += large_allocs;
    out->free_count += large_frees;
    out->bytes_in_use += large_bytes;
    out->large_pages = large_pages;
}

void slab_dump(void)
{
    kprintf("%-18s %8s %8s %6s %6s\n", "kesh", "faol", "hajm", "slab", "order");
    uint64_t f = spin_lock_irqsave(&cache_list_lock);
    struct kmem_cache *c;
    list_for_each_entry(c, &cache_list, cache_list)
        kprintf("%-18s %8zu %8zu %6zu %6u\n", c->name, c->active_objs, c->size, c->nr_slabs,
                c->order);
    spin_unlock_irqrestore(&cache_list_lock, f);
}
