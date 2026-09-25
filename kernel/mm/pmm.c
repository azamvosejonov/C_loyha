/* =============================================================================
 *  mm/pmm.c - fizik xotira menejeri: BITMAP allocator
 * =============================================================================
 *
 *  VAZIFA:
 *    RAM'dagi har bir 4 KB lik freymning holatini bilish: bo'sh yoki band.
 *    Kimdir (virtual xotira, heap, jarayon steki...) xotira so'rasa - bo'sh
 *    freym topib berish; qaytarsa - yana bo'sh deb belgilash.
 *
 *  NIMA UCHUN BITMAP:
 *    Har bir freym uchun 1 bit: 0 = bo'sh, 1 = band.
 *    128 MB RAM = 32768 freym = 32768 bit = atigi 4 KB bitmap!
 *    Afzalligi: juda sodda, xotirani kam yeydi, ketma-ket freymlarni topish oson.
 *    Kamchiligi: bo'sh freym qidirish O(n). Biz buni ikki hiyla bilan tezlashtiramiz:
 *      1) 64 bitni birdaniga tekshirish: agar uint64_t so'z = 0xFFFF...FFFF bo'lsa,
 *         undagi 64 ta freymning hammasi band - butun so'zni o'tkazib yuboramiz.
 *      2) "next-fit": oxirgi topilgan joydan qidirishni davom ettirish.
 *    Linux boshqa usul - "buddy allocator" ishlatadi (docs/mashqlar.md da mashq).
 *
 *  ISHGA TUSHIRISH ALGORITMI (xavfsiz tomondan):
 *    1. HAMMA freymni "band" deb belgilaymiz.
 *    2. Multiboot xotira xaritasidagi "bo'sh RAM" (type=1) hududlarini bo'shatamiz.
 *    3. Haqiqatan band bo'lganlarini qayta "band" qilamiz:
 *         - birinchi 1 MB (BIOS, VGA, multiboot ma'lumotlari)
 *         - yadroning o'zi [kernel_start, kernel_end)
 *         - multiboot modullari (initrd.tar) va buyruq qatori
 *    Nega avval hammasini "band" qilamiz? Xaritada yo'q yoki "teshik" bo'lgan
 *    hududni tasodifan "bo'sh" deb bermaslik uchun. Xavfsiz sukut (safe default)
 *    - kuchli tizim dasturlashning asosiy tamoyillaridan biri.
 * ============================================================================= */
#include "mm/pmm.h"

#include "arch/cpu.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/string.h"

#define MAX_FRAMES (PMM_MAX_PHYS / PAGE_SIZE)   /* 262144 freym (1 GB) */
#define BITMAP_WORDS (MAX_FRAMES / 64)          /* 4096 ta 64-bitli so'z = 32 KB */

/* Bitmap .bss da (statik). U yadroning bir qismi, shuning uchun [kernel_start,
 * kernel_end) oralig'i uni ham qamrab oladi va u o'zini "bo'sh" deb bermaydi. */
static uint64_t bitmap[BITMAP_WORDS];

static size_t total_frames;             /* RAM deb e'lon qilingan freymlar */
static size_t used_frames;              /* hozir band freymlar (total ichida) */
static size_t highest_frame;            /* eng yuqori RAM freymi + 1 (qidiruv chegarasi) */
static size_t search_hint;              /* next-fit: keyingi qidiruvni shu so'zdan boshlash */

/* linker.ld da e'lon qilingan belgilar. Ular o'zgaruvchi EMAS, faqat manzil -
 * shuning uchun massiv sifatida e'lon qilib, &kernel_start o'rniga kernel_start
 * yozamiz. */
extern char kernel_start[];
extern char kernel_end[];

/* ---- Bitta bit bilan ishlash ------------------------------------------------ */

static inline void bit_set(size_t frame)
{
    bitmap[frame / 64] |= 1ULL << (frame % 64);
}

static inline void bit_clear(size_t frame)
{
    bitmap[frame / 64] &= ~(1ULL << (frame % 64));
}

static inline int bit_test(size_t frame)
{
    return (bitmap[frame / 64] >> (frame % 64)) & 1;
}

