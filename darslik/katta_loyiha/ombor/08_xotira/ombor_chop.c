/* ombor_chop.c - chiqarish (o'zgarmadi) */
#include <stdio.h>

#include "ombor_chop.h"

void chop_pul(long tiyin)
{
    printf("%ld.%02ld", tiyin / 100, tiyin % 100);          /* butun qism va tiyin */
}

void chop_sarlavha(void)
{
    printf("================ OMBOR ================\n");
    printf("%-10s %10s %6s %14s\n", "Mahsulot", "Narx", "Soni", "Summa");
    printf("---------------------------------------\n");
}

void chop_qator(const char *nom, long narx, uint16_t soni)
{
    printf("%-10s %10.2f %6u %14.2f\n", nom, narx / 100.0, soni, (narx * soni) / 100.0);
}

void chop_jami(long jami, int qqs_foiz)
{
    printf("---------------------------------------\n");
    printf("%-27s ", "Jami qiymat:");
    chop_pul(jami);
    printf("\n%-27s %d%%\n", "QQS stavkasi:", qqs_foiz);
    printf("%-27s ", "QQS summasi:");
    chop_pul(jami * qqs_foiz / 100);
    printf("\n");
}
