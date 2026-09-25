/* =============================================================================
 *  mm/vmm.c - virtual xotira: 4 darajali sahifa jadvallari
 * =============================================================================
 *
 *  VIRTUAL XOTIRA NIMA BERADI:
 *    1) IZOLYATSIYA: har bir jarayonning o'z sahifa jadvali (o'z CR3 qiymati)
 *       bor. Jarayon A ning 0x40000000 manzili va jarayon B ning 0x40000000
 *       manzili - butunlay boshqa fizik freymlar. A B ning xotirasini hatto
 *       "ko'ra" olmaydi.
 *    2) HIMOYA: har bir sahifada bayroqlar bor. U=0 bo'lsa, user kodi unga
 *       tegsa - page fault. Yadro xotirasi shunday himoyalanadi.
 *    3) QULAYLIK: har bir dastur bir xil manzilga (0x40000000) link qilinadi,
 *       fizik xotira qayerda bo'sh ekanidan qat'i nazar.
 *
 *  TARJIMA (virtual -> fizik), 4 KB sahifa uchun:
 *
 *     virt: | 47..39 | 38..30 | 29..21 | 20..12 | 11..0  |
 *           | PML4 i | PDPT i |  PD i  |  PT i  | siljish|
 *
 *     CR3 -> PML4[i] -> PDPT[i] -> PD[i] -> PT[i] -> freym + siljish
 *
 *  Har bir jadval = bitta 4 KB freym = 512 ta 8 baytlik yozuv.
 *
 *  BIZNING DIZAYN (sodda va xavfsiz):
 *    * Yadro fizik xotiraning birinchi 1 GB ini identity-map qilgan (boot.asm,
 *      2 MB lik sahifalar bilan, bitta PD jadvali: boot_pd). Shu tufayli yadro
 *      istalgan freymga (phys) shunchaki (void *)phys orqali murojaat qila oladi.
 *    * Har bir jarayonning PDPT[0] yozuvi AYNAN SHU boot_pd ga ko'rsatadi -
 *      demak yadro har bir manzil maydonida bir xil ko'rinadi va CR3 ni
 *      almashtirganda yadro kodi "yo'qolib qolmaydi". boot_pd yozuvlarida U=0,
 *      shuning uchun user kodi yadroga tega olmaydi.
 *    * User sahifalari PDPT[1..511] ostida - har bir jarayonga xos.
 *
 *  Linux va boshqa jiddiy yadrolar "higher-half" dizayn ishlatadi: yadro
 *  0xFFFF800000000000 dan yuqorida turadi va user uchun butun pastki yarim
 *  (128 TB) qoladi. Bu kelajakdagi mashq (docs/mashqlar.md).
 * ============================================================================= */
#include "mm/vmm.h"

#include "arch/cpu.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/string.h"
#include "mm/pmm.h"

/* Virtual manzildan har bir daraja indeksini ajratib olish (9 bitdan). */
#define PML4_INDEX(v) (((v) >> 39) & 0x1FF)
#define PDPT_INDEX(v) (((v) >> 30) & 0x1FF)
#define PD_INDEX(v)   (((v) >> 21) & 0x1FF)
#define PT_INDEX(v)   (((v) >> 12) & 0x1FF)

/* boot.asm dagi jadvallar. .bss da, demak virtual = fizik. */
extern uint64_t boot_pml4[512];
extern uint64_t boot_pd[512];
extern char boot_stack_guard[];

/* Birinchi 2 MB uchun 4 KB lik sahifalar jadvali (NULL himoyasi uchun). */
static uint64_t low_pt[512] __attribute__((aligned(4096)));

/* Fizik manzil -> yadro ko'rsatkichi. Identity mapping tufayli bu shunchaki
 * turni o'zgartirish. Higher-half yadroda bu "phys + 0xFFFF800000000000"
 * bo'lardi - shuning uchun uni alohida funksiyaga o'raymiz: dizayn o'zgarsa,
 * faqat shu yerni o'zgartiramiz. */
static inline uint64_t *phys_to_virt(uint64_t phys)
{
    return (uint64_t *)(uintptr_t)phys;
}

