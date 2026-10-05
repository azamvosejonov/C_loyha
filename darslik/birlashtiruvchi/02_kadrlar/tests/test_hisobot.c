#include <stdio.h>
#include <string.h>

#include "hisobot.h"
#include "test.h"

int main(void)
{
    struct ombor o;
    ombor_boshla(&o);

    BOSHLA("T14 hisobot_jami");
    struct natija j;
    memset(&j, 99, sizeof(j));                          /* axlat bilan to'ldirilgan: funksiya o'zi nollashi kerak */
    hisobot_jami(&o, &j);
    TEKSHIR(j.brutto, 0);
    TEKSHIR(j.sof, 0);

    struct xodim x;
    memset(&x, 0, sizeof(x));
    x.id = 1111;
    snprintf(x.ism, sizeof(x.ism), "Ali");
    x.toifa = 1;
    x.tarif = 6000;                                     /* 60 so'm/soat */
    x.oddiy_daq = 120;                                  /* 2 soat */
    x.qosh_daq = 60;                                    /* 1 soat */
    ombor_qosh(&o, &x);
    x.id = 2222;
    x.toifa = 2;
    x.oddiy_daq = 60;
    x.qosh_daq = 0;
    ombor_qosh(&o, &x);

    hisobot_jami(&o, &j);
    struct natija a, b;
    xodim_hisobla(&o.a[0], &a);
    xodim_hisobla(&o.a[1], &b);
    TEKSHIR(j.asosiy, a.asosiy + b.asosiy);
    TEKSHIR(j.ustama, a.ustama + b.ustama);
    TEKSHIR(j.bonus, a.bonus + b.bonus);
    TEKSHIR(j.brutto, a.brutto + b.brutto);
    TEKSHIR(j.soliq, a.soliq + b.soliq);
    TEKSHIR(j.kasaba, a.kasaba + b.kasaba);
    TEKSHIR(j.sof, a.sof + b.sof);
    TEKSHIR(j.asosiy, 12000 + 6000);                    /* qo'lda tekshiruv: 2 soat * 6000 + 1 soat * 6000 */
    TUGAT();
    YAKUN();
}
