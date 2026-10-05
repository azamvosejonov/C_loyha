/* alu.S — RV32I arifmetik va mantiqiy buyruqlar */
#include "test_makro.h"
BOSHLASH
    li a0, 5
    li a1, 7
    add a2, a0, a1
    SINA(1, a2, 12)
    sub a2, a0, a1
    SINA(2, a2, -2)
    li a3, 0x7fffffff
    addi a4, a3, 1                  /* toshish: aylanib manfiyga o'tadi (istisno YO'Q) */
    SINA(3, a4, 0x80000000)
    addi a2, zero, -1
    SINA(4, a2, 0xffffffff)
    li a0, -8
    srai a2, a0, 1                  /* arifmetik: ishora saqlanadi */
    SINA(5, a2, -4)
    srli a2, a0, 28                 /* mantiqiy: chapdan nol */
    SINA(6, a2, 0xf)
    slli a2, a0, 4
    SINA(7, a2, -128)
    li a1, 3
    sra a2, a0, a1
    SINA(8, a2, -1)
    srl a2, a0, a1
    SINA(9, a2, 0x1fffffff)
    sll a2, a1, a1
    SINA(10, a2, 24)
    li a1, 35                       /* siljish faqat pastki 5 bit: 35 & 31 = 3 */
    sll a2, a1, a1
    SINA(11, a2, 280)
    li a0, -1
    li a1, 1
    slt a2, a0, a1                  /* -1 < 1 (ishorali) */
    SINA(12, a2, 1)
    sltu a2, a0, a1                 /* 0xffffffff < 1 ? (ishorasiz) yo'q */
    SINA(13, a2, 0)
    slti a2, a0, 0
    SINA(14, a2, 1)
    sltiu a2, a1, -1                /* 1 < 0xffffffff (ishorasiz) */
    SINA(15, a2, 1)
    li a0, 0xf0f0
    li a1, 0xff00
    xor a2, a0, a1
    SINA(16, a2, 0x0ff0)
    or a2, a0, a1
    SINA(17, a2, 0xfff0)
    and a2, a0, a1
    SINA(18, a2, 0xf000)
    xori a2, a0, -1                 /* not */
    SINA(19, a2, 0xffff0f0f)
    andi a2, a0, 0x0ff
    SINA(20, a2, 0xf0)
    ori a2, a0, 0x00f
    SINA(21, a2, 0xf0ff)
    lui a2, 0x12345
    SINA(22, a2, 0x12345000)
1:  auipc a2, 0                     /* a2 = shu buyruq manzili */
    la a3, 1b                       /* belgining manzili (bog'lovchi hisoblaydi) */
    li gp, 23
    bne a2, a3, test_xato
    li a2, 0
    addi a2, a2, 2047               /* eng katta musbat 12 bitli o'zgarmas */
    addi a2, a2, -2048              /* eng kichik manfiy */
    SINA(24, a2, -1)
    li a2, 123
    add zero, a2, a2                /* x0 ga yozish e'tiborsiz */
    SINA(25, zero, 0)
    j test_otdi
TUGATISH