void vmm_init(void)
{
    /* NULL ko'rsatkich himoyasi.
     * boot.asm birinchi 2 MB ni bitta katta sahifa bilan xaritalagan - demak
     * 0-manzil ham "mavjud" va *(int*)0 = 5 xato bermaydi, jimgina BIOS
     * ma'lumotlarini buzadi. Shuni tuzatamiz: birinchi 2 MB ni 512 ta 4 KB lik
     * sahifaga bo'lamiz va 0-sahifani XARITALAMAYMIZ. Endi NULL ga murojaat
     * darhol page fault beradi - xatoni birinchi qadamdayoq ushlaymiz. */
    for (int i = 1; i < 512; i++)
        low_pt[i] = (uint64_t)i * PAGE_SIZE | PTE_PRESENT | PTE_WRITABLE;
    low_pt[0] = 0;                      /* 0x0000..0x0FFF - mavjud emas */

    /* Yadro stekining himoya sahifasi (boot.asm dagi boot_stack_guard). Yadro
     * birinchi 2 MB ichida, shuning uchun uni ham shu jadvalda o'chira olamiz.
     * Stek to'lsa: #PF -> CPU xato freymini shu to'lgan stekka yoza olmaydi ->
     * #DF (double fault) -> IST1 dagi toza stekda handler ishlaydi. */
    uint64_t guard = (uint64_t)boot_stack_guard;
    ASSERT(guard < 2 * MiB && IS_ALIGNED(guard, PAGE_SIZE));
    low_pt[guard / PAGE_SIZE] = 0;
    boot_pd[0] = (uint64_t)low_pt | PTE_PRESENT | PTE_WRITABLE;   /* huge bayrog'i yo'q -> PT */

    /* CR3 ni qayta yuklash butun TLB ni tozalaydi (global bo'lmagan sahifalar uchun). */
    cpu_write_cr3(cpu_read_cr3());

    kprintf("[vmm] Paging: 0..1 GB identity (yadro), 0-sahifa himoyalangan (NULL -> #PF)\n");
    kprintf("[vmm] User hududi: %p - %p\n", (void *)USER_SPACE_START, (void *)USER_SPACE_END);
}

uint64_t vmm_kernel_pml4(void)
{
    return (uint64_t)boot_pml4;
}

void vmm_switch(uint64_t pml4)
{
    if (cpu_read_cr3() != pml4)         /* bir xil bo'lsa, TLB ni behuda tozalamaymiz */
        cpu_write_cr3(pml4);
}

/* Nollangan freym ajratish (sahifa jadvallari uchun: bo'sh yozuvlar = 0). */
static uint64_t alloc_zeroed_frame(void)
{
    uint64_t phys = pmm_alloc_frame();
    if (phys)
        memset(phys_to_virt(phys), 0, PAGE_SIZE);
    return phys;
}

uint64_t vmm_create_address_space(void)
{
    uint64_t pml4 = alloc_zeroed_frame();
    uint64_t pdpt = alloc_zeroed_frame();
    if (!pml4 || !pdpt) {
        if (pml4)
            pmm_free_frame(pml4);
        if (pdpt)
            pmm_free_frame(pdpt);
        return 0;
    }
    uint64_t *pml4_v = phys_to_virt(pml4);
    uint64_t *pdpt_v = phys_to_virt(pdpt);

    /* PDPT[0] -> yadroning umumiy PD si (0..1 GB). U bayrog'i YO'Q. */
    pdpt_v[0] = (uint64_t)boot_pd | PTE_PRESENT | PTE_WRITABLE;
    /* PML4[0] -> jarayonning o'z PDPT si. U=1, chunki uning ostida user sahifalari
     * bo'ladi. Samarali ruxsat = barcha darajalardagi bayroqlarning "VA"si: yadro
     * PD sida U=0 bo'lgani uchun yadro baribir himoyalangan. */
    pml4_v[0] = pdpt | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    return pml4;
}

