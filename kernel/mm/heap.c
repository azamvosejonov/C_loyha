/* =============================================================================
 *  mm/heap.c - SLAB ALLOCATOR (kmalloc/kfree)
 * =============================================================================
 *
 *  MUAMMO:
 *    PMM faqat 4 KB lik butun freymlar beradi. Lekin yadroga ko'pincha kichik
 *    obyektlar kerak: 40 baytlik jarayon strukturasi, 100 baytlik satr... Har
 *    biriga 4 KB berish - 99% xotirani isrof qilish.
 *
 *  YECHIM - SLAB (Jeff Bonwick, SunOS 1994; Linux'da SLUB):
 *    Obyektlarni o'lcham SINFLARIGA ajratamiz: 16, 32, 64, ..., 1024 bayt.
 *    Har bir sinf uchun 4 KB lik sahifalar ("slab") olib, ularni bir xil
 *    o'lchamdagi katakchalarga bo'lamiz:
 *
 *     slab (4 KB sahifa), sinf = 64 bayt:
 *     ┌────────┬──────┬──────┬──────┬──────┬─────┬──────┐
 *     │ sarlav.│ obj0 │ obj1 │ obj2 │ obj3 │ ... │ obj62│
 *     │ 64 B   │ band │ bo'sh│ band │ bo'sh│     │      │
 *     └────────┴──────┴──┬───┴──────┴──▲───┴─────┴──────┘
 *                        └─────────────┘  bo'sh obyektlar zanjiri (free list)
 *
 *    kmalloc(50): 64 baytlik sinf -> bo'sh joyi bor slab -> free list boshidagi
 *    obyektni olamiz. O(1)! Qidiruv yo'q.
 *    kfree(p): sahifa boshini topamiz (p & ~0xFFF), sarlavhadan sinfni bilamiz,
 *    obyektni free list boshiga qaytaramiz. Bu ham O(1).
 *
 *    BO'SH OBYEKTLAR ZANJIRI XOTIRA TALAB QILMAYDI: bo'sh obyektning o'zining
 *    birinchi 8 baytida keyingi bo'sh obyekt manzilini saqlaymiz. Obyekt
 *    bo'sh ekan, uning ichidagi joy baribir hech kimga kerak emas.
 *
 *    1024 baytdan katta so'rovlar uchun: to'g'ridan-to'g'ri PMM dan ketma-ket
 *    freymlar ("large allocation"), birinchi 64 baytida sarlavha.
 *
 *  XOTIRA XATOLARINI ANIQLASH (debug allocator g'oyalari):
 *    * MAGIC: har bir sahifa sarlavhasida sehrli son. kfree() ga kmalloc dan
 *      kelmagan ko'rsatkich berilsa - darhol panic.
 *    * DOUBLE FREE: bo'sh obyektning 8..15 baytlarida FREE_MAGIC. kfree() da
 *      shu belgi bo'lsa - obyekt allaqachon bo'sh -> panic.
 *    * USE-AFTER-FREE: bo'shatilgan obyekt 0x6B baytlari bilan to'ldiriladi
 *      ("zahar", Linux'dagi POISON_FREE). kmalloc() uni qayta berishdan oldin
 *      zahar buzilmaganini tekshiradi. Buzilgan bo'lsa - kimdir bo'shatilgan
 *      xotiraga YOZGAN -> panic. Bu eng xavfli C xatolaridan biri.
 *    * NOTO'G'RI KO'RSATKICH: obyektning o'rtasiga ko'rsatuvchi p -> panic.
 * ============================================================================= */
#include "mm/heap.h"

#include <stdbool.h>

#include "arch/cpu.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/string.h"
#include "mm/pmm.h"

#define SLAB_MAGIC   0x534C4142u        /* "SLAB" */
#define LARGE_MAGIC  0x4C415247u        /* "LARG" */
#define FREE_MAGIC   0xF4EEF4EEF4EEF4EEULL
#define POISON_FREE  0x6B               /* bo'sh obyekt "zahari" */

#define HEADER_SIZE  64                 /* sarlavha uchun joy: obyektlar 16 ga tekis boshlanishi uchun */
#define MIN_SHIFT    4                  /* 2^4 = 16 bayt - eng kichik sinf */
#define NUM_CLASSES  7                  /* 16, 32, 64, 128, 256, 512, 1024 */
#define MAX_SMALL    (1 << (MIN_SHIFT + NUM_CLASSES - 1))   /* 1024 */

