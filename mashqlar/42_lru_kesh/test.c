#include <time.h>

#include "test.h"
#include "mashq.h"

/* Oddiy (sekin, O(n)) namunaviy LRU - natijalarni solishtirish uchun. */
#define ET_SIG 64
static int et_k[ET_SIG], et_v[ET_SIG], et_n;
static int et_ol(int k, int *v)
{
    for (int i = 0; i < et_n; i++)
        if (et_k[i] == k) {
            int kk = et_k[i], vv = et_v[i];
            memmove(et_k + 1, et_k, (size_t)i * sizeof(int));
            memmove(et_v + 1, et_v, (size_t)i * sizeof(int));
            et_k[0] = kk;
            et_v[0] = vv;
            *v = vv;
            return 0;
        }
    return -1;
}
static void et_qoy(int k, int v)
{
    int t;
    if (et_ol(k, &t) == 0) {
        et_v[0] = v;
        return;
    }
    if (et_n < ET_SIG)
        et_n++;
    memmove(et_k + 1, et_k, (size_t)(et_n - 1) * sizeof(int));
    memmove(et_v + 1, et_v, (size_t)(et_n - 1) * sizeof(int));
    et_k[0] = k;
    et_v[0] = v;
}

int main(void)
{
    TEST_BOSHLA();
    BOLIM("oddiy holat (sig'im 2)");
    struct lru *c = lru_yarat(2);
    CHECK(c != NULL);
    if (!c)
        TEST_TUGADI();
    int v = 0;
    lru_qoy(c, 1, 10);
    lru_qoy(c, 2, 20);
    CHECK_INT(lru_ol(c, 1, &v), 0);             /* 1 endi eng yangi */
    CHECK_INT(v, 10);
    lru_qoy(c, 3, 30);                          /* 2 chiqariladi */
    CHECK_INT(lru_ol(c, 2, &v), -1);
    CHECK_INT(lru_ol(c, 3, &v), 0);
    CHECK_INT(v, 30);
    lru_qoy(c, 1, 11);                          /* yangilash - chiqarish emas */
    CHECK_INT(lru_soni(c), 2);
    lru_qoy(c, 4, 40);                          /* 3 chiqariladi (1 yangilangan edi) */
    CHECK_INT(lru_ol(c, 3, &v), -1);
    CHECK_INT(lru_ol(c, 1, &v), 0);
    CHECK_INT(v, 11);
    lru_yoq(c);

    BOLIM("namunaviy LRU bilan solishtirish: 200000 amal, sig'im 64");
    c = lru_yarat(ET_SIG);
    unsigned long rng = 5;
    int ok = 1;
    for (int i = 0; i < 200000; i++) {
        rng = rng * 6364136223846793005UL + 1442695040888963407UL;
        int k = (int)((rng >> 33) % 100), qiymat = i;
        if ((rng >> 20) & 1) {
            int a = -7, b = -7;
            int ra = lru_ol(c, k, &a), rb = et_ol(k, &b);
            ok &= ra == rb && a == b;
        } else {
            lru_qoy(c, k, qiymat);
            et_qoy(k, qiymat);
        }
    }
    CHECK(ok);
    CHECK_INT(lru_soni(c), et_n);
    lru_yoq(c);

    BOLIM("tezlik: sig'im 50000, 2000000 amal (O(1) kerak)");
    clock_t t0 = clock();
    c = lru_yarat(50000);
    long topildi = 0;
    for (int i = 0; i < 2000000; i++) {
        rng = rng * 6364136223846793005UL + 1442695040888963407UL;
        int k = (int)((rng >> 33) % 80000);
        if (lru_ol(c, k, &v) == 0)
            topildi++;
        else
            lru_qoy(c, k, i);
    }
    double sek = (double)(clock() - t0) / CLOCKS_PER_SEC;
    printf("   (topildi: %ld, vaqt: %.2f s)\n", topildi, sek);
    CHECK_INT(lru_soni(c), 50000);
    CHECK(topildi > 1000000 && topildi < 1400000);          /* ~62% urilish */
    CHECK(sek < 8.0);
    lru_yoq(c);
    TEST_TUGADI();
}
