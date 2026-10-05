/* ombor_chop.c - chiqarish funksiyalarining TA'RIFLARI (tanasi shu yerda) */
#include <stdio.h>

#include "ombor_chop.h"

void chop_sarlavha(void)
{
    printf("============== OMBOR ===============\n");
    printf("%-10s %8s %6s %12s\n", "Mahsulot", "Narx", "Soni", "Summa");
    printf("------------------------------------\n");
}

void chop_qator(const char *nom, int narx, int soni)
{
    printf("%-10s %8d %6d %12d\n", nom, narx, soni, narx * soni);
}

void chop_jami(int jami, int qqs_foiz)
{
    printf("------------------------------------\n");
    printf("%-24s %11d\n", "Jami qiymat:", jami);
    printf("%-24s %10d%%\n", "QQS stavkasi:", qqs_foiz);
    printf("%-24s %11d\n", "QQS summasi:", jami * qqs_foiz / 100);
}
