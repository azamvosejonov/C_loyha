/* =============================================================================
 *  mm/pmm.c - BUDDY ALLOCATOR (Linux'dagi fizik xotira allocatori asosi)
 * =============================================================================
 *
 *  G'OYA:
 *    Xotirani 2 ning darajasi o'lchamidagi bloklarda boshqaramiz:
 *      tartib (order) 0 = 1 sahifa (4 KB), 1 = 2 sahifa, ..., 10 = 1024 sahifa (4 MB)
 *    Har bir tartib uchun bo'sh bloklar ro'yxati (free_area[order]).
 *
 *  AJRATISH (alloc_pages(order)):
 *    order ro'yxati bo'sh bo'lsa, kattaroq blokni olib, IKKIGA BO'LAMIZ.
 *    Bir yarmini beramiz, ikkinchi yarmi ("buddy" - egizak) kichik ro'yxatga
 *    tushadi:
 *
 *        order 2:  [ A A A A ]                      so'rov: order 0
 *        order 1:  [ A A ][ B B ]   <- B ro'yxatga
 *        order 0:  [ A ][ C ]       <- C ro'yxatga, A beriladi
 *
 *  QAYTARISH (free_pages) - BUDDY'NING SEHRI:
 *    Blokning egizagi manzili oddiy XOR bilan topiladi:
 *        buddy_pfn = pfn ^ (1 << order)
 *    Agar egizak ham bo'sh va aynan shu tartibda bo'lsa - ikkalasini
 *    BIRLASHTIRAMIZ va yuqori tartibda takrorlaymiz. Natijada xotira hech qachon
 *    mayda bo'laklarga "parchalanib" qolmaydi (tashqi fragmentatsiya kamayadi).
 *
 *  MURAKKABLIK: ajratish va qaytarish O(MAX_ORDER) = O(1) amalda.
 *
 *  ZONALAR: ba'zi qurilmalar (eski disk kontrollerlari) faqat 32-bitli
 *  manzillarga DMA qila oladi. Shuning uchun 4 GB dan pastki xotirani alohida
 *  "DMA32" zonasida saqlaymiz va oddiy so'rovlar avval yuqori zonadan oladi.
 * ============================================================================= */
#include "mm/pmm.h"

#include <stdbool.h>

#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/spinlock.h"
#include "lib/string.h"
#include "mm/memblock.h"

struct free_area {
    struct list_head list;              /* shu tartibdagi bo'sh bloklar (bosh sahifalari) */
    size_t count;
};

struct zone {
    const char *name;
    uint64_t start_pfn, end_pfn;        /* [start, end) */
    struct free_area free_area[MAX_ORDER];
    size_t free_pages;
    size_t managed_pages;               /* buddy'ga berilgan jami sahifalar */
    spinlock_t lock;
};

struct page *mem_map;
uint64_t max_pfn;

static struct zone zones[NR_ZONES] = {
    [ZONE_DMA32] = { .name = "DMA32", .lock = SPINLOCK_INIT("zone-dma32") },
    [ZONE_NORMAL] = { .name = "Normal", .lock = SPINLOCK_INIT("zone-normal") },
};

#define PFN_4G (0x100000000UL >> PAGE_SHIFT)

static struct zone *pfn_zone(uint64_t pfn)
{
    return pfn < PFN_4G ? &zones[ZONE_DMA32] : &zones[ZONE_NORMAL];
}

unsigned pages_to_order(size_t n)
{
    unsigned order = 0;
    while (((size_t)1 << order) < n)
        order++;
    return order;
}

/* ---- Bo'sh ro'yxatlar bilan ishlash (zone->lock ushlangan holda) ---- */

static void add_free(struct zone *z, struct page *p, unsigned order)
{
    p->flags |= PG_BUDDY;
    p->order = (uint8_t)order;
    list_add(&p->list, &z->free_area[order].list);
    z->free_area[order].count++;
}

static void del_free(struct zone *z, struct page *p, unsigned order)
{
    list_del(&p->list);
    p->flags &= ~PG_BUDDY;
    z->free_area[order].count--;
}

