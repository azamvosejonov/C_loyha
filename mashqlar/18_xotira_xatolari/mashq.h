#pragma once

#include <stddef.h>

struct tugun {
    int qiymat;
    struct tugun *keyingi;
};

char *nusxa(const char *s);
int *massiv_nusxa(const int *a, size_t n);
long yigindi(const int *a, size_t n);
char *katta_harf(const char *s);
void royxat_ozod(struct tugun *bosh);
