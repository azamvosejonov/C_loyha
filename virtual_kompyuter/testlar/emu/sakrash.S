/* sakrash.S — shartli sakrashlar, jal, jalr */
#include "test_makro.h"
BOSHLASH
    li a0, -1
    li a1, 1
    li a2, 0                        /* nechta sakrash bajarildi */
    beq a0, a0, 1f
    j test_xato
1:  bne a0, a1, 1f
    j test_xato
1:  blt a0, a1, 1f                  /* -1 < 1 ishorali: sakraydi */
    j test_xato
1:  bltu a0, a1, test_xato          /* 0xffffffff < 1 ishorasiz: sakramaydi */
    bge a1, a0, 1f
    j test_xato
1:  bgeu a1, a0, test_xato          /* 1 >= 0xffffffff ishorasiz: yo'q */
    bgeu a0, a1, 1f
    j test_xato
1:  bge a0, a0, 1f                  /* teng: >= rost */
    j test_xato
1:  li gp, 1
    jal ra, funksiya                /* ra = keyingi buyruq manzili */
    SINA(2, a2, 42)
    la t0, funksiya
    jalr ra, 0(t0)
    SINA(3, a2, 42)
    la t0, 1f
    jalr t0, 0(t0)                  /* rd == rs1: manzil ESKI qiymatdan hisoblanadi */
1:  la t1, 1b                       /* t0 = jalr dan keyingi buyruq manzili = shu belgi */
    li gp, 4
    bne t0, t1, test_xato
    /* orqaga sakrash (manfiy o'zgarmas): 5 marta aylanish */
    li a3, 0
    li a4, 5
2:  addi a3, a3, 1
    blt a3, a4, 2b
    SINA(5, a3, 5)
    j test_otdi
funksiya:
    li a2, 42
    ret                             /* jalr x0, 0(ra) */
TUGATISH