/* Slab sahifasining sarlavhasi (sahifa boshida turadi). */
struct slab {
    uint32_t magic;
    uint16_t class_index;               /* qaysi o'lcham sinfi */
    uint16_t obj_size;                  /* obyekt hajmi (baytda) */
    uint16_t capacity;                  /* sahifaga nechta obyekt sig'adi */
    uint16_t in_use;                    /* hozir nechtasi band */
    uint32_t reserved;
    void *free_list;                    /* birinchi bo'sh obyekt */
    struct slab *prev;                  /* "bo'sh joyi bor slab'lar" ro'yxatida */
    struct slab *next;
};

/* Katta ajratma sarlavhasi. */
struct large_header {
    uint32_t magic;
    uint32_t pages;                     /* nechta freym */
    uint64_t size;                      /* so'ralgan hajm */
};

_Static_assert(sizeof(struct slab) <= HEADER_SIZE, "slab sarlavhasi juda katta");
_Static_assert(sizeof(struct large_header) <= HEADER_SIZE, "large sarlavhasi juda katta");

/* Har bir sinf uchun: bo'sh joyi bor slab'lar ikki tomonlama ro'yxati.
 * To'la slab'lar hech qayerda saqlanmaydi - ular bizga kerak emas, kfree()
 * ularni sahifa manzilidan topadi. */
static struct slab *partial[NUM_CLASSES];
static struct heap_stats stats;

/* size -> sinf indeksi. Masalan 50 -> 64 baytlik sinf (indeks 2). */
static int size_to_class(size_t size)
{
    int cls = 0;
    size_t cap = 1u << MIN_SHIFT;
    while (cap < size) {
        cap <<= 1;
        cls++;
    }
    return cls;
}

/* ---- Ikki tomonlama ro'yxat amallari ---------------------------------------- */

static void list_push(struct slab **head, struct slab *s)
{
    s->prev = NULL;
    s->next = *head;
    if (*head)
        (*head)->prev = s;
    *head = s;
}

static void list_remove(struct slab **head, struct slab *s)
{
    if (s->prev)
        s->prev->next = s->next;
    else
        *head = s->next;
    if (s->next)
        s->next->prev = s->prev;
    s->prev = s->next = NULL;
}

/* ---- Zahar (poison) --------------------------------------------------------- */

/* Obyektni "bo'sh" holatiga keltirish: zahar + FREE_MAGIC + keyingi ko'rsatkich. */
static void poison_object(void *obj, size_t size, void *next_free)
{
    memset(obj, POISON_FREE, size);
    ((void **)obj)[0] = next_free;      /* 0..7 bayt: zanjir */
    ((uint64_t *)obj)[1] = FREE_MAGIC;  /* 8..15 bayt: "men bo'shman" belgisi */
}

/* Bo'sh obyektning zahari buzilmaganmi? (16-baytdan boshlab tekshiramiz). */
static void check_poison(const struct slab *s, const uint8_t *obj)
{
    if (((const uint64_t *)obj)[1] != FREE_MAGIC)
        panic("heap: bo'sh obyekt %p belgisi buzilgan (use-after-free yozuvi?)", obj);
    for (size_t i = 16; i < s->obj_size; i++) {
        if (obj[i] != POISON_FREE)
            panic("heap: USE-AFTER-FREE! bo'shatilgan obyekt %p ning %zu-baytiga yozilgan "
                  "(sinf %u bayt)", obj, i, s->obj_size);
    }
}

/* ---- Slab yaratish ---------------------------------------------------------- */

static struct slab *slab_create(int cls)
{
    uint64_t phys = pmm_alloc_frame();
    if (!phys)
        return NULL;
    struct slab *s = (struct slab *)(uintptr_t)phys;    /* identity mapping */
    s->magic = SLAB_MAGIC;
    s->class_index = (uint16_t)cls;
    s->obj_size = (uint16_t)(1u << (MIN_SHIFT + cls));
    s->capacity = (uint16_t)((PAGE_SIZE - HEADER_SIZE) / s->obj_size);
    s->in_use = 0;
    s->prev = s->next = NULL;

    /* Barcha obyektlarni free list'ga zanjirlaymiz (oxiridan boshlab, shunda
     * zanjir manzil bo'yicha o'sib boradi). */
    uint8_t *base = (uint8_t *)s + HEADER_SIZE;
    void *next = NULL;
    for (int i = s->capacity - 1; i >= 0; i--) {
        uint8_t *obj = base + (size_t)i * s->obj_size;
        poison_object(obj, s->obj_size, next);
        next = obj;
    }
    s->free_list = next;
    stats.slab_pages++;
    return s;
}

/* ---- Katta ajratmalar ------------------------------------------------------- */