/* Blokni qaytarish va egizaklari bilan birlashtirish. */
static void buddy_free(struct zone *z, uint64_t pfn, unsigned order)
{
    /* >>> LAB buddy_free - vazifa: labs/README.md */
    while (order < MAX_ORDER - 1) {
        uint64_t buddy_pfn = pfn ^ (1UL << order);
        if (buddy_pfn < z->start_pfn || buddy_pfn + (1UL << order) > z->end_pfn)
            break;                      /* egizak zonadan tashqarida */
        struct page *buddy = pfn_to_page(buddy_pfn);
        /* Egizak BO'SH va AYNAN shu tartibda bo'lishi kerak. Aks holda u
         * bo'lingan (bir qismi band) - birlashtirib bo'lmaydi. */
        if (!(buddy->flags & PG_BUDDY) || buddy->order != order)
            break;
        del_free(z, buddy, order);
        pfn &= ~(1UL << order);         /* birlashgan blok boshi - ikkalasining kichigi */
        order++;
    }
    add_free(z, pfn_to_page(pfn), order);
    /* <<< LAB buddy_free */
}

static struct page *buddy_alloc(struct zone *z, unsigned order)
{
    /* >>> LAB buddy_alloc - vazifa: labs/README.md */
    for (unsigned o = order; o < MAX_ORDER; o++) {
        if (list_empty(&z->free_area[o].list))
            continue;
        struct page *p = list_first_entry(&z->free_area[o].list, struct page, list);
        del_free(z, p, o);
        /* Katta blokni bo'lamiz: har safar yuqori yarmini ro'yxatga qaytaramiz. */
        while (o > order) {
            o--;
            add_free(z, p + (1UL << o), o);
        }
        return p;
    }
    return NULL;
    /* <<< LAB buddy_alloc */
}

/* ---- Ommaviy API ---- */

struct page *alloc_pages(unsigned order, unsigned gfp)
{
    if (order >= MAX_ORDER)
        return NULL;
    /* Oddiy so'rov: avval Normal (yuqori) zona, keyin DMA32. DMA32 so'rovi -
     * faqat DMA32 zonasidan. */
    int first = (gfp & GFP_DMA32) ? ZONE_DMA32 : ZONE_NORMAL;
    for (int zi = first; zi >= 0; zi--) {
        struct zone *z = &zones[zi];
        uint64_t flags = spin_lock_irqsave(&z->lock);
        struct page *p = buddy_alloc(z, order);
        if (p)
            z->free_pages -= 1UL << order;
        spin_unlock_irqrestore(&z->lock, flags);
        if (!p)
            continue;
        p->flags |= PG_HEAD;
        p->order = (uint8_t)order;
        p->refcount = 1;
        p->mapcount = 0;
        if (gfp & GFP_ZERO)
            memset(page_to_virt(p), 0, PAGE_SIZE << order);
        return p;
    }
    return NULL;
}

void free_pages(struct page *p, unsigned order)
{
    uint64_t pfn = page_to_pfn(p);
    /* Himoya tekshiruvlari: xotira xatolarini IMKON QADAR ERTA ushlaymiz. */
    if (pfn >= max_pfn)
        panic("free_pages: pfn %lu mavjud emas", pfn);
    if (p->flags & PG_RESERVED)
        panic("free_pages: band (reserved) sahifa %p", (void *)page_to_phys(p));
    if (p->flags & PG_BUDDY)
        panic("free_pages: DOUBLE FREE! %p allaqachon bo'sh", (void *)page_to_phys(p));
    if (pfn & ((1UL << order) - 1))
        panic("free_pages: %p tartib %u ga tekislanmagan", (void *)page_to_phys(p), order);
    if ((p->flags & PG_HEAD) && p->order != order)
        panic("free_pages: %p tartib %u bilan ajratilgan, %u bilan qaytarilyapti",
              (void *)page_to_phys(p), p->order, order);

    p->flags &= ~(PG_HEAD | PG_SLAB);
    p->refcount = 0;
    p->cache = NULL;
    struct zone *z = pfn_zone(pfn);
    uint64_t flags = spin_lock_irqsave(&z->lock);
    buddy_free(z, pfn, order);
    z->free_pages += 1UL << order;
    spin_unlock_irqrestore(&z->lock, flags);
}

void put_page(struct page *p)
{
    int32_t n = __atomic_sub_fetch(&p->refcount, 1, __ATOMIC_ACQ_REL);
    if (n == 0)
        free_pages(p, p->order);
    else if (n < 0)
        panic("put_page: refcount manfiy (%p)", (void *)page_to_phys(p));
}