void vmm_destroy_address_space(uint64_t pml4)
{
    ASSERT(pml4 != vmm_kernel_pml4());
    ASSERT(pml4 != cpu_read_cr3());     /* o'zimiz o'tirgan shoxni kesmaymiz */

    uint64_t *pml4_v = phys_to_virt(pml4);
    uint64_t pdpt = pml4_v[0] & PTE_ADDR_MASK;
    uint64_t *pdpt_v = phys_to_virt(pdpt);

    /* PDPT[0] - yadroning umumiy PD si. UNGA TEGMAYMIZ! 1 dan boshlaymiz. */
    for (int i = 1; i < 512; i++) {
        if (!(pdpt_v[i] & PTE_PRESENT))
            continue;
        uint64_t *pd_v = phys_to_virt(pdpt_v[i] & PTE_ADDR_MASK);
        for (int j = 0; j < 512; j++) {
            if (!(pd_v[j] & PTE_PRESENT))
                continue;
            uint64_t *pt_v = phys_to_virt(pd_v[j] & PTE_ADDR_MASK);
            for (int k = 0; k < 512; k++) {
                if (pt_v[k] & PTE_PRESENT)
                    pmm_free_frame(pt_v[k] & PTE_ADDR_MASK);  /* user ma'lumot freymi */
            }
            pmm_free_frame(pd_v[j] & PTE_ADDR_MASK);         /* PT jadvali */
        }
        pmm_free_frame(pdpt_v[i] & PTE_ADDR_MASK);           /* PD jadvali */
    }
    pmm_free_frame(pdpt);
    pmm_free_frame(pml4);
}

/* table[index] mavjud bo'lsa - keyingi darajadagi jadvalni qaytaradi.
 * Mavjud bo'lmasa va create=true - yangi nollangan jadval yaratadi. */
static uint64_t *next_table(uint64_t *table, size_t index, bool create)
{
    if (table[index] & PTE_PRESENT) {
        if (table[index] & PTE_HUGE)
            return NULL;                /* katta sahifa - ostida jadval yo'q */
        return phys_to_virt(table[index] & PTE_ADDR_MASK);
    }
    if (!create)
        return NULL;
    uint64_t frame = alloc_zeroed_frame();
    if (!frame)
        return NULL;
    /* Oraliq jadvallarga eng keng ruxsat beramiz (W + U). Haqiqiy cheklov oxirgi
     * darajada (PT yozuvida) qo'yiladi. */
    table[index] = frame | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    return phys_to_virt(frame);
}

/* virt uchun PT yozuvining manzilini topish (kerak bo'lsa jadvallarni yaratib). */
static uint64_t *walk(uint64_t pml4, uint64_t virt, bool create)
{
    uint64_t *pml4_v = phys_to_virt(pml4);
    uint64_t *pdpt_v = next_table(pml4_v, PML4_INDEX(virt), create);
    if (!pdpt_v)
        return NULL;
    uint64_t *pd_v = next_table(pdpt_v, PDPT_INDEX(virt), create);
    if (!pd_v)
        return NULL;
    uint64_t *pt_v = next_table(pd_v, PD_INDEX(virt), create);
    if (!pt_v)
        return NULL;
    return &pt_v[PT_INDEX(virt)];
}

static bool is_user_address(uint64_t virt)
{
    return virt >= USER_SPACE_START && virt < USER_SPACE_END;
}

bool vmm_map_page(uint64_t pml4, uint64_t virt, uint64_t phys, uint64_t flags)
{
    ASSERT(IS_ALIGNED(virt, PAGE_SIZE) && IS_ALIGNED(phys, PAGE_SIZE));
    /* Yadro hududini (umumiy boot_pd) o'zgartirishga yo'l qo'ymaymiz - u
     * BARCHA jarayonlarga ta'sir qilardi. */
    if (!is_user_address(virt))
        panic("vmm_map_page: user hududidan tashqari manzil %p", (void *)virt);

    uint64_t *pte = walk(pml4, virt, true);
    if (!pte)
        return false;                   /* jadval uchun xotira yetmadi */
    if (*pte & PTE_PRESENT)
        panic("vmm_map_page: %p allaqachon xaritalangan", (void *)virt);
    *pte = phys | (flags & ~PTE_ADDR_MASK) | PTE_PRESENT;
    if (pml4 == cpu_read_cr3())
        cpu_invlpg(virt);               /* faol maydonda TLB da eski yozuv qolmasin */
    return true;
}

uint64_t vmm_unmap_page(uint64_t pml4, uint64_t virt)
{
    uint64_t *pte = walk(pml4, virt, false);
    if (!pte || !(*pte & PTE_PRESENT))
        return 0;
    uint64_t phys = *pte & PTE_ADDR_MASK;
    *pte = 0;
    if (pml4 == cpu_read_cr3())
        cpu_invlpg(virt);
    /* Eslatma: bo'shab qolgan PT/PD jadvallari bu yerda qaytarilmaydi - ular
     * vmm_destroy_address_space() da tozalanadi. (Mashq: ularni shu yerda
     * qaytaring.) */
    return phys;
}

