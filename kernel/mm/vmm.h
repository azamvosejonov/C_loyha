/* =============================================================================
 *  mm/vmm.h - virtual xotira: sahifa jadvallari va manzil maydonlari
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mm/layout.h"

/* ---- Sahifa yozuvi bitlari (Intel SDM 3A, 4.5) ---- */
#define PTE_PRESENT  (1UL << 0)         /* mavjud */
#define PTE_WRITABLE (1UL << 1)         /* yozish mumkin */
#define PTE_USER     (1UL << 2)         /* ring 3 murojaat qila oladi */
#define PTE_PWT      (1UL << 3)         /* write-through  } PAT bilan birga */
#define PTE_PCD      (1UL << 4)         /* cache disable  } kesh turini tanlaydi */
#define PTE_ACCESSED (1UL << 5)         /* CPU o'zi qo'yadi: sahifaga murojaat bo'lgan */
#define PTE_DIRTY    (1UL << 6)         /* CPU o'zi qo'yadi: sahifaga yozilgan */
#define PTE_HUGE     (1UL << 7)         /* PD/PDPT darajasida katta sahifa */
#define PTE_GLOBAL   (1UL << 8)         /* CR3 almashganda TLB dan o'chirilmaydi */
#define PTE_COW      (1UL << 9)         /* DASTURIY bit (CPU e'tibor bermaydi): copy-on-write */
#define PTE_NX       (1UL << 63)        /* bajarib bo'lmaydi */
#define PTE_ADDR_MASK 0x000FFFFFFFFFF000UL

/* Kesh turlari (cpu.c dagi PAT sozlamasi bilan):
 *   WB (sukut)  - oddiy RAM
 *   WC = PWT    - framebuffer (yozuvlar to'planib yuboriladi)
 *   UC = PCD|PWT- qurilma registrlari (har bir murojaat to'g'ridan-to'g'ri) */
#define PTE_CACHE_WC (PTE_PWT)
#define PTE_CACHE_UC (PTE_PCD | PTE_PWT)

/* CPU NX ni qo'llamasa 0, aks holda PTE_NX. Barcha xaritalashlarda ishlatiladi. */
extern uint64_t pte_nx;

void vmm_init(void);                    /* memblock bilan yadro jadvallarini quradi */
void vmm_late_init(void);               /* buddy tayyor: jadvallar endi buddy'dan olinadi */

uint64_t vmm_kernel_pml4(void);
void vmm_switch(uint64_t pml4);

/* Yangi manzil maydoni: yuqori yarim (yadro) umumiy, pastki yarim bo'sh. */
uint64_t vmm_create_address_space(void);
/* Pastki yarimdagi barcha sahifalar (refcount orqali) va jadvallar qaytariladi. */
void vmm_destroy_address_space(uint64_t pml4);

/* virt -> phys (4 KB). Muvaffaqiyat: true. */
bool vmm_map_page(uint64_t pml4, uint64_t virt, uint64_t phys, uint64_t flags);
/* Qaytaradi: eski fizik manzil yoki 0. Freymni BO'SHATMAYDI. */
uint64_t vmm_unmap_page(uint64_t pml4, uint64_t virt);
/* Oxirgi darajadagi yozuv manzili (create=true bo'lsa jadvallarni yaratadi). */
uint64_t *vmm_get_pte(uint64_t pml4, uint64_t virt, bool create);
uint64_t vmm_translate(uint64_t pml4, uint64_t virt, uint64_t *flags_out);
bool vmm_update_flags(uint64_t pml4, uint64_t virt, uint64_t flags);

bool vmm_map_anonymous(uint64_t pml4, uint64_t virt, size_t pages, uint64_t flags);
bool vmm_copy_to_space(uint64_t pml4, uint64_t virt, const void *src, size_t len);
bool vmm_user_range_ok(uint64_t pml4, uint64_t virt, size_t len, bool write);
uint64_t vmm_count_user_pages(uint64_t pml4);

/* Yadro sahifasini BARCHA CPU'larda TLB dan o'chirish (SMP: IPI orqali). */
void vmm_flush_page(uint64_t virt);
/* [phys, phys+len) to'liq direct map ichidami (RAM/ACPI hududlari)? */
bool vmm_phys_is_direct_mapped(uint64_t phys, uint64_t len);
