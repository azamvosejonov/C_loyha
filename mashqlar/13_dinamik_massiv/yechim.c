/* =============================================================================
 *  13 - Dinamik massiv (Python list)                          [3-modul: xotira]
 * =============================================================================
 *
 *  VAZIFA (struct vec - mashq.h da):
 *    vec_init(v)          - bo'sh massiv: data = NULL, len = cap = 0
 *    vec_push(v, x)       - oxiriga qo'shish. Joy tugasa - kattalashtirish.
 *                           0 - muvaffaqiyat, -1 - xotira yetmadi
 *    vec_pop(v, &x)       - oxirgisini olib tashlab *out ga yozish; bo'sh -> -1
 *    vec_insert(v, i, x)  - i-o'ringa qo'shish (i == len - oxiriga), i > len -> -1
 *    vec_free(v)          - xotirani qaytarish, keyin v yana bo'sh (data = NULL ...)
 *
 *  PYTHON'DA:
 *    a = []; a.append(x); a.pop(); a.insert(i, x)   - xotira haqida o'ylamaysiz
 *
 *  C'DA NIMA BOSHQA - HEAP:
 *    * malloc(bayt) - xotira so'rash (NULL - berilmadi). free(p) - qaytarish.
 *      Python buni sizning o'rningizga qiladi (garbage collector). C'da -
 *      har bir malloc'ga aniq BITTA free. Unutilsa - xotira sizib chiqadi (leak).
 *    * realloc(p, yangi_hajm) - blokni kattalashtiradi (kerak bo'lsa ko'chiradi).
 *      TUZOQ: `v->data = realloc(v->data, ...)` - realloc NULL qaytarsa, eski
 *      blok yo'qoladi (leak)! Avval vaqtinchalik o'zgaruvchiga oling.
 *    * Hajmni ELEMENTLAR soniga emas, BAYTLARGA hisoblang: n * sizeof(int).
 *
 *  MASLAHAT:
 *    * Joy tugaganda cap ni 2 BARAVAR oshiring (0 bo'lsa - 8). Nega +1 emas?
 *      100000 ta push'da +1 - 100000 marta realloc (sekin), x2 - ~17 marta.
 *      Python list ham, Linux'ning ko'p tuzilmalari ham shunday o'sadi.
 *    * vec_insert: elementlarni o'ngga surish - memmove (ustma-ust nusxalash).
 *    * Test oxirida LeakSanitizer free qilinmagan xotirani qidiradi.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 13
 * ============================================================================= */
#include <stdlib.h>
#include <string.h>

#include "mashq.h"

void vec_init(struct vec *v)
{
    /* TODO */
    (void)v;
}

int vec_push(struct vec *v, int x)
{
    /* TODO */
    (void)v; (void)x;
    return -1;
}

int vec_pop(struct vec *v, int *out)
{
    /* TODO */
    (void)v; (void)out;
    return -1;
}

int vec_insert(struct vec *v, size_t i, int x)
{
    /* TODO */
    (void)v; (void)i; (void)x;
    return -1;
}

void vec_free(struct vec *v)
{
    /* TODO */
    (void)v;
}
