/* =============================================================================
 *  mm/layout.h - 64-bitli VIRTUAL MANZIL MAYDONI XARITASI
 * =============================================================================
 *
 *  x86-64 da virtual manzil 48 bit (256 TB). 47-bit butun yuqori qismga
 *  "cho'ziladi" (canonical address). Shuning uchun maydon ikki yarimga bo'linadi:
 *
 *  0x0000_0000_0000_0000 ┌───────────────────────────────┐
 *                        │ 0-sahifa: hech qachon yo'q    │  NULL ushlash
 *  0x0000_0000_0040_0000 │ USER: dastur (ELF)            │  har bir jarayonga
 *                        │       heap (brk) ↓            │  XOS (PML4[0..255])
 *                        │       mmap hududlari          │
 *                        │       stek ↑                  │
 *  0x0000_7FFF_FFFF_F000 └───────────────────────────────┘ USER_SPACE_END
 *                           ... "teshik": kanonik bo'lmagan manzillar ...
 *  0xFFFF_8000_0000_0000 ┌───────────────────────────────┐ HHDM_BASE (PML4[256])
 *                        │ DIRECT MAP: butun fizik RAM   │  phys + HHDM_BASE
 *                        │ (Higher Half Direct Map)      │
 *  0xFFFF_C000_0000_0000 ├───────────────────────────────┤ VMALLOC_START (PML4[384])
 *                        │ vmalloc: yadro steklari,      │  sahifama-sahifa
 *                        │ ioremap (qurilma xotirasi)    │  xaritalanadi
 *  0xFFFF_E000_0000_0000 ├───────────────────────────────┤ VMALLOC_END
 *                        │ (bo'sh)                       │
 *  0xFFFF_FFFF_8000_0000 ├───────────────────────────────┤ KERNEL_VMA (PML4[511])
 *                        │ yadro kodi va ma'lumotlari    │  -mcmodel=kernel
 *  0xFFFF_FFFF_FFFF_FFFF └───────────────────────────────┘
 *
 *  YUQORI YARIM (256..511) BARCHA JARAYONLARDA UMUMIY: har bir PML4 yadro
 *  yozuvlarining nusxasiga ega. Pastki yarim - jarayonning shaxsiy xotirasi.
 *  Linux x86-64 deyarli aynan shunday tuzilgan.
 * ============================================================================= */
#pragma once

#include <stdint.h>

#define KERNEL_VMA      0xFFFFFFFF80000000UL
#define HHDM_BASE       0xFFFF800000000000UL
#define VMALLOC_START   0xFFFFC00000000000UL
#define VMALLOC_END     0xFFFFE00000000000UL

#define USER_SPACE_START 0x0000000000400000UL   /* 4 MB: ELF shu yerga link qilinadi */
#define USER_SPACE_END   0x00007FFFFFFFF000UL   /* kanonik pastki yarim oxiri (1 sahifa zaxira) */
#define USER_STACK_TOP   USER_SPACE_END
#define USER_STACK_MAX   (8UL << 20)            /* stek 8 MB gacha o'sishi mumkin */
#define USER_STACK_PAGES 16                     /* boshlang'ich (darhol ajratiladigan) stek: 64 KB */

#define PAGE_SIZE  4096UL
#define PAGE_SHIFT 12
#define PAGE_MASK  (~(PAGE_SIZE - 1))

/* Fizik manzil <-> direct map'dagi virtual manzil. Direct map butun RAM ni
 * qamrab olgani uchun yadro ISTALGAN fizik sahifaga shu orqali murojaat qiladi. */
static inline void *phys_to_virt(uint64_t phys)
{
    return (void *)(phys + HHDM_BASE);
}

static inline uint64_t virt_to_phys(const void *virt)
{
    uint64_t v = (uint64_t)virt;
    if (v >= KERNEL_VMA)                /* yadro tasviri ichidagi manzil (masalan, .bss) */
        return v - KERNEL_VMA;
    return v - HHDM_BASE;               /* direct map manzili */
}

static inline int is_user_address(uint64_t v)
{
    return v < USER_SPACE_END;
}
