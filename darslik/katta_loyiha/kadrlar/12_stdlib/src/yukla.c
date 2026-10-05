#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "vaqt.h"
#include "yukla.h"

#define QATOR_UZ 256

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

/* qatorni bo'sh joylar bo'yicha so'zlarga bo'ladi (qatorni o'zgartiradi). So'zlar sonini qaytaradi */
static int bol(char *qator, char *soz[], int maks)
{
    int n = 0;
    char *saqla;
    for (char *t = strtok_r(qator, " \t\r\n", &saqla); t && n < maks; t = strtok_r(NULL, " \t\r\n", &saqla))
        soz[n++] = t;
    return n;
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

int xodimlar_yukla(struct ombor *o, const char *fayl, struct yuklash *h)
{
    FILE *f = och(fayl);
    if (!f)
        return -1;
    char qator[QATOR_UZ];
    for (int raqam = 1; fgets(qator, sizeof(qator), f); raqam++) {
        if (qator[0] == '#' || qator[0] == '\n')
            continue;
        char *soz[8];
        if (bol(qator, soz, 8) != 4) {
            rad_et(fayl, raqam, "maydonlar soni 4 emas", h);
            continue;
        }
        long long id, toifa, tarif;
        if (son_oqi(soz[0], &id) || id < 1000 || id > 9999) {
            rad_et(fayl, raqam, "ID 1000..9999 oralig'ida son emas", h);
            continue;
        }
        if (strlen(soz[1]) >= ISM_UZ) {
            rad_et(fayl, raqam, "ism juda uzun", h);
            continue;
        }
        if (son_oqi(soz[2], &toifa) || toifa < 1 || toifa > T_SONI) {
            rad_et(fayl, raqam, "toifa 1..4 oralig'ida son emas", h);
            continue;
        }
        if (son_oqi(soz[3], &tarif) || tarif <= 0) {
            rad_et(fayl, raqam, "tarif musbat son emas", h);
            continue;
        }
        struct xodim x = { .id = (int)id, .toifa = (enum toifa)(toifa - 1), .tarif = tarif };
        snprintf(x.ism, sizeof(x.ism), "%s", soz[1]);
        int r = ombor_qosh(o, &x);
        if (r == -2) {
            rad_et(fayl, raqam, "bu ID allaqachon bor", h);
        } else if (r != 0) {
            rad_et(fayl, raqam, "xotira yetmadi", h);
        } else {
            h->qabul++;
        }
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
    for (int raqam = 1; fgets(qator, sizeof(qator), f); raqam++) {
        if (qator[0] == '#' || qator[0] == '\n')
            continue;
        char *soz[8];
        long long v[4];
        int yaroqli = bol(qator, soz, 8) == 4;
        for (int i = 0; yaroqli && i < 4; i++)
            yaroqli = son_oqi(soz[i], &v[i]) == 0;
        if (!yaroqli) {
            rad_et(fayl, raqam, "to'rtta butun son kerak", h);
            continue;
        }
        struct xodim *x = ombor_top_yoz(o, (int)v[0]);
        if (!x) {
            rad_et(fayl, raqam, "bunday ID li xodim yo'q", h);
            continue;
        }
        int sof = ish_daqiqalari((int)v[2], (int)v[3], TANAFFUS_DAQ);
        if (sof < 0) {
            rad_et(fayl, raqam, "vaqt noto'g'ri (HHMM, chiqish kirishdan keyin bo'lishi kerak)", h);
            continue;
        }
        x->oddiy_daq += sof < NORMA_KUN_DAQ ? sof : NORMA_KUN_DAQ;
        x->qosh_daq += sof > NORMA_KUN_DAQ ? sof - NORMA_KUN_DAQ : 0;
        h->qabul++;
    }
    fclose(f);
    return 0;
}
