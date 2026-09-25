/* =============================================================================
 *  mm/memblock.c - ILK XOTIRA ALLOCATORI ("tovuq va tuxum" muammosi yechimi)
 * =============================================================================
 *
 *  MUAMMO:
 *    Buddy allocator (asosiy fizik xotira menejeri) ishlashi uchun har bir
 *    sahifaga bitta "struct page" kerak - bu 128 MB RAM uchun ~2 MB massiv.
 *    Bu massivni qayerdan olamiz? Allocator hali yo'q! Yangi sahifa jadvallari
 *    uchun ham xotira kerak...
 *
 *  YECHIM (Linux'dagi memblock kabi):
 *    Juda sodda allocator: RAM hududlari ro'yxati, har biridan "tishlab"
 *    olamiz (bump allocator). Qaytarish (free) YO'Q - boot vaqtida ajratilgan
 *    narsalar (sahifa jadvallari, struct page massivi) abadiy kerak.
 *    Buddy tayyor bo'lgach, memblock'da QOLGAN bo'sh joylar unga beriladi va
 *    memblock "nafaqaga chiqadi".
 *
 *  BAND QILINADIGANLAR: 0..1 MB (BIOS, SMP trampolin uchun joy), yadro tasviri,
 *  multiboot ma'lumoti, modullar (initrd). Ular hech qachon hududlar ro'yxatiga
 *  kirmaydi.
 * ============================================================================= */
#include "mm/memblock.h"

#include <stdbool.h>

#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/string.h"
#include "mm/layout.h"

extern char __kernel_phys_start[];
extern char __kernel_phys_end[];

static struct memblock_region regions[MEMBLOCK_MAX];
static int region_count;
static uint64_t max_ram;
static bool retired;
static uint64_t allocated_bytes;

/* [base, end) hududini ro'yxatga qo'shish. */
static void add_region(uint64_t base, uint64_t end)
{
    if (end <= base)
        return;
    if (region_count == MEMBLOCK_MAX) {
        kprintf("[memblock] hududlar juda ko'p, %p tashlab yuborildi\n", (void *)base);
        return;
    }
    regions[region_count++] = (struct memblock_region){ base, end };
}

/* [rbase, rend) ni barcha hududlardan "kesib" tashlash. Hudud ikkiga
 * bo'linishi mumkin (band joy o'rtada bo'lsa). */
static void reserve(uint64_t rbase, uint64_t rend)
{
    rbase = ALIGN_DOWN(rbase, PAGE_SIZE);
    rend = ALIGN_UP(rend, PAGE_SIZE);
    for (int i = 0; i < region_count; i++) {
        struct memblock_region *r = &regions[i];
        if (rend <= r->base || rbase >= r->end)
            continue;                   /* kesishmaydi */
        uint64_t left_end = rbase;      /* chap qoldiq: [r->base, rbase) */
        uint64_t right_base = rend;     /* o'ng qoldiq: [rend, r->end) */
        uint64_t old_base = r->base, old_end = r->end;
        /* Joriy hududni chap qoldiq bilan almashtiramiz, o'ng qoldiqni qo'shamiz. */
        r->base = old_base;
        r->end = MAX(old_base, MIN(left_end, old_end));
        if (right_base < old_end)
            add_region(MAX(right_base, old_base), old_end);
    }
    /* Bo'sh qolgan hududlarni olib tashlaymiz. */
    int j = 0;
    for (int i = 0; i < region_count; i++)
        if (regions[i].end > regions[i].base)
            regions[j++] = regions[i];
    region_count = j;
}

void memblock_init(const struct boot_info *bi)
{
    /* 1. RAM (type=1) hududlarini sahifa chegarasiga "ichkariga" yaxlitlab qo'shamiz. */
    for (size_t i = 0; i < bi->mmap_count; i++) {
        const struct mem_region *m = &bi->mmap[i];
        if (m->type != MEM_USABLE)
            continue;
        uint64_t base = ALIGN_UP(m->base, PAGE_SIZE);
        uint64_t end = ALIGN_DOWN(m->base + m->len, PAGE_SIZE);
        add_region(base, end);
        if (end > max_ram)
            max_ram = end;
    }

    /* 2. Band joylarni kesib tashlaymiz. */
    reserve(0, 1 * MiB);
    reserve((uint64_t)__kernel_phys_start, (uint64_t)__kernel_phys_end);
    reserve(bi->mbi_phys, bi->mbi_phys + bi->mbi_size);
    for (size_t i = 0; i < bi->module_count; i++)
        reserve(bi->modules[i].phys_start, bi->modules[i].phys_end);

    uint64_t total = 0;
    for (int i = 0; i < region_count; i++)
        total += regions[i].end - regions[i].base;
    kprintf("[memblock] %d ta bo'sh hudud, jami %lu MB, eng yuqori RAM %p\n", region_count,
            total / MiB, (void *)max_ram);
}

uint64_t memblock_alloc(uint64_t size, uint64_t align, uint64_t limit)
{
    if (retired)
        panic("memblock_alloc: buddy allocator allaqachon ishlayapti");
    size = ALIGN_UP(size, PAGE_SIZE);
    /* Eng YUQORI mos hududdan olamiz: past xotira (DMA qurilmalar va boshqalar
     * uchun qimmatli) iloji boricha bo'sh qolsin. */
    for (int i = region_count - 1; i >= 0; i--) {
        struct memblock_region *r = &regions[i];
        uint64_t end = MIN(r->end, limit);
        if (end < size)
            continue;
        uint64_t base = ALIGN_DOWN(end - size, align);
        if (base < r->base)
            continue;
        /* Hududning oxiridan kesamiz. Agar kesilgan qism oxirida bo'lmasa
         * (limit tufayli), reserve() hududni ikkiga bo'ladi. */
        reserve(base, base + size);
        memset(phys_to_virt(base), 0, size);
        allocated_bytes += size;
        return base;
    }
    panic("memblock: %lu bayt ajratib bo'lmadi (limit %p)", size, (void *)limit);
}

const struct memblock_region *memblock_free_regions(int *count)
{
    *count = region_count;
    return regions;
}

uint64_t memblock_max_ram(void)
{
    return max_ram;
}

void memblock_retire(void)
{
    retired = true;
    kprintf("[memblock] ishini tugatdi: boot vaqtida %lu KB ajratilgan\n", allocated_bytes / KiB);
}
