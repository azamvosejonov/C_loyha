/* ombor.c - ombor kutubxonasining ichki ishi (12-bosqich: qsort, fayl, errno, assert) */
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "ombor.h"
#include "pul.h"

#define YANGI      BIT(0)
#define TUGAYAPTI  BIT(2)

#define X_NOM(id, nom) nom,
static const char *const nomlar[] = { TOIFALAR(X_NOM) };
#undef X_NOM

struct mahsulot {
    char *nom;
    long narx;
    uint16_t soni;
    enum toifa toifa;
    unsigned holat;
};

struct ombor {                                  /* TA'RIF faqat shu faylda: tashqaridan ko'rinmaydi */
    struct mahsulot *m;
    int n;
    int sig;
};

const char *toifa_nomi(enum toifa t)
{
    return (unsigned)t < TOIFA_SONI ? nomlar[t] : "?";
}

Ombor *ombor_yarat(void)
{
    return calloc(1, sizeof(Ombor));            /* hammasi nolga: m = NULL, n = 0, sig = 0 */
}

void ombor_yoq(Ombor *o)
{
    if (!o)
        return;
    for (int i = 0; i < o->n; i++)
        free(o->m[i].nom);
    free(o->m);
    free(o);
}

static struct mahsulot *topish(const Ombor *o, const char *nom)
{
    for (int i = 0; i < o->n; i++)
        if (strcmp(o->m[i].nom, nom) == 0)
            return &o->m[i];
    return NULL;
}

static void holat_yangila(struct mahsulot *p)
{
    if (p->soni < 10)
        p->holat |= TUGAYAPTI;
    else
        p->holat &= ~TUGAYAPTI;
}

int ombor_qosh(Ombor *o, const char *nom, long narx, uint16_t soni, enum toifa toifa)
{
    if ((unsigned)toifa >= TOIFA_SONI)
        return NOTOGRI_TOIFA;
    if (topish(o, nom))
        return NOM_BAND;
    if (o->n == o->sig) {
        int yangi = o->sig ? o->sig * 2 : 2;
        struct mahsulot *y = realloc(o->m, (size_t)yangi * sizeof(*y));
        if (!y)
            return XOTIRA_YOQ;
        LOG("sig'im %d -> %d", o->sig, yangi);
        o->m = y;
        o->sig = yangi;
    }
    char *nusxa = strdup(nom);
    if (!nusxa)
        return XOTIRA_YOQ;
    assert(o->n < o->sig);                      /* joy borligiga ishonch: yo'q bo'lsa dastur to'xtaydi */
    struct mahsulot *p = &o->m[o->n++];
    p->nom = nusxa;
    p->narx = narx;
    p->soni = soni;
    p->toifa = toifa;
    p->holat = YANGI;
    holat_yangila(p);
    LOG("qo'shildi: %s (%d ta)", nom, o->n);
    return OK;
}

int ombor_sot(Ombor *o, const char *nom, int miqdor, uint16_t *qoldi)
{
    struct mahsulot *p = topish(o, nom);
    if (!p)
        return TOPILMADI;
    if (miqdor <= 0)
        return NOTOGRI_MIQDOR;
    if (miqdor > p->soni)
        return YETARLI_EMAS;
    p->soni -= miqdor;
    holat_yangila(p);
    if (qoldi)
        *qoldi = p->soni;
    LOG("sotildi: %s, %d dona", nom, miqdor);
    return OK;
}

int ombor_ochir(Ombor *o, const char *nom)
{
    struct mahsulot *p = topish(o, nom);
    if (!p)
        return TOPILMADI;
    int i = (int)(p - o->m);
    free(p->nom);
    memmove(&o->m[i], &o->m[i + 1], (size_t)(o->n - i - 1) * sizeof(*o->m));
    o->n--;
    return OK;
}

int ombor_soni(const Ombor *o)
{
    return o->n;
}

