#include <stdio.h>

#include "config.h"
#include "matn.h"
#include "pul.h"
#include "soliq.h"
#include "vaqt.h"
#include "xodim.h"

const char *toifa_nomi(int toifa)
{
    switch (toifa) {
    case 1: return "Boshlovchi";
    case 2: return "Mutaxassis";
    case 3: return "Yetakchi";
    case 4: return "Rahbar";
    default: return "?";
    }
}

void xodim_hisobla(const struct xodim *x, struct natija *n)
{
    n->asosiy = pul_vaqt_haqi(x->tarif, x->oddiy_daq);
    n->ustama = pul_vaqt_haqi(x->tarif, x->qosh_daq) * 3 / 2;      /* ortiqcha ish 1.5 barobar */
    n->bonus = foiz(n->asosiy, toifa_bonusi(x->toifa));
    n->brutto = n->asosiy + n->ustama + n->bonus;
    n->soliq = soliq_hisobla(n->brutto);
    n->kasaba = foiz(n->brutto, KASABA_FOIZ);
    n->sof = n->brutto - n->soliq - n->kasaba;
}

void xodim_varaqa(const struct xodim *x)
{
    struct natija n;
    xodim_hisobla(x, &n);

    printf("=== MAOSH VARAQASI ===\n");
    printf("%-20s%15s\n", "Xodim:", x->ism);
    printf("%-20s%10d-%d\n", "ID:", x->id, id_nazorat(x->id));
    printf("%-20s%15s\n", "Toifa:", toifa_nomi(x->toifa));
    printf("%-20s%11d.%02d soat\n", "Oddiy ish:", soat_yuzdan(x->oddiy_daq) / 100, soat_yuzdan(x->oddiy_daq) % 100);
    printf("%-20s%11d.%02d soat\n", "Qo'shimcha ish:", soat_yuzdan(x->qosh_daq) / 100, soat_yuzdan(x->qosh_daq) % 100);
    pul_chiqar("Soatlik tarif:", x->tarif);
    pul_chiqar("Asosiy ish haqi:", n.asosiy);
    pul_chiqar("Ustama (x1.5):", n.ustama);
    pul_chiqar("Toifa bonusi:", n.bonus);
    pul_chiqar("Brutto:", n.brutto);
    pul_chiqar("Soliq (progressiv):", n.soliq);
    pul_chiqar("Kasaba (1%):", n.kasaba);
    pul_chiqar("Qo'lga tegadi:", n.sof);
}
