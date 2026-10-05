/* ombor.c - Ombor, 4-bosqich: menyu (sikl) va tanlov (switch), sotish tekshiruvlari bilan */
#include <stdint.h>
#include <stdio.h>

#include "ombor_chop.h"

int main(void)
{
    long non_narx = 400000, sut_narx = 1200000, guruch_narx = 1800000;
    uint16_t non_soni = 120, sut_soni = 45, guruch_soni = 8;

    int tanlov;
    while (1) {                                         /* menyu: 0 tanlanguncha takrorlanadi */
        printf("\n1) ro'yxat   2) sotish   0) chiqish\nTanlov:\n");
        if (scanf("%d", &tanlov) != 1)                  /* son o'qilmasa (fayl tugadi) - chiqamiz */
            break;

        if (tanlov == 0)
            break;

        switch (tanlov) {
        case 1:
            chop_sarlavha();
            chop_qator("Non", non_narx, non_soni);
            chop_qator("Sut", sut_narx, sut_soni);
            chop_qator("Guruch", guruch_narx, guruch_soni);
            chop_jami(non_narx * non_soni + sut_narx * sut_soni + guruch_narx * guruch_soni, 12);
            break;

        case 2: {
            int id, miqdor;
            printf("Qaysi mahsulot (1-non, 2-sut, 3-guruch) va necha dona?\n");
            if (scanf("%d %d", &id, &miqdor) != 2)
                return 1;

            if (id < 1 || id > 3) {
                printf("  XATO: bunday mahsulot yo'q\n");
                break;
            }
            if (miqdor <= 0) {
                printf("  XATO: miqdor musbat bo'lishi kerak\n");
                break;
            }

            uint16_t mavjud;                            /* tanlangan mahsulotning zaxirasi */
            switch (id) {
            case 1: mavjud = non_soni; break;
            case 2: mavjud = sut_soni; break;
            default: mavjud = guruch_soni; break;
            }
            if (miqdor > mavjud) {
                printf("  XATO: omborda faqat %u dona bor\n", mavjud);
                break;
            }
            switch (id) {                               /* zaxirani kamaytiramiz */
            case 1: non_soni -= miqdor; break;
            case 2: sut_soni -= miqdor; break;
            default: guruch_soni -= miqdor; break;
            }
            printf("  Sotildi: %d dona. Qoldi: %u dona\n", miqdor, mavjud - miqdor);
            break;
        }

        default:
            printf("  XATO: menyuda bunday band yo'q\n");
        }
    }
    printf("\nXayr!\n");
    return 0;
}
