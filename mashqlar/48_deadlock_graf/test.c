#include "test.h"
#include "mashq.h"

#define N(a) (sizeof(a) / sizeof((a)[0]))

/* Javob haqiqatan sikl ekanini tekshirish. */
static int qirra_bor(const int (*q)[2], size_t m, int u, int v)
{
    for (size_t i = 0; i < m; i++)
        if (q[i][0] == u && q[i][1] == v)
            return 1;
    return 0;
}

static int sikl_togri(int n, const int (*q)[2], size_t m, const int *s, int len)
{
    if (len < 1 || len > n)
        return 0;
    char *bor = calloc((size_t)n, 1);
    int ok = 1;
    for (int i = 0; i < len && ok; i++) {
        if (s[i] < 0 || s[i] >= n || bor[s[i]])
            ok = 0;                             /* takrorlanmasin */
        else
            bor[s[i]] = 1;
        if (ok && m < 100000)
            ok = qirra_bor(q, m, s[i], s[(i + 1) % len]);
    }
    free(bor);
    return ok;
}

int main(void)
{
    TEST_BOSHLA();
    int sikl[64], len = 0;

    BOLIM("sikl yo'q");
    const int a[][2] = { { 0, 1 }, { 1, 2 }, { 0, 2 }, { 2, 3 } };
    CHECK_INT(sikl_top(4, a, N(a), sikl, &len), 0);
    CHECK_INT(sikl_top(3, NULL, 0, sikl, &len), 0);

    BOLIM("oddiy sikl: 3 oqim bir-birini kutadi");
    const int b[][2] = { { 0, 1 }, { 1, 2 }, { 2, 0 }, { 3, 0 } };
    len = 0;
    CHECK_INT(sikl_top(4, b, N(b), sikl, &len), 1);
    CHECK_INT(len, 3);
    CHECK(sikl_togri(4, b, N(b), sikl, len));

    BOLIM("o'z-o'zini kutish");
    const int c[][2] = { { 0, 1 }, { 1, 1 } };
    CHECK_INT(sikl_top(2, c, N(c), sikl, &len), 1);
    CHECK_INT(len, 1);
    CHECK_INT(sikl[0], 1);

    BOLIM("sikl grafning chuqurida, ko'p tarmoqlar bilan");
    const int d[][2] = { { 0, 1 }, { 0, 2 }, { 2, 3 }, { 3, 4 }, { 1, 4 }, { 4, 5 },
                         { 5, 6 }, { 6, 7 }, { 7, 5 }, { 8, 0 } };
    CHECK_INT(sikl_top(9, d, N(d), sikl, &len), 1);
    CHECK_INT(len, 3);
    CHECK(sikl_togri(9, d, N(d), sikl, len));

    BOLIM("ikki qismli graf: faqat ikkinchisida sikl");
    const int e[][2] = { { 0, 1 }, { 1, 2 }, { 3, 4 }, { 4, 5 }, { 5, 6 }, { 6, 3 } };
    CHECK_INT(sikl_top(7, e, N(e), sikl, &len), 1);
    CHECK_INT(len, 4);
    CHECK(sikl_togri(7, e, N(e), sikl, len));

    BOLIM("1 000 000 tugunli zanjir (rekursiya stekni to'ldiradi!)");
    int n = 1000000;
    int (*z)[2] = malloc((size_t)n * sizeof(*z));
    for (int i = 0; i < n - 1; i++) {
        z[i][0] = i;
        z[i][1] = i + 1;
    }
    int *katta = malloc((size_t)n * sizeof(int));
    CHECK_INT(sikl_top(n, (const int (*)[2])z, (size_t)n - 1, katta, &len), 0);
    z[n - 1][0] = n - 1;                        /* oxiridan boshiga - ulkan sikl */
    z[n - 1][1] = 0;
    CHECK_INT(sikl_top(n, (const int (*)[2])z, (size_t)n, katta, &len), 1);
    CHECK_INT(len, n);
    int ok = len == n;
    for (int i = 0; ok && i < n; i++)
        ok = katta[(i + 1) % n] == (katta[i] + 1) % n;
    CHECK(ok);
    free(katta);
    free(z);
    TEST_TUGADI();
}
