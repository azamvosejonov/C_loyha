#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bayroq.h"
#include "config.h"
#include "vaqt.h"
#include "yukla.h"

/* butun son o'qiydi: butun matn son bo'lishi shart ("12abc" - xato), toshmasligi shart. 0 - OK */
static int son_oqi(const char *s, long long *natija)
{
    char *oxir;
    errno = 0;
    long long v = strtoll(s, &oxir, 10);
    if (oxir == s || *oxir != '\0' || errno == ERANGE)
        return -1;
    *natija = v;
    return 0;
}

/* qatorni bo'sh joylar bo'yicha so'zlarga bo'ladi (qatorni o'zgartiradi). So'zlar sonini qaytaradi (maks dan oshsa maks + 1) */
static int bol(char *qator, char *soz[], int maks)
{
    int n = 0;
    char *saqla;
    for (char *t = strtok_r(qator, " \t\r\n", &saqla); t; t = strtok_r(NULL, " \t\r\n", &saqla)) {
        if (n == maks)
            return maks + 1;
        soz[n++] = t;
    }
    return n;
}

int xodim_tahlil(const char *qator, struct xodim *x, const char **sabab)
{
    char nusxa[QATOR_UZ];
    if (strlen(qator) >= sizeof(nusxa)) {       /* strcpy EMAS: avval uzunlikni tekshiramiz */
        *sabab = "qator juda uzun";
        return -1;
    }
    memcpy(nusxa, qator, strlen(qator) + 1);

    char *soz[8];
    int nsoz = bol(nusxa, soz, 5);
    if (nsoz != 4 && nsoz != 5) {
        *sabab = "maydonlar soni 4 yoki 5 bo'lishi kerak";
        return -1;
    }
    long long id, toifa, tarif;
    if (son_oqi(soz[0], &id) || id < 1000 || id > 9999) {
        *sabab = "ID 1000..9999 oralig'ida son emas";
        return -1;
    }
    if (strlen(soz[1]) >= ISM_UZ) {
        *sabab = "ism juda uzun";
        return -1;
    }
    if (son_oqi(soz[2], &toifa) || toifa < 1 || toifa > T_SONI) {
        *sabab = "toifa 1..4 oralig'ida son emas";
        return -1;
    }
    if (son_oqi(soz[3], &tarif) || tarif <= 0 || tarif > TARIF_MAKS) {
        *sabab = "tarif 1..TARIF_MAKS oralig'ida son emas";
        return -1;
    }
    uint8_t bayroq = 0;
    if (nsoz == 5 && bayroq_oqi(soz[4], &bayroq) != 0) {
        *sabab = "noma'lum bayroq harfi (T M R S K dan biri bo'lishi kerak)";
        return -1;
    }
    memset(x, 0, sizeof(*x));
    x->bayroq = bayroq;
    x->id = (int)id;
    x->toifa = (enum toifa)(toifa - 1);
    x->tarif = tarif;
    memcpy(x->ism, soz[1], strlen(soz[1]) + 1);         /* uzunlik yuqorida tekshirilgan */
    return 0;
}

int davomat_tahlil(const char *qator, int *id, int *kirish, int *chiqish, const char **sabab)
{
    char nusxa[QATOR_UZ];
    if (strlen(qator) >= sizeof(nusxa)) {
        *sabab = "qator juda uzun";
        return -1;
    }
    memcpy(nusxa, qator, strlen(qator) + 1);

    char *soz[8];
    long long v[4];
    if (bol(nusxa, soz, 4) != 4) {
        *sabab = "to'rtta butun son kerak";
        return -1;
    }
    for (int i = 0; i < 4; i++)
        if (son_oqi(soz[i], &v[i])) {
            *sabab = "to'rtta butun son kerak";
            return -1;
        }
    if (v[0] < 1000 || v[0] > 9999 || v[1] < 1 || v[1] > 31) {
        *sabab = "ID yoki kun oralig'dan tashqarida";
        return -1;
    }
    if (v[2] < 0 || v[2] > 2359 || v[3] < 0 || v[3] > 2359) {
        *sabab = "vaqt HHMM oralig'idan tashqarida";
        return -1;
    }
    *id = (int)v[0];
    *kirish = (int)v[2];
    *chiqish = (int)v[3];
    return 0;
}

