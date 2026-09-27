/* =============================================================================
 *  45 - O'quvchilar-yozuvchilar qulfi                    [7-modul: nazariya amalda]
 * =============================================================================
 *
 *  VAZIFA (darslik 26-bob):
 *    Bir vaqtda:
 *      * istalgancha O'QUVCHI birga ishlashi mumkin, YOKI
 *      * faqat bitta YOZUVCHI (o'quvchilarsiz).
 *    Qo'shimcha talab - YOZUVCHI OCH QOLMASIN: yozuvchi kutayotgan bo'lsa, YANGI
 *    o'quvchilar kirmaydi (mavjudlari chiqib bo'lgach yozuvchi kiradi). Test buni
 *    tekshiradi: o'quvchilar to'xtovsiz kirib-chiqib tursa ham yozuvchi 1 soniya ichida
 *    kirishi kerak. ("VAQT TUGADI" chiqsa - yozuvchi och qolgan: yangi o'quvchilar uni
 *    doim quvib o'tib ketmoqda.)
 *
 *  QANDAY:
 *    mutex + 2 ta shart o'zgaruvchisi + hisoblagichlar:
 *      oquvchilar (hozir ichkarida), yozuvchi_ichkarida (0/1), kutayotgan_yozuvchilar.
 *    oqish_ol:  yozuvchi ichkarida YOKI kutayotgan yozuvchi bor ekan - kutish
 *    yozish_ol: kutayotgan_yozuvchilar++; o'quvchi yoki yozuvchi bor ekan - kutish;
 *               keyin kutayotgan--, yozuvchi_ichkarida = 1
 *    ...qo'yish amallarida kimni uyg'otish kerakligini o'ylang (broadcast yoki signal?)
 *
 *  NEGA:
 *    O'qish ko'p, yozish kam bo'lgan ma'lumotlar (sozlamalar, marshrutlash jadvallari,
 *    fayl tizimi tuzilmalari) uchun - oddiy mutex'dan ancha tez. Linux yadrosida:
 *    rwlock_t, rw_semaphore (mm->mmap_lock - jarayonning xotira xaritasi), va eng tez
 *    variant - RCU (26-bob).
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 45
 * ============================================================================= */
#include <pthread.h>
#include <stdlib.h>

#include "mashq.h"

struct rwqulf {
    int hali_bosh;      /* TODO */
};

struct rwqulf *rw_yarat(void)
{
    /* TODO */
    return NULL;
}

void rw_oqish_ol(struct rwqulf *q)
{
    /* TODO */
    (void)q;
}

void rw_oqish_qoy(struct rwqulf *q)
{
    /* TODO */
    (void)q;
}

void rw_yozish_ol(struct rwqulf *q)
{
    /* TODO */
    (void)q;
}

void rw_yozish_qoy(struct rwqulf *q)
{
    /* TODO */
    (void)q;
}

void rw_yoq(struct rwqulf *q)
{
    /* TODO */
    (void)q;
}
