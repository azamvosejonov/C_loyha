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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "myos.h"

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
    fprintf(stderr, "\n*** malloc xatosi: %s (%p) ***\n", msg, p);
    abort();                          /* 134 = 128 + SIGABRT (Unix an'anasi) */
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
    /*
     * TODO(M4) - O'ZINGIZ YOZING (darslik/32-printf-malloc.md, 32.7):
     *   1) JOYINI TOPING: free_list manzil bo'yicha o'sib boradi. prev < b < cur bo'ladigan joyni toping
     *      (ko'rsatkichlarni < bilan solishtirish mumkin - ular bitta heap ichida).
     *   2) ULANG: b->next = cur; prev bo'lsa prev->next = b, aks holda free_list = b.
     *   3) O'NG QO'SHNI: b ning oxiri ((char *)b + HDR + b->size) == cur bo'lsa - birlashtiring:
     *      b->size += HDR + cur->size;  b->next = cur->next.
     *   4) CHAP QO'SHNI: prev ning oxiri == b bo'lsa - prev ni kattalashtiring (xuddi shunday).
     *      Tartib muhim: avval o'ng, keyin chap (aks holda b "yo'qolgan" bo'lishi mumkin).
     *   Tekshirish: tools/myos_mashq.sh  (M4 qatori; M4 testlari M3 ga tayanadi)
     *   Yozib bo'lgach: quyidagi VAQTINCHALIK 2 qatorni o'chiring.
     */
    b->next = free_list;                /* VAQTINCHALIK: ro'yxat BOSHIGA - tartib ham, birlashtirish ham yo'q */
    free_list = b;                      /*   (xotira "parchalanadi", trim_top ishlamaydi) */
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

/* VAQTINCHALIK (M3 yozilguncha): "bump" ajratuvchi - har so'rovga sbrk() dan YANGI joy, bo'shatilgan
 * xotira QAYTA ISHLATILMAYDI (free() uni ro'yxatga qo'yadi, lekin bu malloc ro'yxatga qaramaydi).
 * Tizim ishlayveradi, faqat xotira "oqadi". M3 ni yozgach bu funksiyani O'CHIRING. */
static void *malloc_vaqtinchalik(size_t size)
{
    if (size == 0 || size > (1UL << 30))
        return NULL;
    size = ALIGN16(size);
    struct block *b = sbrk((long)(HDR + size));
    if (b == (void *)-1)
        return NULL;
    heap_bytes += HDR + size;
    b->size = size;
    b->next = USED_MAGIC;
    used_bytes += size;
    used_blocks++;
    return b + 1;
}

void *malloc(size_t size)
{
    /*
     * TODO(M3) - O'ZINGIZ YOZING (darslik/32-printf-malloc.md, 32.6):
     *   1) size == 0 yoki juda katta (> 1 GB) -> NULL. size = ALIGN16(size).
     *   2) FIRST FIT: free_list bo'ylab yuring (prev ni ham eslab qoling); b->size >= size bo'lgan
     *      BIRINCHI blokni oling.
     *   3) SPLIT: b->size >= size + MIN_SPLIT bo'lsa - blokni bo'ling: b dan (HDR + size) bayt keyin
     *      yangi sarlavha (rest): rest->size = b->size - size - HDR; rest->next = b->next; b->size = size.
     *      Ro'yxatda b o'rniga rest turadi. Aks holda butun blokni beramiz: ro'yxatda b o'rniga b->next.
     *   4) b ni ro'yxatdan chiqaring (prev->next yoki free_list), b->next = USED_MAGIC (band belgisi),
     *      used_bytes += b->size; used_blocks++; return b + 1 (sarlavhadan KEYINGI manzil).
     *   5) Mos blok topilmasa: grow(size) - yadrodan joy oling va qaytadan qidiring; grow 0 qaytarsa -> NULL.
     *   Tekshirish: tools/myos_mashq.sh  (M3 qatori)
     *   Yozib bo'lgach: quyidagi qatorni va malloc_vaqtinchalik funksiyasini o'chiring.
     */
    (void)grow;
    return malloc_vaqtinchalik(size);
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
