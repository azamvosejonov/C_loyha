/* =============================================================================
 *  20_sonlar.c - ikkiga to'ldirish, ishora kengayishi, float bitlari  (20-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 20_sonlar.c -o sonlar && ./sonlar
 *
 *  Kutilgan natija:
 *      -5 (int8) bitlari: 11111011, ~5 + 1 = -5
 *      int8 -5 -> int32: -5 (0xfffffffb), uint8 251 -> uint32: 251 (0x000000fb)
 *      300 -> int8 qirqish: 44
 *      -1 va 1u taqqoslash: -1 < 1u yolg'on! (-1 -> 4294967295)
 *        6.5 = 0x40d00000: ishora 0, e 129 (daraja 2), mantissa 0x500000
 *        0.1 = 0x3dcccccd: ishora 0, e 123 (daraja -4), mantissa 0x4ccccd
 *       -1.0 = 0xbf800000: ishora 1, e 127 (daraja 0), mantissa 0x000000
 *      0.1 + 0.2 == 0.3 ? yo'q (0.30000000000000004)
 *      (float)16777217 = 16777216 (aniqlik yetmadi)
 *      yuklama: 12.34%  (butun sonlar bilan, float'siz)
 *
 *  Sinab ko'ring:
 *      1) float_korsat(0.5f), float_korsat(1.0f / 0.0f) (cheksizlik: e = 255, mantissa 0)
 *         va float_korsat(0.0f / 0.0f) (NaN) ni qo'shing.
 *      2) (int8_t)200 nechaga teng? Avval hisoblang: 200 - 256.
 *      3) `if (m1 < u1)` ni to'g'ridan-to'g'ri yozing. -Wextra ogohlantirishini o'qing.
 *      4) band = 500000, jami = 1000000 qiling. 50% chiqishi kerak edi - nega 7.05%?
 *         (band * 10000 unsigned'ga sig'madi.) Qanday tuzatasiz?
 * ============================================================================= */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void bitlar8(int8_t x)
{
    for (int i = 7; i >= 0; i--)
        putchar(((uint8_t)x >> i) & 1 ? '1' : '0');
}

static void float_korsat(float f)
{
    uint32_t u;
    memcpy(&u, &f, 4);                          /* float'ning baytlarini butun son sifatida olish */
    unsigned s = u >> 31, e = (u >> 23) & 0xFF, m = u & 0x7FFFFF;
    printf("%6.1f = 0x%08x: ishora %u, e %u (daraja %d), mantissa 0x%06x\n",
           (double)f, u, s, e, (int)e - 127, m);
}

int main(void)
{
    int8_t a = -5;
    printf("-5 (int8) bitlari: ");
    bitlar8(a);
    printf(", ~5 + 1 = %d\n", ~5 + 1);

    int32_t kengaydi = a;                        /* ishora kengayishi */
    uint8_t b = 251;
    uint32_t nol_bilan = b;                      /* nol bilan kengayish */
    printf("int8 -5 -> int32: %d (0x%08x), uint8 251 -> uint32: %u (0x%08x)\n",
           kengaydi, (uint32_t)kengaydi, nol_bilan, nol_bilan);
    printf("300 -> int8 qirqish: %d\n", (int8_t)300);

    int m1 = -1;
    unsigned u1 = 1;
    printf("-1 va 1u taqqoslash: -1 < 1u %s! (-1 -> %u)\n",
           ((unsigned)m1 < u1) ? "rost" : "yolg'on", (unsigned)m1);

    float_korsat(6.5f);
    float_korsat(0.1f);
    float_korsat(-1.0f);

    double x = 0.1 + 0.2;
    printf("0.1 + 0.2 == 0.3 ? %s (%.17g)\n", x == 0.3 ? "ha" : "yo'q", x);
    printf("(float)16777217 = %.0f (aniqlik yetmadi)\n", (double)(float)16777217);

    unsigned band = 1234, jami = 10000;
    unsigned x100 = band * 10000 / jami;         /* qat'iy nuqta: 2 xona aniqlik */
    printf("yuklama: %u.%02u%%  (butun sonlar bilan, float'siz)\n", x100 / 100, x100 % 100);
    return 0;
}
