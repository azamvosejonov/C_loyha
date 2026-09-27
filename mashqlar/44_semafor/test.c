#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "test.h"
#include "mashq.h"

static struct semafor *s;
static int faol, eng_kop;
static long hisob;

static void *resurs_ish(void *arg)
{
    (void)arg;
    for (int i = 0; i < 20; i++) {
        semafor_kut(s);
        int f = __atomic_add_fetch(&faol, 1, __ATOMIC_SEQ_CST);
        int e = __atomic_load_n(&eng_kop, __ATOMIC_SEQ_CST);
        while (f > e && !__atomic_compare_exchange_n(&eng_kop, &e, f, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
            ;
        usleep(200);
        __atomic_sub_fetch(&faol, 1, __ATOMIC_SEQ_CST);
        semafor_ber(s);
    }
    return NULL;
}

static void *qulf_ish(void *arg)
{
    (void)arg;
    for (int i = 0; i < 20000; i++) {
        semafor_kut(s);
        long v = hisob;
        hisob = v + 1;
        semafor_ber(s);
    }
    return NULL;
}

static void *kechikib_ber(void *arg)
{
    (void)arg;
    usleep(200000);
    semafor_ber(s);
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
    BOLIM("semafor_urin (bitta oqim)");
    s = semafor_yarat(3);
    CHECK(s != NULL);
    if (!s)
        TEST_TUGADI();
    CHECK_INT(semafor_urin(s), 1);
    CHECK_INT(semafor_urin(s), 1);
    CHECK_INT(semafor_urin(s), 1);
    CHECK_INT(semafor_urin(s), 0);              /* qiymat 0 */
    semafor_ber(s);
    CHECK_INT(semafor_urin(s), 1);
    semafor_ber(s);
    semafor_ber(s);
    semafor_ber(s);                             /* yana 3 */

    BOLIM("N = 3 resurs: 12 oqim, bir vaqtda ko'pi bilan 3 ta");
    pthread_t t[12];
    for (int i = 0; i < 12; i++)
        pthread_create(&t[i], NULL, resurs_ish, NULL);
    for (int i = 0; i < 12; i++)
        pthread_join(t[i], NULL);
    printf("   (bir vaqtdagi eng ko'p: %d)\n", eng_kop);
    CHECK(eng_kop >= 1 && eng_kop <= 3);
    semafor_yoq(s);

    BOLIM("N = 1: qulf sifatida (8 oqim x 20000)");
    s = semafor_yarat(1);
    for (int i = 0; i < 8; i++)
        pthread_create(&t[i], NULL, qulf_ish, NULL);
    for (int i = 0; i < 8; i++)
        pthread_join(t[i], NULL);
    CHECK_INT(hisob, 160000);
    semafor_yoq(s);

    BOLIM("N = 0: tartiblash (ber() kelguncha uxlaydi)");
    s = semafor_yarat(0);
    double t0 = hozir();
    pthread_create(&t[0], NULL, kechikib_ber, NULL);
    semafor_kut(s);
    double kutdi = hozir() - t0;
    pthread_join(t[0], NULL);
    printf("   (kutdi: %.2f s)\n", kutdi);
    CHECK(kutdi > 0.15);
    CHECK_INT(semafor_urin(s), 0);
    semafor_yoq(s);
    TEST_TUGADI();
}
