/* ombor.c - Ombor, 5-bosqich: hamma ish funksiyalarga bo'lingan, xatolar kod bilan qaytariladi */
#include <stdint.h>
#include <stdio.h>

#include "ombor_chop.h"

/* sotish natijasi kodlari (0 - hammasi joyida, manfiy - xato; yadroda ham shunday) */
enum { OK = 0, YOQ_ID = -1, NOTOGRI_MIQDOR = -2, YETARLI_EMAS = -3 };

/* ombor ma'lumotlari: shu faylga tegishli (static), funksiyalar ularni ko'radi */
static long non_narx = 400000, sut_narx = 1200000, guruch_narx = 1800000;
static uint16_t non_soni = 120, sut_soni = 45, guruch_soni = 8;

/* --- "kirish" funksiyalari: id bo'yicha ma'lumotni olish va yozish --- */
static const char *nom_ol(int id)
{
    switch (id) {
    case 1: return "Non";
    case 2: return "Sut";
    case 3: return "Guruch";
    default: return "?";
    }
}

static uint16_t soni_ol(int id)
{
    switch (id) {
    case 1: return non_soni;
    case 2: return sut_soni;
    default: return guruch_soni;
    }
}

static void soni_yoz(int id, uint16_t yangi)
{
    switch (id) {
    case 1: non_soni = yangi; break;
    case 2: sut_soni = yangi; break;
    default: guruch_soni = yangi; break;
    }
}

/* --- asosiy ishlar --- */
static void royxat(void)
{
    chop_sarlavha();
    chop_qator("Non", non_narx, non_soni);
    chop_qator("Sut", sut_narx, sut_soni);
    chop_qator("Guruch", guruch_narx, guruch_soni);
    chop_jami(non_narx * non_soni + sut_narx * sut_soni + guruch_narx * guruch_soni, 12);
}

/* id mahsulotdan miqdor dona sotadi. Qaytaradi: OK yoki xato kodi */
static int sot(int id, int miqdor)
{
    if (id < 1 || id > 3)
        return YOQ_ID;
    if (miqdor <= 0)
        return NOTOGRI_MIQDOR;
    if (miqdor > soni_ol(id))
        return YETARLI_EMAS;
    soni_yoz(id, soni_ol(id) - miqdor);
    return OK;
}

static const char *xato_matni(int kod)
{
    switch (kod) {
    case YOQ_ID: return "bunday mahsulot yo'q";
    case NOTOGRI_MIQDOR: return "miqdor musbat bo'lishi kerak";
    case YETARLI_EMAS: return "omborda yetarli emas";
    default: return "noma'lum xato";
    }
}

static void sotish_menyusi(void)
{
    int id, miqdor;
    printf("Qaysi mahsulot (1-non, 2-sut, 3-guruch) va necha dona?\n");
    if (scanf("%d %d", &id, &miqdor) != 2)
        return;
    int r = sot(id, miqdor);
    if (r == OK)
        printf("  Sotildi: %d dona %s. Qoldi: %u dona\n", miqdor, nom_ol(id), soni_ol(id));
    else
        printf("  XATO: %s\n", xato_matni(r));
}

int main(void)
{
    int tanlov;
    while (printf("\n1) ro'yxat   2) sotish   0) chiqish\nTanlov:\n"), scanf("%d", &tanlov) == 1 && tanlov != 0) {
        if (tanlov == 1)
            royxat();
        else if (tanlov == 2)
            sotish_menyusi();
        else
            printf("  XATO: menyuda bunday band yo'q\n");
    }
    printf("\nXayr!\n");
    return 0;
}
