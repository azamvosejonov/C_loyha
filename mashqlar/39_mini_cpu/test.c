#include "test.h"
#include "mashq.h"

#define B(o, A, Bb, D, I) { (uint8_t)(o), (uint8_t)(A), (uint8_t)(Bb), (uint8_t)(D), (I) }
#define N(p) (sizeof(p) / sizeof((p)[0]))

static struct cpu m;

int main(void)
{
    TEST_BOSHLA();

    BOLIM("1 + 2 + ... + 100 (sikl)");
    const struct buyruq yig[] = {
        B(LI, 1, 0, 0, 0),              /* 0: r1 = 0      (yig'indi) */
        B(LI, 2, 0, 0, 100),            /* 1: r2 = 100    (i)        */
        B(LI, 3, 0, 0, 1),              /* 2: r3 = 1                 */
        B(ADD, 1, 1, 2, 0),             /* 3: r1 = r1 + r2           */
        B(SUB, 2, 2, 3, 0),             /* 4: r2 = r2 - 1            */
        B(JNZ, 2, 0, 0, 3),             /* 5: r2 != 0 -> 3           */
        B(LI, 0, 0, 0, 1),              /* 6: r0 = 1 (chiqarish)     */
        B(SYS, 0, 0, 0, 0),             /* 7: chiqar(r1)             */
        B(HALT, 0, 0, 0, 0),            /* 8                          */
    };
    memset(&m, 0x55, sizeof(m));
    CHECK_INT(bajar(&m, yig, N(yig), 100000), 0);
    CHECK_INT(m.chiqish_soni, 1);
    CHECK_INT(m.chiqish[0], 5050);
    CHECK_INT(m.sp, XOTIRA);

    BOLIM("10! rekursiya bilan (CALL / RET / PUSH / POP)");
    const struct buyruq fakt[] = {
        B(LI, 1, 0, 0, 10),             /* 0: r1 = 10                   */
        B(CALL, 0, 0, 0, 6),            /* 1: r2 = fakt(r1)             */
        B(MOV, 1, 2, 0, 0),             /* 2: r1 = r2                   */
        B(LI, 0, 0, 0, 1),              /* 3                            */
        B(SYS, 0, 0, 0, 0),             /* 4: chiqar(r1)                */
        B(HALT, 0, 0, 0, 0),            /* 5                            */
        B(JNZ, 1, 0, 0, 9),             /* 6: fakt: r1 != 0 -> 9        */
        B(LI, 2, 0, 0, 1),              /* 7: r2 = 1                    */
        B(RET, 0, 0, 0, 0),             /* 8: return 1                  */
        B(PUSH, 1, 0, 0, 0),            /* 9: n ni saqlash              */
        B(LI, 3, 0, 0, 1),              /* 10                           */
        B(SUB, 1, 1, 3, 0),             /* 11: r1 = n - 1               */
        B(CALL, 0, 0, 0, 6),            /* 12: r2 = fakt(n - 1)         */
        B(POP, 1, 0, 0, 0),             /* 13: r1 = n                   */
        B(MUL, 2, 2, 1, 0),             /* 14: r2 = r2 * n              */
        B(RET, 0, 0, 0, 0),             /* 15                           */
    };
    CHECK_INT(bajar(&m, fakt, N(fakt), 100000), 0);
    CHECK_INT(m.chiqish_soni, 1);
    CHECK_INT(m.chiqish[0], 3628800);
    CHECK_INT(m.sp, XOTIRA);

    BOLIM("xotira: LOAD / STORE");
    const struct buyruq xot[] = {
        B(LI, 1, 0, 0, 0),              /* 0: r1 = 0 (manzil)   */
        B(LI, 4, 0, 0, 1),              /* 1: r4 = 1            */
        B(LI, 5, 0, 0, 0),              /* 2: r5 = 0 (yig'indi) */
        B(LOAD, 2, 1, 0, 0),            /* 3: r2 = x[r1]        */
        B(JZ, 2, 0, 0, 8),              /* 4: 0 - oxiri         */
        B(ADD, 5, 5, 2, 0),             /* 5                    */
        B(ADD, 1, 1, 4, 0),             /* 6: r1++              */
        B(JMP, 0, 0, 0, 3),             /* 7                    */
        B(STORE, 5, 1, 0, 0),           /* 8: x[r1] = yig'indi  */
        B(LI, 0, 0, 0, 2),              /* 9                    */
        B(MOV, 1, 5, 0, 0),             /* 10                   */
        B(SYS, 0, 0, 0, 0),             /* 11: chiqish(yig'indi) */
    };
    memset(m.xotira, 0, sizeof(m.xotira));
    for (int i = 0; i < 10; i++)
        m.xotira[i] = (i + 1) * (i + 1);        /* 1 4 9 ... 100, keyin 0 */
    CHECK_INT(bajar(&m, xot, N(xot), 100000), 0);
    CHECK_INT(m.chiqish_kodi, 385);
    CHECK_INT(m.xotira[10], 385);

    BOLIM("arifmetika aylanadi (UB yo'q)");
    const struct buyruq ayl[] = {
        B(LI, 1, 0, 0, INT64_MAX), B(LI, 2, 0, 0, 1), B(ADD, 1, 1, 2, 0),
        B(LI, 0, 0, 0, 1), B(SYS, 0, 0, 0, 0),
        B(LI, 3, 0, 0, INT64_MIN), B(MUL, 1, 3, 3, 0), B(SYS, 0, 0, 0, 0), B(HALT, 0, 0, 0, 0),
    };
    CHECK_INT(bajar(&m, ayl, N(ayl), 100), 0);
    CHECK_INT(m.chiqish[0], INT64_MIN);
    CHECK_INT(m.chiqish[1], 0);

    BOLIM("istisnolar");
    const struct buyruq e1[] = { B(OP_SONI, 0, 0, 0, 0) };
    CHECK_INT(bajar(&m, e1, 1, 10), -1);
    const struct buyruq e2[] = { B(ADD, 9, 0, 0, 0) };
    CHECK_INT(bajar(&m, e2, 1, 10), -1);
    const struct buyruq e3[] = { B(LI, 1, 0, 0, 300), B(LOAD, 2, 1, 0, 0) };
    CHECK_INT(bajar(&m, e3, 2, 10), -2);
    const struct buyruq e4[] = { B(LI, 1, 0, 0, -1), B(STORE, 2, 1, 0, 0) };
    CHECK_INT(bajar(&m, e4, 2, 10), -2);
    const struct buyruq e5[] = { B(CALL, 0, 0, 0, 0) };               /* cheksiz rekursiya */
    CHECK_INT(bajar(&m, e5, 1, 100000), -3);
    const struct buyruq e6[] = { B(POP, 1, 0, 0, 0) };
    CHECK_INT(bajar(&m, e6, 1, 10), -3);
    const struct buyruq e7[] = { B(RET, 0, 0, 0, 0) };
    CHECK_INT(bajar(&m, e7, 1, 10), -3);
    const struct buyruq e8[] = { B(JMP, 0, 0, 0, 1000) };
    CHECK_INT(bajar(&m, e8, 1, 10), -4);
    const struct buyruq e9[] = { B(LI, 1, 0, 0, 1) };                 /* HALT'siz oxirga yetdi */
    CHECK_INT(bajar(&m, e9, 1, 10), -4);
    const struct buyruq e10[] = { B(JMP, 0, 0, 0, 0) };               /* cheksiz sikl */
    CHECK_INT(bajar(&m, e10, 1, 1000), -5);
    const struct buyruq e11[] = { B(LI, 0, 0, 0, 99), B(SYS, 0, 0, 0, 0) };
    CHECK_INT(bajar(&m, e11, 2, 10), -6);
    const struct buyruq e12[] = { B(LI, 0, 0, 0, 1), B(SYS, 0, 0, 0, 0), B(JMP, 0, 0, 0, 1) };
    CHECK_INT(bajar(&m, e12, 3, 100000), -6);                         /* chiqish buferi to'ldi */
    CHECK_INT(m.chiqish_soni, CHIQISH_MAKS);
    TEST_TUGADI();
}
