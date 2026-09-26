/* =============================================================================
 *  10 - Teskari satr va palindrom                          [2-modul: ko'rsatkichlar]
 * =============================================================================
 *
 *  VAZIFA:
 *    satr_teskari(s)  - satrni JOYIDA teskari aylantirish: "salom" -> "molas"
 *    palindrommi(s)   - faqat harf va raqamlarni hisobga olib, katta-kichik harfga
 *                       qaramasdan, oldidan ham orqasidan ham bir xil o'qiladimi?
 *                       "A man, a plan, a canal: Panama" -> true.  "" -> true.
 *
 *  PYTHON'DA:
 *    s[::-1]
 *    t = [c.lower() for c in s if c.isalnum()]; t == t[::-1]
 *
 *  C'DA NIMA BOSHQA:
 *    * Python'da satr o'zgarmas va s[::-1] yangi satr yaratadi. C'da satr -
 *      shunchaki baytlar massivi, uni joyida o'zgartirish mumkin (xotira ajratmasdan).
 *    * Ikki ko'rsatkich usuli: biri boshdan, biri oxirdan, o'rtada uchrashguncha.
 *    * <ctype.h>: isalnum(), tolower(). TUZOQ: ularga manfiy char berish - UB.
 *      Doim `isalnum((unsigned char)c)` deb yozing.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 10
 * ============================================================================= */
#include <ctype.h>
#include <string.h>

#include "mashq.h"

void satr_teskari(char *s)
{
    /* TODO */
    (void)s;
}

bool palindrommi(const char *s)
{
    /* TODO */
    (void)s;
    return false;
}
