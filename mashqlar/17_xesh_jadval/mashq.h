#pragma once

#include <stddef.h>

/* "Yashirin" (opaque) tur: ichki tuzilishini FAQAT yechim.c biladi.
 * Test uni faqat ko'rsatkich orqali ishlatadi. (Yadroda ham ko'p
 * tuzilmalar shunday yashiriladi - interfeys va amalga oshirish ajratiladi.) */
struct xesh;

struct xesh *xesh_yarat(void);
int xesh_qoy(struct xesh *h, const char *kalit, int qiymat);
int xesh_ol(const struct xesh *h, const char *kalit, int *qiymat);
int xesh_ochir(struct xesh *h, const char *kalit);
size_t xesh_soni(const struct xesh *h);
void xesh_ozod(struct xesh *h);
