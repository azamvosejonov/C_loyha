/*
 * alu.c — hisob amallari. Har amal — RISC-V spetsifikatsiyasining bitta qatori.
 *
 * ISHORALI va ISHORASIZ
 *   Registrda shunchaki 32 bit turadi. 0xFFFFFFFF — ishorali qarasak -1, ishorasiz qarasak 4294967295.
 *   Qaysi biri — BUYRUQ hal qiladi: slt/blt/div ishorali, sltu/bltu/divu ishorasiz. C da buni
 *   (int32_t) va (uint32_t) ga aylantirish bilan ifodalaymiz.
 */
#include "alu.h"

uint32_t alu_asosiy(uint32_t funct3, int alt, int registr, uint32_t a, uint32_t b, int *xato)
{
    uint32_t siljish = b & 31;                  /* RV32: siljish miqdori — faqat pastki 5 bit (0..31) */
    *xato = 0;
    switch (funct3) {
    case 0:                                     /* add / sub (sub faqat R-turda, alt=1) */
        if (registr && alt)
            return a - b;
        return a + b;                           /* ishorasiz qo'shish: toshish 2^32 bo'yicha "aylanadi" — bu RISC-V da to'g'ri */
    case 1:                                     /* sll: chapga mantiqiy siljitish */
        return a << siljish;
    case 2:                                     /* slt: ishorali solishtirish -> 1/0 */
        return (int32_t)a < (int32_t)b;
    case 3:                                     /* sltu: ishorasiz solishtirish */
        return a < b;
    case 4:                                     /* xor */
        return a ^ b;
    case 5:                                     /* srl (mantiqiy: chapdan 0) yoki sra (arifmetik: chapdan ishora) */
        if (alt)
            return (uint32_t)((int32_t)a >> siljish);
        return a >> siljish;
    case 6:                                     /* or */
        return a | b;
    case 7:                                     /* and */
        return a & b;
    }
    *xato = 1;
    return 0;
}

int shart_bajarildimi(uint32_t funct3, uint32_t a, uint32_t b)
{
    /*
     * TODO(E2) — O'ZINGIZ YOZING: Shartli sakrash: funct3 bo'yicha a va b ni solishtiring. Sakrash kerak bo'lsa 1, kerak bo'lmasa 0.
     *   - 0 beq ==, 1 bne !=, 4 blt <, 5 bge >=, 6 bltu <, 7 bgeu >=
     *   - blt/bge — ISHORALI: (int32_t)a < (int32_t)b;  bltu/bgeu — ishorasiz (uint32_t)
     *   - funct3 = 2 yoki 3 — bunday buyruq yo'q: -1 qaytaring (cpu.c uni 'noto'g'ri buyruq' qiladi)
     * Tekshirish: make test  (birlik testida 'E2' qatori)
     */
    (void)funct3;
    (void)a;
    (void)b;
    return -1;
}

uint32_t yuklash_kengaytir(uint32_t funct3, uint32_t qiymat)
{
    /*
     * TODO(E3) — O'ZINGIZ YOZING: Yuklangan qiymatni 32 bitga kengaytirish (lb/lh/lw/lbu/lhu).
     *   - funct3: 0 lb, 1 lh, 2 lw, 4 lbu, 5 lhu
     *   - lb: past 8 bit, ISHORALI kengaytirish; lbu: past 8 bit, nol bilan
     *   - lh/lhu — xuddi shunday, 16 bit;  lw — o'zgarishsiz
     * Tekshirish: make test  (birlik testida 'E3' qatori)
     */
    (void)funct3;
    return qiymat;
}

/*
 * M kengaytmasi. Nozik joylar:
 *   1) mulh* — 32x32 ko'paytmaning YUQORI 32 biti. 64 bitli sonda hisoblab, >> 32 qilamiz.
 *      mulh: ikkalasi ishorali; mulhu: ikkalasi ishorasiz; mulhsu: a ishorali, b ishorasiz.
 *   2) Nolga bo'lish RISC-V da ISTISNO EMAS (x86 dan farqi!). Natija spetsifikatsiyada belgilangan:
 *        div  x/0 = -1 (0xFFFFFFFF)     divu x/0 = 0xFFFFFFFF
 *        rem  x%0 = x                   remu x%0 = x
 *   3) Ishorali toshish: INT_MIN / -1 = +2147483648 — 32 bitga sig'maydi (C da bu UB! dastur qulaydi).
 *      RISC-V da: div natija = INT_MIN, rem natija = 0. Shuning uchun C ning / ini to'g'ridan-to'g'ri
 *      ishlatishdan OLDIN bu holatni alohida tekshiramiz.
 */
uint32_t m_amal(uint32_t funct3, uint32_t a, uint32_t b)
{
    /*
     * TODO(E4) — O'ZINGIZ YOZING: M kengaytmasi: mul, mulh, mulhsu, mulhu, div, divu, rem, remu (funct3 = 0..7).
     *   - mulh*: 64 bitli ko'paytmaning YUQORI 32 biti — int64_t/uint64_t ga o'tkazib ko'paytiring, >> 32
     *   - mulhsu: a ISHORALI, b ISHORASIZ: (int64_t)(int32_t)a * (int64_t)(uint64_t)b
     *   - b == 0: div/divu -> 0xFFFFFFFF, rem/remu -> a  (istisno YO'Q)
     *   - div INT_MIN / -1 -> INT_MIN, rem -> 0. C da bu UB — '/' dan OLDIN tekshiring!
     * Tekshirish: make test  (birlik testida 'E4' qatori)
     */
    (void)funct3;
    (void)a;
    (void)b;
    return 0;
}
