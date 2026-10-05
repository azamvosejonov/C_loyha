#include <stdio.h>
#include <string.h>

#include "config.h"
#include "pul.h"

int64_t foiz(int64_t summa, int p)
{
    return (summa * p + 50) / 100;
}

/* toshishdan himoya: kirish chegaralari (config.h) bilan eng katta oraliq natija int64_t ga sig'ishi KOMPILYATSIYA vaqtida isbotlanadi */
_Static_assert(TARIF_MAKS <= INT64_MAX / OY_MAKS_DAQ / 2, "tarif * daqiqa * 3 int64_t dan oshib ketishi mumkin");

int64_t pul_vaqt_haqi(int64_t tarif, int daqiqa)
{
    return (tarif * daqiqa + 30) / 60;
}

int pul_matn(char *bufer, size_t hajm, int64_t tiyin)
{
    char raqam[32];
    int64_t som = tiyin / 100;
    int n = snprintf(raqam, sizeof(raqam), "%lld", (long long)som);        /* "1234567" */
    char chiq[48];
    int k = 0;
    for (int i = 0; i < n; i++) {
        if (i > 0 && (n - i) % 3 == 0)
            chiq[k++] = ' ';                    /* har uch raqamdan oldin bo'sh joy (oxiridan sanaganda) */
        chiq[k++] = raqam[i];
    }
    chiq[k] = '\0';
    return snprintf(bufer, hajm, "%s.%02lld", chiq, (long long)(tiyin % 100));
}
