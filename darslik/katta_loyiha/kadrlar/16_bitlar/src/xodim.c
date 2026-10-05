#include <stdio.h>

#include "config.h"
#include "pul.h"
#include "soliq.h"
#include "xodim.h"

void xodim_hisobla(const struct xodim *x, struct natija *n)
{
    n->asosiy = pul_vaqt_haqi(x->tarif, x->oddiy_daq);
    if (BAYROQ_BOR(x->bayroq, F_SINOV))
        n->asosiy = foiz(n->asosiy, SINOV_FOIZ);        /* sinov muddati: kamaytirilgan */
    n->ustama = pul_vaqt_haqi(x->tarif, x->qosh_daq) * 3 / 2;
    n->bonus = foiz(n->asosiy, toifa_bonusi(x->toifa));
    if (BAYROQ_BOR(x->bayroq, F_MASOFAVIY))
        n->bonus += MASOFAVIY_TIYIN;                    /* masofaviy kompensatsiya bonusga qo'shiladi */
    n->brutto = n->asosiy + n->ustama + n->bonus;
    n->soliq = soliq_hisobla(n->brutto);
    n->kasaba = foiz(n->brutto, KASABA_FOIZ);
    n->sof = n->brutto - n->soliq - n->kasaba;
}

void xodim_varaqa(const struct xodim *x)
{
    struct natija n;
    xodim_hisobla(x, &n);
    char bay[BAYROQ_MAKS_UZ];
    bayroq_matn(x->bayroq, bay);
    printf("=== MAOSH VARAQASI: %s (ID %d, %s, bayroqlar %s) ===\n", x->ism, x->id, toifa_matni(x->toifa), bay);
    printf("%-18s " PUL_USTUN " so'm\n", "Soatlik tarif:", PUL_ARG(x->tarif));
    printf("%-18s " PUL_USTUN " so'm\n", "Asosiy ish haqi:", PUL_ARG(n.asosiy));
    printf("%-18s " PUL_USTUN " so'm\n", "Ustama (x1.5):", PUL_ARG(n.ustama));
    printf("%-18s " PUL_USTUN " so'm\n", "Toifa bonusi:", PUL_ARG(n.bonus));
    printf("%-18s " PUL_USTUN " so'm\n", "Brutto:", PUL_ARG(n.brutto));
    printf("%-18s " PUL_USTUN " so'm\n", "Soliq:", PUL_ARG(n.soliq));
    printf("%-18s " PUL_USTUN " so'm\n", "Kasaba:", PUL_ARG(n.kasaba));
    printf("%-18s " PUL_USTUN " so'm\n", "Qo'lga tegadi:", PUL_ARG(n.sof));
}
