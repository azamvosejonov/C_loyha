/* =============================================================================
 *  23 - Linux uslubidagi ichki ro'yxat              [4-modul: yadro uslubidagi C]
 * =============================================================================
 *
 *  VAZIFA (struct list_head va container_of - mashq.h da):
 *    list_init(h)             - bo'sh ro'yxat: bosh o'ziga o'zi ko'rsatadi
 *                               (h->next = h->prev = h). NULL yo'q - halqa!
 *    list_add_tail(yangi, h)  - yangi elementni oxiriga (h->prev dan keyin) qo'shish
 *    list_del(e)              - elementni ro'yxatdan chiqarish (qo'shnilarini ulab)
 *                               va e->next = e->prev = NULL qilish
 *    list_empty(h)            - bo'shmi?
 *    eng_ustuvor_id(h)        - ro'yxatdagi struct vazifa'lar ichida ustuvorligi eng
 *                               katta bo'lganining id si (tenglarda - birinchisi).
 *                               Bo'sh ro'yxat -> -1.
 *
 *  BU NIMA VA NEGA:
 *    16-mashqdagi ro'yxatda tugun ma'lumotni "o'z ichiga" olardi. Linux'da
 *    aksincha: ma'lumot (struct vazifa) ro'yxat tugunini (struct list_head)
 *    o'z ICHIGA oladi. Bitta list_head kodi har qanday strukturalar uchun
 *    ishlaydi, bitta obyekt esa bir nechta ro'yxatda bo'lishi mumkin
 *    (bir nechta list_head maydoni bilan). Linux yadrosida bu tuzilma
 *    o'n minglab joyda ishlatiladi. MyOS: kernel/lib/list.h.
 *
 *    container_of: `node` maydonining manzilidan butun `struct vazifa` ning
 *    manzilini topadi - a'zoning struktura ichidagi siljishini (offsetof)
 *    ayirib. Qog'ozda chizing: vazifa @ 1000, id @ 1000, ustuvorlik @ 1004,
 *    node @ 1008. node manzili 1008 bo'lsa -> 1008 - 8 = 1000.
 *
 *  MASLAHAT:
 *    * Aylanish: `for (struct list_head *p = h->next; p != h; p = p->next)`
 *    * Element: `struct vazifa *v = container_of(p, struct vazifa, node);`
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 23
 * ============================================================================= */
#include "mashq.h"

void list_init(struct list_head *h)
{
    /* TODO */
    (void)h;
}

void list_add_tail(struct list_head *yangi, struct list_head *h)
{
    /* TODO */
    (void)yangi; (void)h;
}

void list_del(struct list_head *e)
{
    /* TODO */
    (void)e;
}

bool list_empty(const struct list_head *h)
{
    /* TODO */
    (void)h;
    return true;
}

int eng_ustuvor_id(struct list_head *h)
{
    /* TODO */
    (void)h;
    return -1;
}
