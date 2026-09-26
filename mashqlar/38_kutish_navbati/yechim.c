/* =============================================================================
 *  38 - Bloklanadigan navbat: uxlash va uyg'otish        [6-modul: yadro mexanizmlari]
 * =============================================================================
 *
 *  VAZIFA - ko'p oqimli "ishlab chiqaruvchi - iste'molchi" navbati:
 *    navbat_yarat(sigim)  - sigim ta int sig'adigan bo'sh navbat (halqa bufer - 22-mashq)
 *    navbat_qoy(q, x)     - oxiriga qo'yish. Navbat TO'LA bo'lsa - joy bo'shaguncha
 *                           UXLASH (CPU'ni band qilmasdan kutish).
 *    navbat_ol(q)         - boshidagini olish. Navbat BO'SH bo'lsa - element
 *                           kelguncha uxlash.
 *    navbat_yoq(q)        - resurslarni ozod qilish.
 *
 *  VOSITALAR (<pthread.h>):
 *    pthread_mutex_t        - navbat tuzilmasini himoya qiladi
 *    pthread_cond_t         - "shart o'zgaruvchisi": oqim unda uxlaydi
 *    pthread_cond_wait(&c, &m) - m ni ATOMIK ravishda qo'yib yuborib uxlaydi,
 *                             uyg'onganda m ni qayta oladi
 *    pthread_cond_signal(&c) / pthread_cond_broadcast(&c) - uxlaganlarni uyg'otish
 *
 *  IKKI KLASSIK XATO (ikkalasi ham oqimlarni abadiy uxlatib qo'yadi - test "VAQT TUGADI"):
 *    1) "Yo'qolgan uyg'otish" (lost wakeup): shartni mutex'siz tekshirib, keyin
 *       uxlash. Tekshiruv va uxlash orasida boshqa oqim signal bersa - u yo'qoladi.
 *       Shuning uchun cond_wait mutex bilan birga ishlaydi.
 *    2) `if (bosh) wait;` - `while (bosh) wait;` bo'lishi SHART. Uyg'otilganda shart
 *       hali ham (yoki yana) yolg'on bo'lishi mumkin: boshqa oqim elementni birinchi
 *       olib ketgan, yoki "soxta uyg'onish" (spurious wakeup) bo'lgan.
 *
 *  NEGA BU YADRO UCHUN:
 *    Yadro ichida aynan shu mexanizm: pipe to'la - yozuvchi uxlaydi, bo'sh - o'quvchi
 *    uxlaydi (kernel/fs/pipe.c); disk javobini kutish; klaviatura kirishini kutish.
 *    MyOS'da: proc_sleep(kalit, qulf) - qulfni atomik qo'yib yuborib uxlash,
 *    proc_wakeup(kalit) - uyg'otish (kernel/proc/process.c, sleep_wakeup lab'i).
 *    pthread_cond_wait - user rejimidagi xuddi o'sha g'oya.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 38
 * ============================================================================= */
#include <pthread.h>
#include <stdlib.h>

#include "mashq.h"

struct navbat {
    int hali_bosh;      /* TODO: bufer, bosh, soni, sigim, mutex, ikkita cond */
};

struct navbat *navbat_yarat(size_t sigim)
{
    /* TODO */
    (void)sigim;
    return NULL;
}

void navbat_qoy(struct navbat *q, int x)
{
    /* TODO */
    (void)q; (void)x;
}

int navbat_ol(struct navbat *q)
{
    /* TODO */
    (void)q;
    return -1;
}

void navbat_yoq(struct navbat *q)
{
    /* TODO */
    (void)q;
}
