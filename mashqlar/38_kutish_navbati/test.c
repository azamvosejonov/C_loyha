#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "test.h"
#include "mashq.h"

#define ISHLAB 4
#define ISTEMOL 4
#define HAR_BIRI 20000

static struct navbat *q;
static unsigned char korildi[ISHLAB * HAR_BIRI];
static long istemolchi_yigindi[ISTEMOL];

static void *ishlab_chiqaruvchi(void *arg)
{
    int id = (int)(long)arg;
    for (int i = 0; i < HAR_BIRI; i++)
        navbat_qoy(q, id * HAR_BIRI + i);
    return NULL;
}

static void *istemolchi(void *arg)
{
    int id = (int)(long)arg;
    for (int i = 0; i < ISHLAB * HAR_BIRI / ISTEMOL; i++) {
        int x = navbat_ol(q);
        if (x >= 0 && x < ISHLAB * HAR_BIRI)
            __atomic_add_fetch(&korildi[x], 1, __ATOMIC_RELAXED);
        istemolchi_yigindi[id] += x;
    }
    return NULL;
}

static double hozir(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

static void *kechikib_qoy(void *arg)
{
    (void)arg;
    usleep(200000);                         /* 0.2 s dan keyin */
    navbat_qoy(q, 42);
    return NULL;
}

int main(void)
{
    TEST_BOSHLA();
    BOLIM("bitta oqim: FIFO tartibi");
    q = navbat_yarat(4);
    CHECK(q != NULL);
    if (!q)
        TEST_TUGADI();
    navbat_qoy(q, 1);
    navbat_qoy(q, 2);
    navbat_qoy(q, 3);
    CHECK_INT(navbat_ol(q), 1);
    navbat_qoy(q, 4);
    navbat_qoy(q, 5);                       /* to'la (2 3 4 5), halqa aylandi */
    CHECK_INT(navbat_ol(q), 2);
    CHECK_INT(navbat_ol(q), 3);
    CHECK_INT(navbat_ol(q), 4);
    CHECK_INT(navbat_ol(q), 5);

    BOLIM("bo'sh navbat: navbat_ol element kelguncha UXLAYDI");
    pthread_t t;
    double t0 = hozir();
    pthread_create(&t, NULL, kechikib_qoy, NULL);
    CHECK_INT(navbat_ol(q), 42);
    double kutdi = hozir() - t0;
    pthread_join(t, NULL);
    CHECK(kutdi > 0.15);
    printf("   (kutdi: %.2f s)\n", kutdi);
    navbat_yoq(q);

    BOLIM("4 ishlab chiqaruvchi + 4 iste'molchi, sig'im 8 (tez-tez to'ladi)");
    q = navbat_yarat(8);
    pthread_t p[ISHLAB], c[ISTEMOL];
    for (long i = 0; i < ISTEMOL; i++)
        pthread_create(&c[i], NULL, istemolchi, (void *)i);
    for (long i = 0; i < ISHLAB; i++)
        pthread_create(&p[i], NULL, ishlab_chiqaruvchi, (void *)i);
    for (int i = 0; i < ISHLAB; i++)
        pthread_join(p[i], NULL);
    for (int i = 0; i < ISTEMOL; i++)
        pthread_join(c[i], NULL);
    int ok = 1;
    for (int i = 0; i < ISHLAB * HAR_BIRI; i++)
        ok &= korildi[i] == 1;              /* har bir son aniq bir marta olindi */
    CHECK(ok);
    long jami = 0;
    for (int i = 0; i < ISTEMOL; i++)
        jami += istemolchi_yigindi[i];
    CHECK_INT(jami, (long)ISHLAB * HAR_BIRI * (ISHLAB * HAR_BIRI - 1) / 2);
    navbat_yoq(q);
    TEST_TUGADI();
}
