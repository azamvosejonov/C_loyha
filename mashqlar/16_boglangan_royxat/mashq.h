#pragma once

#include <stddef.h>

struct tugun {
    int qiymat;
    struct tugun *keyingi;      /* keyingi tugun yoki NULL (ro'yxat oxiri) */
};

struct tugun *boshiga_qosh(struct tugun *bosh, int x);
size_t uzunlik(const struct tugun *bosh);
struct tugun *teskari_royxat(struct tugun *bosh);
struct tugun *ochir(struct tugun *bosh, int x);
void royxat_ozod(struct tugun *bosh);
