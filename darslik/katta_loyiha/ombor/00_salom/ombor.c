/* ombor.c - Ombor, 0-bosqich: mahsulotlar ro'yxatini chiroyli chiqarish */
#include <stdio.h>

int main(void)
{
    int non_narx = 4000, non_soni = 120;            /* narx so'mda */
    int sut_narx = 12000, sut_soni = 45;
    int guruch_narx = 18000, guruch_soni = 8;

    int jami = non_narx * non_soni + sut_narx * sut_soni + guruch_narx * guruch_soni;

    printf("============== OMBOR ===============\n");
    printf("%-10s %8s %6s %12s\n", "Mahsulot", "Narx", "Soni", "Summa");
    printf("------------------------------------\n");
    printf("%-10s %8d %6d %12d\n", "Non", non_narx, non_soni, non_narx * non_soni);
    printf("%-10s %8d %6d %12d\n", "Sut", sut_narx, sut_soni, sut_narx * sut_soni);
    printf("%-10s %8d %6d %12d\n", "Guruch", guruch_narx, guruch_soni, guruch_narx * guruch_soni);
    printf("------------------------------------\n");
    printf("%-24s %11d\n", "Jami qiymat:", jami);
    printf("%-24s %10d%%\n", "QQS stavkasi:", 12);
    printf("%-24s %11d\n", "QQS summasi:", jami * 12 / 100);
    return 0;
}
