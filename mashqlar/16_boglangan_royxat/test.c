#include "test.h"
#include "mashq.h"

/* Ro'yxatni "3 1 2" ko'rinishidagi satrga aylantirish (tekshirish uchun). */
static const char *korinish(const struct tugun *p)
{
    static char buf[256];
    size_t k = 0;
    buf[0] = '\0';
    for (int i = 0; p && i < 20; p = p->keyingi, i++)
        k += (size_t)snprintf(buf + k, sizeof(buf) - k, k ? " %d" : "%d", p->qiymat);
    return buf;
}

int main(void)
{
    TEST_BOSHLA();
    BOLIM("bo'sh ro'yxat");
    CHECK_INT(uzunlik(NULL), 0);
    CHECK(teskari_royxat(NULL) == NULL);
    royxat_ozod(NULL);

    BOLIM("boshiga_qosh va uzunlik");
    struct tugun *r = NULL;
    for (int i = 1; i <= 5; i++)
        r = boshiga_qosh(r, i);
    CHECK_INT(uzunlik(r), 5);
    CHECK_STR(korinish(r), "5 4 3 2 1");

    BOLIM("teskari_royxat");
    r = teskari_royxat(r);
    CHECK_STR(korinish(r), "1 2 3 4 5");
    struct tugun *bitta = boshiga_qosh(NULL, 9);
    bitta = teskari_royxat(bitta);
    CHECK_STR(korinish(bitta), "9");
    royxat_ozod(bitta);

    BOLIM("ochir");
    r = boshiga_qosh(r, 3);             /* 3 1 2 3 4 5 */
    r = boshiga_qosh(r, 3);             /* 3 3 1 2 3 4 5 */
    r = ochir(r, 3);
    CHECK_STR(korinish(r), "1 2 4 5");
    r = ochir(r, 5);                    /* oxirgisi */
    CHECK_STR(korinish(r), "1 2 4");
    r = ochir(r, 42);                   /* yo'q */
    CHECK_STR(korinish(r), "1 2 4");
    r = ochir(r, 1);
    r = ochir(r, 2);
    r = ochir(r, 4);
    CHECK(r == NULL);

    BOLIM("royxat_ozod: 10000 ta tugun");
    for (int i = 0; i < 10000; i++)
        r = boshiga_qosh(r, i);
    CHECK_INT(uzunlik(r), 10000);
    royxat_ozod(r);
    TEST_TUGADI();
}
