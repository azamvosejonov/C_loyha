#include "test.h"
#include "mashq.h"

int main(void)
{
    TEST_BOSHLA();
    mm_init();
    size_t boshida = mm_bosh_joy();
    BOLIM("mm_init");
    CHECK(boshida > ARENA_HAJMI - 256 && boshida <= ARENA_HAJMI);

    BOLIM("oddiy ajratish");
    CHECK(mm_alloc(0) == NULL);
    char *p = mm_alloc(100);
    CHECK(p != NULL);
    if (!p)
        TEST_TUGADI();
    CHECK(((uintptr_t)p & 15) == 0);
    memset(p, 'A', 100);
    char *q = mm_alloc(1);
    CHECK(q != NULL && ((uintptr_t)q & 15) == 0);
    CHECK(q && (q >= p + 100 || q + 1 <= p));
    CHECK(mm_alloc(ARENA_HAJMI) == NULL);        /* sig'maydi */
    mm_free(q);
    mm_free(p);
    mm_free(NULL);
    CHECK_INT(mm_bosh_joy(), boshida);

    BOLIM("100 ta blok - bir-birini bosib ketmaydi");
    char *b[100];
    int ok = 1;
    for (int i = 0; i < 100; i++) {
        b[i] = mm_alloc((size_t)(i * 5 + 1));
        ok &= b[i] != NULL && ((uintptr_t)b[i] & 15) == 0;
        if (b[i])
            memset(b[i], i, (size_t)(i * 5 + 1));
    }
    CHECK(ok);
    for (int i = 0; ok && i < 100; i++)
        for (int k = 0; k < i * 5 + 1; k++)
            ok &= (unsigned char)b[i][k] == (unsigned char)i;
    CHECK(ok);

    BOLIM("birlashtirish: hammasini free qilgach katta blok sig'ishi kerak");
    for (int i = 0; i < 100; i += 2)
        mm_free(b[i]);
    for (int i = 1; i < 100; i += 2)
        mm_free(b[i]);
    CHECK_INT(mm_bosh_joy(), boshida);
    void *katta = mm_alloc(ARENA_HAJMI - 1024);
    CHECK(katta != NULL);
    mm_free(katta);

    BOLIM("qayta ishlatish: ko'p marta ajratish/bo'shatish");
    ok = 1;
    for (int r = 0; r < 2000; r++) {
        void *x = mm_alloc((size_t)(r % 500) + 1), *y = mm_alloc(3000);
        ok &= x && y;
        mm_free(x);
        mm_free(y);
    }
    CHECK(ok);
    CHECK_INT(mm_bosh_joy(), boshida);

    BOLIM("mm_init qayta boshlaydi");
    mm_alloc(1000);
    mm_init();
    CHECK_INT(mm_bosh_joy(), boshida);
    TEST_TUGADI();
}
