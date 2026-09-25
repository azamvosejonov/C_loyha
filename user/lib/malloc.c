/* =============================================================================
 *  user/lib/malloc.c - user rejimidagi malloc/free (sbrk ustida)
 * =============================================================================
 *
 *  YADRODAGI SLAB BILAN SOLISHTIRING (kernel/mm/heap.c):
 *    Slab - bir xil o'lchamli obyektlar uchun ideal (O(1), fragmentatsiya yo'q).
 *    User dasturlari esa istalgan o'lchamni so'raydi: 7 bayt, 3000 bayt,
 *    1 MB... Bu yerda klassik "FREE LIST" allocator ishlatamiz (K&R C kitobidagi
 *    va eski Unix malloc g'oyasi):
 *
 *    Heap - ketma-ket bloklar. Har bir blok oldida 16 baytlik sarlavha:
 *
 *    ┌──────┬─────────┬──────┬──────────────┬──────┬────┬──────┬──────────┐
 *    │ hdr  │ BAND 32 │ hdr  │ BO'SH 200    │ hdr  │BAND│ hdr  │ BO'SH ...│
 *    └──────┴─────────┴──────┴──────▲───────┴──────┴────┴──────┴────▲─────┘
 *                                   └── free_list ──────── next ────┘
 *
 *    Bo'sh bloklar MANZIL BO'YICHA TARTIBLANGAN ro'yxatda. Bu ikki narsani
 *    oson qiladi:
 *
 *    * malloc(n): "first fit" - ro'yxatdagi n dan katta birinchi blok.
 *      Blok ancha katta bo'lsa - uni ikkiga BO'LAMIZ (split): boshi beriladi,
 *      qolgani ro'yxatda qoladi.
 *    * free(p): blokni ro'yxatdagi to'g'ri joyiga qo'yamiz va QO'SHNI bo'sh
 *      bloklar bilan BIRLASHTIRAMIZ (coalesce). Aks holda heap mayda
 *      bo'laklarga parchalanib ketadi (FRAGMENTATSIYA): jami 1 MB bo'sh bo'lsa
 *      ham, 1 KB lik uzluksiz joy topilmasligi mumkin.
 *    * Joy yetmasa - yadrodan sbrk() bilan kamida 64 KB so'raymiz.
 *    * Heap tepasida katta bo'sh blok qolsa - sbrk(-n) bilan xotirani
 *      yadroga QAYTARAMIZ.
 *
 *  XATOLARNI ANIQLASH: band blok sarlavhasidagi `next` maydoniga maxsus
 *  qiymat (USED_MAGIC) yozamiz. free() uni tekshiradi: yo'q bo'lsa -
 *  ko'rsatkich malloc'dan kelmagan yoki blok allaqachon bo'shatilgan.
 *  glibc ham "free(): double free detected" xabarini shunday beradi.
 * ============================================================================= */
#include "ulib.h"

struct block {
    size_t size;                        /* foydali yuk hajmi (sarlavhasiz), 16 ga karrali */
    struct block *next;                 /* bo'sh: keyingi bo'sh blok; band: USED_MAGIC */
};

#define HDR        sizeof(struct block) /* 16 bayt - foydali yuk ham 16 ga tekis bo'ladi */
#define ALIGN16(x) (((x) + 15) & ~(size_t)15)
#define MIN_SPLIT  (HDR + 16)           /* bundan kichik qoldiq bo'lmaydi - bo'lmaymiz */
#define GROW_MIN   (64 * 1024)          /* sbrk ni har safar kamida shuncha chaqiramiz */
#define TRIM_KEEP  (64 * 1024)          /* heap tepasida shuncha bo'sh joy qoldiramiz */
#define USED_MAGIC ((struct block *)0xA110CA7EDB10C000ULL)

static struct block *free_list;         /* manzil bo'yicha tartiblangan */
static size_t heap_bytes;
static size_t used_bytes;
static size_t used_blocks;

__attribute__((noreturn)) static void malloc_abort(const char *msg, void *p)
{
    printf("\n*** malloc xatosi: %s (%p) ***\n", msg, p);
    exit(134);                          /* 134 = 128 + SIGABRT (Unix an'anasi) */
}

/* Heap tepasidagi bo'sh blokni kichraytirib, xotirani yadroga qaytarish. */
static void trim_top(void)
{
    struct block *last = free_list;
    if (!last)
        return;
    while (last->next)                  /* manzil bo'yicha eng oxirgi bo'sh blok */
        last = last->next;
    char *end = (char *)last + HDR + last->size;
    if (end != (char *)sbrk(0))         /* heap tepasida emas - tegmaymiz */
        return;
    if (last->size <= TRIM_KEEP + 4096)
        return;
    size_t release = (last->size - TRIM_KEEP) & ~(size_t)4095;
    if (sbrk(-(long)release) == (void *)-1)
        return;
    last->size -= release;
    heap_bytes -= release;
}

