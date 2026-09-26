#pragma once

#include <stddef.h>

/* Python'dagi list'ning C'dagi varianti (faqat int uchun). */
struct vec {
    int *data;          /* malloc qilingan massiv (yoki NULL) */
    size_t len;         /* nechta element bor */
    size_t cap;         /* nechta elementga joy ajratilgan (cap >= len) */
};

void vec_init(struct vec *v);
int vec_push(struct vec *v, int x);
int vec_pop(struct vec *v, int *out);
int vec_insert(struct vec *v, size_t i, int x);
void vec_free(struct vec *v);
