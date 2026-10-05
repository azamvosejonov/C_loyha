/* float_lab.c - IEEE 754 laboratoriyasi: bitlarni ajratish, yaxlitlash xatosi, ULP, yig'ish aniqligi */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* float ning 32 bitini butun songa "o'qiymiz" (turni o'zgartirmasdan: memcpy - Asos bob, A.7) */
static uint32_t bitlar(float f)
{
    uint32_t b;
    memcpy(&b, &f, sizeof(b));
    return b;
}

static void ikkilik(uint32_t v, int bit_soni)
{
    for (int i = bit_soni - 1; i >= 0; i--)
        putchar((v >> i) & 1 ? '1' : '0');
}

/* float ni ishora (1 bit) | daraja (8 bit) | mantissa (23 bit) ga ajratib tushuntiradi */
static void ajrat(float f)
{
    uint32_t b = bitlar(f);
    uint32_t ishora = b >> 31;
    uint32_t daraja = (b >> 23) & 0xFF;
    uint32_t mantissa = b & 0x7FFFFF;

    printf("%-14g 0x%08X  ", f, b);
    printf("%u ", ishora);
    ikkilik(daraja, 8);
    putchar(' ');
    ikkilik(mantissa, 23);

    const char *tur;
    if (daraja == 0xFF)
        tur = mantissa ? "NaN (son emas)" : (ishora ? "-cheksizlik" : "+cheksizlik");
    else if (daraja == 0)
        tur = mantissa ? "denormal (juda kichik)" : "nol";
    else
        tur = "oddiy";
    printf("  %s", tur);
    if (daraja != 0 && daraja != 0xFF)
        printf(", 2^%d * 1.%06X(hex)", (int)daraja - 127, mantissa << 1);
    printf("\n");
}

/* ikki float orasida nechta "qadam" (ULP) bor: bitlarni tartibli sonlar kabi ayiramiz */
static long ulp_farq(float a, float b)
{
    int32_t x = (int32_t)bitlar(a), y = (int32_t)bitlar(b);
    return (long)y - (long)x;
}

/* oddiy yig'ish va Kahan (xatoni eslab qoluvchi) yig'ish */
static float oddiy_yigindi(float qiymat, long n)
{
    float s = 0;
    for (long i = 0; i < n; i++)
        s += qiymat;
    return s;
}

static float kahan_yigindi(float qiymat, long n)
{
    float s = 0, tuzatish = 0;                  /* tuzatish - oldingi qadamlarda yo'qotilgan qism */
    for (long i = 0; i < n; i++) {
        float y = qiymat - tuzatish;
        float t = s + y;
        tuzatish = (t - s) - y;                 /* (t - s) - y: qo'shishda yo'qotilgan xato */
        s = t;
    }
    return s;
}

int main(void)
{
    printf("1) float bitlari: ishora | daraja(8) | mantissa(23)\n");
    float sinov[] = { 1.0f, 0.5f, -2.5f, 0.1f, 16777216.0f, 1e-40f, INFINITY, NAN };
    for (int i = 0; i < 8; i++) {
        printf("  ");
        ajrat(sinov[i]);
    }

    printf("\n2) 0.1 aniq saqlanmaydi:\n");
    printf("  0.1f           = %.20f\n", (double)0.1f);
    printf("  0.1  (double)  = %.20f\n", 0.1);
    printf("  0.1 + 0.2      = %.20f\n", 0.1 + 0.2);
    printf("  0.3            = %.20f\n", 0.3);
    printf("  0.1 + 0.2 == 0.3 ? %s\n", 0.1 + 0.2 == 0.3 ? "ha" : "YO'Q");
    printf("  |farq| < 1e-9 ?    %s   (to'g'ri taqqoslash: epsilon bilan)\n", fabs(0.1 + 0.2 - 0.3) < 1e-9 ? "ha" : "yo'q");

    printf("\n3) katta sonlarda butun sonlar ham yo'qoladi (float 24 bit mantissa):\n");
    float f = 16777216.0f;                      /* 2^24 */
    printf("  16777216 + 1 = %.0f (float'da 1 qo'shilmadi!)\n", f + 1.0f);
    printf("  16777216 + 2 = %.0f\n", f + 2.0f);

    printf("\n4) ULP: qo'shni float'lar orasidagi masofa\n");
    printf("  1.0f dan keyingi float: %.10f (nextafterf), ULP farqi: %ld\n", nextafterf(1.0f, 2.0f),
           ulp_farq(1.0f, nextafterf(1.0f, 2.0f)));
    printf("  1.0f va 1.000001f orasida %ld ta float bor\n", ulp_farq(1.0f, 1.000001f) - 1);
    printf("  float epsilon (1.0 dan keyingi qadam): %g\n", (double)(nextafterf(1.0f, 2.0f) - 1.0f));

    printf("\n5) 0.1 ni 10 million marta qo'shish (to'g'ri javob: 1000000):\n");
    long n = 10000000;
    printf("  oddiy yig'ish: %.1f   (xato: %.1f)\n", (double)oddiy_yigindi(0.1f, n), 1000000.0 - (double)oddiy_yigindi(0.1f, n));
    printf("  Kahan yig'ish: %.1f   (xato: %.1f)\n", (double)kahan_yigindi(0.1f, n), 1000000.0 - (double)kahan_yigindi(0.1f, n));
    return 0;
}
