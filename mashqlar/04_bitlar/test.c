#include "test.h"
#include "mashq.h"

int main(void)
{
    TEST_BOSHLA();
    BOLIM("bitlar_soni");
    CHECK_INT(bitlar_soni(0), 0);
    CHECK_INT(bitlar_soni(0xB), 3);
    CHECK_INT(bitlar_soni(0x80000000u), 1);
    CHECK_INT(bitlar_soni(0xFFFFFFFFu), 32);
    BOLIM("ikkining_darajasimi");
    CHECK(!ikkining_darajasimi(0));
    CHECK(ikkining_darajasimi(1));
    CHECK(ikkining_darajasimi(4096));
    CHECK(!ikkining_darajasimi(4095));
    CHECK(!ikkining_darajasimi(6));
    CHECK(ikkining_darajasimi(1ull << 63));
    BOLIM("yuqoriga_tekislash");
    CHECK_HEX(yuqoriga_tekislash(5, 4), 8);
    CHECK_HEX(yuqoriga_tekislash(8, 4), 8);
    CHECK_HEX(yuqoriga_tekislash(0, 4096), 0);
    CHECK_HEX(yuqoriga_tekislash(1, 4096), 4096);
    CHECK_HEX(yuqoriga_tekislash(0x12345, 0x1000), 0x13000);
    CHECK_HEX(yuqoriga_tekislash(17, 1), 17);
    BOLIM("bitni yoqish / o'chirish / tekshirish");
    CHECK_HEX(bitni_yoq(0, 0), 1);
    CHECK_HEX(bitni_yoq(0x10, 3), 0x18);
    CHECK_HEX(bitni_yoq(0, 31), 0x80000000u);
    CHECK_HEX(bitni_ochir(0xFF, 0), 0xFE);
    CHECK_HEX(bitni_ochir(0xFFFFFFFFu, 31), 0x7FFFFFFFu);
    CHECK_HEX(bitni_ochir(0x10, 3), 0x10);        /* allaqachon 0 */
    CHECK(bit_bormi(0x80000000u, 31));
    CHECK(!bit_bormi(0x7FFFFFFFu, 31));
    CHECK(bit_bormi(5, 2));
    CHECK(!bit_bormi(5, 1));
    TEST_TUGADI();
}
