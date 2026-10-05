/* parser_zaif.c - ZAIF variant: ataylab xatolar bilan (hech qachon bunday yozmang!) */
#include <stdlib.h>
#include <string.h>

#include "parser.h"

int yozuv_oqi(const char *satr, struct yozuv *y)
{
    char nusxa[64];
    strcpy(nusxa, satr);                        /* XATO 1: satr 64 baytdan uzun bo'lsa stek buziladi */

    char *birinchi = strchr(nusxa, ':');
    if (!birinchi)
        return -1;
    *birinchi = '\0';
    strcpy(y->nom, nusxa);                      /* XATO 2: nom 15 belgidan uzun bo'lsa y->nom dan chiqib ketadi */

    char *ikkinchi = strchr(birinchi + 1, ':');
    if (!ikkinchi)
        return -1;
    *ikkinchi = '\0';
    y->narx = atoi(birinchi + 1);               /* XATO 3: atoi xatoni bildirmaydi, katta sonda UB */
    y->soni = atoi(ikkinchi + 1);
    return 0;
}

int yozuvlar_jami(const struct yozuv *y, size_t n, long *jami)
{
    int yig = 0;
    for (size_t i = 0; i < n; i++)
        yig += y[i].narx * y[i].soni;           /* XATO 4: int toshishi (aniqlanmagan xatti-harakat) */
    *jami = yig;
    return 0;
}
