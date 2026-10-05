/* asm_lab.c - assembly funksiyalarini C dan chaqirish va C versiyalari bilan solishtirish */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* bular asm_funk.asm da yozilgan: bu yerda faqat E'LON (chaqirish qoidasi - System V) */
long yig_massiv(const int *a, long n);
long satr_uzunligi(const char *s);
int popcount64(unsigned long x);
unsigned bayt_almashtir32(unsigned x);

/* C dagi muqobillari (to'g'ri javob etaloni) */
static long yig_massiv_c(const int *a, long n)
{
    long s = 0;
    for (long i = 0; i < n; i++)
        s += a[i];
    return s;
}

static unsigned bayt_almashtir_c(unsigned x)
{
    return (x >> 24) | ((x >> 8) & 0xFF00u) | ((x << 8) & 0xFF0000u) | (x << 24);
}

static int tekshir(const char *nom, long asm_natija, long c_natija)
{
    int mos = asm_natija == c_natija;
    printf("  %-34s asm = %-12ld C = %-12ld %s\n", nom, asm_natija, c_natija, mos ? "MOS" : "FARQ!");
    return mos;
}

int main(void)
{
    int hammasi = 1;

    int a[] = { 5, -3, 100, 7, -250, 42 };
    int n = (int)(sizeof(a) / sizeof(a[0]));
    printf("1) yig_massiv:\n");
    hammasi &= tekshir("yig_massiv({5,-3,100,7,-250,42})", yig_massiv(a, n), yig_massiv_c(a, n));
    hammasi &= tekshir("yig_massiv(bo'sh massiv)", yig_massiv(a, 0), 0);
    int katta[1000];
    for (int i = 0; i < 1000; i++)
        katta[i] = i * 37 % 101 - 50;
    hammasi &= tekshir("yig_massiv(1000 ta element)", yig_massiv(katta, 1000), yig_massiv_c(katta, 1000));

    printf("2) satr_uzunligi:\n");
    const char *satrlar[] = { "", "a", "salom", "Operatsion tizim yadrosi" };
    for (int i = 0; i < 4; i++) {
        char nom[64];
        snprintf(nom, sizeof(nom), "satr_uzunligi(\"%s\")", satrlar[i]);
        hammasi &= tekshir(nom, satr_uzunligi(satrlar[i]), (long)strlen(satrlar[i]));
    }

    printf("3) popcount64 (yoniq bitlar):\n");
    unsigned long sinov[] = { 0, 1, 0x8000, 0xFF, 0xFFFFFFFFFFFFFFFFUL, 0x123456789ABCDEFUL };
    for (int i = 0; i < 6; i++) {
        char nom[64];
        snprintf(nom, sizeof(nom), "popcount64(0x%lX)", sinov[i]);
        hammasi &= tekshir(nom, popcount64(sinov[i]), __builtin_popcountl(sinov[i]));
    }

    printf("4) bayt_almashtir32 (endianness):\n");
    unsigned x = 0x12345678;
    unsigned r = bayt_almashtir32(x);
    printf("  0x%08X -> 0x%08X (asm), C versiyasi: 0x%08X, %s\n", x, r, bayt_almashtir_c(x),
           r == bayt_almashtir_c(x) ? "MOS" : "FARQ!");
    hammasi &= r == bayt_almashtir_c(x);

    printf("\nHammasi: %s\n", hammasi ? "TO'G'RI" : "XATO bor");
    return hammasi ? 0 : 1;
}
