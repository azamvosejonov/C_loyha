#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* n ta bit uchun nechta uint64_t so'z kerak. */
#define BITMAP_SOZLAR(n) (((n) + 63) / 64)

void bm_yoq(uint64_t *bm, size_t i);
void bm_ochir(uint64_t *bm, size_t i);
bool bm_bormi(const uint64_t *bm, size_t i);
long bm_birinchi_nol(const uint64_t *bm, size_t n);
long bm_ketma_ket_nollar(const uint64_t *bm, size_t n, size_t k);