static void *large_alloc(size_t size)
{
    size_t pages = (size + HEADER_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;
    uint64_t phys = pmm_alloc_frames(pages);
    if (!phys)
        return NULL;
    struct large_header *h = (struct large_header *)(uintptr_t)phys;
    h->magic = LARGE_MAGIC;
    h->pages = (uint32_t)pages;
    h->size = size;
    stats.large_pages += pages;
    stats.bytes_in_use += size;
    return (uint8_t *)h + HEADER_SIZE;
}

/* ---- Ommaviy interfeys ------------------------------------------------------ */

void *kmalloc(size_t size)
{
    if (size == 0)
        return NULL;

    uint64_t flags = irq_save();        /* heap - umumiy resurs */
    void *result;
    stats.alloc_count++;

    if (size > MAX_SMALL) {
        result = large_alloc(size);
        irq_restore(flags);
        return result;
    }

    int cls = size_to_class(size);
    struct slab *s = partial[cls];
    if (!s) {                           /* bo'sh joyi bor slab yo'q - yangisini yaratamiz */
        s = slab_create(cls);
        if (!s) {
            irq_restore(flags);
            return NULL;
        }
        list_push(&partial[cls], s);
    }

    uint8_t *obj = s->free_list;
    check_poison(s, obj);               /* bo'sh turgan paytida kimdir tegdimi? */
    s->free_list = ((void **)obj)[0];   /* zanjirdan chiqaramiz */
    ((uint64_t *)obj)[1] = 0;           /* FREE_MAGIC ni o'chiramiz - endi band */
    s->in_use++;
    stats.bytes_in_use += s->obj_size;

    if (s->in_use == s->capacity)       /* slab to'ldi - "partial" ro'yxatdan chiqaramiz */
        list_remove(&partial[cls], s);

    irq_restore(flags);
    return obj;
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
        return;                         /* free(NULL) - standart bo'yicha ruxsat */

    uint64_t flags = irq_save();
    uint8_t *page = (uint8_t *)ALIGN_DOWN((uintptr_t)ptr, PAGE_SIZE);
    uint32_t magic = *(uint32_t *)page;
    stats.free_count++;

    if (magic == LARGE_MAGIC) {
        struct large_header *h = (struct large_header *)page;
        if ((uint8_t *)ptr != page + HEADER_SIZE)
            panic("kfree: %p katta ajratmaning boshi emas", ptr);
        h->magic = 0;                   /* ikkinchi kfree endi "noto'g'ri ko'rsatkich" deydi */
        stats.large_pages -= h->pages;
        stats.bytes_in_use -= h->size;
        pmm_free_frames((uint64_t)(uintptr_t)page, h->pages);
        irq_restore(flags);
        return;
    }

    if (magic != SLAB_MAGIC)
        panic("kfree: %p kmalloc() dan olinmagan (yoki sarlavha buzilgan)", ptr);

    struct slab *s = (struct slab *)page;
    uint8_t *base = page + HEADER_SIZE;
    if ((uint8_t *)ptr < base || ((uint8_t *)ptr - base) % s->obj_size != 0)
        panic("kfree: %p obyekt boshiga ko'rsatmaydi", ptr);
    if (((uint64_t *)ptr)[1] == FREE_MAGIC)
        panic("kfree: DOUBLE FREE! %p allaqachon bo'shatilgan", ptr);

    bool was_full = (s->in_use == s->capacity);
    poison_object(ptr, s->obj_size, s->free_list);
    s->free_list = ptr;
    s->in_use--;
    stats.bytes_in_use -= s->obj_size;

    if (was_full)                       /* yana bo'sh joy paydo bo'ldi */
        list_push(&partial[s->class_index], s);

    /* Slab butunlay bo'shadi. Agar sinfda boshqa slab ham bo'lsa - sahifani PMM
     * ga qaytaramiz. Bittasini zaxirada qoldiramiz: aks holda "ajrat-bo'shat"
     * tsiklida har safar sahifa olib-qaytarib yurardik (thrashing). */
    if (s->in_use == 0 && (s->prev || s->next)) {
        list_remove(&partial[s->class_index], s);
        s->magic = 0;
        stats.slab_pages--;
        pmm_free_frame((uint64_t)(uintptr_t)page);
    }
    irq_restore(flags);
}

void heap_get_stats(struct heap_stats *out)
{
    uint64_t flags = irq_save();
    *out = stats;
    irq_restore(flags);
}

void heap_dump(void)
{
    kprintf("[heap] ajratish=%lu bo'shatish=%lu band=%lu bayt slab=%lu sahifa katta=%lu sahifa\n",
            stats.alloc_count, stats.free_count, stats.bytes_in_use, stats.slab_pages,
            stats.large_pages);
    for (int c = 0; c < NUM_CLASSES; c++) {
        int n = 0;
        for (struct slab *s = partial[c]; s; s = s->next)
            n++;
        kprintf("       sinf %4u bayt: %d ta qisman bo'sh slab\n", 1u << (MIN_SHIFT + c), n);
    }
}
