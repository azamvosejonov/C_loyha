#include <stdio.h>

#include "pul.h"

int64_t foiz(int64_t summa, int p)
{
    /* TODO T1: shu yerga yozing (butun sonlar, yaxlitlash) */
    (void)summa;
    (void)p;
    return 0;
}

int64_t pul_vaqt_haqi(int64_t tarif, int daqiqa)
{
    return (tarif * daqiqa + 30) / 60;      /* +30: yarmini qo'shib, 60 ga bo'lganda yaxlitlash */
}

void pul_chiqar(const char *yorliq, int64_t tiyin)
{
    printf("%-20s%12lld.%02lld so'm\n", yorliq, (long long)(tiyin / 100), (long long)(tiyin % 100));
}
