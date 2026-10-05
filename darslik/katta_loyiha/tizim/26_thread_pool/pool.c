/* pool.c - bitta mutex + ikkita shart o'zgaruvchisi: ishlab chiqaruvchi-iste'molchi (26-bob) asosida */
#include <pthread.h>
#include <stdlib.h>

#include "pool.h"

struct topshiriq {
    topshiriq_fn f;
    void *arg;
};

struct pool {
    pthread_mutex_t qulf;
    pthread_cond_t joy_bor;                     /* "navbatda joy bo'shadi" - yuboruvchilarni uyg'otadi */
    pthread_cond_t ish_bor;                     /* "navbatga ish tushdi / pool yopildi" - ishchilarni uyg'otadi */
    struct topshiriq *navbat;                   /* aylanma bufer */
    int sigim, bosh, soni;
    int yopilgan;                               /* 1 - yangi ish qabul qilinmaydi */
    pthread_t *ishchilar;
    int ishchilar_soni;
};

static void *ishchi(void *arg)
{
    struct pool *p = arg;
    for (;;) {
        pthread_mutex_lock(&p->qulf);
        while (p->soni == 0 && !p->yopilgan)    /* while: soxta uyg'onishdan himoya (26-bob) */
            pthread_cond_wait(&p->ish_bor, &p->qulf);
        if (p->soni == 0 && p->yopilgan) {      /* ish ham yo'q, yangisi ham kelmaydi: chiqamiz */
            pthread_mutex_unlock(&p->qulf);
            return NULL;
        }
        struct topshiriq t = p->navbat[p->bosh];
        p->bosh = (p->bosh + 1) % p->sigim;
        p->soni--;
        pthread_cond_signal(&p->joy_bor);       /* joy bo'shadi: bitta yuboruvchi davom etsin */
        pthread_mutex_unlock(&p->qulf);

        t.f(t.arg);                             /* topshiriqni QULFSIZ bajaramiz: boshqa ishchilar to'xtab qolmasin */
    }
}

struct pool *pool_yarat(int ishchilar, int navbat_sigimi)
{
    struct pool *p = calloc(1, sizeof(*p));
    if (!p)
        return NULL;
    p->navbat = calloc((size_t)navbat_sigimi, sizeof(*p->navbat));
    p->ishchilar = calloc((size_t)ishchilar, sizeof(*p->ishchilar));
    if (!p->navbat || !p->ishchilar) {
        free(p->navbat);
        free(p->ishchilar);
        free(p);
        return NULL;
    }
    p->sigim = navbat_sigimi;
    p->ishchilar_soni = ishchilar;
    pthread_mutex_init(&p->qulf, NULL);
    pthread_cond_init(&p->joy_bor, NULL);
    pthread_cond_init(&p->ish_bor, NULL);
    for (int i = 0; i < ishchilar; i++)
        pthread_create(&p->ishchilar[i], NULL, ishchi, p);
    return p;
}

int pool_yubor(struct pool *p, topshiriq_fn f, void *arg)
{
    pthread_mutex_lock(&p->qulf);
    while (p->soni == p->sigim && !p->yopilgan)
        pthread_cond_wait(&p->joy_bor, &p->qulf);   /* navbat to'la: joy bo'shaguncha uxlaymiz (qulfni qo'yib yuborib) */
    if (p->yopilgan) {
        pthread_mutex_unlock(&p->qulf);
        return -1;
    }
    p->navbat[(p->bosh + p->soni) % p->sigim] = (struct topshiriq){ f, arg };
    p->soni++;
    pthread_cond_signal(&p->ish_bor);           /* bitta ishchini uyg'otamiz */
    pthread_mutex_unlock(&p->qulf);
    return 0;
}

void pool_tugat(struct pool *p)
{
    pthread_mutex_lock(&p->qulf);
    p->yopilgan = 1;
    pthread_cond_broadcast(&p->ish_bor);        /* HAMMA ishchi uyg'onsin: navbatni tugatib chiqsin */
    pthread_cond_broadcast(&p->joy_bor);
    pthread_mutex_unlock(&p->qulf);
    for (int i = 0; i < p->ishchilar_soni; i++)
        pthread_join(p->ishchilar[i], NULL);    /* hamma ishchi tugashini kutamiz */
    pthread_mutex_destroy(&p->qulf);
    pthread_cond_destroy(&p->joy_bor);
    pthread_cond_destroy(&p->ish_bor);
    free(p->ishchilar);
    free(p->navbat);
    free(p);
}
