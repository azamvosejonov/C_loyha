#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "parallel.h"

static void qosh(struct natija *a, const struct natija *b)
{
    a->asosiy += b->asosiy;
    a->ustama += b->ustama;
    a->bonus += b->bonus;
    a->brutto += b->brutto;
    a->soliq += b->soliq;
    a->kasaba += b->kasaba;
    a->sof += b->sof;
}

void jami_ketma_ket(const struct xodim *a, size_t n, struct natija *jami)
{
    memset(jami, 0, sizeof(*jami));
    for (size_t i = 0; i < n; i++) {
        struct natija r;
        xodim_hisobla(&a[i], &r);
        qosh(jami, &r);
    }
}

/* oqimga beriladigan topshiriq */
struct ish {
    const struct xodim *a;
    size_t bosh, oxir;                          /* shu oqim [bosh, oxir) oralig'ini hisoblaydi */
    struct natija *jami;                        /* nimaga yozadi: o'z yig'indisiga (parallel) yoki umumiyga (qulf, poyga) */
    pthread_mutex_t *qulf;                      /* NULL - qulfsiz */
    int bolishilgan;                            /* 1 - jami boshqa oqimlar bilan UMUMIY (poyga/qulf variantlari) */
};

static void *oqim_ishi(void *arg)
{
    struct ish *s = arg;
    for (size_t i = s->bosh; i < s->oxir; i++) {
        struct natija r;
        xodim_hisobla(&s->a[i], &r);
        if (s->qulf)
            pthread_mutex_lock(s->qulf);
        qosh(s->jami, &r);                      /* bolishilgan bo'lsa va qulf bo'lmasa: POYGA */
        if (s->qulf)
            pthread_mutex_unlock(s->qulf);
    }
    return NULL;
}

/* umumiy mexanizm: n ni T bo'lakka bo'ladi, oqimlarni ishga tushiradi, hammasini kutadi */
static int yurgiz(const struct xodim *a, size_t n, int T, struct natija *jami, int bolishilgan, pthread_mutex_t *qulf)
{
    if (T < 1 || T > MAKS_OQIM)
        return -1;
    struct natija bolak[MAKS_OQIM];             /* har oqim uchun o'z yig'indisi (faqat bolishilmagan variantda ishlatiladi) */
    struct ish ish[MAKS_OQIM];
    pthread_t oqim[MAKS_OQIM];
    memset(bolak, 0, sizeof(bolak));
    memset(jami, 0, sizeof(*jami));

    int ishga = 0, xato = 0;
    for (int t = 0; t < T; t++) {
        ish[t] = (struct ish){ a, n * (size_t)t / (size_t)T, n * (size_t)(t + 1) / (size_t)T,
                               bolishilgan ? jami : &bolak[t], qulf, bolishilgan };
        if (pthread_create(&oqim[t], NULL, oqim_ishi, &ish[t]) != 0) {
            xato = 1;
            break;
        }
        ishga++;
    }
    for (int t = 0; t < ishga; t++)
        pthread_join(oqim[t], NULL);            /* ishga tushganlarning HAMMASINI kutamiz (xato bo'lsa ham) */
    if (xato)
        return -1;
    if (!bolishilgan)
        for (int t = 0; t < T; t++)
            qosh(jami, &bolak[t]);              /* yig'indilarni bitta oqimda (asosiy) qo'shamiz: poyga yo'q */
    return 0;
}

int jami_parallel(const struct xodim *a, size_t n, int oqimlar, struct natija *jami)
{
    return yurgiz(a, n, oqimlar, jami, 0, NULL);
}

int jami_qulf(const struct xodim *a, size_t n, int oqimlar, struct natija *jami)
{
    pthread_mutex_t qulf = PTHREAD_MUTEX_INITIALIZER;
    int r = yurgiz(a, n, oqimlar, jami, 1, &qulf);
    pthread_mutex_destroy(&qulf);
    return r;
}

int jami_poyga(const struct xodim *a, size_t n, int oqimlar, struct natija *jami)
{
    return yurgiz(a, n, oqimlar, jami, 1, NULL);
}

struct xodim *sinov_xodimlari(size_t n)
{
    struct xodim *a = calloc(n ? n : 1, sizeof(*a));
    if (!a)
        return NULL;
    unsigned long h = 12345;
    for (size_t i = 0; i < n; i++) {
        h = h * 6364136223846793005UL + 1442695040888963407UL;      /* LCG: bir xil ketma-ketlik har safar */
        unsigned r = (unsigned)(h >> 33);
        a[i].id = 1000 + (int)(i % 9000);
        a[i].toifa = (enum toifa)(i % T_SONI);
        a[i].tarif = 1000000 + (int64_t)(r % 5000000);
        a[i].oddiy_daq = 600 + (int)(r % 2000);
        a[i].qosh_daq = (int)((r >> 8) % 600);
    }
    return a;
}
