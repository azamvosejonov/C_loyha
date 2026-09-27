#include <time.h>

#include "test.h"
#include "mashq.h"

static int heapmi(const struct heap *h)
{
    for (size_t i = 1; i < h->n; i++)
        if (h->a[i] < h->a[(i - 1) / 2])
            return 0;
    return 1;
}

static unsigned long rng = 17;
static int tasodif(void)
{
    rng = rng * 6364136223846793005UL + 1442695040888963407UL;
    return (int)(rng >> 33) - (1 << 30);
}

int main(void)
{
    TEST_BOSHLA();
    struct heap h;
    memset(&h, 0x33, sizeof(h));
    BOLIM("heap_init va bo'sh heap");
    heap_init(&h);
    CHECK(h.n == 0);
    if (h.n != 0)
        TEST_TUGADI();
    int x = 777;
    CHECK_INT(heap_ol(&h, &x), -1);
    CHECK_INT(heap_tepa(&h, &x), -1);
    CHECK_INT(x, 777);

    BOLIM("qo'shish va olish");
    int k[] = { 5, 3, 8, 1, 9, 2, 7, 1 };
    int ok = 1;
    for (int i = 0; i < 8; i++) {
        ok &= heap_qosh(&h, k[i]) == 0;
        ok &= heapmi(&h);
    }
    CHECK(ok);
    CHECK_INT(h.n, 8);
    CHECK_INT(heap_tepa(&h, &x), 0);
    CHECK_INT(x, 1);
    int kut[] = { 1, 1, 2, 3, 5, 7, 8, 9 };
    ok = 1;
    for (int i = 0; i < 8; i++) {
        ok &= heap_ol(&h, &x) == 0 && x == kut[i];
        ok &= heapmi(&h);
    }
    CHECK(ok);
    CHECK_INT(heap_ol(&h, &x), -1);

    BOLIM("200000 ta tasodifiy son: tartiblangan chiqishi kerak");
    clock_t t0 = clock();
    ok = 1;
    for (int i = 0; i < 200000; i++)
        ok &= heap_qosh(&h, tasodif()) == 0;
    CHECK(ok && heapmi(&h));
    int oldingi = INT_MIN;
    ok = 1;
    for (int i = 0; i < 200000; i++) {
        ok &= heap_ol(&h, &x) == 0 && x >= oldingi;
        oldingi = x;
    }
    CHECK(ok);
    double sek = (double)(clock() - t0) / CLOCKS_PER_SEC;
    printf("   (vaqt: %.2f s)\n", sek);
    CHECK(sek < 3.0);

    BOLIM("heap_qur (O(n) qurish)");
    int *m = malloc(100000 * sizeof(int));
    for (int i = 0; i < 100000; i++)
        m[i] = tasodif() % 1000;
    CHECK_INT(heap_qur(&h, m, 100000), 0);
    CHECK_INT(h.n, 100000);
    CHECK(heapmi(&h));
    CHECK(h.a != m);                            /* nusxa bo'lishi kerak */
    ok = 1;
    oldingi = INT_MIN;
    for (int i = 0; i < 100000; i++) {
        ok &= heap_ol(&h, &x) == 0 && x >= oldingi;
        oldingi = x;
    }
    CHECK(ok);
    free(m);
    heap_yoq(&h);
    TEST_TUGADI();
}