void ombor_royxat(const Ombor *o)
{
    printf("%-10s %-12s %10s %6s %s\n", "Mahsulot", "Toifa", "Narx", "Soni", "Holat");
    printf("----------------------------------------------------\n");
    long jami = 0;
    for (int i = 0; i < o->n; i++) {
        const struct mahsulot *p = &o->m[i];
        char narx_matn[32];
        pul_matn(narx_matn, sizeof(narx_matn), p->narx);
        printf("%-10s %-12s %10s %6u %s%s\n", p->nom, toifa_nomi(p->toifa), narx_matn, p->soni,
               (p->holat & YANGI) ? "[yangi] " : "", (p->holat & TUGAYAPTI) ? "[TUGAYAPTI]" : "");
        jami += p->narx * p->soni;
    }
    printf("----------------------------------------------------\n");
    printf("Jami qiymat: ");
    pul_chiqar(jami);
    printf(" so'm\n");
}

void ombor_hisobot(const Ombor *o)
{
    long toifa_jami[TOIFA_SONI] = { 0 };
    for (int i = 0; i < o->n; i++)
        toifa_jami[o->m[i].toifa] += o->m[i].narx * o->m[i].soni;
    for (int t = 0; t < TOIFA_SONI; t++) {
        printf("  %-12s ", toifa_nomi((enum toifa)t));
        pul_chiqar(toifa_jami[t]);
        printf(" so'm\n");
    }
}

/* qsort uchun taqqoslagichlar: ikki mahsulotni solishtiradi (manfiy / 0 / musbat) */
static int solishtir_nom(const void *a, const void *b)
{
    const struct mahsulot *x = a, *y = b;
    return strcmp(x->nom, y->nom);
}

static int solishtir_narx(const void *a, const void *b)
{
    const struct mahsulot *x = a, *y = b;
    return (y->narx > x->narx) - (y->narx < x->narx);   /* qimmati birinchi; ayirmasiz: toshmaydi */
}

void ombor_saralash(Ombor *o, enum saralash mezon)
{
    if (o->n > 1)
        qsort(o->m, (size_t)o->n, sizeof(*o->m), mezon == SARALASH_NARX ? solishtir_narx : solishtir_nom);
}

/* matn fayl: har qatorda "nom;narx;soni;toifa;holat" */
int ombor_saqla(const Ombor *o, const char *fayl)
{
    FILE *f = fopen(fayl, "w");
    if (!f)
        return -1;                              /* errno ni fopen o'zi qo'ydi */
    for (int i = 0; i < o->n; i++) {
        const struct mahsulot *p = &o->m[i];
        if (fprintf(f, "%s;%ld;%u;%d;%u\n", p->nom, p->narx, p->soni, (int)p->toifa, p->holat) < 0) {
            fclose(f);
            return -1;
        }
    }
    return fclose(f) == 0 ? 0 : -1;             /* fclose ham xato berishi mumkin (disk to'lgan) */
}

int ombor_yukla(Ombor *o, const char *fayl)
{
    FILE *f = fopen(fayl, "r");
    if (!f)
        return -1;
    Ombor *t = ombor_yarat();                   /* vaqtincha ombor: xato bo'lsa asl ombor buzilmaydi */
    if (!t) {
        fclose(f);
        errno = ENOMEM;
        return -1;
    }
    char qator[128];
    int son = 0;
    while (fgets(qator, sizeof(qator), f)) {
        char nom[24];
        long narx;
        unsigned short soni;
        int toifa;
        unsigned holat;
        if (sscanf(qator, "%23[^;];%ld;%hu;%d;%u", nom, &narx, &soni, &toifa, &holat) != 5 ||
            ombor_qosh(t, nom, narx, soni, (enum toifa)toifa) != OK) {
            ombor_yoq(t);
            fclose(f);
            errno = EINVAL;                     /* "Invalid argument": fayl formati noto'g'ri */
            return -1;
        }
        t->m[t->n - 1].holat = holat;
        son++;
    }
    fclose(f);
    for (int i = 0; i < o->n; i++)              /* hammasi yaxshi: eskisini almashtiramiz */
        free(o->m[i].nom);
    free(o->m);
    *o = *t;
    free(t);
    return son;
}

const char *ombor_xato(int kod)
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
