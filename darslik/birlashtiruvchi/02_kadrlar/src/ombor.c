#include <stdio.h>
#include <string.h>

#include "matn.h"
#include "ombor.h"

void ombor_boshla(struct ombor *o)
{
    o->soni = 0;
}

int ombor_qosh(struct ombor *o, const struct xodim *x)
{
    if (o->soni >= MAKS_XODIM)
        return -1;
    if (ombor_top(o, x->id) != NULL)
        return -2;
    o->a[o->soni++] = *x;                   /* struct nusxalanadi */
    return 0;
}

struct xodim *ombor_top(struct ombor *o, int id)
{
    /* TODO T10: shu yerga yozing (massiv bo'ylab yuring; elementning MANZILINI (&) qaytaring) */
    (void)o;
    (void)id;
    return NULL;
}

struct xodim *ombor_ism_bilan_top(struct ombor *o, const char *ism)
{
    /* TODO T11: shu yerga yozing (nom_teng() dan foydalaning) */
    (void)o;
    (void)ism;
    return NULL;
}

void ombor_saralash_tarif(struct ombor *o)
{
    /* TODO T12: shu yerga yozing (ikki ichma-ich sikl; almashtirish: tmp = a; a = b; b = tmp) */
    (void)o;
}

int ombor_yukla(struct ombor *o, const char *fayl, int *rad)
{
    /* TODO T13: shu yerga yozing (fopen / fgets / sscanf / ombor_qosh / fclose). Hozircha hech narsa o'qimaydi. */
    (void)o;
    (void)fayl;
    (void)rad;
    return 0;
}
