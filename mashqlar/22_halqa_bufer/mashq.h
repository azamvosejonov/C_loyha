#pragma once

#include <stddef.h>

#define HALQA_HAJMI 8

struct halqa {
    unsigned char buf[HALQA_HAJMI];
    size_t bosh;        /* keyingi O'QILADIGAN bayt indeksi */
    size_t soni;        /* buferda nechta bayt bor */
};

void halqa_init(struct halqa *h);
size_t halqa_yoz(struct halqa *h, const void *data, size_t n);
size_t halqa_oqi(struct halqa *h, void *out, size_t n);
