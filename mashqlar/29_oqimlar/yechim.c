/* =============================================================================
 *  29 - Oqimlar (threads) va mutex                  [5-modul: tizim chaqiruvlari]
 * =============================================================================
 *
 *  VAZIFA:
 *    parallel_yigindi(a, n, k)
 *        a[0..n-1] yig'indisini k ta oqimda hisoblang: massivni k bo'lakka
 *        bo'ling (n k ga bo'linmasligi ham mumkin!), har bir oqim o'z bo'lagini
 *        yig'adi, oxirida natijalarni qo'shing.
 *    hisoblagich(k, m)
 *        k ta oqim BITTA umumiy hisoblagichni har biri m marta 1 ga oshiradi.
 *        Natija aniq k * m bo'lishi SHART. Mutex bilan himoya qiling.
 *
 *  PYTHON'DA:
 *    threading.Thread(...) - lekin GIL tufayli CPU ishini parallel qilmaydi.
 *    C'da oqimlar HAQIQATAN bir vaqtda turli yadrolarda ishlaydi - tezlik
 *    ham, xatolar ham shundan.
 *
 *  POYGA HOLATI (RACE CONDITION) - YADRONING ENG QIYIN MAVZUSI:
 *    `hisob++` bitta amalga o'xshaydi, aslida uchta: o'qish, oshirish, yozish.
 *    Ikki oqim bir vaqtda o'qisa - ikkalasi ham 5 ni o'qiydi, ikkalasi ham 6
 *    yozadi: bitta oshirish YO'QOLDI. Mutex'siz yozib ko'ring - natija har safar
 *    boshqacha va k * m dan kichik bo'ladi. MyOS'da buning yadro versiyasi:
 *    kernel/lib/spinlock.c, kernel/lib/mutex.c (va spinlock lab'i).
 *
 *  MASLAHAT:
 *    * pthread_create(&t, NULL, funksiya, argument) / pthread_join(t, NULL).
 *    * Oqim funksiyasi: `void *f(void *arg)` - bitta `void *` argument. Bir
 *      nechta qiymat kerak bo'lsa - struct yarating va uning manzilini bering.
 *      Har bir oqimga O'Z struct'i (massiv) - bitta umumiy o'zgaruvchi emas!
 *    * pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER; lock / unlock.
 *    * Tezlik: mutex'ni har bir ++ da olish - sekin, lekin to'g'ri. Qanday
 *      tezlashtirish mumkin? (Maslahat: lokal hisoblab, oxirida bir marta qo'shish.)
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 29
 * ============================================================================= */
#include <pthread.h>
#include <stdlib.h>

#include "mashq.h"

long long parallel_yigindi(const int *a, size_t n, int oqimlar_soni)
{
    /* TODO */
    (void)a; (void)n; (void)oqimlar_soni;
    return 0;
}

long hisoblagich(int oqimlar_soni, int har_biri)
{
    /* TODO */
    (void)oqimlar_soni; (void)har_biri;
    return 0;
}
