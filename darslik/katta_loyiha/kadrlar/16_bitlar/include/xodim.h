/* xodim.h - xodim ma'lumoti va uning maoshini hisoblash */
#ifndef XODIM_H
#define XODIM_H

#include <stdint.h>

#include "config.h"
#include "bayroq.h"
#include "toifa.h"

struct xodim {
    int id;
    char ism[ISM_UZ];
    enum toifa toifa;
    uint8_t bayroq;                             /* F_* bitlar yig'indisi (bayroq.h) */
    int64_t tarif;                              /* tiyin / soat */
    int oddiy_daq, qosh_daq;                    /* oy davomida oddiy va qo'shimcha daqiqalar */
};

struct natija {
    int64_t asosiy, ustama, bonus, brutto, soliq, kasaba, sof;
};

void xodim_hisobla(const struct xodim *x, struct natija *n);
void xodim_varaqa(const struct xodim *x);       /* to'liq maosh varaqasini chiqaradi */

#endif
