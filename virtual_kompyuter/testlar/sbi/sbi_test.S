/*
 * sbi_test.S — firmware (SBI) ni tekshiruvchi kichik "yadro" (S rejim).
 * Har tekshiruv xato bo'lsa: konsolga "XATO N" yozib, SRST orqali 1 kodi bilan o'chiradi.
 * Hammasi to'g'ri: "SBI testlari: OK" va 0 kodi bilan o'chiradi.
 */
    .option norvc
    .text
    .globl _start
_start:
    li sp, 0x80500000
    la t0, s_trap
    csrw stvec, t0

    /* 1) BASE: spetsifikatsiya versiyasi = 0x02000000 (v2.0) */
    li s11, 1
    li a7, 0x10
    li a6, 0
    ecall
    bnez a0, xato
    li t0, 0x02000000
    bne a1, t0, xato

    /* 2) BASE: TIME kengaytmasi bor (probe = 1), noma'lum kengaytma yo'q (probe = 0) */
    li s11, 2
    li a7, 0x10
    li a6, 3
    li a0, 0x54494D45
    ecall
    li t0, 1
    bne a1, t0, xato
    li s11, 3
    li a7, 0x10
    li a6, 3
    li a0, 0x12345
    ecall
    bnez a1, xato

    /* 3) noma'lum kengaytma -> SBI_ERR_NOT_SUPPORTED (-2) */
    li s11, 4
    li a7, 0x7777
    li a6, 0
    ecall
    li t0, -2
    bne a0, t0, xato

    /* 4) konsol (legacy putchar) */
    la s0, salom
1:  lbu a0, 0(s0)
    beqz a0, 2f
    li a7, 1
    ecall
    addi s0, s0, 1
    j 1b
2:
    /* 5) rdtime S rejimda ishlaydi (firmware mcounteren ni yoqgan bo'lishi kerak) */
    li s11, 5
    rdtime t1
    rdtime t2
    bleu t2, t1, xato

    /* 6) TIME set_timer: 2000 tikdan keyin S taymer uzilishi kelishi kerak */
    li s11, 6
    li s1, 0                        /* uzilishlar soni */
    li t0, (1 << 5)
    csrs sie, t0
    csrsi sstatus, 2
    rdtime a0
    addi a0, a0, 2000
    li a1, 0
    li a7, 0x54494D45
    li a6, 0
    ecall
3:  wfi
    beqz s1, 3b
    li t0, 1
    bne s1, t0, xato
    /* uzilish faqat BIR marta kelishi kerak (ishlovchi taymerni "cheksiz"ga surdi) */
    li t3, 5000
4:  addi t3, t3, -1
    bnez t3, 4b
    li t0, 1
    bne s1, t0, xato

    la s0, ok
1:  lbu a0, 0(s0)
    beqz a0, 2f
    li a7, 1
    ecall
    addi s0, s0, 1
    j 1b
2:  li a7, 0x53525354               /* SRST: shutdown (0), sabab 0 */
    li a6, 0
    li a0, 0
    li a1, 0
    ecall
    j .

xato:
    la s0, xato_matn
1:  lbu a0, 0(s0)
    beqz a0, 2f
    li a7, 1
    ecall
    addi s0, s0, 1
    j 1b
2:  addi a0, s11, '0'
    li a7, 1
    ecall
    li a0, '\n'
    li a7, 1
    ecall
    li a7, 0x53525354
    li a6, 0
    li a0, 0
    li a1, 1                        /* sabab 1: xato bilan o'chirish */
    ecall
    j .

    .balign 4
s_trap:
    csrr t0, scause
    li t1, 0x80000005               /* S taymer uzilishi */
    bne t0, t1, xato
    addi s1, s1, 1
    li a0, -1                       /* keyingi taymer — "hech qachon" (STIP o'chadi) */
    li a1, -1
    li a7, 0x54494D45
    li a6, 0
    ecall
    sret

    .data
salom: .asciz "SBI testi: konsol ishlayapti\n"
ok: .asciz "SBI testlari: OK\n"
xato_matn: .asciz "SBI testi: XATO "
