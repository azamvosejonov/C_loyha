#pragma once

#include <stddef.h>

struct talaba {
    char ism[32];
    int ball;
};

void mening_saralashim(int *a, size_t n);
void talabalarni_saralash(struct talaba *a, size_t n);