uint64_t vmm_translate(uint64_t pml4, uint64_t virt, uint64_t *flags_out)
{
    uint64_t *table = phys_to_virt(pml4);
    uint64_t entry = table[PML4_INDEX(virt)];
    if (!(entry & PTE_PRESENT))
        return 0;

    table = phys_to_virt(entry & PTE_ADDR_MASK);
    entry = table[PDPT_INDEX(virt)];
    if (!(entry & PTE_PRESENT))
        return 0;
    if (entry & PTE_HUGE) {             /* 1 GB sahifa */
        if (flags_out)
            *flags_out = entry & ~PTE_ADDR_MASK;
        return (entry & PTE_ADDR_MASK & ~(GiB - 1)) + (virt & (GiB - 1));
    }

    table = phys_to_virt(entry & PTE_ADDR_MASK);
    entry = table[PD_INDEX(virt)];
    if (!(entry & PTE_PRESENT))
        return 0;
    if (entry & PTE_HUGE) {             /* 2 MB sahifa */
        if (flags_out)
            *flags_out = entry & ~PTE_ADDR_MASK;
        return (entry & PTE_ADDR_MASK & ~(2 * MiB - 1)) + (virt & (2 * MiB - 1));
    }

    table = phys_to_virt(entry & PTE_ADDR_MASK);
    entry = table[PT_INDEX(virt)];
    if (!(entry & PTE_PRESENT))
        return 0;
    if (flags_out)
        *flags_out = entry & ~PTE_ADDR_MASK;
    return (entry & PTE_ADDR_MASK) + (virt & (PAGE_SIZE - 1));
}

bool vmm_update_flags(uint64_t pml4, uint64_t virt, uint64_t flags)
{
    uint64_t *pte = walk(pml4, virt, false);
    if (!pte || !(*pte & PTE_PRESENT))
        return false;
    *pte = (*pte & PTE_ADDR_MASK) | (flags & ~PTE_ADDR_MASK) | PTE_PRESENT;
    if (pml4 == cpu_read_cr3())
        cpu_invlpg(virt);
    return true;
}

uint64_t vmm_count_user_pages(uint64_t pml4)
{
    uint64_t count = 0;
    uint64_t *pdpt_v = phys_to_virt(phys_to_virt(pml4)[0] & PTE_ADDR_MASK);
    for (int i = 1; i < 512; i++) {             /* 0 - yadro, sanamaymiz */
        if (!(pdpt_v[i] & PTE_PRESENT))
            continue;
        uint64_t *pd_v = phys_to_virt(pdpt_v[i] & PTE_ADDR_MASK);
        for (int j = 0; j < 512; j++) {
            if (!(pd_v[j] & PTE_PRESENT))
                continue;
            uint64_t *pt_v = phys_to_virt(pd_v[j] & PTE_ADDR_MASK);
            for (int k = 0; k < 512; k++)
                if (pt_v[k] & PTE_PRESENT)
                    count++;
        }
    }
    return count;
}

bool vmm_map_anonymous(uint64_t pml4, uint64_t virt, size_t pages, uint64_t flags)
{
    for (size_t i = 0; i < pages; i++) {
        uint64_t frame = alloc_zeroed_frame();  /* NOLLASH SHART: aks holda jarayon boshqa
                                                 * jarayonning eski ma'lumotini o'qiydi
                                                 * (parollar, kalitlar...) - xavfsizlik teshigi */
        if (!frame)
            return false;
        if (!vmm_map_page(pml4, virt + i * PAGE_SIZE, frame, flags)) {
            pmm_free_frame(frame);
            return false;
        }
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
        /* Bitta sahifa chegarasigacha nusxalaymiz: keyingi virtual sahifa
         * butunlay boshqa fizik freymda bo'lishi mumkin! */
        size_t chunk = PAGE_SIZE - (virt & (PAGE_SIZE - 1));
        if (chunk > len)
            chunk = len;
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
    if (end < virt)                     /* butun son to'lib ketishi (overflow) hujumi */
        return false;
    if (!is_user_address(virt) || end > USER_SPACE_END)
        return false;
    for (uint64_t page = ALIGN_DOWN(virt, PAGE_SIZE); page < end; page += PAGE_SIZE) {
        uint64_t flags;
        if (!vmm_translate(pml4, page, &flags))
            return false;
        if (!(flags & PTE_USER))
            return false;
        if (write && !(flags & PTE_WRITABLE))
            return false;
    }
    return true;
}
