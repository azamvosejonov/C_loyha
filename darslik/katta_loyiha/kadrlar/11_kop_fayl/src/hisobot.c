#include <stdio.h>

#include "hisobot.h"
#include "pul.h"

void hisobot_royxat(const struct ombor *o)
{
    printf("%-5s %-9s %-11s %14s %14s %14s\n", "ID", "Ism", "Toifa", "Brutto", "Soliq", "Qo'lga");
    for (size_t i = 0; i < ombor_soni(o); i++) {
        const struct xodim *x = ombor_ol(o, i);
        struct natija n;
        xodim_hisobla(x, &n);
        printf("%-5d %-9s %-11s " PUL_USTUN " " PUL_USTUN " " PUL_USTUN "\n", x->id, x->ism, toifa_matni(x->toifa),
               PUL_ARG(n.brutto), PUL_ARG(n.soliq), PUL_ARG(n.sof));
    }
}

void hisobot_jami(const struct ombor *o)
{
    int64_t brutto = 0, soliq = 0, sof = 0;
    for (size_t i = 0; i < ombor_soni(o); i++) {
        struct natija n;
        xodim_hisobla(ombor_ol(o, i), &n);
        brutto += n.brutto;
        soliq += n.soliq;
        sof += n.sof;
    }
    printf("Jami (%zu xodim): brutto " PUL_FMT ", soliq " PUL_FMT ", qo'lga tegadi " PUL_FMT " so'm\n", ombor_soni(o),
           PUL_ARG(brutto), PUL_ARG(soliq), PUL_ARG(sof));
}
