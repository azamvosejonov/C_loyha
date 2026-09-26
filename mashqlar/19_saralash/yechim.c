/* =============================================================================
 *  19 - Saralash va struct'lar                      [4-modul: yadro uslubidagi C]
 * =============================================================================
 *
 *  VAZIFA:
 *    mening_saralashim(a, n)
 *        int massivini o'sish tartibida saralang - qsort ISHLATMASDAN.
 *        Test 200 000 ta element beradi: O(n^2) algoritm (pufakcha, qo'yish)
 *        juda sekin bo'ladi. O(n log n) kerak: birlashtirib saralash (merge
 *        sort) yoki heapsort.
 *    talabalarni_saralash(a, n)
 *        ball bo'yicha KAMAYISH tartibida; ball teng bo'lsa - ism bo'yicha
 *        alifbo tartibida (strcmp). Bu yerda standart qsort() ni ishlating.
 *
 *  PYTHON'DA:
 *    a.sort()
 *    a.sort(key=lambda t: (-t.ball, t.ism))
 *
 *  C'DA NIMA BOSHQA:
 *    * qsort(massiv, soni, element_hajmi, taqqoslash_funksiyasi) - funksiyaga
 *      FUNKSIYA uzatiladi (funksiya ko'rsatkichi, 20-mashqda batafsil).
 *      Taqqoslash `const void *` oladi - uni o'zingiz to'g'ri turga aylantirasiz.
 *    * TUZOQ: `return x->ball - y->ball;` - katta sonlarda TOSHADI. Xavfsizi:
 *      `(a > b) - (a < b)`.
 *    * struct nusxalanadi `=` bilan (massiv maydoni ham ichida!) - Python'dagi
 *      obyektlardan farqli, bu HAQIQIY nusxa, havola emas.
 *
 *  MASLAHAT (merge sort):
 *    Yarmini saralash, ikkinchi yarmini saralash, ikkalasini yordamchi
 *    buferga birlashtirish. Yordamchi buferni BIR MARTA malloc qiling (har
 *    bir rekursiv chaqiruvda emas) va oxirida free qiling.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 19
 * ============================================================================= */
#include <stdlib.h>
#include <string.h>

#include "mashq.h"

void mening_saralashim(int *a, size_t n)
{
    /* TODO */
    (void)a; (void)n;
}

void talabalarni_saralash(struct talaba *a, size_t n)
{
    /* TODO */
    (void)a; (void)n;
}
