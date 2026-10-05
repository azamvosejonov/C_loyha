/* parser_xavfsiz.c - XAVFSIZ variant: har xatoga qarshi chora */
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

/* matnni butun songa o'giradi: faqat raqamlar, chegarada. 0 - OK */
static int son_oqi(const char *s, size_t uzunlik, int *natija)
{
    if (uzunlik == 0 || uzunlik > 9)            /* bo'sh yoki juda uzun: int ga sig'maydi */
        return -1;
    long v = 0;
    for (size_t i = 0; i < uzunlik; i++) {
        if (s[i] < '0' || s[i] > '9')
            return -1;                          /* raqam bo'lmagan belgi */
        v = v * 10 + (s[i] - '0');
    }
    *natija = (int)v;                           /* 9 raqam <= 999999999 < INT_MAX */
    return 0;
}

int yozuv_oqi(const char *satr, struct yozuv *y)
{
    const char *a = strchr(satr, ':');
    if (!a)
        return -1;
    const char *b = strchr(a + 1, ':');
    if (!b)
        return -1;

    size_t nom_uz = (size_t)(a - satr);
    if (nom_uz == 0 || nom_uz >= sizeof(y->nom))
        return -1;                              /* nom bo'sh yoki joyga sig'maydi */

    int narx, soni;
    if (son_oqi(a + 1, (size_t)(b - a - 1), &narx) != 0)
        return -1;
    const char *oxir = b + 1;
    if (son_oqi(oxir, strlen(oxir), &soni) != 0)
        return -1;

    memcpy(y->nom, satr, nom_uz);               /* uzunlik tekshirilgan: chegaradan chiqmaydi */
    y->nom[nom_uz] = '\0';
    y->narx = narx;
    y->soni = soni;
    return 0;
}

int yozuvlar_jami(const struct yozuv *y, size_t n, long *jami)
{
    long yig = 0;
    for (size_t i = 0; i < n; i++) {
        long p;
        if (__builtin_mul_overflow((long)y[i].narx, (long)y[i].soni, &p) || __builtin_add_overflow(yig, p, &yig))
            return -1;                          /* toshishni TEKSHIRIB aniqlaymiz */
    }
    *jami = yig;
    return 0;
}
