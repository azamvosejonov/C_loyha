#include "test.h"
#include "mashq.h"

#define TOGRI(s, kutilgan)                          \
    do {                                            \
        long x_ = 12345;                            \
        CHECK_INT(satr_songa(s, &x_), 0);           \
        CHECK_INT(x_, kutilgan);                    \
    } while (0)

#define NOTOGRI(s, kod)                             \
    do {                                            \
        long x_ = 12345;                            \
        CHECK_INT(satr_songa(s, &x_), kod);         \
        CHECK_INT(x_, 12345);                       \
    } while (0)

int main(void)
{
    TEST_BOSHLA();
    BOLIM("to'g'ri sonlar");
    TOGRI("0", 0);
    TOGRI("7", 7);
    TOGRI("123", 123);
    TOGRI("-45", -45);
    TOGRI("+45", 45);
    TOGRI("007", 7);
    TOGRI("9223372036854775807", LONG_MAX);
    TOGRI("-9223372036854775808", LONG_MIN);
    BOLIM("noto'g'ri kirish -> -1");
    NOTOGRI("", -1);
    NOTOGRI("-", -1);
    NOTOGRI("+", -1);
    NOTOGRI("12a", -1);
    NOTOGRI("a12", -1);
    NOTOGRI(" 12", -1);
    NOTOGRI("1 2", -1);
    NOTOGRI("--1", -1);
    NOTOGRI("0x10", -1);
    BOLIM("toshish -> -2");
    NOTOGRI("9223372036854775808", -2);
    NOTOGRI("-9223372036854775809", -2);
    NOTOGRI("99999999999999999999999", -2);
    TEST_TUGADI();
}
