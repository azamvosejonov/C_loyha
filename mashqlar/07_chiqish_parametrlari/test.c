#include "test.h"
#include "mashq.h"

int main(void)
{
    TEST_BOSHLA();
    BOLIM("min_max");
    int a[] = { 4, -2, 9, 0, 9, -2 };
    int mn = 111, mx = 222;
    CHECK_INT(min_max(a, 6, &mn, &mx), 0);
    CHECK_INT(mn, -2);
    CHECK_INT(mx, 9);
    mn = 111, mx = 222;
    CHECK_INT(min_max(a, 1, &mn, &mx), 0);
    CHECK_INT(mn, 4);
    CHECK_INT(mx, 4);
    mn = 111, mx = 222;
    CHECK_INT(min_max(a, 0, &mn, &mx), -1);
    CHECK(mn == 111 && mx == 222);

    BOLIM("qidir");
    int b[] = { 7, 1, 7, 7, 3, 7 };
    size_t *idx = malloc(10 * sizeof(size_t));   /* aniq 10 o'rin */
    CHECK_INT(qidir(b, 6, 7, idx, 10), 4);
    CHECK(idx[0] == 0 && idx[1] == 2 && idx[2] == 3 && idx[3] == 5);
    CHECK_INT(qidir(b, 6, 42, idx, 10), 0);
    free(idx);
    idx = malloc(2 * sizeof(size_t));           /* faqat 2 o'rin! */
    CHECK_INT(qidir(b, 6, 7, idx, 2), 4);       /* jami 4 ta, lekin 2 tasi yoziladi */
    CHECK(idx[0] == 0 && idx[1] == 2);
    free(idx);
    CHECK_INT(qidir(b, 6, 7, NULL, 0), 4);      /* faqat sanash */
    TEST_TUGADI();
}
