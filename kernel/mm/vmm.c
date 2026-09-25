/* =============================================================================
 *  mm/vmm.c - VIRTUAL XOTIRA MENEJERI
 * =============================================================================
 *
 *  Bu fayl ikki narsani qiladi:
 *
 *  1) YADRONING DOIMIY XARITASINI QURISH (vmm_init) - layout.h ga qarang:
 *       * direct map (HHDM): barcha RAM -> 0xFFFF800000000000 + phys
 *         Eng katta mumkin bo'lgan sahifalar bilan (1 GB > 2 MB > 4 KB):
 *         TLB'da kamroq joy egallaydi va jadvallar uchun kam xotira ketadi.
 *       * yadro tasviri -> 0xFFFFFFFF80000000 + phys, 4 KB aniqlikda va W^X:
 *             .text   : o'qish + bajarish   (yozib bo'lmaydi)
 *             .rodata : faqat o'qish         (NX)
 *             .data/.bss : o'qish + yozish   (NX)
 *         Xatolik tufayli kod o'z-o'zini buzib qo'ya olmaydi, ma'lumot esa kod
 *         sifatida bajarila olmaydi. Linux, Windows va macOS shunday qiladi.
 *       * boot stekining himoya sahifasi xaritalanMAYDI.
 *       * identity xarita (pastki yarim) OLIB TASHLANADI - pastki yarim
 *         faqat user dasturlariga tegishli.
 *
 *  2) JARAYON MANZIL MAYDONLARI:
 *       Har bir jarayon PML4 ining yuqori yarmi (256..511) yadro PML4 idan
 *       NUSXALANADI. Yadro PDPT larining hammasini boshida yaratib qo'yamiz
 *       (256 ta x 4 KB = 1 MB), shuning uchun yadro xaritasi keyin o'zgarsa
 *       ham (vmalloc) PML4 yozuvlari o'zgarmaydi - barcha jarayonlar
 *       o'zgarishni avtomatik ko'radi. (Linux x86-64 ham aynan shunday.)
 * ============================================================================= */
#include "mm/vmm.h"

#include "arch/cpu.h"
#include "arch/smp.h"
#include "boot/bootinfo.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/string.h"
#include "mm/memblock.h"
#include "mm/page.h"
#include "mm/pmm.h"

#define PML4_INDEX(v) (((v) >> 39) & 0x1FF)
#define PDPT_INDEX(v) (((v) >> 30) & 0x1FF)
#define PD_INDEX(v)   (((v) >> 21) & 0x1FF)
#define PT_INDEX(v)   (((v) >> 12) & 0x1FF)

uint64_t pte_nx;
static uint64_t kernel_pml4;
static bool use_buddy;                  /* false: memblock, true: buddy */

extern char __text_start[], __text_end[];
extern char __rodata_start[], __rodata_end[];
extern char __data_start[], __kernel_end[];
extern char boot_stack_guard[];

static inline uint64_t *table_virt(uint64_t phys)
{
    return phys_to_virt(phys & PTE_ADDR_MASK);
}

/* Sahifa jadvali uchun nollangan freym. Boot paytida - memblock'dan (4 GB dan
 * past: boot direct map faqat shuni qamrab oladi), keyin - buddy'dan. */
static uint64_t alloc_table(void)
{
    if (!use_buddy)
        return memblock_alloc(PAGE_SIZE, PAGE_SIZE, 0x100000000UL);
    return pmm_alloc_page(GFP_ZERO);
}

void vmm_flush_page(uint64_t virt)
{
    if (is_user_address(virt))
        cpu_invlpg(virt);               /* user sahifalar faqat shu CPU'da (bitta oqimli jarayon) */
    else
        tlb_shootdown(virt, 1);         /* yadro sahifalari - barcha CPU'larda umumiy */
}

