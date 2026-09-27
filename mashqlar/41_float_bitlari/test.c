#include "test.h"
#include "mashq.h"

static uint32_t bit(float f)
{
    uint32_t u;
    memcpy(&u, &f, 4);
    return u;
}

static float fl(uint32_t u)
{
    float f;
    memcpy(&f, &u, 4);
    return f;
}

#define AJRAT(f, is, ee, mm, dar, tt)                                   \
    do {                                                                \
        struct float_qism q_;                                           \
        memset(&q_, 0x7f, sizeof(q_));                                  \
        float_ajrat(bit(f), &q_);                                       \
        CHECK_INT(q_.ishora, is);                                       \
        CHECK_INT(q_.e, ee);                                            \
        CHECK_HEX(q_.m, mm);                                            \
        CHECK_INT(q_.daraja, dar);                                      \
        CHECK_INT(q_.tur, tt);                                          \
    } while (0)

int main(void)
{
    TEST_BOSHLA();
    BOLIM("float_ajrat");
    AJRAT(1.0f, 0, 127, 0, 0, F_NORMAL);
    AJRAT(-2.5f, 1, 128, 0x200000, 1, F_NORMAL);           /* -1.01b x 2^1 */
    AJRAT(6.5f, 0, 129, 0x500000, 2, F_NORMAL);
    AJRAT(0.0f, 0, 0, 0, 0, F_NOL);
    AJRAT(-0.0f, 1, 0, 0, 0, F_NOL);
    AJRAT(fl(0x00000001), 0, 0, 1, -126, F_DENORMAL);      /* eng kichik musbat float */
    AJRAT(fl(0x7F800000), 0, 255, 0, 0, F_CHEKSIZ);
    AJRAT(fl(0xFF800000), 1, 255, 0, 0, F_CHEKSIZ);
    AJRAT(fl(0x7FC00000), 0, 255, 0x400000, 0, F_NAN);

    BOLIM("butundan_float: aniq sonlar");
    CHECK_HEX(butundan_float(0), 0);
    CHECK_HEX(butundan_float(1), bit(1.0f));
    CHECK_HEX(butundan_float(-1), bit(-1.0f));
    CHECK_HEX(butundan_float(6), bit(6.0f));
    CHECK_HEX(butundan_float(16777216), bit(16777216.0f));  /* 2^24 */

    BOLIM("butundan_float: yaxlitlash (eng yaqiniga, teng bo'lsa juftiga)");
    CHECK_HEX(butundan_float(16777217), bit(16777216.0f));  /* teng - juftiga (pastga) */
    CHECK_HEX(butundan_float(16777219), bit(16777220.0f));  /* teng - juftiga (yuqoriga) */
    CHECK_HEX(butundan_float(16777218), bit(16777218.0f));
    CHECK_HEX(butundan_float(33554435), bit(33554436.0f));  /* 2^25 + 3 */
    CHECK_HEX(butundan_float(INT32_MAX), bit(2147483648.0f)); /* mantissa to'lib ketadi -> e++ */
    CHECK_HEX(butundan_float(INT32_MIN), bit(-2147483648.0f));
    CHECK_HEX(butundan_float(-16777217), bit(-16777216.0f));

    BOLIM("butundan_float: 200000 ta tasodifiy son");
    unsigned long rng = 99;
    int ok = 1;
    int32_t yomon = 0;
    for (int i = 0; i < 200000; i++) {
        rng = rng * 6364136223846793005UL + 1442695040888963407UL;
        int32_t x = (int32_t)(uint32_t)(rng >> 32);
        if (i % 3 == 0)
            x >>= (i % 29);                 /* har xil kattalikdagi sonlar */
        if (butundan_float(x) != bit((float)x)) {
            if (ok)
                yomon = x;
            ok = 0;
        }
    }
    CHECK(ok);
    if (!ok)
        printf("         birinchi mos kelmagan son: %d\n", yomon);
    TEST_TUGADI();
}
