/* =============================================================================
 *  17 - Xesh jadval (Python dict)                              [3-modul: xotira]
 * =============================================================================
 *
 *  VAZIFA (satr -> int lug'ati):
 *    xesh_yarat()             - bo'sh jadval (malloc), xato -> NULL
 *    xesh_qoy(h, k, v)        - k -> v. k bor bo'lsa - qiymatni yangilash.
 *                               Kalitning NUSXASINI saqlang (chaqiruvchining satri
 *                               keyin o'zgarishi yoki yo'qolishi mumkin!). 0 / -1
 *    xesh_ol(h, k, &v)        - topilsa *qiymat = v, return 0; yo'q -> -1
 *    xesh_ochir(h, k)         - o'chirish: 0; yo'q edi -> -1
 *    xesh_soni(h)             - nechta kalit bor
 *    xesh_ozod(h)             - HAMMA xotirani qaytarish (kalit nusxalari ham!)
 *
 *  PYTHON'DA:
 *    d = {}; d[k] = v; d.get(k); del d[k]; len(d)
 *
 *  QANDAY ISHLAYDI:
 *    Massiv ("chelaklar", buckets) + har bir chelakda bog'langan ro'yxat.
 *    Kalitdan son hisoblanadi (xesh funksiya), `xesh % chelaklar_soni` - qaysi
 *    chelakka tushishi. Yaxshi xesh kalitlarni chelaklarga TEKIS taqsimlaydi.
 *
 *  MASLAHAT:
 *    * Xesh funksiya - FNV-1a (oddiy va yaxshi):
 *          uint64_t h = 1469598103934665603ULL;
 *          for (har bir bayt c) { h ^= c; h *= 1099511628211ULL; }
 *    * Test 100000 ta kalit qo'yadi. 16 ta chelak bilan har bir ro'yxat ~6000
 *      tugun - juda sekin! Elementlar soni chelaklar sonidan oshganda, jadvalni
 *      2 BARAVAR kattalashtiring (rehash): yangi massiv, hamma tugunlarni
 *      ko'chirish. (Python dict ham, Linux'ning rhashtable'i ham shunday qiladi.)
 *    * struct xesh ni o'zingiz shu faylda ta'riflaysiz (hozir bo'sh "qolip" bor).
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 17
 * ============================================================================= */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "mashq.h"

struct xesh {
    int hali_bosh;      /* TODO: o'z maydonlaringiz bilan almashtiring */
};

struct xesh *xesh_yarat(void)
{
    /* TODO */
    return NULL;
}

int xesh_qoy(struct xesh *h, const char *kalit, int qiymat)
{
    /* TODO */
    (void)h; (void)kalit; (void)qiymat;
    return -1;
}

int xesh_ol(const struct xesh *h, const char *kalit, int *qiymat)
{
    /* TODO */
    (void)h; (void)kalit; (void)qiymat;
    return -1;
}

int xesh_ochir(struct xesh *h, const char *kalit)
{
    /* TODO */
    (void)h; (void)kalit;
    return -1;
}

size_t xesh_soni(const struct xesh *h)
{
    /* TODO */
    (void)h;
    return 0;
}

void xesh_ozod(struct xesh *h)
{
    /* TODO */
    (void)h;
}
