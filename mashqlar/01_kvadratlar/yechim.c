/* =============================================================================
 *  01 - Kvadratlar yig'indisi                          [1-modul: turlar va sikllar]
 * =============================================================================
 *
 *  VAZIFA:
 *    kvadratlar_yigindisi(n) = 1*1 + 2*2 + ... + n*n.   n <= 0 bo'lsa -> 0.
 *
 *  PYTHON'DA:
 *    def kvadratlar_yigindisi(n): return sum(i * i for i in range(1, n + 1))
 *
 *  C'DA NIMA BOSHQA:
 *    * Har bir o'zgaruvchining TURI bor va u o'zgarmaydi: `int i = 0;`
 *    * int - 32 bit: eng kattasi 2 147 483 647. Python'da son cheksiz o'sadi,
 *      C'da esa TOSHIB KETADI. long - 64 bit (Linux x86-64 da).
 *    * `for (boshlash; shart; qadam) { ... }` - Python'dagi range() ning o'rni.
 *
 *  MASLAHAT:
 *    Test n = 100000 ni ham tekshiradi. 100000 * 100000 = 10^10 - int'ga
 *    sig'maydi! `i * i` ni hisoblashdan OLDIN kattaroq turga o'tkazing:
 *    `(long)i * i`. (Sanitizer "signed integer overflow" deb ushlaydi.)
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 01
 * ============================================================================= */
#include "mashq.h"

long kvadratlar_yigindisi(int n)
{
    /* TODO: shu yerga yozing */
    (void)n;            /* "n hali ishlatilmayapti" ogohlantirishini o'chirish - yozganda olib tashlang */
    return 0;
}
