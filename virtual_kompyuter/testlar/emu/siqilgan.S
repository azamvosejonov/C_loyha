/* siqilgan.S — har siqilgan buyruqni ALOHIDA sinash (assembler ularni 16 bitli qilib yozadi) */
#include "test_makro.h"
#undef BOSHLASH
    .option rvc
    .section .text.boshi
    .globl _start
_start:
    li sp, 0x80100000
    c.li a0, -3
    SINA(1, a0, -3)
    c.addi a0, 10
    SINA(2, a0, 7)
    c.lui a1, 0x12                  /* a1 = 0x12000 */
    SINA(3, a1, 0x12000)
    c.mv a2, a1
    c.add a2, a0                    /* 0x12007 */
    SINA(4, a2, 0x12007)
    c.addi16sp sp, -32
    li t0, 0x80100000 - 32
    li gp, 5
    bne sp, t0, test_xato
    c.addi4spn a3, sp, 8            /* a3 = sp + 8 */
    addi t0, t0, 8
    li gp, 6
    bne a3, t0, test_xato
    c.swsp a2, 12(sp)
    c.lwsp a4, 12(sp)
    SINA(7, a4, 0x12007)
    c.sw a2, 4(a3)                  /* a3 = s0..a5 oralig'idagi registr */
    c.lw a5, 4(a3)
    SINA(8, a5, 0x12007)
    c.li a0, 31
    c.slli a0, 3
    SINA(9, a0, 248)
    c.srli a0, 2
    SINA(10, a0, 62)
    c.li a1, -16
    c.srai a1, 2
    SINA(11, a1, -4)
    c.andi a0, 0xa                  /* 62 & 10 = 10 */
    SINA(12, a0, 10)
    c.li a1, 3
    c.sub a0, a1
    SINA(13, a0, 7)
    c.xor a0, a1                    /* 7 ^ 3 = 4 */
    SINA(14, a0, 4)
    c.or a0, a1
    SINA(15, a0, 7)
    c.and a0, a1
    SINA(16, a0, 3)
    c.li a0, 0
    c.beqz a0, 1f
    j test_xato
1:  c.bnez a0, test_xato
    c.jal funksiya                  /* ra = keyingi (2 bayt!) */
2:  SINA(17, a0, 25)
    la t0, 2b
    li gp, 18
    bne ra, t0, test_xato
    la a1, funksiya
    c.jalr a1
    SINA(19, a0, 25)
    c.j 3f
    j test_xato
3:  c.nop
    j test_otdi
funksiya:
    c.li a0, 25
    c.jr ra
TUGATISH
