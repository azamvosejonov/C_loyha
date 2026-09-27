#pragma once

#include <stddef.h>

struct heap {
    int *a;             /* a[0] - eng kichik; i ning bolalari 2i+1, 2i+2; otasi (i-1)/2 */
    size_t n;           /* elementlar soni */
    size_t sig;         /* ajratilgan joy */
};

void heap_init(struct heap *h);
int heap_qosh(struct heap *h, int x);
int heap_ol(struct heap *h, int *x);
int heap_tepa(const struct heap *h, int *x);
int heap_qur(struct heap *h, const int *massiv, size_t n);
void heap_yoq(struct heap *h);
