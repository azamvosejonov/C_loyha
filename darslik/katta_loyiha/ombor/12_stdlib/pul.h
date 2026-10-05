/* pul.h - pul bilan ishlash (tiyin <-> so'm) */
#ifndef PUL_H
#define PUL_H

#include <stddef.h>

/* 400000 tiyin -> "4000.00" (buferga yozadi) */
void pul_matn(char *bufer, size_t hajm, long tiyin);

/* narxni (tiyinda) ekranga chiqaradi: "4000.00" */
void pul_chiqar(long tiyin);

#endif