/* Blokni tartiblangan bo'sh ro'yxatga qo'shish va qo'shnilar bilan birlashtirish. */
static void insert_free(struct block *b)
{
    /* Tartiblangan ro'yxatdagi joyini topamiz: prev < b < cur */
    struct block *prev = NULL, *cur = free_list;
    while (cur && cur < b) {
        prev = cur;
        cur = cur->next;
    }
    b->next = cur;
    if (prev)
        prev->next = b;
    else
        free_list = b;

    /* O'ngdagi qo'shni bilan birlashtirish: b ning oxiri == cur ning boshi? */
    if (cur && (char *)b + HDR + b->size == (char *)cur) {
        b->size += HDR + cur->size;
        b->next = cur->next;
    }
    /* Chapdagi qo'shni bilan birlashtirish. */
    if (prev && (char *)prev + HDR + prev->size == (char *)b) {
        prev->size += HDR + b->size;
        prev->next = b->next;
    }
}

void free(void *ptr)
{
    if (!ptr)
        return;
    struct block *b = (struct block *)ptr - 1;
    if (b->next != USED_MAGIC)
        malloc_abort("free(): noto'g'ri ko'rsatkich yoki DOUBLE FREE", ptr);

    used_bytes -= b->size;
    used_blocks--;
    insert_free(b);
    trim_top();                         /* faqat haqiqiy free() da - grow() da EMAS! */
}

/* Yadrodan yangi xotira olib, uni free list'ga qo'shish. */
static int grow(size_t need)
{
    size_t bytes = need + HDR;
    if (bytes < GROW_MIN)
        bytes = GROW_MIN;
    bytes = (bytes + 4095) & ~(size_t)4095;     /* butun sahifalar */
    void *p = sbrk((long)bytes);
    if (p == (void *)-1)
        return 0;                       /* yadroda xotira yo'q */
    heap_bytes += bytes;
    struct block *b = p;
    b->size = bytes - HDR;
    /* DIQQAT: bu yerda free() emas, insert_free() chaqiramiz. free() oxirida
     * trim_top() bor - u biz hozirgina olgan xotirani darhol yadroga qaytarib
     * yuborardi va malloc() abadiy "o'stir-qaytar" tsikliga tushardi. (Bu bug
     * haqiqatan shu loyihani yozishda uchragan - docs/07-user-mode.md.) */
    insert_free(b);
    return 1;
}

void *malloc(size_t size)
{
    if (size == 0 || size > (1UL << 30))
        return NULL;
    size = ALIGN16(size);

    for (;;) {
        struct block *prev = NULL;
        for (struct block *b = free_list; b; prev = b, b = b->next) {
            if (b->size < size)
                continue;               /* sig'maydi - keyingisi */

            struct block *replacement;
            if (b->size >= size + MIN_SPLIT) {
                /* SPLIT: blok boshini beramiz, qolganidan yangi bo'sh blok. */
                struct block *rest = (struct block *)((char *)b + HDR + size);
                rest->size = b->size - size - HDR;
                rest->next = b->next;
                b->size = size;
                replacement = rest;
            } else {
                replacement = b->next;  /* butun blokni beramiz */
            }
            if (prev)
                prev->next = replacement;
            else
                free_list = replacement;

            b->next = USED_MAGIC;
            used_bytes += b->size;
            used_blocks++;
            return b + 1;               /* sarlavhadan keyingi manzil */
        }
        if (!grow(size))
            return NULL;
    }
}

void *calloc(size_t count, size_t size)
{
    if (size && count > (size_t)-1 / size)
        return NULL;                    /* count * size to'lib ketadi - xavfsizlik tekshiruvi! */
    void *p = malloc(count * size);
    if (p)
        memset(p, 0, count * size);
    return p;
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr)
        return malloc(size);
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    struct block *b = (struct block *)ptr - 1;
    if (b->next != USED_MAGIC)
        malloc_abort("realloc(): noto'g'ri ko'rsatkich", ptr);
    if (b->size >= size)
        return ptr;                     /* joy yetarli */
    void *n = malloc(size);
    if (!n)
        return NULL;
    memcpy(n, ptr, b->size);
    free(ptr);
    return n;
}

void malloc_get_stats(struct malloc_stats *out)
{
    out->heap_bytes = heap_bytes;
    out->used_bytes = used_bytes;
    out->used_blocks = used_blocks;
    out->free_bytes = 0;
    out->free_blocks = 0;
    for (struct block *b = free_list; b; b = b->next) {
        out->free_bytes += b->size;
        out->free_blocks++;
    }
}
