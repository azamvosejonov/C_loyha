/* =============================================================================
 *  mm/pmm.h - Physical Memory Manager (fizik xotira menejeri)
 * ============================================================================= */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "boot/multiboot.h"

/* Sahifa (page) / freym (frame) hajmi - x86 ning asosiy xotira birligi.
 * "Freym" - fizik xotiradagi 4 KB lik bo'lak, "sahifa" - virtual xotiradagi. */
#define PAGE_SIZE  4096UL
#define PAGE_SHIFT 12                   /* 2^12 = 4096 */

/* Biz faqat birinchi 1 GB fizik xotirani boshqaramiz, chunki boot.asm faqat
 * shuni identity-map qilgan: yadro istalgan fizik freymga ko'rsatkich orqali
 * to'g'ridan-to'g'ri murojaat qila olishi kerak. */
#define PMM_MAX_PHYS (1UL << 30)

void pmm_init(const struct multiboot_info *mbi);

/* Bitta 4 KB freym ajratish. Qaytaradi: fizik manzil, xotira tugagan bo'lsa 0.
 * (0-freym hech qachon berilmaydi, shuning uchun 0 = "xato" degan ma'noni
 *  bildira oladi.) Freym tarkibi NOLLANMAGAN. */
uint64_t pmm_alloc_frame(void);

/* Ketma-ket (fizik jihatdan uzluksiz) count ta freym ajratish. */
uint64_t pmm_alloc_frames(size_t count);

void pmm_free_frame(uint64_t phys);
void pmm_free_frames(uint64_t phys, size_t count);

size_t pmm_total_frames(void);          /* boshqariladigan RAM freymlari soni */
size_t pmm_free_frames_count(void);     /* hozir bo'sh freymlar soni */
