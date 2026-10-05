/* atomik.S — A kengaytmasi: lr.w/sc.w, amo*.w */
#include "test_makro.h"
BOSHLASH
    la s0, joy
    li t0, 10
    sw t0, 0(s0)
    li t1, 5
    amoadd.w a0, t1, (s0)           /* a0 = eski (10), xotira = 15 */
    SINA(1, a0, 10)
    lw a0, 0(s0)
    SINA(2, a0, 15)
    li t1, 99
    amoswap.w a0, t1, (s0)
    SINA(3, a0, 15)
    lw a0, 0(s0)
    SINA(4, a0, 99)
    li t1, 0x0f
    amoand.w a0, t1, (s0)           /* 99 & 15 = 3 */
    lw a0, 0(s0)
    SINA(5, a0, 3)
    li t1, 0x30
    amoor.w a0, t1, (s0)
    lw a0, 0(s0)
    SINA(6, a0, 0x33)
    li t1, 0xff
    amoxor.w a0, t1, (s0)
    lw a0, 0(s0)
    SINA(7, a0, 0xcc)
    li t1, -5
    amomin.w a0, t1, (s0)           /* ishorali: min(0xcc, -5) = -5 */
    lw a0, 0(s0)
    SINA(8, a0, -5)
    li t1, 7
    amomax.w a0, t1, (s0)           /* max(-5, 7) = 7 */
    lw a0, 0(s0)
    SINA(9, a0, 7)
    li t1, -1
    amominu.w a0, t1, (s0)          /* ishorasiz: min(7, 0xffffffff) = 7 */
    lw a0, 0(s0)
    SINA(10, a0, 7)
    amomaxu.w a0, t1, (s0)
    lw a0, 0(s0)
    SINA(11, a0, -1)
    /* lr/sc: muvaffaqiyatli */
    lr.w a0, (s0)
    li t1, 42
    sc.w a1, t1, (s0)
    SINA(12, a1, 0)
    lw a0, 0(s0)
    SINA(13, a0, 42)
    /* lr/sc: band qilishsiz sc muvaffaqiyatsiz va YOZMAYDI */
    li t1, 77
    sc.w a1, t1, (s0)
    SINA(14, a1, 1)
    lw a0, 0(s0)
    SINA(15, a0, 42)
    /* boshqa manzilga sc — muvaffaqiyatsiz */
    lr.w a0, (s0)
    addi t5, s0, 4
    sc.w a1, t1, (t5)
    SINA(16, a1, 1)
    /* atomik hisoblagich sikli (spinlock g'oyasi): 100 marta +1 */
    li t2, 100
    sw zero, 4(s0)
    addi s1, s0, 4
1:  lr.w t3, (s1)
    addi t3, t3, 1
    sc.w t4, t3, (s1)
    bnez t4, 1b
    addi t2, t2, -1
    bnez t2, 1b
    lw a0, 0(s1)
    SINA(17, a0, 100)
    j test_otdi
TUGATISH
    .data
    .balign 4
joy: .word 0, 0