/* ---- Hududlarni belgilash --------------------------------------------------- */

/* [base, base+len) ni o'z ichiga olgan freymlarni BO'SH qilish.
 * Faqat TO'LIQ freymlar: qisman bo'sh freymni berib bo'lmaydi, shuning uchun
 * boshini yuqoriga, oxirini pastga yaxlitlaymiz. */
static void mark_region_free(uint64_t base, uint64_t len)
{
    uint64_t start = ALIGN_UP(base, PAGE_SIZE);
    uint64_t end = ALIGN_DOWN(base + len, PAGE_SIZE);
    if (end > PMM_MAX_PHYS)
        end = PMM_MAX_PHYS;
    for (uint64_t addr = start; addr < end; addr += PAGE_SIZE) {
        size_t frame = addr / PAGE_SIZE;
        if (bit_test(frame)) {          /* bir hudud ikki marta sanalmasin */
            bit_clear(frame);
            total_frames++;
        }
        if (frame + 1 > highest_frame)
            highest_frame = frame + 1;
    }
}

/* [base, base+len) ga TEGADIGAN barcha freymlarni BAND qilish.
 * Bu yerda aksincha: qisman band freym ham to'liq band hisoblanadi. */
static void mark_region_used(uint64_t base, uint64_t len)
{
    uint64_t start = ALIGN_DOWN(base, PAGE_SIZE);
    uint64_t end = ALIGN_UP(base + len, PAGE_SIZE);
    if (end > PMM_MAX_PHYS)
        end = PMM_MAX_PHYS;
    for (uint64_t addr = start; addr < end; addr += PAGE_SIZE) {
        size_t frame = addr / PAGE_SIZE;
        if (!bit_test(frame)) {
            bit_set(frame);
            if (frame < highest_frame)
                used_frames++;          /* faqat RAM freymlarini sanaymiz */
        }
    }
}

static const char *mmap_type_name(uint32_t type)
{
    switch (type) {
    case 1: return "bo'sh RAM";
    case 2: return "band (reserved)";
    case 3: return "ACPI (qayta ishlatiladi)";
    case 4: return "ACPI NVS";
    case 5: return "buzilgan RAM";
    default: return "noma'lum";
    }
}

void pmm_init(const struct multiboot_info *mbi)
{
    if (!(mbi->flags & MB_INFO_MMAP))
        panic("Yuklovchi xotira xaritasini bermadi");

    /* 1. Hammasi band. */
    memset(bitmap, 0xFF, sizeof(bitmap));

    /* 2. Xotira xaritasini o'qib, bo'sh RAM ni belgilaymiz. */
    kprintf("[pmm] BIOS xotira xaritasi:\n");
    uint64_t addr = mbi->mmap_addr;
    uint64_t end = addr + mbi->mmap_length;
    while (addr < end) {
        const struct multiboot_mmap_entry *e = (const void *)(uintptr_t)addr;
        kprintf("      %016lx - %016lx  %s\n", e->addr, e->addr + e->len - 1,
                mmap_type_name(e->type));
        if (e->type == MULTIBOOT_MEMORY_AVAILABLE)
            mark_region_free(e->addr, e->len);
        addr += e->size + sizeof(e->size);  /* "size" o'zini hisobga olmaydi! */
    }

    /* 3. Haqiqatan band hududlar. used_frames shu yerda sanala boshlaydi. */
    used_frames = 0;
    mark_region_used(0, 1 * MiB);                           /* BIOS, VGA, 0-freym */
    mark_region_used((uint64_t)kernel_start,
                     (uint64_t)(kernel_end - kernel_start)); /* yadro + bitmap + steklar */
    mark_region_used((uint64_t)(uintptr_t)mbi, sizeof(*mbi));
    mark_region_used(mbi->mmap_addr, mbi->mmap_length);
    if (mbi->flags & MB_INFO_CMDLINE)
        mark_region_used(mbi->cmdline, strlen((const char *)(uintptr_t)mbi->cmdline) + 1);
    if (mbi->flags & MB_INFO_MODS) {
        const struct multiboot_module *mods = (const void *)(uintptr_t)mbi->mods_addr;
        mark_region_used(mbi->mods_addr, mbi->mods_count * sizeof(*mods));
        for (uint32_t i = 0; i < mbi->mods_count; i++) {
            mark_region_used(mods[i].mod_start, mods[i].mod_end - mods[i].mod_start);
            if (mods[i].cmdline)
                mark_region_used(mods[i].cmdline,
                                 strlen((const char *)(uintptr_t)mods[i].cmdline) + 1);
        }
    }

    kprintf("[pmm] Yadro: %p - %p (%lu KB)\n", (void *)kernel_start, (void *)kernel_end,
            (uint64_t)(kernel_end - kernel_start) / KiB);
    kprintf("[pmm] RAM: %zu freym (%zu MB), bo'sh: %zu freym (%zu MB)\n",
            total_frames, total_frames * PAGE_SIZE / MiB,
            pmm_free_frames_count(), pmm_free_frames_count() * PAGE_SIZE / MiB);
}

