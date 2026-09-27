/* =============================================================================
 *  12_stdlib.c - printf formatlari, fayl bilan ishlash, errno, qsort, va_list (12-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 12_stdlib.c -o stdlib && ./stdlib
 *
 *  Kutilgan natija:
 *      [   42] [42   ] [00042] [+42] [ff FF 0xff] [0000beef] [sal] [3.14]
 *      faylga 3 qator yozildi, qaytadan o'qildi:
 *        1: birinchi qator
 *        2: ikkinchi qator
 *        3: uchinchi qator
 *      fopen("yoq_fayl.txt"): xato 2 - No such file or directory
 *      qsort: -7 -1 0 3 3 5 9
 *      yigindi(4, 10, 20, 30, 40) = 100
 *
 *  Sinab ko'ring:
 *      1) printf formatlarini o'zgartiring: %5d -> %10d, %.3s -> %.1s, %.2f -> %e.
 *      2) "w" ni "a" (qo'shib yozish) ga o'zgartiring, remove() ni o'chiring va dasturni 2 marta
 *         ishga tushiring. Ikkinchi safar necha qator o'qildi?
 *      3) taqqosla() ni teskari tartibda saralaydigan qiling.
 *      4) Massivga INT_MAX va -2 ni qo'shing (<limits.h>), taqqosla() ni `return x - y;` ga
 *         almashtiring va -fsanitize=undefined bilan yig'ing. Nega bu yozuv xavfli?
 *      5) `yigindi(4, 10, 20, 30)` - 4 deb aytib, 3 ta berish. Natija? (va_arg soni bilmaydi!)
 * ============================================================================= */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int taqqosla(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);                   /* x - y toshishi mumkin - shunday xavfsiz */
}

static int yigindi(int n, ...)                  /* o'zgaruvchan sonli argumentlar */
{
    va_list ap;
    va_start(ap, n);
    int s = 0;
    for (int i = 0; i < n; i++)
        s += va_arg(ap, int);
    va_end(ap);
    return s;
}

int main(void)
{
    printf("[%5d] [%-5d] [%05d] [%+d] [%x %X %#x] [%08lx] [%.3s] [%.2f]\n",
           42, 42, 42, 42, 255, 255, 255, 0xBEEFUL, "salom", 3.14159);

    FILE *f = fopen("12_vaqtincha.txt", "w");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fprintf(f, "birinchi qator\nikkinchi qator\nuchinchi qator\n");
    fclose(f);                                  /* bufer diskka yoziladi */

    f = fopen("12_vaqtincha.txt", "r");
    char qator[128];
    int n = 0;
    printf("faylga 3 qator yozildi, qaytadan o'qildi:\n");
    while (fgets(qator, sizeof(qator), f)) {
        qator[strcspn(qator, "\n")] = '\0';     /* oxiridagi \n ni olib tashlash */
        printf("  %d: %s\n", ++n, qator);
    }
    fclose(f);
    remove("12_vaqtincha.txt");

    if (!fopen("yoq_fayl.txt", "r"))
        printf("fopen(\"yoq_fayl.txt\"): xato %d - %s\n", errno, strerror(errno));

    int a[] = { 5, -1, 9, 3, 0, -7, 3 };
    qsort(a, 7, sizeof(a[0]), taqqosla);
    printf("qsort:");
    for (int i = 0; i < 7; i++)
        printf(" %d", a[i]);
    printf("\n");

    printf("yigindi(4, 10, 20, 30, 40) = %d\n", yigindi(4, 10, 20, 30, 40));
    return 0;
}
