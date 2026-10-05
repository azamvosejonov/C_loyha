/* xotira.S — yuklash/saqlash, ishora bilan kengaytirish, little-endian tartib */
#include "test_makro.h"
BOSHLASH
    la a0, malumot
    lw a1, 0(a0)
    SINA(1, a1, 0x80c0ff7f)
    lb a1, 0(a0)                    /* 0x7f: musbat */
    SINA(2, a1, 0x7f)
    lb a1, 1(a0)                    /* 0xff: -1 */
    SINA(3, a1, -1)
    lbu a1, 1(a0)
    SINA(4, a1, 0xff)
    lh a1, 2(a0)                    /* 0x80c0 manfiy */
    SINA(5, a1, 0xffff80c0)
    lhu a1, 2(a0)
    SINA(6, a1, 0x80c0)
    la a2, joy
    li a3, 0x11223344
    sw a3, 0(a2)
    lbu a1, 0(a2)                   /* little-endian: past bayt birinchi */
    SINA(7, a1, 0x44)
    lbu a1, 3(a2)
    SINA(8, a1, 0x11)
    li a3, 0xaabb
    sh a3, 2(a2)
    lw a1, 0(a2)
    SINA(9, a1, 0xaabb3344)
    li a3, 0x99
    sb a3, 1(a2)
    lw a1, 0(a2)
    SINA(10, a1, 0xaabb9944)
    sw a3, -4(a2)                   /* manfiy siljish */
    lw a1, -4(a2)
    SINA(11, a1, 0x99)
    j test_otdi
TUGATISH
    .data
    .balign 4
malumot: .word 0x80c0ff7f
    .word 0
joy: .word 0
