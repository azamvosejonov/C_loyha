/* =============================================================================
 *  46 - Min-heap (ustuvorlik navbati)                    [7-modul: nazariya amalda]
 * =============================================================================
 *
 *  VAZIFA (darslik 28-bob, struct heap - mashq.h da):
 *    heap_init(h)         - bo'sh heap (a = NULL, n = sig = 0)
 *    heap_qosh(h, x)      - qo'shish, O(log n): oxiriga qo'yib "yuqoriga suzdirish"
 *                           (otasidan kichik ekan - almashtirish). 0 / -1 (xotira)
 *    heap_ol(h, &x)       - eng kichigini olish, O(log n): a[0] ni olib, oxirgisini
 *                           tepaga qo'yib "pastga cho'ktirish" (kichikroq bolasi bilan
 *                           almashtirish). Bo'sh bo'lsa -1.
 *    heap_tepa(h, &x)     - eng kichigini olmasdan ko'rish, O(1). Bo'sh bo'lsa -1.
 *    heap_qur(h, m, n)    - h ning eski mazmunini tashlab (xotirasini qayta ishlatish
 *                           yoki ozod qilish mumkin), massivdan heap qurish O(n) da: nusxalab, oxirgi otadan
 *                           (n/2 - 1) boshlab 0 gacha har birini "cho'ktirish".
 *                           (n marta heap_qosh - O(n log n); bu - tezroq.) 0 / -1
 *    heap_yoq(h)
 *    Test har bir amaldan keyin HEAP XOSSASINI tekshiradi: a[i] >= a[(i-1)/2].
 *
 *  NEGA:
 *    "Eng yaqin muddatli taymer", "eng kichik vruntime", "eng yuqori ustuvorlik" - yadroda
 *    har doim "eng kichigini tez olish" kerak. Heapsort (Linux lib/sort.c) ham shu tuzilma.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 46
 * ============================================================================= */
#include <stdlib.h>
#include <string.h>

#include "mashq.h"

void heap_init(struct heap *h)
{
    /* TODO */
    (void)h;
}

int heap_qosh(struct heap *h, int x)
{
    /* TODO */
    (void)h; (void)x;
    return -1;
}

int heap_ol(struct heap *h, int *x)
{
    /* TODO */
    (void)h; (void)x;
    return -1;
}

int heap_tepa(const struct heap *h, int *x)
{
    /* TODO */
    (void)h; (void)x;
    return -1;
}

int heap_qur(struct heap *h, const int *massiv, size_t n)
{
    /* TODO */
    (void)h; (void)massiv; (void)n;
    return -1;
}

void heap_yoq(struct heap *h)
{
    /* TODO */
    (void)h;
}
