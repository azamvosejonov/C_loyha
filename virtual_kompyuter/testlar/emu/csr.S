/* csr.S — CSR buyruqlari va hisoblagichlar */
#include "test_makro.h"
BOSHLASH
    li a0, 0x1234
    csrw sscratch, a0
    csrr a1, sscratch
    SINA(1, a1, 0x1234)
    li a0, 0xff
    csrrs a1, sscratch, a0          /* a1 = eski; sscratch |= 0xff */
    SINA(2, a1, 0x1234)
    csrr a1, sscratch
    SINA(3, a1, 0x12ff)
    li a0, 0x0f
    csrrc a1, sscratch, a0
    csrr a1, sscratch
    SINA(4, a1, 0x12f0)
    csrrwi a1, sscratch, 7          /* o'zgarmas variant */
    SINA(5, a1, 0x12f0)
    csrr a1, sscratch
    SINA(6, a1, 7)
    csrsi sscratch, 8
    csrr a1, sscratch
    SINA(7, a1, 15)
    /* instret o'sib boradi */
    csrr a2, instret
    nop
    nop
    csrr a3, instret
    sub a4, a3, a2
    SINA(8, a4, 3)                  /* 2 ta nop + birinchi csrr */
    /* sstatus: faqat mavjud bitlar yoziladi */
    li a0, -1
    csrw sstatus, a0
    csrr a1, sstatus
    SINA(9, a1, 0x000c0122)         /* SIE|SPIE|SPP|SUM|MXR */
    csrw sstatus, zero
    /* sepc: past bit doim 0 (C kengaytmasi bor: buyruqlar 2 ga tekis) */
    li a0, 0x80000007
    csrw sepc, a0
    csrr a1, sepc
    SINA(10, a1, 0x80000006)
    /* stimecmp 64 bit: ikki yarmi */
    li a0, 5
    csrw stimecmph, a0
    li a0, 6
    csrw stimecmp, a0
    csrr a1, stimecmph
    SINA(11, a1, 5)
    csrr a1, stimecmp
    SINA(12, a1, 6)
    /* time = instret (deterministik vaqt) */
    csrr a1, time
    csrr a2, instret
    sub a3, a2, a1
    SINA(13, a3, 1)
    j test_otdi
TUGATISH
