/* =============================================================================
 *  mm/mm.h - jarayon manzil maydoni: VMA'lar, page fault, fork, brk, mmap
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "lib/list.h"
#include "lib/spinlock.h"
#include "myos/abi.h"

#define VMA_ANON  (1u << 0)             /* anonim xotira (nollar bilan boshlanadi) */
#define VMA_STACK (1u << 1)             /* stek - pastga o'sadi */
#define VMA_HEAP  (1u << 2)             /* brk/sbrk hududi */
#define VMA_ELF   (1u << 3)             /* dastur segmenti */

#define MMAP_TOP  0x0000700000000000UL  /* mmap hududlari shu yerdan PASTGA ajratiladi */

/* VMA - Virtual Memory Area: bir xil ruxsatli uzluksiz manzillar hududi. */
struct vma {
    uint64_t start, end;                /* [start, end), sahifaga tekislangan */
    uint32_t prot;                      /* PROT_READ | PROT_WRITE | PROT_EXEC */
    uint32_t flags;                     /* VMA_* */
    struct list_head node;              /* mm->vmas da, start bo'yicha tartiblangan */
};

struct mm {
    uint64_t pml4;                      /* sahifa jadvali (fizik) */
    struct list_head vmas;
    uint64_t brk_start, brk;            /* heap: [brk_start, brk) */
    spinlock_t lock;                    /* vmas va sahifa jadvallarini himoya qiladi */
};

struct mm *mm_create(void);
void mm_destroy(struct mm *mm);
/* fork: copy-on-write nusxa. */
struct mm *mm_fork(struct mm *parent);

/* Hudud qo'shish. addr = 0 bo'lsa, bo'sh joy o'zi topiladi. Qaytaradi: manzil yoki 0. */
uint64_t mm_map(struct mm *mm, uint64_t addr, uint64_t len, uint32_t prot, uint32_t flags);
int mm_unmap(struct mm *mm, uint64_t addr, uint64_t len);
/* sbrk: heap'ni o'zgartirish. Qaytaradi: eski brk yoki (uint64_t)-1. */
uint64_t mm_sbrk(struct mm *mm, int64_t increment);

/* Page fault: hal qilindi -> true; aks holda false (Segmentation fault). */
bool mm_handle_fault(struct mm *mm, uint64_t addr, uint64_t err);
/* Yadro user xotirasiga murojaat qilishidan oldin: hudud to'g'rimi va sahifalar
 * joyidami (kerak bo'lsa hozir yaratiladi, COW buziladi)? */
bool mm_prefault(struct mm *mm, uint64_t addr, uint64_t len, bool write);

/* Sahifalarni DARHOL ajratib, ma'lumot nusxalash (ELF, argv). */
bool mm_populate(struct mm *mm, uint64_t addr, uint64_t len);

uint64_t prot_to_pte(uint32_t prot);
