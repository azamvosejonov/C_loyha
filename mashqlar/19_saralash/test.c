#include <time.h>

#include "test.h"
#include "mashq.h"

static unsigned long rng = 12345;
static int tasodifiy(void)
{
    rng = rng * 6364136223846793005UL + 1442695040888963407UL;
    return (int)(rng >> 33) - (1 << 30);
}

static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static int teng(const int *a, const int *b, size_t n)
{
    return memcmp(a, b, n * sizeof(int)) == 0;
}

int main(void)
{
    TEST_BOSHLA();
    BOLIM("mening_saralashim: kichik");
    int a[] = { 5, -1, 3, 3, 0, 9, -7 }, b[] = { -7, -1, 0, 3, 3, 5, 9 };
    mening_saralashim(a, 7);
    CHECK(teng(a, b, 7));
    int c[] = { 1 };
    mening_saralashim(c, 1);
    CHECK_INT(c[0], 1);
    mening_saralashim(c, 0);
    int d[] = { INT_MAX, INT_MIN, 0 }, e[] = { INT_MIN, 0, INT_MAX };
    mening_saralashim(d, 3);
    CHECK(teng(d, e, 3));

    BOLIM("mening_saralashim: 200000 ta element (tezlik)");
    size_t n = 200000;
    int *x = malloc(n * sizeof(int)), *y = malloc(n * sizeof(int));
    for (size_t i = 0; i < n; i++)
        x[i] = y[i] = tasodifiy();
    qsort(y, n, sizeof(int), cmp_int);
    clock_t t0 = clock();
    mening_saralashim(x, n);
    double sek = (double)(clock() - t0) / CLOCKS_PER_SEC;
    CHECK(teng(x, y, n));
    printf("   (vaqt: %.2f s)\n", sek);
    CHECK(sek < 3.0);
    free(x);
    free(y);

    BOLIM("talabalarni_saralash");
    struct talaba t[] = {
        { "Vali", 80 }, { "Ali", 95 }, { "Zarina", 80 }, { "Bobur", 95 }, { "Dilnoza", 60 },
    };
    talabalarni_saralash(t, 5);
    CHECK_STR(t[0].ism, "Ali");
    CHECK_STR(t[1].ism, "Bobur");
    CHECK_STR(t[2].ism, "Vali");
    CHECK_STR(t[3].ism, "Zarina");
    CHECK_STR(t[4].ism, "Dilnoza");
    struct talaba u[] = { { "A", INT_MIN }, { "B", INT_MAX } };
    talabalarni_saralash(u, 2);                 /* ayirish bilan taqqoslash toshadi */
    CHECK_STR(u[0].ism, "B");
    TEST_TUGADI();
}
