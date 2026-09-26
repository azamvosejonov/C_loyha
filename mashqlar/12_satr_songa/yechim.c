/* =============================================================================
 *  12 - Satrni songa aylantirish                           [2-modul: ko'rsatkichlar]
 * =============================================================================
 *
 *  VAZIFA (atoi/strtol ISHLATMANG):
 *    satr_songa(s, &x) - butun satr o'nlik son bo'lishi kerak:
 *        ixtiyoriy '+' yoki '-', keyin KAMIDA bitta raqam, boshqa hech narsa yo'q.
 *        To'g'ri:     *natija = son, return 0
 *        Noto'g'ri:   return -1     ("", "-", "+", "12a", " 12", "1 2", "--1")
 *        Toshish:     return -2     (long'ga sig'maydi)
 *        Xato bo'lsa *natija ga TEGMANG.
 *
 *  PYTHON'DA:
 *    try: x = int(s)  except ValueError: ...   (Python'da toshish yo'q)
 *
 *  NEGA:
 *    Foydalanuvchi kiritgan har bir son (buyruq argumenti, konfiguratsiya,
 *    yadro parametri `init=`, `kill -9 123`) aynan shunday tekshiriladi.
 *    "Yarim to'g'ri" kirishni qabul qilish - xatolar manbai.
 *
 *  MASLAHAT:
 *    * Toshishni HISOBLASHDAN OLDIN tekshiring (03-mashqni eslang):
 *      x * 10 + raqam > LONG_MAX  <=>  x > (LONG_MAX - raqam) / 10
 *    * LONG_MIN (-9223372036854775808) to'g'ri son, lekin uning musbati
 *      long'ga sig'maydi! Yechim: manfiy tomonda yig'ish yoki unsigned long.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 12
 * ============================================================================= */
#include <limits.h>

#include "mashq.h"

int satr_songa(const char *s, long *natija)
{
    /* TODO */
    (void)s; (void)natija;
    return -1;
}
