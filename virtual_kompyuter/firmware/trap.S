/*
 * trap.S — M rejim trap'ining "kirish eshigi": registrlarni saqlash, C ishlovchini chaqirish, tiklash, mret.
 *
 * MUAMMO: trap istalgan payt keladi — yadro o'rtasida, har qanday registr qiymati bilan. Ishlovchi
 * hech bir registrni BUZMASLIGI kerak (yadro hech narsani sezmasligi kerak). Lekin registrlarni saqlash
 * uchun xotira manzili kerak, manzil uchun esa — registr! Bu "tovuq va tuxum" muammosi.
 *
 * YECHIM: mscratch. U oldindan firmware stekiga ishora qiladi. `csrrw sp, mscratch, sp` BITTA buyruqda
 * sp va mscratch ni almashtiradi: endi sp — firmware steki, mscratch — yadroning sp si (saqlab qo'yildi).
 *
 * Stekdagi kadr (struct kadr, fw.h) — 32 ta registr + mepc:
 *   kadr[0]  = x0 o'rni (ishlatilmaydi)   kadr[1] = ra   kadr[2] = sp (yadroniki!) ... kadr[31] = t6
 *   kadr[32] = mepc
 * C ishlovchi kadrni o'zgartirishi mumkin (masalan a0 = natija, mepc += 4) — tiklashda shular ishlatiladi.
 */
    .option norvc
    .text
    .balign 4
    .globl m_trap_kirish
m_trap_kirish:
    csrrw sp, mscratch, sp              /* sp <-> mscratch: sp = firmware steki */
    addi sp, sp, -36*4                  /* 33 so'z kerak, 16 baytga tekislash uchun 36 */
    sw ra, 1*4(sp)
    sw gp, 3*4(sp)
    sw tp, 4*4(sp)
    sw t0, 5*4(sp)
    sw t1, 6*4(sp)
    sw t2, 7*4(sp)
    sw s0, 8*4(sp)
    sw s1, 9*4(sp)
    sw a0, 10*4(sp)
    sw a1, 11*4(sp)
    sw a2, 12*4(sp)
    sw a3, 13*4(sp)
    sw a4, 14*4(sp)
    sw a5, 15*4(sp)
    sw a6, 16*4(sp)
    sw a7, 17*4(sp)
    sw s2, 18*4(sp)
    sw s3, 19*4(sp)
    sw s4, 20*4(sp)
    sw s5, 21*4(sp)
    sw s6, 22*4(sp)
    sw s7, 23*4(sp)
    sw s8, 24*4(sp)
    sw s9, 25*4(sp)
    sw s10, 26*4(sp)
    sw s11, 27*4(sp)
    sw t3, 28*4(sp)
    sw t4, 29*4(sp)
    sw t5, 30*4(sp)
    sw t6, 31*4(sp)
    csrr t0, mscratch                   /* yadroning sp si */
    sw t0, 2*4(sp)
    csrr t0, mepc
    sw t0, 32*4(sp)
    addi t0, sp, 36*4                   /* mscratch ni yana firmware stek TEPASIGA qaytaramiz (keyingi trap uchun) */
    csrw mscratch, t0

    mv a0, sp                           /* m_trap(struct kadr *k) */
    call m_trap

    lw t0, 32*4(sp)
    csrw mepc, t0
    lw ra, 1*4(sp)
    lw gp, 3*4(sp)
    lw tp, 4*4(sp)
    lw t0, 5*4(sp)
    lw t1, 6*4(sp)
    lw t2, 7*4(sp)
    lw s0, 8*4(sp)
    lw s1, 9*4(sp)
    lw a0, 10*4(sp)
    lw a1, 11*4(sp)
    lw a2, 12*4(sp)
    lw a3, 13*4(sp)
    lw a4, 14*4(sp)
    lw a5, 15*4(sp)
    lw a6, 16*4(sp)
    lw a7, 17*4(sp)
    lw s2, 18*4(sp)
    lw s3, 19*4(sp)
    lw s4, 20*4(sp)
    lw s5, 21*4(sp)
    lw s6, 22*4(sp)
    lw s7, 23*4(sp)
    lw s8, 24*4(sp)
    lw s9, 25*4(sp)
    lw s10, 26*4(sp)
    lw s11, 27*4(sp)
    lw t3, 28*4(sp)
    lw t4, 29*4(sp)
    lw t5, 30*4(sp)
    lw t6, 31*4(sp)
    lw sp, 2*4(sp)                      /* eng oxirida: yadroning sp si (kadr bilan ishlash tugadi) */
    mret
