/* mmu.S — Sv32: sahifa jadvali tuzish, tarjima, 4 MB katta sahifa, sahifa xatosi, A/D bitlari, U-bit */
#include "test_makro.h"
#define PTE_V 1
#define PTE_R 2
#define PTE_W 4
#define PTE_X 8
#define PTE_U 16
#define PTE_A 64
#define PTE_D 128
BOSHLASH
    la t0, ishlovchi
    csrw stvec, t0

    /* 1-darajali jadval: 0x8000_0000 (VPN1 = 0x200) -> 4 MB katta sahifa, o'zini-o'zi (identity) — kod ishlashda davom etsin */
    la s0, jadval1
    li t0, 0x80000000
    srli t0, t0, 12
    slli t0, t0, 10
    ori t0, t0, PTE_V | PTE_R | PTE_W | PTE_X
    li t1, 0x200*4
    add t1, s0, t1
    sw t0, 0(t1)

    /* VA 0x4000_0000 (VPN1 = 0x100) -> 2-darajali jadval */
    la s1, jadval0
    srli t0, s1, 12
    slli t0, t0, 10
    ori t0, t0, PTE_V               /* R=W=X=0: barg emas, ko'rsatkich */
    sw t0, 0x100*4(s0)
    /* jadval0[1] : VA 0x4000_1000 -> fizik "sahifa" (U=0, faqat o'qish) */
    la s2, sahifa
    srli t0, s2, 12
    slli t0, t0, 10
    ori t0, t0, PTE_V | PTE_R
    sw t0, 1*4(s1)
    /* jadval0[2] : VA 0x4000_2000 -> o'sha sahifa, o'qish+yozish */
    srli t0, s2, 12
    slli t0, t0, 10
    ori t0, t0, PTE_V | PTE_R | PTE_W
    sw t0, 2*4(s1)
    /* jadval0[3] : VA 0x4000_3000 -> o'sha sahifa, U=1 */
    srli t0, s2, 12
    slli t0, t0, 10
    ori t0, t0, PTE_V | PTE_R | PTE_W | PTE_U
    sw t0, 3*4(s1)

    /* satp = MODE(1) | PPN(jadval1) */
    srli t0, s0, 12
    li t1, 0x80000000
    or t0, t0, t1
    csrw satp, t0
    sfence.vma

    li s5, 0                        /* ishlovchi sababni shu yerga yozadi */
    li t2, 0x12345678
    sw t2, 0(s2)                    /* fizik sahifaga (identity orqali) yozamiz */
    li t3, 0x40001000
    lw a0, 0(t3)                    /* virtual orqali o'qiymiz */
    SINA(1, a0, 0x12345678)
    li t3, 0x40002004
    li t2, 99
    sw t2, 0(t3)                    /* yozish ruxsati bor */
    lw a0, 4(s2)
    SINA(2, a0, 99)

    li t3, 0x40001000
    sw t2, 0(t3)                    /* W=0 -> store page fault (15) */
    SINA(3, s5, 15)
    li t3, 0x40005000
    lw a0, 0(t3)                    /* yozuv yo'q (V=0) -> load page fault (13) */
    SINA(4, s5, 13)
    li t3, 0x40003000
    li s5, 0
    lw a0, 0(t3)                    /* U-sahifa, SUM=0, S rejim -> 13 */
    SINA(5, s5, 13)
    li t0, 0x40000                  /* SUM */
    csrs sstatus, t0
    li s5, 0
    lw a0, 0(t3)                    /* endi ruxsat */
    SINA(6, s5, 0)
    SINA(7, a0, 0x12345678)

    /* A va D bitlari apparat tomonidan qo'yilgan bo'lishi kerak */
    lw a0, 1*4(s1)                  /* faqat o'qilgan sahifa: A=1, D=0 */
    andi a0, a0, PTE_A | PTE_D
    SINA(8, a0, PTE_A)
    lw a0, 2*4(s1)                  /* yozilgan sahifa: A=1, D=1 */
    andi a0, a0, PTE_A | PTE_D
    SINA(9, a0, PTE_A | PTE_D)

    /* TLB: PTE ni o'zgartirib sfence QILMASAK — eski tarjima ishlatiladi (haqiqiy protsessordagi kabi) */
    li t3, 0x40001000
    lw a0, 0(t3)                    /* TLB ga tushdi */
    sw zero, 1*4(s1)                /* PTE ni o'chirdik */
    li s5, 0
    lw a0, 0(t3)                    /* hali ishlaydi (eski TLB yozuvi) */
    SINA(10, s5, 0)
    sfence.vma
    lw a0, 0(t3)                    /* endi sahifa xatosi */
    SINA(11, s5, 13)

    csrw satp, zero
    sfence.vma
    j test_otdi

/* ishlovchi: sababni s5 ga yozib, xatoli buyruqdan keyingisiga o'tadi */
    .balign 4
ishlovchi:
    csrr s5, scause
    csrr t0, sepc
    addi t0, t0, 4
    csrw sepc, t0
    sret
TUGATISH
    .bss
    .balign 4096
jadval1: .space 4096
jadval0: .space 4096
sahifa: .space 4096
