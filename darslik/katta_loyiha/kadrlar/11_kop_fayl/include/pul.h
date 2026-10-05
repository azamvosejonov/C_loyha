/* pul.h - hamma summa TIYINDA (1 so'm = 100 tiyin), turi int64_t */
#ifndef PUL_H
#define PUL_H

#include <stdint.h>

#define PUL_FMT "%lld.%02lld"
#define PUL_USTUN "%11lld.%02lld"               /* jadval ustuni: 14 belgi */
#define PUL_ARG(t) (long long)((t) / 100), (long long)((t) % 100)

int64_t foiz(int64_t summa, int p);             /* summaning p foizi, tiyinga yaxlitlab (.5 yuqoriga) */
int64_t pul_vaqt_haqi(int64_t tarif, int daqiqa);       /* tarif (tiyin/soat) * daqiqa / 60, yaxlitlab */

#endif
