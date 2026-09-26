#include "test.h"
#include "mashq.h"

int main(void)
{
    TEST_BOSHLA();
    struct vec v;
    memset(&v, 0x55, sizeof(v));        /* "axlat" - vec_init tozalashi kerak */
    BOLIM("vec_init");
    vec_init(&v);
    CHECK(v.data == NULL && v.len == 0 && v.cap == 0);
    if (v.len != 0 || v.cap != 0) {
        printf("  (vec_init ishlamaguncha qolgan testlar o'tkazib yuboriladi)\n");
        TEST_TUGADI();
    }

    BOLIM("vec_push: 100000 ta element");
    int ok = 1;
    for (int i = 0; i < 100000; i++)
        ok &= vec_push(&v, i * 3) == 0;
    CHECK(ok);
    CHECK_INT(v.len, 100000);
    CHECK(v.cap >= v.len);
    if (v.len == 100000 && v.data) {
        ok = 1;
        for (int i = 0; i < 100000; i++)
            ok &= v.data[i] == i * 3;
        CHECK(ok);
    }

    BOLIM("vec_pop");
    int x = -1;
    CHECK_INT(vec_pop(&v, &x), 0);
    CHECK_INT(x, 99999 * 3);
    CHECK_INT(v.len, 99999);

    BOLIM("vec_free");
    vec_free(&v);
    CHECK(v.data == NULL && v.len == 0 && v.cap == 0);
    x = 777;
    CHECK_INT(vec_pop(&v, &x), -1);
    CHECK_INT(x, 777);

    BOLIM("vec_insert");
    vec_init(&v);
    CHECK_INT(vec_insert(&v, 0, 2), 0);        /* [2] */
    CHECK_INT(vec_insert(&v, 0, 1), 0);        /* [1, 2] */
    CHECK_INT(vec_insert(&v, 2, 4), 0);        /* [1, 2, 4] */
    CHECK_INT(vec_insert(&v, 2, 3), 0);        /* [1, 2, 3, 4] */
    CHECK_INT(vec_insert(&v, 9, 5), -1);       /* i > len */
    CHECK_INT(v.len, 4);
    if (v.len == 4 && v.data)
        CHECK(v.data[0] == 1 && v.data[1] == 2 && v.data[2] == 3 && v.data[3] == 4);
    for (int i = 0; i < 1000; i++)
        vec_insert(&v, v.len / 2, i);
    CHECK_INT(v.len, 1004);
    vec_free(&v);
    TEST_TUGADI();
}
