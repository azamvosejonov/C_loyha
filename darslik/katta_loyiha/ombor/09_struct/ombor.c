/* ombor.c - Ombor, 9-bosqich: struct, enum, typedef - bitta mahsulotning hamma ma'lumoti bitta tuzilmada */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ombor_chop.h"

/* mahsulot toifasi: nomlangan butun sonlar */
enum toifa { OZIQ_OVQAT, ICHIMLIK, UY_RUZGOR, TOIFA_SONI };
static const char *toifa_nomi[TOIFA_SONI] = { "oziq-ovqat", "ichimlik", "uy-ro'zg'or" };

#define YANGI      (1u << 0)
#define TUGAYAPTI  (1u << 2)

/* BITTA MAHSULOT: avval 4 ta parallel massiv edi, endi bitta tuzilma */
struct mahsulot {
    char *nom;
    long narx;                                  /* tiyinda */
    uint16_t soni;
    enum toifa toifa;
    unsigned holat;                             /* bitli bayroqlar (3-bob) */
};

struct ombor {
    struct mahsulot *m;                         /* heap dagi massiv */
    int n;                                      /* nechta */
    int sig;                                    /* sig'im */
};
typedef struct ombor Ombor;                     /* endi "struct ombor" o'rniga "Ombor" desak bo'ladi */

enum { OK = 0, XOTIRA_YOQ = -1, NOM_BAND = -2, TOPILMADI = -3, NOTOGRI_MIQDOR = -4, YETARLI_EMAS = -5, NOTOGRI_TOIFA = -6 };

static struct mahsulot *ombor_topish(Ombor *o, const char *nom)
{
    for (int i = 0; i < o->n; i++)
        if (strcmp(o->m[i].nom, nom) == 0)
            return &o->m[i];                    /* tuzilmaning MANZILI */
    return NULL;
}

static void holat_yangila(struct mahsulot *p)
{
    if (p->soni < 10)
        p->holat |= TUGAYAPTI;
    else
        p->holat &= ~TUGAYAPTI;
}

static int ombor_qosh(Ombor *o, const char *nom, long narx, uint16_t soni, enum toifa toifa)
{
    if (toifa < 0 || toifa >= TOIFA_SONI)
        return NOTOGRI_TOIFA;
    if (ombor_topish(o, nom))
        return NOM_BAND;
    if (o->n == o->sig) {
        int yangi = o->sig ? o->sig * 2 : 2;
        struct mahsulot *y = realloc(o->m, (size_t)yangi * sizeof(*y));
        if (!y)
            return XOTIRA_YOQ;
        o->m = y;
        o->sig = yangi;
    }
    char *nusxa = strdup(nom);
    if (!nusxa)
        return XOTIRA_YOQ;
    struct mahsulot *p = &o->m[o->n++];
    p->nom = nusxa;
    p->narx = narx;
    p->soni = soni;
    p->toifa = toifa;
    p->holat = YANGI;                           /* yangi qo'shilgan mahsulot */
    holat_yangila(p);
    return OK;
}

static int ombor_sot(struct mahsulot *p, int miqdor)
{
    if (miqdor <= 0)
        return NOTOGRI_MIQDOR;
    if (miqdor > p->soni)
        return YETARLI_EMAS;
    p->soni -= miqdor;
    holat_yangila(p);                           /* zaxira kamaydi: holat o'zgarishi mumkin */
    return OK;
}

static void ombor_tozala(Ombor *o)
{
    for (int i = 0; i < o->n; i++)
        free(o->m[i].nom);
    free(o->m);
    o->m = NULL;
    o->n = o->sig = 0;
}

static void ombor_royxat(const Ombor *o)
{
    printf("%-10s %-12s %10s %6s %s\n", "Mahsulot", "Toifa", "Narx", "Soni", "Holat");
    printf("----------------------------------------------------\n");
    long jami = 0;
    for (int i = 0; i < o->n; i++) {
        const struct mahsulot *p = &o->m[i];
        printf("%-10s %-12s %10.2f %6u %s%s\n", p->nom, toifa_nomi[p->toifa], p->narx / 100.0, p->soni,
               (p->holat & YANGI) ? "[yangi] " : "", (p->holat & TUGAYAPTI) ? "[TUGAYAPTI]" : "");
        jami += p->narx * p->soni;
    }
    chop_jami(jami, 12);
}

/* toifalar bo'yicha jami qiymat: enum massiv indeksi bo'lib xizmat qiladi */
static void ombor_hisobot(const Ombor *o)
{
    long toifa_jami[TOIFA_SONI] = { 0 };
    for (int i = 0; i < o->n; i++)
        toifa_jami[o->m[i].toifa] += o->m[i].narx * o->m[i].soni;
    for (int t = 0; t < TOIFA_SONI; t++) {
        printf("  %-12s ", toifa_nomi[t]);
        chop_pul(toifa_jami[t]);
        printf(" so'm\n");
    }
}

static const char *xato_matni(int kod)
{
    switch (kod) {
    case XOTIRA_YOQ: return "xotira yetmadi";
    case NOM_BAND: return "bunday nomli mahsulot allaqachon bor";
    case TOPILMADI: return "bunday mahsulot topilmadi";
    case NOTOGRI_MIQDOR: return "miqdor musbat bo'lishi kerak";
    case YETARLI_EMAS: return "omborda yetarli emas";
    case NOTOGRI_TOIFA: return "toifa 0, 1 yoki 2 bo'lishi kerak";
    default: return "noma'lum xato";
    }
}

int main(void)
{
    Ombor ombor = { NULL, 0, 0 };
    ombor_qosh(&ombor, "non", 400000, 120, OZIQ_OVQAT);
    ombor_qosh(&ombor, "sut", 1200000, 45, ICHIMLIK);
    ombor_qosh(&ombor, "guruch", 1800000, 8, OZIQ_OVQAT);

    int tanlov;
    while (printf("\n1) ro'yxat  2) sotish  3) qo'shish  4) hisobot  0) chiqish\nTanlov:\n"),
           scanf("%d", &tanlov) == 1 && tanlov != 0) {
        char s[24];
        long so_m;
        int miqdor, toifa, r;
        struct mahsulot *p;
        switch (tanlov) {
        case 1:
            ombor_royxat(&ombor);
            break;
        case 2:
            printf("Mahsulot nomi va necha dona?\n");
            if (scanf("%23s %d", s, &miqdor) != 2)
                break;
            p = ombor_topish(&ombor, s);
            r = p ? ombor_sot(p, miqdor) : TOPILMADI;
            if (r == OK)
                printf("  Sotildi: %d dona %s. Qoldi: %u dona\n", miqdor, p->nom, p->soni);
            else
                printf("  XATO: %s\n", xato_matni(r));
            break;
        case 3:
            printf("Nom, narx (so'mda), soni va toifa (0-oziq-ovqat, 1-ichimlik, 2-uy-ro'zg'or)?\n");
            if (scanf("%23s %ld %d %d", s, &so_m, &miqdor, &toifa) != 4)
                break;
            r = ombor_qosh(&ombor, s, so_m * 100, (uint16_t)miqdor, (enum toifa)toifa);
            if (r == OK)
                printf("  Qo'shildi: %s (%s)\n", s, toifa_nomi[toifa]);
            else
                printf("  XATO: %s\n", xato_matni(r));
            break;
        case 4:
            ombor_hisobot(&ombor);
            break;
        default:
            printf("  XATO: menyuda bunday band yo'q\n");
        }
    }
    ombor_tozala(&ombor);
    printf("\nXayr!\n");
    return 0;
}
