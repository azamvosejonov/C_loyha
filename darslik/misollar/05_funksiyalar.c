/* =============================================================================
 *  05_funksiyalar.c - nusxa bo'yicha uzatish, ko'rsatkich, static   (5-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 05_funksiyalar.c -o funksiyalar && ./funksiyalar
 *
 *  Kutilgan natija:
 *      nusxa bilan: a = 5 (o'zgarmadi)
 *      manzil bilan: a = 6 (o'zgardi)
 *      chaqiruv 1, chaqiruv 2, chaqiruv 3
 *      min = -2, max = 9
 *      5! = 120
 *
 *  Sinab ko'ring:
 *      1) oshir_nusxa() ichiga `printf("ichkarida x = %d\n", x);` qo'shing: ichkarida 6, tashqarida 5.
 *      2) hisobla() dagi `static` so'zini o'chiring. Nima chiqadi va nega?
 *      3) faktorial(20) va faktorial(21) ni chop eting. 21! nega noto'g'ri? (unsigned long 64 bit -
 *         2-bob, toshish.)
 *      4) min_max(m, 0, ...) chaqirilsa nima bo'ladi? Funksiyani qanday himoya qilasiz?
 * ============================================================================= */
#include <stdio.h>

static void oshir_nusxa(int x)              /* x - chaqiruvchining NUSXASI */
{
    x = x + 1;
}

static void oshir_manzil(int *x)            /* x - chaqiruvchi o'zgaruvchisining MANZILI */
{
    *x = *x + 1;
}

static void hisobla(void)
{
    static int marta = 0;                   /* chaqiruvlar orasida saqlanadi */
    marta++;
    printf("%schaqiruv %d", marta > 1 ? ", " : "", marta);
}

/* Ikki natija: qaytish qiymati emas, chiqish parametrlari orqali */
static void min_max(const int *a, int n, int *mn, int *mx)
{
    *mn = *mx = a[0];
    for (int i = 1; i < n; i++) {
        if (a[i] < *mn)
            *mn = a[i];
        if (a[i] > *mx)
            *mx = a[i];
    }
}

static unsigned long faktorial(unsigned n)
{
    return n <= 1 ? 1 : n * faktorial(n - 1);
}

int main(void)
{
    int a = 5;
    oshir_nusxa(a);
    printf("nusxa bilan: a = %d (o'zgarmadi)\n", a);
    oshir_manzil(&a);
    printf("manzil bilan: a = %d (o'zgardi)\n", a);

    hisobla();
    hisobla();
    hisobla();
    printf("\n");

    int m[] = { 4, 9, -2, 7 }, mn, mx;
    min_max(m, 4, &mn, &mx);
    printf("min = %d, max = %d\n", mn, mx);
    printf("5! = %lu\n", faktorial(5));
    return 0;
}
