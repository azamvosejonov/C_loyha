/*
 * turlar.h — emulyatorning hamma fayllari ishlatadigan umumiy turlar va kichik yordamchilar.
 *
 * NEGA ALOHIDA FAYL?
 *   Emulyator "apparat"ni dasturda tasvirlaydi. Apparatda hamma narsa ANIQ o'lchamli: registr 32 bit,
 *   bayt 8 bit, manzil 32 bit. Shuning uchun `int` (o'lchami kompilyatorga bog'liq) o'rniga <stdint.h>
 *   dagi aniq turlarni ishlatamiz: uint32_t — aynan 32 bit, ishorasiz.
 *
 * BITLAR BILAN ISHLASH (16-bob)
 *   RISC-V buyrug'i — 32 bitlik son. Uning ichida maydonlar (opcode, rd, rs1 ...) aniq BIT oraliqlarida turadi.
 *   Masalan rd — 11..7-bitlar. Uni ajratib olish: (buyruq >> 7) & 0x1F.
 *   Bu amalni yuzlab marta yozmaslik uchun BITLAR(x, yuqori, past) makrosini yozamiz.
 */
#ifndef TURLAR_H
#define TURLAR_H

#include <stddef.h>
#include <stdint.h>

/* x ning [yuqori .. past] bitlarini (ikkalasi ham kiradi) o'ngga surib qaytaradi.
   Misol: BITLAR(0b1101100, 5, 3) -> 0b101 = 5.
   Qadamlar: 1) x >> past — kerakli bitlarni 0-o'ringa suradi;
             2) & ((1 << kenglik) - 1) — faqat "kenglik" ta past bitni qoldiradi (maska).
   (1ull ishlatiladi: kenglik 32 bo'lsa 1u << 32 — aniqlanmagan xatti-harakat, 64 bitda esa xavfsiz.) */
#define BITLAR(x, yuqori, past) \
    ((uint32_t)(((x) >> (past)) & (uint32_t)((1ull << ((yuqori) - (past) + 1)) - 1)))

/* bitta bit: BIT(x, n) -> 0 yoki 1 */
#define BIT(x, n) (((x) >> (n)) & 1u)

/*
 * ishora_kengaytir(qiymat, bitlar) — "bitlar" bitlik ISHORALI sonni 32 bitga kengaytiradi.
 *
 * Nega kerak: RISC-V buyrug'idagi o'zgarmas son (immediate) masalan 12 bit. 12 bitda 0xFFF = -1
 * (ikkiga to'ldirish, 2-bob). Uni 32 bitli registrga qo'shish uchun 0xFFFFFFFF ga aylantirish kerak:
 * eng yuqori (ishora) biti 1 bo'lsa — yuqoridagi hamma bitlar 1 bilan to'ldiriladi.
 *
 * Usul: avval chapga surib, ishora bitini 31-o'ringa olib chiqamiz, keyin ISHORALI (int32_t)
 * o'ngga surish ishora bitini "ko'paytirib" qaytaradi. Masalan bitlar=12:
 *     0x00000FFF << 20 = 0xFFF00000  ->  (int32_t) >> 20 = 0xFFFFFFFF  (= -1)
 *     0x000007FF << 20 = 0x7FF00000  ->  (int32_t) >> 20 = 0x000007FF  (= 2047)
 * Eslatma: manfiy sonni o'ngga surish C standartida "implementatsiyaga bog'liq", lekin GCC va Clang
 * hujjatlashtirgan holda arifmetik siljitish qiladi — bu loyiha shunga tayanadi.
 */
static inline uint32_t ishora_kengaytir(uint32_t qiymat, int bitlar)
{
    int siljish = 32 - bitlar;
    return (uint32_t)((int32_t)(qiymat << siljish) >> siljish);
}

#endif
