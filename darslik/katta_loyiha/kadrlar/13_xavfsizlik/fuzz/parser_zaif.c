/* parser_zaif.c - ATAYLAB ZAIF variant (hech qachon bunday yozmang!). Faqat fuzzer unga qarshi ishlashini ko'rsatish uchun. */
#include <stdio.h>
#include <string.h>

#include "yukla.h"

int xodim_tahlil_zaif(const char *qator, struct xodim *x, const char **sabab)
{
    char nusxa[64];
    strcpy(nusxa, qator);                       /* XATO 1: qator 64 baytdan uzun bo'lsa stek buziladi */

    int toifa;
    long long tarif;
    if (sscanf(nusxa, "%d %s %d %lld", &x->id, x->ism, &toifa, &tarif) != 4) {   /* XATO 2: %s uzunlik chegarasiz: ism[24] dan chiqib ketadi */
        *sabab = "format noto'g'ri";
        return -1;
    }
    x->toifa = (enum toifa)(toifa - 1);         /* XATO 3: toifa oralig'i tekshirilmagan */
    x->tarif = tarif;                           /* XATO 4: tarif musbatmi, juda kattami - tekshirilmagan */
    x->oddiy_daq = x->qosh_daq = 0;
    return 0;
}
