/* =============================================================================
 *  sys/elf.h - ELF64 bajariladigan fayllarni yuklash
 * ============================================================================= */
#pragma once

#include <stddef.h>
#include <stdint.h>

struct mm;

/* ELF faylni mm manzil maydoniga yuklaydi (har bir segment - alohida VMA).
 * Muvaffaqiyat: 0, *entry = kirish nuqtasi, *image_end = eng yuqori segment oxiri.
 * Xato: manfiy son (fayl buzilgan yoki xotira yetmadi). */
int elf_load(struct mm *mm, const uint8_t *data, size_t size, uint64_t *entry,
             uint64_t *image_end);
