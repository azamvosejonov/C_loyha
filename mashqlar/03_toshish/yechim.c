/* =============================================================================
 *  03 - Toshishsiz arifmetika                          [1-modul: turlar va sikllar]
 * =============================================================================
 *
 *  VAZIFA:
 *    xavfsiz_qoshish(a, b, &r)     - natija int'ga sig'sa: *r = a + b, return 0
 *                                    sig'masa: *r ga TEGMANG, return -1
 *    xavfsiz_kopaytirish(a, b, &r) - xuddi shunday, a * b uchun
 *
 *  PYTHON'DA:
 *    Bunday muammo yo'q: 2**31 + 1 shunchaki kattaroq son.
 *
 *  C'DA NIMA BOSHQA (MUHIM!):
 *    Ishorali son toshsa - bu "aniqlanmagan xatti-harakat" (undefined behavior,
 *    UB). Kompilyator "UB hech qachon bo'lmaydi" deb faraz qiladi va shunga
 *    tayanib kodingizni "optimallashtirishi" mumkin. Shuning uchun AVVAL a + b
 *    ni hisoblab, KEYIN "manfiy bo'lib qoldimi?" deb tekshirish - XATO.
 *    Toshishni hisoblashdan OLDIN aniqlash kerak. Yadroda bunday xato -
 *    xavfsizlik teshigi (masalan, buferning hajmini hisoblashda).
 *
 *  MASLAHAT:
 *    * Qo'shish: b > 0 bo'lsa, a > INT_MAX - b bo'lganda toshadi.
 *                b < 0 bo'lsa, a < INT_MIN - b bo'lganda toshadi.
 *    * Ko'paytirish: eng oson yo'l - `long long` da hisoblab, chegarani
 *      tekshirish (64 bit 32 bitli ikki sonning ko'paytmasiga doim sig'adi).
 *    * INT_MAX va INT_MIN - <limits.h> da.
 *    * `int *natija` - chiqish parametri: natijani `*natija = ...` bilan yozasiz.
 *      (Batafsil - 06/07-mashqlarda.)
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 03
 * ============================================================================= */
#include <limits.h>

#include "mashq.h"

int xavfsiz_qoshish(int a, int b, int *natija)
{
    /* TODO */
    (void)a; (void)b; (void)natija;
    return -1;
}

int xavfsiz_kopaytirish(int a, int b, int *natija)
{
    /* TODO */
    (void)a; (void)b; (void)natija;
    return -1;
}
