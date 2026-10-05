/*
 * dekod.c — o'zgarmaslarni (immediate) buyruqdan yig'ish. Formatlar dekod.h da chizilgan.
 *
 * Usul har birida bir xil: kerakli bit bo'laklarini BITLAR() bilan olib, chapga to'g'ri o'ringa
 * siljitib (<<), OR (|) bilan birlashtiramiz, oxirida ishora bilan kengaytiramiz.
 *
 * Tekshirish uchun misollar (testlar/birlik/birlik.c da ko'proq):
 *   addi a0, a0, -1   = 0xFFF50513  -> imm_i = 0xFFFFFFFF (-1)
 *   sw   a1, 8(sp)    = 0x00B12423  -> imm_s = 8
 *   beq  a0, a1, -4   = 0xFEB50EE3  -> imm_b = 0xFFFFFFFC (-4)
 *   lui  a0, 0x12345  = 0x12345537  -> imm_u = 0x12345000
 *   jal  ra, 2048     = 0x001000EF  -> imm_j = 0x800 (2048)
 */
#include "dekod.h"

uint32_t imm_i(uint32_t b)
{
    /*
     * TODO(E1a) — O'ZINGIZ YOZING: I-tur immediate: 12 bit (31..20), ishorali kengaytiriladi.
     *   - BITLAR(b, 31, 20) — 12 bitli maydonni oling
     *   - ishora_kengaytir(qiymat, 12) — 12-bit ishorani yuqoriga yoying
     *   - tekshirish: imm_i(0x80050513) == 0xFFFFF800 (-2048)
     * Tekshirish: make test  (birlik testida 'E1' qatori)
     */
    (void)b;
    return 0;
}

uint32_t imm_s(uint32_t b)
{
    /*
     * TODO(E1b) — O'ZINGIZ YOZING: S-tur immediate: ikki bo'lakda — imm[11:5] = 31..25 bitlar, imm[4:0] = 11..7 bitlar.
     *   - yuqori bo'lakni 5 ga chapga suring, pastkisi bilan OR qiling
     *   - natija 12 bit — ishora_kengaytir(..., 12)
     * Tekshirish: make test  (birlik testida 'E1' qatori)
     */
    (void)b;
    return 0;
}

uint32_t imm_b(uint32_t b)
{
    /*
     * TODO(E1c) — O'ZINGIZ YOZING: B-tur immediate (sakrash siljishi): bitlar ARALASHGAN, imm[0] doim 0.
     *   - imm[12] <- 31-bit,  imm[11] <- 7-bit,  imm[10:5] <- 30..25,  imm[4:1] <- 11..8
     *   - har bo'lakni BIT/BITLAR bilan olib, kerakli joyga << bilan qo'ying, OR qiling
     *   - 13 bitli son — ishora_kengaytir(v, 13)
     *   - kitob: 02-bob, 'B va J tur — nega bitlar aralash?'
     * Tekshirish: make test  (birlik testida 'E1' qatori)
     */
    (void)b;
    return 0;
}

uint32_t imm_u(uint32_t b)
{
    return b & 0xFFFFF000u;                     /* yuqori 20 bit o'z joyida, pastki 12 bit — nol */
}

uint32_t imm_j(uint32_t b)
{
    /*
     * TODO(E1d) — O'ZINGIZ YOZING: J-tur immediate (jal): imm[20] <- 31, imm[19:12] <- 19..12, imm[11] <- 20, imm[10:1] <- 30..21.
     *   - 21 bitli son — ishora_kengaytir(v, 21)
     *   - tekshirish: imm_j(0x8000006F) == 0xFFF00000 (-1 MB)
     * Tekshirish: make test  (birlik testida 'E1' qatori)
     */
    (void)b;
    return 0;
}