bool vmm_phys_is_direct_mapped(uint64_t phys, uint64_t len)
{
    uint64_t end = phys + len;
    if (end <= 1 * MiB)
        return true;
    for (size_t i = 0; i < boot_info.mmap_count; i++) {
        const struct mem_region *r = &boot_info.mmap[i];
        if (r->type != MEM_USABLE && r->type != MEM_ACPI_RECLAIM && r->type != MEM_ACPI_NVS)
            continue;
        if (phys >= r->base && end <= r->base + r->len)
            return true;
    }
    return false;
}

/* ---- Jadvallar bo'ylab yurish ---- */

static uint64_t *next_level(uint64_t *table, size_t idx, bool create, bool user)
{
    uint64_t e = table[idx];
    if (e & PTE_PRESENT) {
        if (e & PTE_HUGE)
            return NULL;
        return table_virt(e);
    }
    if (!create)
        return NULL;
    uint64_t t = alloc_table();
    if (!t)
        return NULL;
    /* Oraliq jadvallarda keng ruxsat (W, U). Haqiqiy cheklov - oxirgi darajada.
     * Samarali ruxsat = barcha darajalarning "VA"si, NX esa "YOKI"si. */
    table[idx] = t | PTE_PRESENT | PTE_WRITABLE | (user ? PTE_USER : 0);
    return table_virt(t);
}

uint64_t *vmm_get_pte(uint64_t pml4, uint64_t virt, bool create)
{
    bool user = is_user_address(virt);
    uint64_t *t = table_virt(pml4);
    if (!(t = next_level(t, PML4_INDEX(virt), create, user)))
        return NULL;
    if (!(t = next_level(t, PDPT_INDEX(virt), create, user)))
        return NULL;
    if (!(t = next_level(t, PD_INDEX(virt), create, user)))
        return NULL;
    return &t[PT_INDEX(virt)];
}

/* ---- Yadro xaritasini qurish ---- */

/* [virt, virt+size) -> [phys, ...) ni eng katta mos sahifalar bilan xaritalash. */
static void map_range(uint64_t pml4, uint64_t virt, uint64_t phys, uint64_t size, uint64_t flags)
{
    uint64_t end = virt + size;
    while (virt < end) {
        uint64_t left = end - virt;
        uint64_t *pml4_t = table_virt(pml4);
        uint64_t *pdpt = next_level(pml4_t, PML4_INDEX(virt), true, false);
        if (!pdpt)
            panic("map_range: xotira yetmadi");

        /* 1 GB sahifa: CPU qo'llasa, ikkala manzil ham 1 GB ga tekis va joy yetarli. */
        if (cpu_features.page1gb && !(virt & (GiB - 1)) && !(phys & (GiB - 1)) && left >= GiB &&
            !(pdpt[PDPT_INDEX(virt)] & PTE_PRESENT)) {
            pdpt[PDPT_INDEX(virt)] = phys | flags | PTE_PRESENT | PTE_HUGE;
            virt += GiB, phys += GiB;
            continue;
        }
        uint64_t *pd = next_level(pdpt, PDPT_INDEX(virt), true, false);
        if (!pd)
            panic("map_range: xotira yetmadi");
        if (!(virt & (2 * MiB - 1)) && !(phys & (2 * MiB - 1)) && left >= 2 * MiB &&
            !(pd[PD_INDEX(virt)] & PTE_PRESENT)) {
            pd[PD_INDEX(virt)] = phys | flags | PTE_PRESENT | PTE_HUGE;
            virt += 2 * MiB, phys += 2 * MiB;
            continue;
        }
        uint64_t *pt = next_level(pd, PD_INDEX(virt), true, false);
        if (!pt)
            panic("map_range: xotira yetmadi");
        pt[PT_INDEX(virt)] = phys | flags | PTE_PRESENT;
        virt += PAGE_SIZE, phys += PAGE_SIZE;
    }
}

