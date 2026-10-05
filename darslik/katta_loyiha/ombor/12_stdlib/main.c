/* main.c - menyu (12-bosqich: saralash, saqlash, yuklash) */
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "ombor.h"

int main(void)
{
    Ombor *ombor = ombor_yarat();
    if (!ombor)
        return 1;
    ombor_qosh(ombor, "non", 400000, 120, OZIQ_OVQAT);
    ombor_qosh(ombor, "sut", 1200000, 45, ICHIMLIK);
    ombor_qosh(ombor, "guruch", 1800000, 8, OZIQ_OVQAT);

    int tanlov;
    while (printf("\n1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish\nTanlov:\n"),
           scanf("%d", &tanlov) == 1 && tanlov != 0) {
        char s[24];
        long so_m;
        int miqdor, toifa, r;
        uint16_t qoldi;
        switch (tanlov) {
        case 1:
            ombor_royxat(ombor);
            break;
        case 2:
            printf("Mahsulot nomi va necha dona?\n");
            if (scanf("%23s %d", s, &miqdor) != 2)
                break;
            r = ombor_sot(ombor, s, miqdor, &qoldi);
            if (r == OK)
                printf("  Sotildi: %d dona %s. Qoldi: %u dona\n", miqdor, s, qoldi);
            else
                printf("  XATO: %s\n", ombor_xato(r));
            break;
        case 3:
            printf("Nom, narx (so'mda), soni va toifa (0-oziq-ovqat, 1-ichimlik, 2-uy-ro'zg'or)?\n");
            if (scanf("%23s %ld %d %d", s, &so_m, &miqdor, &toifa) != 4)
                break;
            r = ombor_qosh(ombor, s, so_m * 100, (uint16_t)miqdor, (enum toifa)toifa);
            if (r == OK)
                printf("  Qo'shildi: %s (%s)\n", s, toifa_nomi((enum toifa)toifa));
            else
                printf("  XATO: %s\n", ombor_xato(r));
            break;
        case 4:
            printf("Qaysi mahsulot o'chirilsin?\n");
            if (scanf("%23s", s) != 1)
                break;
            r = ombor_ochir(ombor, s);
            if (r == OK)
                printf("  O'chirildi: %s (qoldi %d ta)\n", s, ombor_soni(ombor));
            else
                printf("  XATO: %s\n", ombor_xato(r));
            break;
        case 5:
            ombor_hisobot(ombor);
            break;
        case 6:
            printf("Saralash: 1-nom, 2-narx (qimmati birinchi)?\n");
            if (scanf("%d", &miqdor) != 1)
                break;
            ombor_saralash(ombor, miqdor == 2 ? SARALASH_NARX : SARALASH_NOM);
            printf("  Saralandi\n");
            break;
        case 7:
            printf("Qaysi faylga saqlansin?\n");
            if (scanf("%23s", s) != 1)
                break;
            if (ombor_saqla(ombor, s) == 0)
                printf("  Saqlandi: %s (%d ta mahsulot)\n", s, ombor_soni(ombor));
            else
                printf("  XATO: %s: %s\n", s, strerror(errno));
            break;
        case 8:
            printf("Qaysi fayldan yuklansin?\n");
            if (scanf("%23s", s) != 1)
                break;
            r = ombor_yukla(ombor, s);
            if (r >= 0)
                printf("  Yuklandi: %s (%d ta mahsulot)\n", s, r);
            else
                printf("  XATO: %s: %s\n", s, strerror(errno));
            break;
        default:
            printf("  XATO: menyuda bunday band yo'q\n");
        }
    }
    ombor_yoq(ombor);
    printf("\nXayr!\n");
    return 0;
}
