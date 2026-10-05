/* uart.S — konsolga matn chiqarish va kirishni o'qish (16550 registrlari orqali) */
#include "test_makro.h"
#define UART 0x10000000
BOSHLASH
    li s0, UART
    la s1, matn
1:  lbu a0, 0(s1)
    beqz a0, 2f
3:  lbu t0, 5(s0)                   /* LSR: THRE (5-bit) bo'lguncha kutish */
    andi t0, t0, 0x20
    beqz t0, 3b
    sb a0, 0(s0)
    addi s1, s1, 1
    j 1b
2:  /* kirishdan baytlarni o'qib, katta harfga o'tkazib qaytaramiz (stdin tugaguncha) */
4:  lbu t0, 5(s0)
    andi t0, t0, 1                  /* DR: bayt bormi */
    beqz t0, 5f
    lbu a0, 0(s0)
    li t1, 'a'
    bltu a0, t1, 6f
    li t1, 'z' + 1
    bgeu a0, t1, 6f
    addi a0, a0, -32
6:  sb a0, 0(s0)
    j 4b
5:  j test_otdi
TUGATISH
    .section .rodata
matn: .asciz "salom, RISC-V!\n"
