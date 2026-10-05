/* plic.S — UART "bayt keldi" uzilishi PLIC orqali S rejimga (SEIP), claim/complete; CLINT M taymeri */
#include "test_makro.h"
#define UART 0x10000000
#define PLIC 0x0c000000
BOSHLASH
    la t0, s_ishlovchi
    csrw stvec, t0
    la t0, m_ishlovchi
    csrw mtvec, t0
    li s1, 0                        /* S tashqi uzilishlar soni */
    li s3, 0                        /* qabul qilingan baytlar */

    /* PLIC: UART (10) ustuvorligi 1, S kontekstida (1) yoqish, chegara 0 */
    li t0, PLIC + 10*4
    li t1, 1
    sw t1, 0(t0)
    li t0, PLIC + 0x2080
    li t1, (1 << 10)
    sw t1, 0(t0)
    li t0, PLIC + 0x201000
    sw zero, 0(t0)
    /* UART: "bayt keldi" uzilishini yoqish */
    li t0, UART
    li t1, 1
    sb t1, 1(t0)
    /* M: SEI ni S ga topshirish, S da yoqish */
    li t0, (1 << 9)
    csrw mideleg, t0
    csrw mie, t0                    /* sie.SEIE (mie orqali) */
    li t0, 0x1800                   /* MPP = S */
    csrc mstatus, t0
    li t0, 0x0800
    csrs mstatus, t0
    li t0, 2                        /* sstatus.SIE */
    csrs mstatus, t0
    la t0, s_kod
    csrw mepc, t0
    mret
s_kod:
    li t2, 3
1:  bltu s3, t2, 1b                 /* 3 bayt kelguncha kutamiz: hammasi uzilish orqali */
    SINA(1, s3, 3)
    li gp, 2
    beqz s1, test_xato
    SINA(3, s4, 0x63)               /* oxirgi bayt 'c' */
    li a7, 1
    ecall                           /* M ga: CLINT taymer testi */

    .balign 4
s_ishlovchi:
    csrr t0, scause
    li t1, 0x80000009
    bne t0, t1, test_xato
    addi s1, s1, 1
    li t0, PLIC + 0x201004          /* claim (kontekst 1) */
    lw t1, 0(t0)
    li t3, 10
    bne t1, t3, test_xato
    li t4, UART
2:  lbu t3, 5(t4)                   /* barcha kelgan baytlarni o'qiymiz (sababni yo'q qilish) */
    andi t3, t3, 1
    beqz t3, 3f
    lbu s4, 0(t4)
    addi s3, s3, 1
    j 2b
3:  sw t1, 0(t0)                    /* complete */
    sret

    .balign 4
m_ishlovchi:
    csrr t0, mcause
    li t1, 9
    bne t0, t1, 5f
    /* CLINT: mtimecmp = mtime + 500, MTIE yoqish, wfi */
    li t0, 0x0200bff8
    lw t1, 0(t0)
    addi t1, t1, 500
    li t0, 0x02004000
    li t2, -1
    sw t2, 0(t0)
    sw zero, 4(t0)
    sw t1, 0(t0)
    li t0, (1 << 7)
    csrw mie, t0
    li s5, 0
    csrsi mstatus, 8                /* MIE */
4:  beqz s5, 4b
    SINA(4, s5, 1)
    j test_otdi
5:  li t1, 0x80000007
    bne t0, t1, test_xato
    li s5, 1
    li t0, 0x02004004
    li t2, -1
    sw t2, 0(t0)                    /* keyingi taymerni o'chirish */
    mret
TUGATISH
