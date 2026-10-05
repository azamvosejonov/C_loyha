#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "hisobot.h"
#include "pul.h"

/* qsort taqqoslagichlari: har biri ikkita struct xodim * ni (void * sifatida) oladi */
static int ism_boyicha(const void *a, const void *b)
{
    const struct xodim *x = a, *y = b;
    return strcmp(x->ism, y->ism);
}

static int64_t sof_maosh(const struct xodim *x)
{
    struct natija n;
    xodim_hisobla(x, &n);
    return n.sof;
}

static int maosh_boyicha(const void *a, const void *b)
{
    int64_t p = sof_maosh(a), q = sof_maosh(b);
    return (p < q) - (p > q);                   /* kamayish tartibi; ayirish EMAS: int64_t ayirmasi int ga sig'masligi mumkin */
}

void hisobot_royxat(struct ombor *o, enum tartib t, FILE *chiqish)
{
    if (t == TARTIB_ISM)
        ombor_saralash(o, ism_boyicha);
    else if (t == TARTIB_MAOSH)
        ombor_saralash(o, maosh_boyicha);

    fprintf(chiqish, "%-5s %-9s %-11s %-6s %14s %14s %20s\n", "ID", "Ism", "Toifa", "Bayroq", "Brutto", "Soliq", "Qo'lga");
    for (size_t i = 0; i < ombor_soni(o); i++) {
        const struct xodim *x = ombor_ol(o, i);
        struct natija n;
        xodim_hisobla(x, &n);
        char sof[32];
        pul_matn(sof, sizeof(sof), n.sof);
        char bay[BAYROQ_MAKS_UZ];
        bayroq_matn(x->bayroq, bay);
        fprintf(chiqish, "%-5d %-9s %-11s %-6s " PUL_USTUN " " PUL_USTUN " %20s\n", x->id, x->ism, toifa_matni(x->toifa), bay,
                PUL_ARG(n.brutto), PUL_ARG(n.soliq), sof);
    }
}

void hisobot_jami(const struct ombor *o, FILE *chiqish)
{
    int64_t brutto = 0, soliq = 0, sof = 0;
    for (size_t i = 0; i < ombor_soni(o); i++) {
        struct natija n;
        xodim_hisobla(ombor_ol(o, i), &n);
        brutto += n.brutto;
        soliq += n.soliq;
        sof += n.sof;
    }
    char matn[32];
    pul_matn(matn, sizeof(matn), sof);
    fprintf(chiqish, "Jami (%zu xodim): brutto " PUL_FMT ", soliq " PUL_FMT ", qo'lga tegadi %s so'm\n", ombor_soni(o),
            PUL_ARG(brutto), PUL_ARG(soliq), matn);
}

int hisobot_saqla(struct ombor *o, const char *fayl)
{
    FILE *f = fopen(fayl, "w");
    if (!f) {
        fprintf(stderr, "%s: yozish uchun ochilmadi: %s\n", fayl, strerror(errno));
        return -1;
    }
    hisobot_royxat(o, TARTIB_MAOSH, f);
    hisobot_jami(o, f);
    if (ferror(f) || fclose(f) != 0) {          /* disk to'lgan bo'lsa, xato aynan shu yerda bilinadi */
        fprintf(stderr, "%s: yozishda xato: %s\n", fayl, strerror(errno));
        return -1;
    }
    return 0;
}
