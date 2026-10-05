/* ombor.c - Ombor, 8-bosqich: xotira - massivlar o'zi kengayadi (malloc/realloc), hammasi free qilinadi */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ombor_chop.h"

enum { OK = 0, XOTIRA_YOQ = -1, NOM_BAND = -2, TOPILMADI = -3, NOTOGRI_MIQDOR = -4, YETARLI_EMAS = -5 };

/* HEAP dagi massivlar: boshida bo'sh (NULL), kerak bo'lsa kengayadi */
static char **nom;                              /* nomlar: har biri alohida ajratilgan satr */
static long *narx;
static uint16_t *soni;
static int n;                                   /* nechta mahsulot bor */
static int sig;                                 /* nechtaga joy ajratilgan (sig'im) */

/* sig'imni 2 barobar oshiradi (birinchi marta 2 ga). 0 - muvaffaqiyat */
static int sigim_oshir(void)
{
    int yangi = sig ? sig * 2 : 2;
    char **a = realloc(nom, (size_t)yangi * sizeof(*a));
    if (!a)
        return XOTIRA_YOQ;
    nom = a;                                    /* eskisi realloc ichida bo'shatildi yoki ko'chirildi */
    long *b = realloc(narx, (size_t)yangi * sizeof(*b));
    if (!b)
        return XOTIRA_YOQ;
    narx = b;
    uint16_t *c = realloc(soni, (size_t)yangi * sizeof(*c));
    if (!c)
        return XOTIRA_YOQ;
    soni = c;
    printf("  [xotira] sig'im %d -> %d\n", sig, yangi);
    sig = yangi;
    return OK;
}

static int topish(const char *qidirilgan)
{
    for (int i = 0; i < n; i++)
        if (strcmp(nom[i], qidirilgan) == 0)
            return i;
    return -1;
}

static int qosh(const char *yangi_nom, long yangi_narx, uint16_t yangi_soni)
{
    if (topish(yangi_nom) >= 0)
        return NOM_BAND;
    if (n == sig && sigim_oshir() != OK)
        return XOTIRA_YOQ;
    char *nusxa = strdup(yangi_nom);            /* nomning o'z nusxasi: malloc + strcpy */
    if (!nusxa)
        return XOTIRA_YOQ;
    nom[n] = nusxa;
    narx[n] = yangi_narx;
    soni[n] = yangi_soni;
    n++;
    return OK;
}

static void ochir(int i)
{
    free(nom[i]);                               /* nom nusxasini qaytaramiz */
    memmove(&nom[i], &nom[i + 1], (size_t)(n - i - 1) * sizeof(*nom));      /* qolganlarni chapga suramiz */
    memmove(&narx[i], &narx[i + 1], (size_t)(n - i - 1) * sizeof(*narx));
    memmove(&soni[i], &soni[i + 1], (size_t)(n - i - 1) * sizeof(*soni));
    n--;
}

static void hammasini_qaytar(void)
{
    for (int i = 0; i < n; i++)
        free(nom[i]);
    free(nom);
    free(narx);
    free(soni);
    nom = NULL;
    narx = NULL;
    soni = NULL;
    n = sig = 0;
}

static int sot(uint16_t *zaxira, int miqdor)
{
    if (miqdor <= 0)
        return NOTOGRI_MIQDOR;
    if (miqdor > *zaxira)
        return YETARLI_EMAS;
    *zaxira -= miqdor;
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
    case XOTIRA_YOQ: return "xotira yetmadi";
    case NOM_BAND: return "bunday nomli mahsulot allaqachon bor";
    case TOPILMADI: return "bunday mahsulot topilmadi";
    case NOTOGRI_MIQDOR: return "miqdor musbat bo'lishi kerak";
    case YETARLI_EMAS: return "omborda yetarli emas";
    default: return "noma'lum xato";
    }
}

int main(void)
{
    qosh("non", 400000, 120);
    qosh("sut", 1200000, 45);
    qosh("guruch", 1800000, 8);

    int tanlov;
    while (printf("\n1) ro'yxat  2) sotish  3) qo'shish  4) o'chirish  0) chiqish\nTanlov:\n"),
           scanf("%d", &tanlov) == 1 && tanlov != 0) {
        char s[24];
        long so_m;
        int miqdor, i, r;
        switch (tanlov) {
        case 1:
            royxat();
            break;
        case 2:
            printf("Mahsulot nomi va necha dona?\n");
            if (scanf("%23s %d", s, &miqdor) != 2)
                break;
            i = topish(s);
            r = i < 0 ? TOPILMADI : sot(&soni[i], miqdor);
            if (r == OK)
                printf("  Sotildi: %d dona %s. Qoldi: %u dona\n", miqdor, nom[i], soni[i]);
            else
                printf("  XATO: %s\n", xato_matni(r));
            break;
        case 3:
            printf("Nom, narx (so'mda) va soni?\n");
            if (scanf("%23s %ld %d", s, &so_m, &miqdor) != 3)
                break;
            r = qosh(s, so_m * 100, (uint16_t)miqdor);
            if (r == OK)
                printf("  Qo'shildi: %s (jami %d ta)\n", s, n);
            else
                printf("  XATO: %s\n", xato_matni(r));
            break;
        case 4:
            printf("Qaysi mahsulot o'chirilsin?\n");
            if (scanf("%23s", s) != 1)
                break;
            i = topish(s);
            if (i < 0) {
                printf("  XATO: %s\n", xato_matni(TOPILMADI));
            } else {
                ochir(i);
                printf("  O'chirildi: %s (qoldi %d ta)\n", s, n);
            }
            break;
        default:
            printf("  XATO: menyuda bunday band yo'q\n");
        }
    }
    hammasini_qaytar();                         /* chiqishdan oldin hamma xotirani qaytarish */
    printf("\nXayr!\n");
    return 0;
}
