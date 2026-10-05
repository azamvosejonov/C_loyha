/* ombor.c - Ombor, 3-bosqich: chegirma (arifmetika) va mahsulot holati (bitli bayroqlar) */
#include <stdint.h>
#include <stdio.h>

#include "ombor_chop.h"

/* mahsulot holati: har bir "kalit" - bitta bit (32 kalitli panel, ulardan 3 tasi ishlatilyapti) */
#define YANGI      (1u << 0)                    /* 0000 0001 */
#define CHEGIRMA   (1u << 1)                    /* 0000 0010 */
#define TUGAYAPTI  (1u << 2)                    /* 0000 0100 */

/* chegirmali narx: butun sonlarda, yaxlitlash pastga */
static long chegirmali(long narx, int foiz)
{
    return narx - narx * foiz / 100;
}

/* zaxraga qarab holatni yangilaydi: kam qolsa TUGAYAPTI bayrog'ini yoqamiz, ko'p bo'lsa o'chiramiz */
static unsigned holat_yangila(unsigned holat, uint16_t soni)
{
    if (soni < 10)
        holat |= TUGAYAPTI;                     /* yoqish: boshqa bitlarga tegmaydi */
    else
        holat &= ~TUGAYAPTI;                    /* o'chirish */
    return holat;
}

static void holat_chiqar(const char *nom, unsigned holat)
{
    printf("  %-8s holati: 0x%X (", nom, holat);
    printf("yangi:%s ", (holat & YANGI) ? "ha" : "yo'q");
    printf("chegirma:%s ", (holat & CHEGIRMA) ? "ha" : "yo'q");
    printf("tugayapti:%s)\n", (holat & TUGAYAPTI) ? "ha" : "yo'q");
}

int main(void)
{
    long non_narx = 400000, sut_narx = 1200000, guruch_narx = 1800000;     /* tiyinda */
    uint16_t non_soni = 120, sut_soni = 45, guruch_soni = 8;
    unsigned non_holat = YANGI, sut_holat = 0, guruch_holat = 0;           /* non yangi mahsulot */

    non_holat = holat_yangila(non_holat, non_soni);
    sut_holat = holat_yangila(sut_holat, sut_soni);
    guruch_holat = holat_yangila(guruch_holat, guruch_soni);

    printf("--- Dastlabki holat ---\n");
    holat_chiqar("Non", non_holat);
    holat_chiqar("Sut", sut_holat);
    holat_chiqar("Guruch", guruch_holat);

    printf("\n--- Guruchga 25%% chegirma e'lon qilindi ---\n");
    guruch_holat ^= CHEGIRMA;                   /* almashtirish: o'chiq edi -> yondi */
    if (guruch_holat & CHEGIRMA)
        guruch_narx = chegirmali(guruch_narx, 25);
    holat_chiqar("Guruch", guruch_holat);
    printf("  yangi narx: ");
    chop_pul(guruch_narx);
    printf(" so'm\n");

    printf("\n--- 30 kun o'tdi: non endi yangi emas ---\n");
    non_holat &= ~YANGI;                        /* faqat YANGI bitini o'chirish */
    holat_chiqar("Non", non_holat);

    printf("\n--- Ro'yxat ---\n");
    long jami = non_narx * non_soni + sut_narx * sut_soni + guruch_narx * guruch_soni;
    chop_sarlavha();
    chop_qator("Non", non_narx, non_soni);
    chop_qator("Sut", sut_narx, sut_soni);
    chop_qator("Guruch", guruch_narx, guruch_soni);
    chop_jami(jami, 12);

    printf("\nTiyin ostidagi qoldiq: 1234567 tiyin = %ld so'm va %ld tiyin\n", 1234567L / 100, 1234567L % 100);
    return 0;
}
