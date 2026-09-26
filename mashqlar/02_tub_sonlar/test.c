#include "test.h"
#include "mashq.h"

int main(void)
{
    TEST_BOSHLA();
    BOLIM("tubmi: kichik sonlar");
    CHECK(!tubmi(0));
    CHECK(!tubmi(1));
    CHECK(tubmi(2));
    CHECK(tubmi(3));
    CHECK(!tubmi(4));
    CHECK(tubmi(97));
    CHECK(!tubmi(91));              /* 7 * 13 - aldamchi */
    CHECK(!tubmi(1000000));
    BOLIM("tubmi: katta sonlar (toshish tuzog'i)");
    CHECK(tubmi(2147483647u));      /* 2^31 - 1 */
    CHECK(tubmi(4294967291u));      /* eng katta 32 bitli tub son */
    CHECK(!tubmi(4294967295u));
    BOLIM("tublar_soni");
    CHECK_INT(tublar_soni(1), 0);
    CHECK_INT(tublar_soni(2), 1);
    CHECK_INT(tublar_soni(10), 4);
    CHECK_INT(tublar_soni(100), 25);
    CHECK_INT(tublar_soni(100000), 9592);
    TEST_TUGADI();
}
