#include "test.h"
#include "mashq.h"

int main(void)
{
    TEST_BOSHLA();
    BOLIM("kichik sonlar");
    CHECK_INT(kvadratlar_yigindisi(1), 1);
    CHECK_INT(kvadratlar_yigindisi(2), 5);
    CHECK_INT(kvadratlar_yigindisi(3), 14);
    CHECK_INT(kvadratlar_yigindisi(10), 385);
    BOLIM("chegaraviy holatlar: 0 va manfiy");
    CHECK_INT(kvadratlar_yigindisi(0), 0);
    CHECK_INT(kvadratlar_yigindisi(-5), 0);
    BOLIM("katta son: int toshib ketadi");
    CHECK_INT(kvadratlar_yigindisi(100000), 333338333350000LL);
    TEST_TUGADI();
}
