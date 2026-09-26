#include "test.h"
#include "mashq.h"

#define OK(s, kutilgan)                         \
    do {                                        \
        long r_ = 777;                          \
        CHECK_INT(rpn(s, &r_), 0);              \
        CHECK_INT(r_, kutilgan);                \
    } while (0)

#define XATO(s)                                 \
    do {                                        \
        long r_ = 777;                          \
        CHECK_INT(rpn(s, &r_), -1);             \
        CHECK_INT(r_, 777);                     \
    } while (0)

int main(void)
{
    TEST_BOSHLA();
    BOLIM("to'g'ri ifodalar");
    OK("42", 42);
    OK("3 4 +", 7);
    OK("3 4 + 2 *", 14);
    OK("5 1 2 + 4 * + 3 -", 14);
    OK("2 -3 *", -6);
    OK("10 3 -", 7);
    OK("7 2 /", 3);
    OK("   1    2   +  ", 3);
    OK("-5", -5);
    OK("1 2 3 4 5 + + + +", 15);
    BOLIM("xatolar");
    XATO("");
    XATO("   ");
    XATO("+");
    XATO("1 +");
    XATO("1 2");
    XATO("4 0 /");
    XATO("2 x +");
    XATO("12a");
    XATO("1 2 ^");
    TEST_TUGADI();
}
