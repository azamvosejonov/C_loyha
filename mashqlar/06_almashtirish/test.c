#include "test.h"
#include "mashq.h"

static int tekshir3(int x, int y, int z)
{
    int a = x, b = y, c = z;
    uchtasini_saralash(&a, &b, &c);
    int m1 = x < y ? x : y, mn = m1 < z ? m1 : z;
    int M1 = x > y ? x : y, mx = M1 > z ? M1 : z;
    return a == mn && c == mx && a <= b && b <= c && a + b + c == x + y + z;
}

int main(void)
{
    TEST_BOSHLA();
    BOLIM("almashtir");
    int x = 3, y = 8;
    almashtir(&x, &y);
    CHECK_INT(x, 8);
    CHECK_INT(y, 3);
    int z = 5;
    almashtir(&z, &z);              /* o'zi bilan - o'zgarmasligi kerak */
    CHECK_INT(z, 5);
    BOLIM("uchtasini_saralash - 6 ta tartib");
    CHECK(tekshir3(1, 2, 3));
    CHECK(tekshir3(1, 3, 2));
    CHECK(tekshir3(2, 1, 3));
    CHECK(tekshir3(2, 3, 1));
    CHECK(tekshir3(3, 1, 2));
    CHECK(tekshir3(3, 2, 1));
    CHECK(tekshir3(5, 5, -1));
    CHECK(tekshir3(-7, 0, -7));
    TEST_TUGADI();
}
