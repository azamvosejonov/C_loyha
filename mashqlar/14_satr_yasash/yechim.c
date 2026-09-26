/* =============================================================================
 *  14 - Yangi satrlar yasash                                   [3-modul: xotira]
 * =============================================================================
 *
 *  VAZIFA - har bir funksiya YANGI satrni malloc qilib qaytaradi (chaqiruvchi
 *  keyin uni free qiladi). Xotira yetmasa - NULL.
 *    birlashtir("sa", "lom")               -> "salom"
 *    takrorla("ab", 3)                     -> "ababab"      (n = 0 -> "")
 *    qoshib_yoz({"a","b","c"}, 3, ", ")    -> "a, b, c"     (n = 0 -> "")
 *
 *  PYTHON'DA:
 *    a + b,   s * n,   ", ".join(qismlar)
 *
 *  C'DA NIMA BOSHQA:
 *    * Python'da `a + b` yangi satrni avtomatik yaratadi. C'da: 1) kerakli
 *      hajmni HISOBLASH, 2) malloc, 3) nusxalash, 4) '\0' qo'yish.
 *    * ENG KO'P UCHRAYDIGAN XATO: malloc(strlen(a) + strlen(b)) - '\0' uchun
 *      +1 unutilgan. Sanitizer buni "heap-buffer-overflow" deb ushlaydi.
 *    * "Kim free qiladi?" - C'da har bir funksiyaning hujjatida yozilishi shart.
 *      Bu yerda: chaqiruvchi. (Yadroda ham: kmalloc qaytaradigan har bir
 *      funksiya "egalik" qoidasini aytadi.)
 *    * `const char *const *qismlar` - "o'zgarmas satrlarning o'zgarmas massivi".
 *      O'ngdan chapga o'qing: qismlar -> ko'rsatkich -> const ko'rsatkich -> const char.
 *
 *  MASLAHAT:
 *    Ikki o'tish: avval jami uzunlikni hisoblang, keyin bir marta malloc qiling
 *    va memcpy bilan ko'chiring. (Har bir qism uchun realloc - sekin.)
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 14
 * ============================================================================= */
#include <stdlib.h>
#include <string.h>

#include "mashq.h"

char *birlashtir(const char *a, const char *b)
{
    /* TODO */
    (void)a; (void)b;
    return NULL;
}

char *takrorla(const char *s, size_t n)
{
    /* TODO */
    (void)s; (void)n;
    return NULL;
}

char *qoshib_yoz(const char *const *qismlar, size_t n, const char *ajratgich)
{
    /* TODO */
    (void)qismlar; (void)n; (void)ajratgich;
    return NULL;
}