uint64_t pmm_alloc_frame(void)
{
    uint64_t flags = irq_save();        /* bitmap umumiy resurs - uzilishdan himoya */
    size_t words = (highest_frame + 63) / 64;

    for (size_t n = 0; n < words; n++) {
        size_t w = (search_hint + n) % words;   /* next-fit: hint dan boshlab aylanib */
        if (bitmap[w] == ~0ULL)
            continue;                   /* 64 ta freymning hammasi band - o'tkazib yuboramiz */
        /* __builtin_ctzll(x) - "count trailing zeros": eng past 1-bit o'rni.
         * ~bitmap[w] da 1-bit = bo'sh freym. Bu bitta CPU instruksiyasi (tzcnt/bsf). */
        int bit = __builtin_ctzll(~bitmap[w]);
        size_t frame = w * 64 + bit;
        if (frame >= highest_frame)
            continue;
        bit_set(frame);
        used_frames++;
        search_hint = w;
        irq_restore(flags);
        return (uint64_t)frame * PAGE_SIZE;
    }
    irq_restore(flags);
    return 0;                           /* xotira tugadi (Out Of Memory) */
}

uint64_t pmm_alloc_frames(size_t count)
{
    if (count == 0)
        return 0;
    if (count == 1)
        return pmm_alloc_frame();

    uint64_t flags = irq_save();
    size_t run = 0;                     /* hozirgi ketma-ket bo'sh freymlar uzunligi */
    for (size_t frame = 0; frame < highest_frame; frame++) {
        if (bit_test(frame)) {
            run = 0;                    /* zanjir uzildi */
            continue;
        }
        if (++run == count) {
            size_t first = frame + 1 - count;
            for (size_t f = first; f <= frame; f++)
                bit_set(f);
            used_frames += count;
            irq_restore(flags);
            return (uint64_t)first * PAGE_SIZE;
        }
    }
    irq_restore(flags);
    return 0;
}

void pmm_free_frame(uint64_t phys)
{
    /* Himoya tekshiruvlari. Xotira xatolari eng xavfli xatolar - ular
     * ko'pincha boshqa joyda, ancha keyin, tushunarsiz ko'rinishda chiqadi.
     * Shuning uchun ularni IMKON QADAR ERTA ushlaymiz. */
    if (!IS_ALIGNED(phys, PAGE_SIZE))
        panic("pmm_free_frame: tekislanmagan manzil %p", (void *)phys);
    size_t frame = phys / PAGE_SIZE;
    if (frame >= highest_frame || phys < 1 * MiB)
        panic("pmm_free_frame: boshqarilmaydigan manzil %p", (void *)phys);

    uint64_t flags = irq_save();
    if (!bit_test(frame))
        panic("pmm_free_frame: DOUBLE FREE! %p allaqachon bo'sh", (void *)phys);
    bit_clear(frame);
    used_frames--;
    irq_restore(flags);
}

void pmm_free_frames(uint64_t phys, size_t count)
{
    for (size_t i = 0; i < count; i++)
        pmm_free_frame(phys + i * PAGE_SIZE);
}

size_t pmm_total_frames(void)
{
    return total_frames;
}

size_t pmm_free_frames_count(void)
{
    return total_frames - used_frames;
}
