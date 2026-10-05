/* eksport.h - hisobotni CSV (vergul bilan ajratilgan matn) ko'rinishiga aylantirish */
#ifndef EKSPORT_H
#define EKSPORT_H

#include <stddef.h>

#include "ombor.h"

/* Har xodim = bitta qator: id,ism,toifa,brutto,soliq,sof  (pul TIYINDA, butun son: boshqa dasturlar bilan ishlash oson).
   Heap da yangi bufer qaytaradi (chaqiruvchi free qiladi), *uzunlik ga baytlar sonini yozadi. NULL - xotira yo'q */
char *eksport_csv(const struct ombor *o, size_t *uzunlik);

#endif
