#include <stdio.h>
#include <stdlib.h>

#include "eksport.h"

char *eksport_csv(const struct ombor *o, size_t *uzunlik)
{
    size_t sigim = 128, n = 0;
    char *bufer = malloc(sigim);
    if (!bufer)
        return NULL;

    for (size_t i = 0; i < ombor_soni(o); i++) {
        const struct xodim *x = ombor_ol(o, i);
        struct natija r;
        xodim_hisobla(x, &r);

        char qator[128];
        int k = snprintf(qator, sizeof(qator), "%d,%s,%d,%lld,%lld,%lld\n", x->id, x->ism, (int)x->toifa + 1,
                         (long long)r.brutto, (long long)r.soliq, (long long)r.sof);
        if (k < 0 || (size_t)k >= sizeof(qator)) {      /* snprintf: sig'masa kerak bo'lgan uzunlikni qaytaradi */
            free(bufer);
            return NULL;
        }
        while (n + (size_t)k > sigim) {         /* joy yetmasa, ikki barobar kattalashtiramiz */
            char *yangi = realloc(bufer, sigim * 2);
            if (!yangi) {
                free(bufer);
                return NULL;
            }
            bufer = yangi;
            sigim *= 2;
        }
        for (int j = 0; j < k; j++)
            bufer[n++] = qator[j];
    }
    *uzunlik = n;
    return bufer;
}
