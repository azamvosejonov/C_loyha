/* trap.S — istisnolar: ecall, noto'g'ri buyruq, tekis bo'lmagan murojaat, U rejimga o'tish, sret */
#include "test_makro.h"
BOSHLASH
    la t0, ishlovchi
    csrw stvec, t0

    /* 1) S rejimdan ecall -> scause = 9, sepc = ecall manzili */
    li s1, 0                        /* ishlovchi shu yerga scause ni yozadi */
1:  ecall
    SINA(1, s1, 9)
    la t1, 1b
    li gp, 2
    bne s2, t1, test_xato           /* s2 = sepc */

    /* 2) noto'g'ri buyruq -> scause = 2, stval = buyruqning o'zi */
    .word 0xffffffff
    SINA(3, s1, 2)
    SINA(4, s3, 0xffffffff)         /* s3 = stval */

    /* 3) tekis bo'lmagan lw: apparat o'zi bajaradi (istisno YO'Q), baytlar to'g'ri yig'iladi */
    li s1, 0
    la t2, malumot
    li t3, 0x44332211
    sw t3, 0(t2)
    li t3, 0x88776655
    sw t3, 4(t2)
    lw a0, 1(t2)                    /* baytlar 1..4: 22 33 44 55 */
    SINA(5, a0, 0x55443322)
    SINA(6, s1, 0)
    li t3, 0xaabbccdd
    sw t3, 3(t2)                    /* 3..6 baytlarga */
    lw a0, 4(t2)
    SINA(60, a0, 0x88aabbcc)

    /* 4) mavjud bo'lmagan manzildan o'qish -> load access fault (5) */
    li t2, 0x40000000
    lw a0, 0(t2)
    SINA(7, s1, 5)

    /* 5) U rejimga o'tish: sstatus.SPP = 0, sepc = user_kod, sret */
    li t0, 0x100                    /* SPP */
    csrc sstatus, t0
    la t0, user_kod
    csrw sepc, t0
    sret
user_qaytdi:
    SINA(8, s1, 8)                  /* U rejimdan ecall = 8 */
    SINA(9, s4, 0)                  /* trap paytida SPP = 0 (U dan kelgan) */
    j test_otdi

user_kod:
    csrr a0, sstatus                /* U rejimda S-CSR -> noto'g'ri buyruq (2) */
    li a0, 77
    ecall

/* trap ishlovchisi: sababni s1 ga, sepc ni s2 ga, stval ni s3 ga, SPP ni s4 ga yozadi.
   ecall U dan bo'lsa — S rejimda user_qaytdi ga qaytadi; aks holda keyingi buyruqqa (sepc + 4). */
    .balign 4
ishlovchi:
    csrr s1, scause
    csrr s2, sepc
    csrr s3, stval
    csrr s4, sstatus
    andi s4, s4, 0x100
    li t0, 2
    bne s1, t0, 1f
    csrr t0, sstatus                /* U rejimdagi noto'g'ri buyruq bo'lsa ham +4 qilib davom */
1:  li t0, 8
    beq s1, t0, 2f
    addi s2, s2, 4
    csrw sepc, s2
    addi s2, s2, -4
    sret
2:  li t0, 0x100                    /* U dan ecall: S rejimda testga qaytamiz */
    csrs sstatus, t0
    la t0, user_qaytdi
    csrw sepc, t0
    sret
TUGATISH
    .data
    .balign 4
malumot: .word 0, 0
