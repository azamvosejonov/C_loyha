/* =============================================================================
 *  42 - LRU kesh (O(1))                                  [7-modul: nazariya amalda]
 * =============================================================================
 *
 *  VAZIFA (darslik 21, 24, 28-boblar):
 *    Sig'imi cheklangan kalit -> qiymat keshi. To'lganda ENG UZOQ VAQT ISHLATILMAGAN
 *    (least recently used) yozuv chiqarib tashlanadi.
 *      lru_yarat(sigim)       - bo'sh kesh (sigim >= 1)
 *      lru_ol(c, k, &v)       - bor bo'lsa: *v = qiymat, return 0 va bu yozuv "eng yangi" bo'ladi;
 *                               yo'q bo'lsa: -1
 *      lru_qoy(c, k, v)       - bor bo'lsa: qiymatni yangilash (va "eng yangi" qilish);
 *                               yo'q bo'lsa: qo'shish; sig'im oshsa - eng eskisini chiqarish
 *      lru_soni(c), lru_yoq(c)
 *    Har bir amal O(1) bo'lishi SHART: test sig'imi 50 000 li keshda 2 000 000 ta amal
 *    bajaradi - O(n) yechim (massivda qidirish) daqiqalar oladi.
 *
 *  QANDAY (klassik dizayn):
 *    * XESH JADVAL: kalit -> tugun  (17-mashq)  -> O(1) topish
 *    * IKKI TOMONLAMA BOG'LANGAN RO'YXAT: tugunlar ishlatilish tartibida (boshida - eng
 *      yangi, oxirida - eng eski)  -> O(1) ko'chirish va chiqarish (23-mashq g'oyasi)
 *    Har bir tugun ikkala tuzilmada ham bor.
 *
 *  NEGA:
 *    CPU keshlari, TLB, sahifa keshi, inode/dentry keshlari, veb-keshlar - hammasi
 *    LRU yoki uning yaqinlashuvi (Clock - 43-mashq). Bu - suhbatlarda eng ko'p
 *    so'raladigan loyihalash masalalaridan biri ham.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 42
 * ============================================================================= */
#include <stdlib.h>

#include "mashq.h"

struct lru {
    int hali_bosh;      /* TODO */
};

struct lru *lru_yarat(size_t sigim)
{
    /* TODO */
    (void)sigim;
    return NULL;
}

int lru_ol(struct lru *c, int kalit, int *qiymat)
{
    /* TODO */
    (void)c; (void)kalit; (void)qiymat;
    return -1;
}

void lru_qoy(struct lru *c, int kalit, int qiymat)
{
    /* TODO */
    (void)c; (void)kalit; (void)qiymat;
}

size_t lru_soni(const struct lru *c)
{
    /* TODO */
    (void)c;
    return 0;
}

void lru_yoq(struct lru *c)
{
    /* TODO */
    (void)c;
}