/* Yadro tasvirining bir bo'limini (virtual manzillari bilan) xaritalash. */
static void map_kernel_section(const char *start, const char *end, uint64_t flags)
{
    for (uint64_t v = (uint64_t)start; v < ALIGN_UP((uint64_t)end, PAGE_SIZE); v += PAGE_SIZE) {
        if (v == (uint64_t)boot_stack_guard)
            continue;                   /* himoya sahifasi - atayin bo'sh */
        uint64_t *pte = vmm_get_pte(kernel_pml4, v, true);
        if (!pte)
            panic("yadro xaritasi: xotira yetmadi");
        *pte = (v - KERNEL_VMA) | flags | PTE_PRESENT;
    }
}

void vmm_init(void)
{
    pte_nx = cpu_features.nx ? PTE_NX : 0;
    uint64_t global = cpu_features.pge ? PTE_GLOBAL : 0;

    kernel_pml4 = alloc_table();
    uint64_t *pml4 = table_virt(kernel_pml4);
    /* Yuqori yarimning barcha 256 ta PDPT sini oldindan yaratamiz. */
    for (int i = 256; i < 512; i++)
        pml4[i] = alloc_table() | PTE_PRESENT | PTE_WRITABLE;

    /* 1. Direct map: faqat RAM va ACPI hududlari (+ birinchi 1 MB). Qurilma
     * xotirasi (MMIO) BU YERDA YO'Q: uni WB keshi bilan xaritalash haqiqiy
     * apparatda xavfli (CPU spekulyativ o'qishi qurilmani buzishi mumkin).
     * Qurilmalar ioremap() orqali, to'g'ri kesh turi bilan xaritalanadi. */
    uint64_t hhdm_flags = PTE_WRITABLE | pte_nx | global;
    map_range(kernel_pml4, HHDM_BASE, 0, 1 * MiB, hhdm_flags);
    uint64_t mapped = 0;
    for (size_t i = 0; i < boot_info.mmap_count; i++) {
        const struct mem_region *r = &boot_info.mmap[i];
        if (r->type != MEM_USABLE && r->type != MEM_ACPI_RECLAIM && r->type != MEM_ACPI_NVS)
            continue;
        uint64_t base = ALIGN_DOWN(r->base, PAGE_SIZE);
        uint64_t end = ALIGN_UP(r->base + r->len, PAGE_SIZE);
        if (end <= 1 * MiB)
            continue;
        if (base < 1 * MiB)
            base = 1 * MiB;
        map_range(kernel_pml4, HHDM_BASE + base, base, end - base, hhdm_flags);
        mapped += end - base;
    }

    /* 2. Yadro tasviri - W^X. */
    map_kernel_section(__text_start, __text_end, global);                         /* R-X */
    map_kernel_section(__rodata_start, __rodata_end, pte_nx | global);             /* R-- */
    map_kernel_section(__data_start, __kernel_end, PTE_WRITABLE | pte_nx | global); /* RW- */

    /* 3. O'tish. Shu lahzadan identity xarita va boot jadvallari yo'q. */
    cpu_write_cr3(kernel_pml4);

    kprintf("[vmm]  Yadro xaritasi: direct map %lu MB (%s sahifalar), W^X, NX:%s, himoya sahifasi\n",
            mapped / MiB, cpu_features.page1gb ? "1 GB/2 MB/4 KB" : "2 MB/4 KB",
            pte_nx ? "ha" : "yo'q");
}

void vmm_late_init(void)
{
    use_buddy = true;
}

uint64_t vmm_kernel_pml4(void)
{
    return kernel_pml4;
}

void vmm_switch(uint64_t pml4)
{
    if (cpu_read_cr3() != pml4)
        cpu_write_cr3(pml4);
}

/* ---- Manzil maydonlari ---- */

uint64_t vmm_create_address_space(void)
{
    uint64_t pml4 = pmm_alloc_page(GFP_ZERO);
    if (!pml4)
        return 0;
    /* Yuqori yarim - yadroniki (umumiy PDPT lar), pastki yarim - bo'sh. */
    memcpy(table_virt(pml4) + 256, table_virt(kernel_pml4) + 256, 256 * sizeof(uint64_t));
    return pml4;
}

