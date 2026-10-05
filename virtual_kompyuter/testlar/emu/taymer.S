/* taymer.S — Sstc taymeri: stimecmp, taymer uzilishi, wfi, sie/SIE */
#include "test_makro.h"
BOSHLASH
    la t0, ishlovchi
    csrw stvec, t0
    li s1, 0                        /* uzilishlar soni */

    /* uzilishlar o'chiq (SIE=0) bo'lsa, taymer kelsa ham trap YO'Q, lekin sip.STIP = 1 */
    li t0, 0x20                     /* STIE */
    csrs sie, t0
    csrw stimecmph, zero            /* 64 bitli stimecmp: yuqori yarmini ham nollaymiz */
    csrw stimecmp, zero             /* time >= 0 — darhol kutadi */
    csrr a0, sip
    andi a0, a0, 0x20
    SINA(1, a0, 0x20)
    SINA(2, s1, 0)

    /* SIE=1: darhol trap */
    csrsi sstatus, 2
    nop
    SINA(3, s1, 1)                  /* ishlovchi stimecmp ni uzoqqa surib qo'ydi */

    /* wfi: keyingi uzilishgacha "uxlash". 1000 buyruqdan keyin taymer */
    csrr t0, time
    addi t0, t0, 1000
    csrw stimecmph, zero            /* avval yuqori yarmi (past yarmi hali 0xffffffff: soxta uzilish bo'lmaydi) */
    csrw stimecmp, t0
    wfi
    SINA(4, s1, 2)
    csrr t1, time
    li gp, 5
    bltu t1, t0, test_xato          /* vaqt haqiqatan oldinga surildi */
    j test_otdi

    .balign 4
ishlovchi:
    csrr t2, scause
    li t3, 0x80000005               /* uzilish biti | taymer */
    bne t2, t3, test_xato
    addi s1, s1, 1
    li t2, -1
    csrw stimecmp, t2               /* keyingi uzilishni "hech qachon" ga suramiz */
    csrw stimecmph, t2
    sret
TUGATISH
