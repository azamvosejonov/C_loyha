#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "test.h"
#include "mashq.h"

static struct rwqulf *q;
static int oquvchilar, yozuvchilar, eng_kop_oquvchi, buzilish, toxta;
static long malumot[2];                     /* yozuvchi ikkalasini birga o'zgartiradi */

static void max_yangila(int *m, int v)
{
    int e = __atomic_load_n(m, __ATOMIC_SEQ_CST);
    while (v > e && !__atomic_compare_exchange_n(m, &e, v, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
        ;
}

static void *oquvchi(void *arg)
{
    int marta = (int)(long)arg;
    for (int i = 0; marta ? i < marta : !__atomic_load_n(&toxta, __ATOMIC_SEQ_CST); i++) {
        rw_oqish_ol(q);
        int o = __atomic_add_fetch(&oquvchilar, 1, __ATOMIC_SEQ_CST);
        max_yangila(&eng_kop_oquvchi, o);
        if (__atomic_load_n(&yozuvchilar, __ATOMIC_SEQ_CST) != 0 || malumot[0] != malumot[1])
            __atomic_store_n(&buzilish, 1, __ATOMIC_SEQ_CST);
        usleep(1000);
        __atomic_sub_fetch(&oquvchilar, 1, __ATOMIC_SEQ_CST);
        rw_oqish_qoy(q);
    }
    return NULL;
}

static void *yozuvchi(void *arg)
{
    int marta = (int)(long)arg;
    for (int i = 0; i < marta; i++) {
        rw_yozish_ol(q);
        int y = __atomic_add_fetch(&yozuvchilar, 1, __ATOMIC_SEQ_CST);
        if (y != 1 || __atomic_load_n(&oquvchilar, __ATOMIC_SEQ_CST) != 0)
            __atomic_store_n(&buzilish, 1, __ATOMIC_SEQ_CST);
        malumot[0]++;
        usleep(100);
        malumot[1]++;
        __atomic_sub_fetch(&yozuvchilar, 1, __ATOMIC_SEQ_CST);
        rw_yozish_qoy(q);
    }
    return NULL;
}

static double hozir(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

int main(void)
{
    TEST_BOSHLA();
    q = rw_yarat();
    CHECK(q != NULL);
    if (!q)
        TEST_TUGADI();

    BOLIM("8 o'quvchi + 2 yozuvchi: qoidalar buzilmaydi");
    pthread_t t[16];
    for (long i = 0; i < 8; i++)
        pthread_create(&t[i], NULL, oquvchi, (void *)50L);
    for (long i = 8; i < 10; i++)
        pthread_create(&t[i], NULL, yozuvchi, (void *)100L);
    for (int i = 0; i < 10; i++)
        pthread_join(t[i], NULL);
    CHECK_INT(buzilish, 0);
    CHECK_INT(malumot[0], 200);
    CHECK_INT(malumot[1], 200);
    printf("   (bir vaqtdagi eng ko'p o'quvchi: %d)\n", eng_kop_oquvchi);
    CHECK(eng_kop_oquvchi >= 2);               /* o'quvchilar haqiqatan BIRGA o'qiydi */

    BOLIM("yozuvchi och qolmaydi (o'quvchilar to'xtovsiz)");
    toxta = 0;
    for (long i = 0; i < 6; i++)
        pthread_create(&t[i], NULL, oquvchi, (void *)0L);
    usleep(50000);                              /* o'quvchilar oqimi boshlandi */
    double t0 = hozir();
    rw_yozish_ol(q);
    double kutdi = hozir() - t0;
    if (__atomic_load_n(&oquvchilar, __ATOMIC_SEQ_CST) != 0)
        buzilish = 1;
    rw_yozish_qoy(q);
    __atomic_store_n(&toxta, 1, __ATOMIC_SEQ_CST);
    for (int i = 0; i < 6; i++)
        pthread_join(t[i], NULL);
    printf("   (yozuvchi kutdi: %.3f s)\n", kutdi);
    CHECK(kutdi < 1.0);
    CHECK_INT(buzilish, 0);
    rw_yoq(q);
    TEST_TUGADI();
}
