/* yukla.h - matn fayllardan o'qish (12-bob: fopen, fgets, strtol, errno). Noto'g'ri qator dasturni to'xtatmaydi:
   sababi bilan stderr ga yoziladi ("fayl:qator: sabab"), qator o'tkazib yuboriladi, hisobga olinadi. */
#ifndef YUKLA_H
#define YUKLA_H

#include "ombor.h"

struct yuklash {
    int qabul;                                  /* muvaffaqiyatli o'qilgan qatorlar */
    int rad;                                    /* rad etilgan (noto'g'ri) qatorlar */
};

/* 0 - fayl o'qildi (rad bo'lsa ham), -1 - fayl ochilmadi (sabab stderr ga yozilgan) */
int xodimlar_yukla(struct ombor *o, const char *fayl, struct yuklash *h);
int davomat_yukla(struct ombor *o, const char *fayl, struct yuklash *h);

#endif
