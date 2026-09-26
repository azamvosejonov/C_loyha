/* =============================================================================
 *  05 - Massivlar                                      [1-modul: turlar va sikllar]
 * =============================================================================
 *
 *  VAZIFA:
 *    eng_katta(a, n)     - a[0..n-1] ichidagi eng katta son (n >= 1 kafolatlangan)
 *    teskari(a, n)       - massivni JOYIDA teskari aylantirish (n = 0 ham bo'lishi mumkin!)
 *    takrorlanmas(a, n)  - SARALANGAN massivdan takrorlarni joyida olib tashlash:
 *                          [1,1,2,3,3,3,4] -> boshiga [1,2,3,4] yoziladi, 4 qaytadi
 *
 *  PYTHON'DA:
 *    max(a),  a.reverse(),  sorted(set(a))
 *
 *  C'DA NIMA BOSHQA:
 *    * C'da massiv o'z uzunligini BILMAYDI. Funksiyaga faqat birinchi elementning
 *      manzili (`int *a`) keladi, uzunlikni (`size_t n`) alohida berish shart.
 *      (Shuning uchun C'dagi funksiyalarda doim `buf, len` juftligi bo'ladi.)
 *    * `const int *a` - "men a ni faqat o'qiyman, o'zgartirmayman" degan va'da.
 *    * size_t - ishorasiz. TUZOQ: n = 0 bo'lsa, `n - 1` = 18446744073709551615!
 *      `for (size_t i = 0, j = n - 1; i < j; ...)` n = 0 da nima bo'ladi?
 *
 *  MASLAHAT:
 *    takrorlanmas: ikki ko'rsatkich - "o'qish" (i) va "yozish" (k) indekslari.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 05
 * ============================================================================= */
#include "mashq.h"

int eng_katta(const int *a, size_t n)
{
    /* TODO */
    (void)a; (void)n;
    return 0;
}

void teskari(int *a, size_t n)
{
    /* TODO */
    (void)a; (void)n;
}

size_t takrorlanmas(int *a, size_t n)
{
    /* TODO */
    (void)a; (void)n;
    return 0;
}