static FILE *och(const char *fayl)
{
    FILE *f = fopen(fayl, "r");
    if (!f)
        fprintf(stderr, "%s: ochilmadi: %s\n", fayl, strerror(errno));
    return f;
}

static void rad_et(const char *fayl, int qator_raqami, const char *sabab, struct yuklash *h)
{
    fprintf(stderr, "%s:%d: %s\n", fayl, qator_raqami, sabab);
    h->rad++;
}

/* bitta qator o'qiydi. Juda uzun qatorning QOLGAN qismini tashlab yuboradi. 1 - qator olindi, 0 - fayl tugadi, -1 - qator uzun edi */
static int qator_ol(FILE *f, char *qator, size_t hajm)
{
    if (!fgets(qator, (int)hajm, f))
        return 0;
    if (strchr(qator, '\n') == NULL && !feof(f)) {
        int c;
        while ((c = fgetc(f)) != '\n' && c != EOF)
            ;
        return -1;
    }
    return 1;
}

int xodimlar_yukla(struct ombor *o, const char *fayl, struct yuklash *h)
{
    FILE *f = och(fayl);
    if (!f)
        return -1;
    char qator[QATOR_UZ];
    int r;
    for (int raqam = 1; (r = qator_ol(f, qator, sizeof(qator))) != 0; raqam++) {
        if (r < 0) {
            rad_et(fayl, raqam, "qator juda uzun", h);
            continue;
        }
        if (qator[0] == '#' || qator[0] == '\n')
            continue;
        struct xodim x;
        const char *sabab;
        if (xodim_tahlil(qator, &x, &sabab) != 0) {
            rad_et(fayl, raqam, sabab, h);
            continue;
        }
        int q = ombor_qosh(o, &x);
        if (q == -2)
            rad_et(fayl, raqam, "bu ID allaqachon bor", h);
        else if (q != 0)
            rad_et(fayl, raqam, "xotira yetmadi", h);
        else
            h->qabul++;
    }
    fclose(f);
    return 0;
}

int davomat_yukla(struct ombor *o, const char *fayl, struct yuklash *h)
{
    FILE *f = och(fayl);
    if (!f)
        return -1;
    char qator[QATOR_UZ];
    int r;
    for (int raqam = 1; (r = qator_ol(f, qator, sizeof(qator))) != 0; raqam++) {
        if (r < 0) {
            rad_et(fayl, raqam, "qator juda uzun", h);
            continue;
        }
        if (qator[0] == '#' || qator[0] == '\n')
            continue;
        int id, kirish, chiqish;
        const char *sabab;
        if (davomat_tahlil(qator, &id, &kirish, &chiqish, &sabab) != 0) {
            rad_et(fayl, raqam, sabab, h);
            continue;
        }
        struct xodim *x = ombor_top_yoz(o, id);
        if (!x) {
            rad_et(fayl, raqam, "bunday ID li xodim yo'q", h);
            continue;
        }
        int sof = ish_daqiqalari(kirish, chiqish, TANAFFUS_DAQ);
        if (sof < 0) {
            rad_et(fayl, raqam, "vaqt noto'g'ri (chiqish kirishdan keyin bo'lishi kerak)", h);
            continue;
        }
        int oddiy = sof < NORMA_KUN_DAQ ? sof : NORMA_KUN_DAQ;
        int qosh = sof - oddiy;
        if (x->oddiy_daq + x->qosh_daq + sof > OY_MAKS_DAQ) {   /* oylik yig'indi chegaradan oshmasin: hisob toshmasligi kafolati */
            rad_et(fayl, raqam, "oylik ish vaqti chegaradan oshib ketadi", h);
            continue;
        }
        x->oddiy_daq += oddiy;
        x->qosh_daq += qosh;
        h->qabul++;
    }
    fclose(f);
    return 0;
}
