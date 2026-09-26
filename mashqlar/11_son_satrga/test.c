#include "test.h"
#include "mashq.h"

#define SON(x, kutilgan)                                                 \
    do {                                                                 \
        size_t h_ = strlen(kutilgan) + 1;                                \
        char *b_ = calloc(h_, 1);                                           \
        CHECK_INT(son_satrga(x, b_, h_), (long long)strlen(kutilgan));   \
        CHECK_STR(b_, kutilgan);                                         \
        free(b_);                                                        \
    } while (0)

#define HEX(x, kutilgan)                                                 \
    do {                                                                 \
        size_t h_ = strlen(kutilgan) + 1;                                \
        char *b_ = calloc(h_, 1);                                           \
        CHECK_INT(hex_satrga(x, b_, h_), (long long)strlen(kutilgan));   \
        CHECK_STR(b_, kutilgan);                                         \
        free(b_);                                                        \
    } while (0)

int main(void)
{
    TEST_BOSHLA();
    BOLIM("son_satrga (bufer aniq o'lchamda)");
    SON(0, "0");
    SON(7, "7");
    SON(42, "42");
    SON(-42, "-42");
    SON(1000000, "1000000");
    SON(LONG_MAX, "9223372036854775807");
    SON(LONG_MIN, "-9223372036854775808");
    BOLIM("son_satrga: sig'maydi");
    char *b = calloc(3, 1);
    CHECK_INT(son_satrga(123, b, 3), -1);     /* "123" + '\0' = 4 bayt kerak */
    CHECK_INT(son_satrga(-12, b, 3), -1);
    CHECK_INT(son_satrga(12, b, 3), 2);
    CHECK_STR(b, "12");
    free(b);
    BOLIM("hex_satrga");
    HEX(0, "0");
    HEX(255, "ff");
    HEX(4096, "1000");
    HEX(0xdeadbeefUL, "deadbeef");
    HEX(0xffffffff80000000UL, "ffffffff80000000");
    b = malloc(2);
    CHECK_INT(hex_satrga(256, b, 2), -1);
    free(b);
    TEST_TUGADI();
}
