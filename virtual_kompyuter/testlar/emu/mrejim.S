/* mrejim.S — M rejimdan boshlanish, delegatsiya, mret, S dan ecall M ga, MPRV, TSR (emulyator -S SIZ ishga tushiriladi) */
#include "test_makro.h"
BOSHLASH
    csrr a0, misa                   /* RV32 + A C I M S U */
    SINA(1, a0, 0x40141105)
    csrr a0, mhartid
    SINA(2, a0, 0)
    la t0, m_ishlovchi
    csrw mtvec, t0
    la t0, s_ishlovchi
    csrw stvec, t0

    /* 1) delegatsiya: noto'g'ri buyruq (2) S ga topshirilgan, ecall-S (9) M da qoladi */
    li t0, (1 << 2)
    csrw medeleg, t0
    li s1, 0                        /* M ishlovchi shu yerga mcause ni yozadi */
    li s2, 0                        /* S ishlovchi — scause ni */

    /* S rejimga o'tish: MPP = 01, mepc = s_kod, mret */
    li t0, 0x1800
    csrc mstatus, t0
    li t0, 0x0800
    csrs mstatus, t0
    la t0, s_kod
    csrw mepc, t0
    mret

s_kod:
    .word 0xffffffff                /* noto'g'ri buyruq -> S ishlovchiga (delegatsiya) */
    SINA(3, s2, 2)
    SINA(4, s1, 0)                  /* M ga bormadi */
    csrr a0, mstatus                /* S rejimda M CSR -> noto'g'ri buyruq (S ga) */
    SINA(5, s2, 2)
    li a7, 123
    ecall                           /* S dan ecall -> M (delegatsiya qilinmagan): scause emas, mcause = 9 */
    SINA(6, s1, 9)
    SINA(7, a0, 124)                /* M ishlovchi a0 = a7 + 1 qaytardi */
    /* TSR: M sret ni taqiqladi -> S da sret noto'g'ri buyruq. Bu safar noto'g'ri buyruqni M ushlaydi
       (S ishlovchining o'zi ham sret qiladi — u ham taqiqlangan bo'lardi!) */
    li a7, 1000
    ecall                           /* M: TSR = 1, medeleg = 0 */
    sret                            /* noto'g'ri buyruq -> M ishlovchi (s1 = 2) */
    SINA(8, s1, 2)
    li a7, 2000                     /* testni M rejimda tugatish */
    ecall

/* M ishlovchi: mcause -> s1. a7=1000 — TSR yoqish; a7=2000 — test tugadi; boshqa — a0 = a7 + 1 */
    .balign 4
m_ishlovchi:
    csrr s1, mcause
    li t0, 1000
    beq a7, t0, 1f
    li t0, 2000
    beq a7, t0, m_tugadi
    addi a0, a7, 1
    j 2f
1:  li t0, (1 << 22)
    csrs mstatus, t0
    csrw medeleg, zero
2:  csrr t0, mepc
    addi t0, t0, 4
    csrw mepc, t0
    mret
m_tugadi:
    /* MPRV: M rejimda, MPP=S va satp=0 bo'lsa ham o'qish ishlaydi (tarjima yo'q) */
    li t0, 0x20000
    csrs mstatus, t0
    la t1, qiymat
    lw a0, 0(t1)
    SINA(9, a0, 0x55aa)
    csrc mstatus, t0
    j test_otdi

/* S ishlovchi: scause -> s2, keyingi buyruqqa */
    .balign 4
s_ishlovchi:
    csrr s2, scause
    csrr t0, sepc
    addi t0, t0, 4
    csrw sepc, t0
    sret
TUGATISH
    .data
    .balign 4
qiymat: .word 0x55aa
