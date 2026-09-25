/* =============================================================================
 *  mm/vmm.h - Virtual Memory Manager (virtual xotira menejeri)
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ---- Sahifa jadvali yozuvi (PTE) bayroqlari - Intel SDM 3A, 4.5 ---- */
#define PTE_PRESENT  (1UL << 0)         /* sahifa mavjud */
#define PTE_WRITABLE (1UL << 1)         /* yozish mumkin */
#define PTE_USER     (1UL << 2)         /* ring 3 (user) murojaat qila oladi */
#define PTE_HUGE     (1UL << 7)         /* PD/PDPT darajasida: katta sahifa (2 MB / 1 GB) */
/* Yozuvning 12..51 bitlari - keyingi jadval yoki sahifaning fizik manzili. */
#define PTE_ADDR_MASK 0x000FFFFFFFFFF000UL

/* ---- Virtual manzil maydoni xaritasi ----
 *
 *   0x0000_0000_0000_0000 ┌──────────────────────────┐
 *                         │ 0-sahifa: XARITALANMAGAN │ <- NULL ko'rsatkichni ushlash
 *   0x0000_0000_0000_1000 ├──────────────────────────┤
 *                         │ YADRO: identity 0..1 GB  │ <- faqat ring 0 (U=0)
 *                         │ (kod, heap, fizik RAM)   │    barcha jarayonlarda UMUMIY
 *   0x0000_0000_4000_0000 ├──────────────────────────┤ <- USER_SPACE_START (1 GB)
 *                         │ user dastur kodi (ELF)   │
 *                         │ user heap (sbrk) ↓       │
 *                         │         ...              │
 *                         │ user steki ↑             │
 *   0x0000_0000_8000_0000 ├──────────────────────────┤ <- USER_STACK_TOP (2 GB)
 *                         │ (ishlatilmaydi)          │
 *   0x0000_0080_0000_0000 └──────────────────────────┘ <- USER_SPACE_END (512 GB)
 */
#define USER_SPACE_START 0x40000000UL
#define USER_SPACE_END   0x8000000000UL
#define USER_STACK_TOP   0x80000000UL
#define USER_STACK_PAGES 16             /* 64 KB user steki */

void vmm_init(void);

/* Yadroning asosiy PML4 jadvali (boot.asm qurgan) fizik manzili. */
uint64_t vmm_kernel_pml4(void);

/* Yangi manzil maydoni: yadro qismi umumiy, user qismi bo'sh. Qaytaradi: PML4
 * fizik manzili yoki 0 (xotira yetmasa). */
uint64_t vmm_create_address_space(void);

/* User qismidagi BARCHA sahifalarni, jadvallarni va PML4 ning o'zini bo'shatadi.
 * Joriy (faol) manzil maydonini yo'q qilib bo'lmaydi. */
void vmm_destroy_address_space(uint64_t pml4);

/* CR3 ga yozish: boshqa manzil maydoniga o'tish. */
void vmm_switch(uint64_t pml4);

/* virt -> phys bog'lash (4 KB). Faqat user hududida. Muvaffaqiyat: true. */
bool vmm_map_page(uint64_t pml4, uint64_t virt, uint64_t phys, uint64_t flags);

/* Bog'lanishni olib tashlash. Qaytaradi: bo'lgan fizik manzil yoki 0.
 * Freymni BO'SHATMAYDI - bu chaqiruvchining ishi. */
uint64_t vmm_unmap_page(uint64_t pml4, uint64_t virt);

/* virt qaysi fizik manzilga tushadi? Xaritalanmagan bo'lsa 0.
 * flags_out != NULL bo'lsa, sahifa bayroqlari ham qaytariladi. */
uint64_t vmm_translate(uint64_t pml4, uint64_t virt, uint64_t *flags_out);

/* [virt, virt + pages*4K) oralig'iga yangi, NOLLANGAN freymlarni bog'lash. */
bool vmm_map_anonymous(uint64_t pml4, uint64_t virt, size_t pages, uint64_t flags);

/* Boshqa (faol bo'lmagan) manzil maydoniga ma'lumot yozish - fizik manzillar
 * orqali. ELF yuklovchi va argv ni user stekiga qo'yish uchun kerak. */
bool vmm_copy_to_space(uint64_t pml4, uint64_t virt, const void *src, size_t len);

/* Allaqachon xaritalangan sahifaning bayroqlarini o'zgartirish (ELF yuklovchi). */
bool vmm_update_flags(uint64_t pml4, uint64_t virt, uint64_t flags);

/* User hududida nechta sahifa xaritalangan (ps buyrug'i uchun). */
uint64_t vmm_count_user_pages(uint64_t pml4);

/* SYSCALL XAVFSIZLIGI: user bergan [virt, virt+len) buferi haqiqatan user
 * hududidami, xaritalanganmi, U=1 va (kerak bo'lsa) yozish mumkinmi? */
bool vmm_user_range_ok(uint64_t pml4, uint64_t virt, size_t len, bool write);
