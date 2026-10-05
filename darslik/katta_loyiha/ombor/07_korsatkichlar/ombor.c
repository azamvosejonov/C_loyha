/* ombor.c - Ombor, 7-bosqich: ko'rsatkichlar - zaxirani manzil bo'yicha o'zgartirish, saralash, eng qimmatni topish */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ombor_chop.h"

#define MAKS 8
#define NOM_UZ 24

enum { OK = 0, TOLA = -1, NOM_BAND = -2, TOPILMADI = -3, NOTOGRI_MIQDOR = -4, YETARLI_EMAS = -5 };

static char nom[MAKS][NOM_UZ];
static long narx[MAKS];
static uint16_t soni[MAKS];
static int n;

static int topish(const char *qidirilgan)
{
    for (int i = 0; i < n; i++)
        if (strcmp(nom[i], qidirilgan) == 0)
            return i;
    return -1;
}

static int qosh(const char *yangi_nom, long yangi_narx, uint16_t yangi_soni)
{
    if (n == MAKS)
        return TOLA;
    if (topish(yangi_nom) >= 0)
        return NOM_BAND;
    snprintf(nom[n], NOM_UZ, "%s", yangi_nom);
    narx[n] = yangi_narx;
    soni[n] = yangi_soni;
    n++;
    return OK;
}

/* zaxira - o'zgartiriladigan katakning MANZILI: funksiya shu katakka borib yozadi */
static int sot(uint16_t *zaxira, int miqdor)
{
    if (miqdor <= 0)
        return NOTOGRI_MIQDOR;
    if (miqdor > *zaxira)                       /* *zaxira - manzil ko'rsatgan joydagi qiymat */
        return YETARLI_EMAS;
    *zaxira -= miqdor;
    return OK;
}

/* massivdagi eng katta narxning MANZILINI qaytaradi (bo'sh bo'lsa NULL) */
static const long *eng_qimmat(const long *a, int soni_a)
{
    if (soni_a == 0)
        return NULL;
    const long *eng = a;
    for (const long *p = a + 1; p < a + soni_a; p++)    /* p++ - keyingi elementga o'tish */
        if (*p > *eng)
            eng = p;
    return eng;
}

static long jami_qiymat(const long *a_narx, const uint16_t *a_soni, int soni_a)
{
    long jami = 0;
    for (int i = 0; i < soni_a; i++)
        jami += *(a_narx + i) * *(a_soni + i);          /* *(a + i) xuddi a[i] */
    return jami;
}

static void almashtir_long(long *a, long *b)
{
    long t = *a;
    *a = *b;
    *b = t;
}

static void almashtir_soni(uint16_t *a, uint16_t *b)
{
    uint16_t t = *a;
    *a = *b;
    *b = t;
}

static void almashtir_nom(char *a, char *b)
{
    char t[NOM_UZ];
    strcpy(t, a);
    strcpy(a, b);
    strcpy(b, t);
}

/* narx bo'yicha kamayish tartibida (qimmati birinchi): "pufakcha" saralash */
static void narx_boyicha_saralash(void)
{
    for (int o = 0; o < n - 1; o++)
        for (int i = 0; i < n - 1 - o; i++)
            if (narx[i] < narx[i + 1]) {
                almashtir_long(&narx[i], &narx[i + 1]);         /* uchta massivni birga almashtirish kerak */
                almashtir_soni(&soni[i], &soni[i + 1]);
                almashtir_nom(nom[i], nom[i + 1]);
            }
}

static void royxat(void)
{
    chop_sarlavha();
    for (int i = 0; i < n; i++)
        chop_qator(nom[i], narx[i], soni[i]);
    chop_jami(jami_qiymat(narx, soni, n), 12);
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
    if (scanf("%23s %ld %d", s, &so_m, &miqdor) != 3)
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
    int r = i < 0 ? TOPILMADI : sot(&soni[i], miqdor);      /* &soni[i] - i-katakning manzili */
    if (r == OK)
        printf("  Sotildi: %d dona %s. Qoldi: %u dona\n", miqdor, nom[i], soni[i]);
    else
        printf("  XATO: %s\n", xato_matni(r));
}

static void hisobot(void)
{
    const long *q = eng_qimmat(narx, n);
    if (!q) {
        printf("  ombor bo'sh\n");
        return;
    }
    int i = (int)(q - narx);                    /* ikki ko'rsatkich ayirmasi = elementlar soni */
    printf("  Eng qimmat: %s (", nom[i]);
    chop_pul(*q);
    printf(" so'm)\n  Jami qiymat: ");
    chop_pul(jami_qiymat(narx, soni, n));
    printf(" so'm\n");
}

int main(void)
{
    qosh("non", 400000, 120);
    qosh("sut", 1200000, 45);
    qosh("guruch", 1800000, 8);

    int tanlov;
    while (printf("\n1) ro'yxat  2) sotish  3) qo'shish  4) saralash  5) hisobot  0) chiqish\nTanlov:\n"),
           scanf("%d", &tanlov) == 1 && tanlov != 0) {
        switch (tanlov) {
        case 1: royxat(); break;
        case 2: sotish_menyusi(); break;
        case 3: qoshish_menyusi(); break;
        case 4: narx_boyicha_saralash(); printf("  Narx bo'yicha saralandi\n"); break;
        case 5: hisobot(); break;
        default: printf("  XATO: menyuda bunday band yo'q\n");
        }
    }
    printf("\nXayr!\n");
    return 0;
}
