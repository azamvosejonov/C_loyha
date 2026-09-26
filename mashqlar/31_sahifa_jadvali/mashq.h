#pragma once

#include <stddef.h>
#include <stdint.h>

#define SAHIFA        4096ull
#define PTE_PRESENT   (1ull << 0)               /* yozuv haqiqiy */
#define PTE_WRITABLE  (1ull << 1)               /* yozish mumkin */
#define PTE_USER      (1ull << 2)               /* user rejimi kira oladi */
#define PTE_ADDR_MASK 0x000FFFFFFFFFF000ull     /* yozuvdagi fizik manzil bitlari (12..51) */

/* --- Bular TEST tomonidan beriladi (test.c): simulyatsiya qilingan fizik xotira --- */
uint64_t sahifa_ajrat(void);        /* yangi NOLLANGAN fizik sahifa manzili; 0 - xotira tugadi */
void *fiz_ptr(uint64_t fiz);        /* fizik manzil -> ko'rsatkich (yadrodagi "direct map" kabi) */

/* --- Bularni SIZ yozasiz --- */
int xarita(uint64_t pml4, uint64_t virt, uint64_t fiz, uint64_t bayroqlar);
int tarjima(uint64_t pml4, uint64_t virt, uint64_t *fiz);
int xaritani_ochir(uint64_t pml4, uint64_t virt);
