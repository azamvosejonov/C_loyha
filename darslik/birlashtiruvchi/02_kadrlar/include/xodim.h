/* xodim.h - xodim ma'lumoti va uning maoshini hisoblash */
#ifndef XODIM_H
#define XODIM_H

#include <stdint.h>

#include "config.h"

struct xodim {
    int id;                                 /* 4 xonali (1000..9999) */
    char ism[ISM_UZ];
    int toifa;                              /* 1..4 */
    int64_t tarif;                          /* tiyin / SOAT */
    int oddiy_daq;                          /* oy davomida oddiy ishlagan daqiqalar (davomat.c to'ldiradi) */
    int qosh_daq;                           /* qo'shimcha (ustama) daqiqalar */
};

/* bitta xodimning maosh hisobi, hammasi tiyinda */
struct natija {
    int64_t asosiy, ustama, bonus, brutto, soliq, kasaba, sof;
};

/* toifa raqamidan nom: 1 "Boshlovchi", 2 "Mutaxassis", 3 "Yetakchi", 4 "Rahbar", boshqasi "?" */
const char *toifa_nomi(int toifa);

/* x ning hisobini n ga yozadi (pul.h, soliq.h funksiyalaridan foydalanadi) */
void xodim_hisobla(const struct xodim *x, struct natija *n);

/* to'liq maosh varaqasini chiqaradi */
void xodim_varaqa(const struct xodim *x);

#endif
