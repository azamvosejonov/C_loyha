/* =============================================================================
 *  mm/vmalloc.h - virtual jihatdan uzluksiz yadro xotirasi va ioremap
 * ============================================================================= */
#pragma once

#include <stddef.h>
#include <stdint.h>

void vmalloc_init(void);

/* size bayt (sahifaga yaxlitlanadi). Ikki tomonida xaritalanmagan HIMOYA
 * sahifasi bor - chegaradan chiqish darhol page fault beradi. */
void *vmalloc(size_t size);
void vfree(void *addr);

/* Qurilma xotirasini (MMIO) yadroga xaritalash. */
void *ioremap(uint64_t phys, size_t size);      /* UC - qurilma registrlari */
void *ioremap_wc(uint64_t phys, size_t size);   /* WC - framebuffer */
/* Oddiy (WB keshli) xaritalash - direct map'da bo'lmagan RAM uchun (masalan,
 * firmware "band" deb belgilagan hududdagi ACPI jadvallari). Agar hudud direct
 * map'da bo'lsa, shunchaki phys_to_virt qaytariladi (va iounmap SHART EMAS). */
void *memremap(uint64_t phys, size_t size);
void iounmap(void *addr);

void vmalloc_dump(void);
