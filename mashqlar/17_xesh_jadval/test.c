#include <time.h>

#include "test.h"
#include "mashq.h"

int main(void)
{
    TEST_BOSHLA();
    BOLIM("xesh_yarat");
    struct xesh *h = xesh_yarat();
    CHECK(h != NULL);
    if (!h)
        TEST_TUGADI();
    CHECK_INT(xesh_soni(h), 0);

    BOLIM("qo'yish, olish, yangilash");
    int v = 0;
    CHECK_INT(xesh_qoy(h, "olma", 5), 0);
    CHECK_INT(xesh_qoy(h, "nok", 7), 0);
    CHECK_INT(xesh_ol(h, "olma", &v), 0);
    CHECK_INT(v, 5);
    CHECK_INT(xesh_ol(h, "nok", &v), 0);
    CHECK_INT(v, 7);
    CHECK_INT(xesh_ol(h, "uzum", &v), -1);
    CHECK_INT(xesh_qoy(h, "olma", 50), 0);      /* yangilash */
    CHECK_INT(xesh_ol(h, "olma", &v), 0);
    CHECK_INT(v, 50);
    CHECK_INT(xesh_soni(h), 2);

    BOLIM("kalit nusxalanadimi");
    char kalit[16];
    strcpy(kalit, "anor");
    xesh_qoy(h, kalit, 9);
    strcpy(kalit, "XXXX");                      /* chaqiruvchi satrini o'zgartirdi */
    CHECK_INT(xesh_ol(h, "anor", &v), 0);
    CHECK_INT(v, 9);

    BOLIM("o'chirish");
    CHECK_INT(xesh_ochir(h, "nok"), 0);
    CHECK_INT(xesh_ochir(h, "nok"), -1);
    CHECK_INT(xesh_ol(h, "nok", &v), -1);
    CHECK_INT(xesh_soni(h), 2);

    BOLIM("100000 ta kalit (tezlik: rehash kerak)");
    clock_t t0 = clock();
    char k[32];
    int ok = 1;
    for (int i = 0; i < 100000; i++) {
        snprintf(k, sizeof(k), "kalit-%d", i);
        ok &= xesh_qoy(h, k, i) == 0;
    }
    CHECK(ok);
    CHECK_INT(xesh_soni(h), 100002);
    ok = 1;
    for (int i = 0; i < 100000; i += 7) {
        snprintf(k, sizeof(k), "kalit-%d", i);
        ok &= xesh_ol(h, k, &v) == 0 && v == i;
    }
    CHECK(ok);
    for (int i = 0; i < 100000; i += 2) {
        snprintf(k, sizeof(k), "kalit-%d", i);
        xesh_ochir(h, k);
    }
    CHECK_INT(xesh_soni(h), 50002);
    double sek = (double)(clock() - t0) / CLOCKS_PER_SEC;
    printf("   (vaqt: %.2f s)\n", sek);
    CHECK(sek < 5.0);
    xesh_ozod(h);
    TEST_TUGADI();
}
