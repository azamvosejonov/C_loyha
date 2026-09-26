#include "test.h"
#include "mashq.h"

int main(void)
{
    TEST_BOSHLA();
    int r;
    BOLIM("qo'shish: oddiy");
    r = 0;
    CHECK_INT(xavfsiz_qoshish(5, 7, &r), 0);
    CHECK_INT(r, 12);
    CHECK_INT(xavfsiz_qoshish(-5, -7, &r), 0);
    CHECK_INT(r, -12);
    CHECK_INT(xavfsiz_qoshish(INT_MAX, 0, &r), 0);
    CHECK_INT(r, INT_MAX);
    CHECK_INT(xavfsiz_qoshish(INT_MAX, INT_MIN, &r), 0);
    CHECK_INT(r, -1);
    BOLIM("qo'shish: toshish");
    r = 777;
    CHECK_INT(xavfsiz_qoshish(INT_MAX, 1, &r), -1);
    CHECK_INT(r, 777);              /* toshganda natijaga tegilmaydi */
    CHECK_INT(xavfsiz_qoshish(INT_MIN, -1, &r), -1);
    CHECK_INT(xavfsiz_qoshish(INT_MAX / 2 + 1, INT_MAX / 2 + 1, &r), -1);
    BOLIM("ko'paytirish: oddiy");
    CHECK_INT(xavfsiz_kopaytirish(6, 7, &r), 0);
    CHECK_INT(r, 42);
    CHECK_INT(xavfsiz_kopaytirish(46340, 46340, &r), 0);
    CHECK_INT(r, 2147395600);
    CHECK_INT(xavfsiz_kopaytirish(0, INT_MIN, &r), 0);
    CHECK_INT(r, 0);
    CHECK_INT(xavfsiz_kopaytirish(-1, INT_MAX, &r), 0);
    CHECK_INT(r, -INT_MAX);
    BOLIM("ko'paytirish: toshish");
    r = 777;
    CHECK_INT(xavfsiz_kopaytirish(46341, 46341, &r), -1);
    CHECK_INT(r, 777);
    CHECK_INT(xavfsiz_kopaytirish(INT_MIN, -1, &r), -1);   /* -INT_MIN sig'maydi! */
    CHECK_INT(xavfsiz_kopaytirish(INT_MAX, 2, &r), -1);
    TEST_TUGADI();
}