uint64_t pmm_alloc_page(unsigned gfp)
{
    struct page *p = alloc_pages(0, gfp);
    return p ? page_to_phys(p) : 0;
}

void pmm_free_page(uint64_t phys)
{
    free_pages(phys_to_page(phys), 0);
}

uint64_t pmm_alloc_pages(size_t n, unsigned gfp)
{
    struct page *p = alloc_pages(pages_to_order(n), gfp);
    return p ? page_to_phys(p) : 0;
}

void pmm_free_pages(uint64_t phys, size_t n)
{
    free_pages(phys_to_page(phys), pages_to_order(n));
}

size_t pmm_total_pages(void)
{
    return zones[0].managed_pages + zones[1].managed_pages;
}

size_t pmm_free_pages_count(void)
{
    return zones[0].free_pages + zones[1].free_pages;
}

/* ---- Ishga tushirish ---- */

/* [start, end) pfn oralig'ini buddy'ga eng katta tekislangan bloklar bilan berish. */
static void free_range(uint64_t start, uint64_t end)
{
    while (start < end) {
        struct zone *z = pfn_zone(start);
        uint64_t zone_end = MIN(end, z->end_pfn);
        unsigned order = MAX_ORDER - 1;
        /* Blok start ga tekislangan va oraliqdan chiqmasligi kerak. */
        while (order > 0 && ((start & ((1UL << order) - 1)) || start + (1UL << order) > zone_end))
            order--;
        for (uint64_t i = 0; i < (1UL << order); i++)
            pfn_to_page(start + i)->flags &= ~PG_RESERVED;
        buddy_free(z, start, order);
        z->free_pages += 1UL << order;
        z->managed_pages += 1UL << order;
        start += 1UL << order;
    }
}

void pmm_init(void)
{
    max_pfn = memblock_max_ram() >> PAGE_SHIFT;

    /* struct page massivi: har bir sahifa uchun sizeof(struct page) bayt. */
    uint64_t map_size = max_pfn * sizeof(struct page);
    uint64_t map_phys = memblock_alloc(map_size, PAGE_SIZE, UINT64_MAX);
    mem_map = phys_to_virt(map_phys);
    for (uint64_t pfn = 0; pfn < max_pfn; pfn++) {
        mem_map[pfn].flags = PG_RESERVED;       /* sukut: hammasi band */
        list_init(&mem_map[pfn].list);
    }

    zones[ZONE_DMA32].start_pfn = 0;
    zones[ZONE_DMA32].end_pfn = MIN(max_pfn, PFN_4G);
    zones[ZONE_NORMAL].start_pfn = PFN_4G;
    zones[ZONE_NORMAL].end_pfn = MAX(max_pfn, PFN_4G);
    for (int zi = 0; zi < NR_ZONES; zi++)
        for (int o = 0; o < MAX_ORDER; o++)
            list_init(&zones[zi].free_area[o].list);

    /* memblock'da QOLGAN bo'sh hududlarni buddy'ga beramiz. memblock ajratgan
     * narsalar (sahifa jadvallari, mem_map ning o'zi) PG_RESERVED bo'lib qoladi. */
    memblock_retire();
    int count;
    const struct memblock_region *r = memblock_free_regions(&count);
    for (int i = 0; i < count; i++)
        free_range(r[i].base >> PAGE_SHIFT, r[i].end >> PAGE_SHIFT);

    kprintf("[pmm]  Buddy: mem_map %lu KB (%lu sahifa x %zu bayt), boshqariladi %zu MB, bo'sh %zu MB\n",
            map_size / KiB, max_pfn, sizeof(struct page), pmm_total_pages() * PAGE_SIZE / MiB,
            pmm_free_pages_count() * PAGE_SIZE / MiB);
}

void pmm_dump(void)
{
    for (int zi = 0; zi < NR_ZONES; zi++) {
        struct zone *z = &zones[zi];
        if (!z->managed_pages)
            continue;
        kprintf("[pmm]  zona %-6s bo'sh %6zu sahifa | tartib:", z->name, z->free_pages);
        for (int o = 0; o < MAX_ORDER; o++)
            kprintf(" %zu", z->free_area[o].count);
        kprintf("\n");
    }
}