/* Bitta user sahifasini "tashlab yuborish": COW tufayli sahifa bir nechta
 * jarayonda bo'lishi mumkin - shuning uchun to'g'ridan-to'g'ri free emas, put_page. */
static void release_user_frame(uint64_t pte)
{
    uint64_t phys = pte & PTE_ADDR_MASK;
    if (phys >> PAGE_SHIFT >= max_pfn)
        return;                         /* RAM emas (masalan, framebuffer xaritasi) */
    struct page *p = phys_to_page(phys);
    if (p->flags & PG_RESERVED)
        return;
    if (p->mapcount)
        p->mapcount--;
    put_page(p);
}

void vmm_destroy_address_space(uint64_t pml4)
{
    ASSERT(pml4 != kernel_pml4);
    ASSERT(pml4 != cpu_read_cr3());

    uint64_t *l4 = table_virt(pml4);
    for (int i = 0; i < 256; i++) {                     /* FAQAT pastki yarim */
        if (!(l4[i] & PTE_PRESENT))
            continue;
        uint64_t *l3 = table_virt(l4[i]);
        for (int j = 0; j < 512; j++) {
            if (!(l3[j] & PTE_PRESENT))
                continue;
            uint64_t *l2 = table_virt(l3[j]);
            for (int k = 0; k < 512; k++) {
                if (!(l2[k] & PTE_PRESENT))
                    continue;
                uint64_t *l1 = table_virt(l2[k]);
                for (int m = 0; m < 512; m++)
                    if (l1[m] & PTE_PRESENT)
                        release_user_frame(l1[m]);
                pmm_free_page(l2[k] & PTE_ADDR_MASK);
            }
            pmm_free_page(l3[j] & PTE_ADDR_MASK);
        }
        pmm_free_page(l4[i] & PTE_ADDR_MASK);
    }
    pmm_free_page(pml4);
}

/* ---- Xaritalash ---- */

bool vmm_map_page(uint64_t pml4, uint64_t virt, uint64_t phys, uint64_t flags)
{
    ASSERT(IS_ALIGNED(virt, PAGE_SIZE) && IS_ALIGNED(phys, PAGE_SIZE));
    uint64_t *pte = vmm_get_pte(pml4, virt, true);
    if (!pte)
        return false;
    if (*pte & PTE_PRESENT)
        panic("vmm_map_page: %p allaqachon xaritalangan", (void *)virt);
    *pte = phys | (flags & ~PTE_ADDR_MASK) | PTE_PRESENT;
    /* Yangi xaritalashda TLB ni tozalash shart emas: CPU "yo'q" yozuvlarni
     * keshlamaydi. Lekin eski yozuvni O'ZGARTIRISH yoki O'CHIRISH - shart. */
    return true;
}

uint64_t vmm_unmap_page(uint64_t pml4, uint64_t virt)
{
    uint64_t *pte = vmm_get_pte(pml4, virt, false);
    if (!pte || !(*pte & PTE_PRESENT))
        return 0;
    uint64_t phys = *pte & PTE_ADDR_MASK;
    *pte = 0;
    if (pml4 == cpu_read_cr3() || !is_user_address(virt))
        vmm_flush_page(virt);           /* yadro sahifasi - barcha maydonlarda ko'rinadi */
    return phys;
}

