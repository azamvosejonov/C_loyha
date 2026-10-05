#include <assert.h>
#include <stdlib.h>

#include "ombor.h"

struct ombor {                                  /* ta'rif FAQAT shu faylda ko'rinadi */
    struct xodim *a;                            /* heap dagi massiv */
    size_t soni, sigim;
};

struct ombor *ombor_yarat(void)
{
    return calloc(1, sizeof(struct ombor));     /* hammasi nol: a = NULL, soni = sigim = 0 */
}

void ombor_yoq_qil(struct ombor *o)
{
    if (!o)
        return;
    free(o->a);
    free(o);
}

int ombor_qosh(struct ombor *o, const struct xodim *x)
{
    if (ombor_top(o, x->id))
        return -2;
    if (o->soni == o->sigim) {                  /* joy tugadi: sig'imni ikki barobar oshiramiz */
        size_t yangi = o->sigim ? o->sigim * 2 : 4;
        struct xodim *b = realloc(o->a, yangi * sizeof(*b));    /* natijani AVVAL vaqtinchalik o'zgaruvchiga */
        if (!b)
            return -1;                          /* eski massiv o'z joyida saqlanib qoldi */
        o->a = b;
        o->sigim = yangi;
    }
    assert(o->soni < o->sigim);                 /* ichki qoida: bu yerda joy HAR DOIM bor (aks holda dastur xatosi) */
    o->a[o->soni++] = *x;
    return 0;
}

size_t ombor_soni(const struct ombor *o)
{
    return o->soni;
}

const struct xodim *ombor_ol(const struct ombor *o, size_t i)
{
    return i < o->soni ? &o->a[i] : NULL;
}

const struct xodim *ombor_top(const struct ombor *o, int id)
{
    for (size_t i = 0; i < o->soni; i++)
        if (o->a[i].id == id)
            return &o->a[i];
    return NULL;
}

struct xodim *ombor_top_yoz(struct ombor *o, int id)
{
    for (size_t i = 0; i < o->soni; i++)
        if (o->a[i].id == id)
            return &o->a[i];
    return NULL;
}

void ombor_saralash(struct ombor *o, int (*taqqoslash)(const void *, const void *))
{
    if (o->soni > 1)
        qsort(o->a, o->soni, sizeof(o->a[0]), taqqoslash);
}
