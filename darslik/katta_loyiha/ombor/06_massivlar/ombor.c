/* ombor.c - Ombor, 6-bosqich: massivlar va satrlar - istalgancha (MAKS gacha) mahsulot, nom bo'yicha qidirish */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ombor_chop.h"

#define MAKS 8                                  /* eng ko'pi bilan nechta mahsulot */
#define NOM_UZ 24                               /* nom uchun joy (oxirgi '\0' bilan) */

enum { OK = 0, TOLA = -1, NOM_BAND = -2, TOPILMADI = -3, NOTOGRI_MIQDOR = -4, YETARLI_EMAS = -5 };

/* "parallel massivlar": i-mahsulotning ma'lumoti nom[i], narx[i], soni[i] da */
static char nom[MAKS][NOM_UZ];
static long narx[MAKS];
static uint16_t soni[MAKS];
static int n;                                   /* hozir nechta mahsulot bor */

/* nom bo'yicha qidiradi: indeksni yoki -1 ni qaytaradi */
static int topish(const char *qidirilgan)
{
    for (int i = 0; i < n; i++)
        if (strcmp(nom[i], qidirilgan) == 0)    /* satrlarni == bilan emas, strcmp bilan solishtiramiz */
            return i;
    return -1;
}

static int qosh(const char *yangi_nom, long yangi_narx, uint16_t yangi_soni)
{
    if (n == MAKS)
        return TOLA;
    if (topish(yangi_nom) >= 0)
        return NOM_BAND;
    snprintf(nom[n], NOM_UZ, "%s", yangi_nom);  /* uzun bo'lsa qirqiladi, '\0' doim qo'yiladi */
    narx[n] = yangi_narx;
    soni[n] = yangi_soni;
    n++;
    return OK;
}

static int sot(int i, int miqdor)
{
    if (miqdor <= 0)
        return NOTOGRI_MIQDOR;
    if (miqdor > soni[i])
        return YETARLI_EMAS;
    soni[i] -= miqdor;
    return OK;
}

static void royxat(void)
{
    long jami = 0;
    chop_sarlavha();
    for (int i = 0; i < n; i++) {
        chop_qator(nom[i], narx[i], soni[i]);
        jami += narx[i] * soni[i];
    }
    chop_jami(jami, 12);
}

static const char *xato_matni(int kod)
{
    switch (kod) {
    case TOLA: return "ombor to'lgan";
    case NOM_BAND: return "bunday nomli mahsulot allaqachon bor";
    case TOPILMADI: return "bunday mahsulot topilmadi";
    case NOTOGRI_MIQDOR: return "miqdor musbat bo'lishi kerak";
    case YETARLI_EMAS: return "omborda yetarli emas";
    default: return "noma'lum xato";
    }
}

static void qoshish_menyusi(void)
{
    char s[NOM_UZ];
    long so_m;
    int miqdor;
    printf("Nom, narx (so'mda) va soni?\n");
    if (scanf("%23s %ld %d", s, &so_m, &miqdor) != 3)   /* %23s: 23 belgidan ko'pini o'qimaydi */
        return;
    int r = qosh(s, so_m * 100, (uint16_t)miqdor);
    if (r == OK)
        printf("  Qo'shildi: %s\n", s);
    else
        printf("  XATO: %s\n", xato_matni(r));
}

static void sotish_menyusi(void)
{
    char s[NOM_UZ];
    int miqdor;
    printf("Mahsulot nomi va necha dona?\n");
    if (scanf("%23s %d", s, &miqdor) != 2)
        return;
    int i = topish(s);
    int r = i < 0 ? TOPILMADI : sot(i, miqdor);
    if (r == OK)
        printf("  Sotildi: %d dona %s. Qoldi: %u dona\n", miqdor, nom[i], soni[i]);
    else
        printf("  XATO: %s\n", xato_matni(r));
}

static void qidirish_menyusi(void)
{
    char s[NOM_UZ];
    printf("Nom boshlanishi?\n");
    if (scanf("%23s", s) != 1)
        return;
    int topildi = 0;
    for (int i = 0; i < n; i++)
        if (strncmp(nom[i], s, strlen(s)) == 0) {       /* faqat boshidagi belgilarni solishtiramiz */
            printf("  %s (%u dona)\n", nom[i], soni[i]);
            topildi++;
        }
    if (!topildi)
        printf("  hech narsa topilmadi\n");
}

int main(void)
{
    qosh("non", 400000, 120);
    qosh("sut", 1200000, 45);
    qosh("guruch", 1800000, 8);

    int tanlov;
    while (printf("\n1) ro'yxat  2) sotish  3) qo'shish  4) qidirish  0) chiqish\nTanlov:\n"),
           scanf("%d", &tanlov) == 1 && tanlov != 0) {
        switch (tanlov) {
        case 1: royxat(); break;
        case 2: sotish_menyusi(); break;
        case 3: qoshish_menyusi(); break;
        case 4: qidirish_menyusi(); break;
        default: printf("  XATO: menyuda bunday band yo'q\n");
        }
    }
    printf("\nXayr!\n");
    return 0;
}