uint64_t vmm_translate(uint64_t pml4, uint64_t virt, uint64_t *flags_out)
{
    uint64_t *t = table_virt(pml4);
    uint64_t e = t[PML4_INDEX(virt)];
    if (!(e & PTE_PRESENT))
        return 0;
    e = table_virt(e)[PDPT_INDEX(virt)];
    if (!(e & PTE_PRESENT))
        return 0;
    if (e & PTE_HUGE) {
        if (flags_out)
            *flags_out = e & ~PTE_ADDR_MASK;
        return (e & PTE_ADDR_MASK & ~(GiB - 1)) + (virt & (GiB - 1));
    }
    e = table_virt(e)[PD_INDEX(virt)];
    if (!(e & PTE_PRESENT))
        return 0;
    if (e & PTE_HUGE) {
        if (flags_out)
            *flags_out = e & ~PTE_ADDR_MASK;
        return (e & PTE_ADDR_MASK & ~(2 * MiB - 1)) + (virt & (2 * MiB - 1));
    }
    e = table_virt(e)[PT_INDEX(virt)];
    if (!(e & PTE_PRESENT))
        return 0;
    if (flags_out)
        *flags_out = e & ~PTE_ADDR_MASK;
    return (e & PTE_ADDR_MASK) + (virt & (PAGE_SIZE - 1));
}

bool vmm_update_flags(uint64_t pml4, uint64_t virt, uint64_t flags)
{
    uint64_t *pte = vmm_get_pte(pml4, virt, false);
    if (!pte || !(*pte & PTE_PRESENT))
        return false;
    *pte = (*pte & PTE_ADDR_MASK) | (flags & ~PTE_ADDR_MASK) | PTE_PRESENT;
    if (pml4 == cpu_read_cr3() || !is_user_address(virt))
        vmm_flush_page(virt);
    return true;
}

bool vmm_map_anonymous(uint64_t pml4, uint64_t virt, size_t pages, uint64_t flags)
{
    for (size_t i = 0; i < pages; i++) {
        /* NOLLASH SHART: aks holda jarayon boshqa jarayonning eski ma'lumotini
         * (parollar, kalitlar) o'qiydi. */
        struct page *p = alloc_pages(0, GFP_ZERO);
        if (!p)
            return false;
        if (!vmm_map_page(pml4, virt + i * PAGE_SIZE, page_to_phys(p), flags)) {
            put_page(p);
            return false;
        }
        p->mapcount = 1;
    }
    return true;
}

bool vmm_copy_to_space(uint64_t pml4, uint64_t virt, const void *src, size_t len)
{
    const uint8_t *s = src;
    while (len) {
        uint64_t phys = vmm_translate(pml4, virt, NULL);
        if (!phys)
            return false;
        size_t chunk = MIN(len, PAGE_SIZE - (virt & (PAGE_SIZE - 1)));
        memcpy(phys_to_virt(phys), s, chunk);
        s += chunk;
        virt += chunk;
        len -= chunk;
    }
    return true;
}

bool vmm_user_range_ok(uint64_t pml4, uint64_t virt, size_t len, bool write)
{
    if (len == 0)
        return true;
    uint64_t end = virt + len;
    if (end < virt || !is_user_address(virt) || end > USER_SPACE_END)
        return false;
    for (uint64_t page = ALIGN_DOWN(virt, PAGE_SIZE); page < end; page += PAGE_SIZE) {
        uint64_t flags;
        if (!vmm_translate(pml4, page, &flags))
            return false;
        if (!(flags & PTE_USER) || (write && !(flags & PTE_WRITABLE)))
            return false;
    }
    return true;
}

uint64_t vmm_count_user_pages(uint64_t pml4)
{
    uint64_t count = 0;
    uint64_t *l4 = table_virt(pml4);
    for (int i = 0; i < 256; i++) {
        if (!(l4[i] & PTE_PRESENT))
            continue;
        uint64_t *l3 = table_virt(l4[i]);
        for (int j = 0; j < 512; j++) {
            if (!(l3[j] & PTE_PRESENT))
                continue;
            uint64_t *l2 = table_virt(l3[j]);
            for (int k = 0; k < 512; k++) {
                if (!(l2[k] & PTE_PRESENT))
                    continue;
                uint64_t *l1 = table_virt(l2[k]);
                for (int m = 0; m < 512; m++)
                    if (l1[m] & PTE_PRESENT)
                        count++;
            }
        }
    }
    return count;
}
