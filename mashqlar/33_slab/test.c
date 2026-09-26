#include "test.h"
#include "mashq.h"

static int joyida(const void *p, size_t hajm)
{
    uintptr_t a = (uintptr_t)p;
    return p && (a & 15) == 0 && (a & ~(uintptr_t)4095) == ((a + hajm - 1) & ~(uintptr_t)4095);
}

static unsigned long rng = 3;
static unsigned tasodif(unsigned n)
{
    rng = rng * 6364136223846793005UL + 1442695040888963407UL;
    return (unsigned)(rng >> 33) % n;
}

int main(void)
{
    TEST_BOSHLA();
    BOLIM("kesh_yarat");
    struct kesh *k = kesh_yarat(64);
    CHECK(k != NULL);
    if (!k)
        TEST_TUGADI();
    CHECK_INT(kesh_slablar(k), 0);

    BOLIM("1000 ta 64 baytli obyekt");
    void **p = malloc(1000 * sizeof(void *));
    int ok = 1;
    for (int i = 0; i < 1000; i++) {
        p[i] = kesh_ol(k);
        ok &= joyida(p[i], 64);
        if (p[i])
            memset(p[i], i & 0xff, 64);
    }
    CHECK(ok);
    CHECK(kesh_slablar(k) >= 16 && kesh_slablar(k) <= 18);
    printf("   (slablar: %zu)\n", kesh_slablar(k));
    ok = 1;
    for (int i = 0; ok && i < 1000; i++)
        for (int j = 0; j < 64; j++)
            ok &= ((unsigned char *)p[i])[j] == (i & 0xff);
    CHECK(ok);                                  /* bir-birini bosib ketmagan */

    BOLIM("hammasini qaytarish -> slablar bo'shatiladi");
    for (int i = 0; i < 1000; i += 2)
        kesh_ber(k, p[i]);
    for (int i = 1; i < 1000; i += 2)
        kesh_ber(k, p[i]);
    CHECK(kesh_slablar(k) <= 1);

    BOLIM("qayta ishlatish - yangi slab kerak emas");
    void *q = kesh_ol(k);
    CHECK(joyida(q, 64));
    CHECK(kesh_slablar(k) == 1);
    kesh_ber(k, q);

    BOLIM("turli hajmlar: 1, 100, 1000 bayt");
    struct kesh *k1 = kesh_yarat(1), *k2 = kesh_yarat(100), *k3 = kesh_yarat(1000);
    CHECK(k1 && k2 && k3);
    if (k1 && k2 && k3) {
        void *a = kesh_ol(k1), *b = kesh_ol(k2), *c = kesh_ol(k3), *c2 = kesh_ol(k3), *c3 = kesh_ol(k3);
        CHECK(joyida(a, 1) && joyida(b, 100) && joyida(c, 1000) && joyida(c2, 1000) && joyida(c3, 1000));
        CHECK(c != c2 && c2 != c3 && c != c3);
        kesh_ber(k1, a);
        kesh_ber(k2, b);
        kesh_ber(k3, c);
        kesh_ber(k3, c2);
        kesh_ber(k3, c3);
    }

    BOLIM("tasodifiy stress");
    int n = 0;
    ok = 1;
    for (int r = 0; r < 30000; r++) {
        if (n < 1000 && (n == 0 || tasodif(2))) {
            p[n] = kesh_ol(k);
            ok &= joyida(p[n], 64);
            if (p[n])
                memset(p[n], 0x5a, 64);
            n++;
        } else {
            int i = (int)tasodif((unsigned)n);
            ok &= ((unsigned char *)p[i])[63] == 0x5a;
            kesh_ber(k, p[i]);
            p[i] = p[--n];
        }
    }
    CHECK(ok);
    while (n > 0)
        kesh_ber(k, p[--n]);
    CHECK(kesh_slablar(k) <= 1);
    free(p);

    BOLIM("kesh_yoq (LeakSanitizer hamma slablar ozod qilinganini tekshiradi)");
    kesh_yoq(k);
    if (k1 && k2 && k3) {
        kesh_ol(k2);                            /* ishlatilayotgan obyekt bilan yo'q qilish */
        kesh_yoq(k1);
        kesh_yoq(k2);
        kesh_yoq(k3);
    }
    TEST_TUGADI();
}
