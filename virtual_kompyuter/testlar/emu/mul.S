/* mul.S — M kengaytmasi, chegara holatlari (spetsifikatsiya jadvali) */
#include "test_makro.h"
BOSHLASH
    li a0, 7
    li a1, -3
    mul a2, a0, a1
    SINA(1, a2, -21)
    li a0, 0x80000000
    li a1, 0x80000000
    mulh a2, a0, a1                 /* (-2^31)^2 = 2^62 -> yuqori 32 bit = 0x40000000 */
    SINA(2, a2, 0x40000000)
    mulhu a2, a0, a1                /* (2^31)^2 = 2^62 */
    SINA(3, a2, 0x40000000)
    li a0, -1
    li a1, -1
    mulhu a2, a0, a1                /* (2^32-1)^2 yuqori = 0xfffffffe */
    SINA(4, a2, 0xfffffffe)
    mulh a2, a0, a1                 /* (-1)*(-1) = 1 -> yuqori 0 */
    SINA(5, a2, 0)
    mulhsu a2, a0, a1               /* -1 * (2^32-1) -> yuqori 0xffffffff */
    SINA(6, a2, 0xffffffff)
    li a0, -7
    li a1, 2
    div a2, a0, a1                  /* nolga qarab yaxlitlash: -3 */
    SINA(7, a2, -3)
    rem a2, a0, a1                  /* qoldiq ishorasi bo'linuvchiniki: -1 */
    SINA(8, a2, -1)
    divu a2, a0, a1
    SINA(9, a2, 0x7ffffffc)
    remu a2, a0, a1
    SINA(10, a2, 1)
    li a1, 0
    div a2, a0, a1                  /* nolga bo'lish: -1, istisno YO'Q */
    SINA(11, a2, -1)
    divu a2, a0, a1
    SINA(12, a2, 0xffffffff)
    rem a2, a0, a1                  /* x % 0 = x */
    SINA(13, a2, -7)
    remu a2, a0, a1
    SINA(14, a2, -7)
    li a0, 0x80000000
    li a1, -1
    div a2, a0, a1                  /* toshish: INT_MIN / -1 = INT_MIN */
    SINA(15, a2, 0x80000000)
    rem a2, a0, a1
    SINA(16, a2, 0)
    j test_otdi
TUGATISH
