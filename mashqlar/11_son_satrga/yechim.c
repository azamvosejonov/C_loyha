/* =============================================================================
 *  11 - Sonni satrga aylantirish                           [2-modul: ko'rsatkichlar]
 * =============================================================================
 *
 *  VAZIFA (printf, snprintf va sprintf ISHLATMANG - ularni o'zingiz yozyapsiz):
 *    son_satrga(x, buf, hajm)  - x ni o'nlik satrga: -42 -> "-42"
 *    hex_satrga(x, buf, hajm)  - x ni o'n oltilik satrga, kichik harflar,
 *                                "0x" siz: 255 -> "ff", 0 -> "0"
 *    Ikkalasi ham: natija (+ '\0') buf ga sig'sa - yozadi va UZUNLIKNI qaytaradi;
 *    sig'masa - -1 qaytaradi (buf da nima qolishi muhim emas, lekin hajmdan
 *    tashqariga YOZMANG).
 *
 *  PYTHON'DA:
 *    str(x),  format(x, 'x')
 *
 *  NEGA:
 *    Yadroda printf yo'q - uni o'zingiz yozasiz (MyOS: kernel/lib/kprintf.c,
 *    user/libc/printf.c -> emit_number). Bu funksiya shuning yuragi.
 *
 *  MASLAHAT:
 *    * Raqamlar "orqadan" chiqadi: 1234 % 10 = 4, 1234 / 10 = 123 ... Avval
 *      vaqtinchalik massivga teskari yozib, keyin to'g'ri tartibda ko'chiring.
 *    * TUZOQ: LONG_MIN = -9223372036854775808. `-x` qilsangiz - toshish (UB)!
 *      Musbat qiymatni `unsigned long` da hisoblang: `0UL - (unsigned long)x`.
 *    * Raqam belgisi: '0' + raqam.  O'n oltilikda: "0123456789abcdef"[raqam].
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 11
 * ============================================================================= */
#include "mashq.h"

int son_satrga(long x, char *buf, size_t hajm)
{
    /* TODO */
    (void)x; (void)buf; (void)hajm;
    return -1;
}

int hex_satrga(unsigned long x, char *buf, size_t hajm)
{
    /* TODO */
    (void)x; (void)buf; (void)hajm;
    return -1;
}
