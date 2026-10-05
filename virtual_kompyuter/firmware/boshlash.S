/*
 * boshlash.S — kompyuter yoqilganda BIRINCHI bajariladigan kod (M rejim, pc = 0x8000_0000).
 *
 * Bu paytda hech narsa tayyor emas: stek yo'q, .bss nollanmagan, trap ishlovchisi yo'q.
 * C funksiyasini chaqirishdan oldin minimal muhitni assemblyda tayyorlaymiz:
 *   1) sp — stek tepasi (stek PASTGA o'sadi);
 *   2) mscratch — trap paytida ishlatiladigan stek manzili (trap.S ga qarang);
 *   3) .bss ni nollash (C standarti: global o'zgaruvchilar boshida 0);
 *   4) mtvec — trap ishlovchisi manzili;
 *   5) firmware_asosiy(hartid, dtb) ni chaqirish. a0 va a1 ni emulyator bergan (cpu.c / main.c).
 */
    .option norvc
    .section .text.boshi
    .globl _start
_start:
    la sp, __stek_tepasi
    csrw mscratch, sp

    la t0, __bss_boshi
    la t1, __bss_oxiri
1:  bgeu t0, t1, 2f
    sw zero, 0(t0)
    addi t0, t0, 4
    j 1b
2:
    la t0, m_trap_kirish
    csrw mtvec, t0
    call firmware_asosiy                /* a0 = hartid, a1 = DTB manzili — tegilmagan */
3:  wfi                                 /* bu yerga qaytmasligi kerak */
    j 3b
