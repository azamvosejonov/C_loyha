#include "bayroq.h"

static const char harflar[] = {
#define X(nom, harf, bit) [bit] = harf,
    BAYROQLAR(X)
#undef X
};

void bayroq_matn(uint8_t b, char *chiqish)
{
    for (size_t i = 0; i < sizeof(harflar); i++)
        chiqish[i] = (b >> i) & 1 ? harflar[i] : '-';
    chiqish[sizeof(harflar)] = '\0';
}

int bayroq_oqi(const char *matn, uint8_t *b)
{
    uint8_t natija = 0;
    for (; *matn; matn++) {
        size_t i = 0;
        while (i < sizeof(harflar) && harflar[i] != *matn)
            i++;
        if (i == sizeof(harflar))
            return -1;                          /* noma'lum harf */
        BAYROQ_YOQ(natija, 1u << i);
    }
    *b = natija;
    return 0;
}
