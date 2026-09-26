/* =============================================================================
 *  34 - Spinlock va ticket lock                         [6-modul: yadro mexanizmlari]
 * =============================================================================
 *
 *  VAZIFA - yadroning eng asosiy qulflarini ATOMIK amallar bilan yozing
 *  (pthread_mutex ishlatmang - maqsad aynan mutex'ning ichini yozish):
 *
 *    spin_lock(l)     - l->band ni atomik ravishda 0 dan 1 ga o'zgartira olguncha
 *                       aylanish. ACQUIRE tartibi.
 *    spin_unlock(l)   - l->band = 0, RELEASE tartibi.
 *    spin_trylock(l)  - bir marta urinish: oldi -> true, band edi -> false (kutmasdan).
 *
 *    ticket_lock(l)   - "navbat chiptasi": atomik ravishda keyingi++ qilib o'z
 *                       chiptangizni olasiz, keyin xizmatda == chiptangiz bo'lguncha
 *                       kutasiz. Adolatli: kim oldin kelsa, o'sha oldin kiradi.
 *    ticket_unlock(l) - xizmatda++ (RELEASE).
 *
 *  ATOMIK AMALLAR (GCC builtin'lari, 15-bob):
 *    __atomic_exchange_n(&x, yangi, __ATOMIC_ACQUIRE)   - yozib, eski qiymatni qaytaradi
 *    __atomic_fetch_add(&x, 1, __ATOMIC_RELAXED)         - qo'shib, eski qiymatni qaytaradi
 *    __atomic_load_n(&x, __ATOMIC_ACQUIRE)
 *    __atomic_store_n(&x, 0, __ATOMIC_RELEASE)
 *    __builtin_ia32_pause()                              - "aylanyapman" ishorasi (x86 pause)
 *
 *  NEGA:
 *    Oddiy `while (band) ; band = 1;` ISHLAMAYDI: ikki oqim bir vaqtda band == 0 ni
 *    ko'rib, ikkalasi ham kiradi. Tekshirish va o'rnatish BITTA bo'linmas amal
 *    bo'lishi kerak - buni faqat CPU'ning atomik buyruqlari (xchg, lock xadd) beradi.
 *    Linux 2008-yilda oddiy spinlock'dan ticket lock'ga o'tgan (adolat uchun), keyinroq
 *    esa qspinlock'ga. MyOS: kernel/lib/spinlock.c (spinlock lab'i).
 *
 *  MASLAHAT:
 *    * "Test-and-test-and-set": band bo'lsa, exchange qilib turmang - avval ODDIY o'qish
 *      bilan bo'shashini kuting, keyin exchange. Kesh qatori CPU'lar orasida "sakramaydi".
 *    * Test har bir CPU yadrosi uchun bitta oqim bilan himoyasiz hisoblagichni oshiradi:
 *      qulf noto'g'ri bo'lsa, natija kam chiqadi.
 *    * User rejimida oqim istalgan paytda to'xtatilishi mumkin (yadroda spinlock
 *      ushlagan kod to'xtatilmaydi). Kutish juda cho'zilsa (masalan, 10000 marta
 *      aylangandan keyin) sched_yield() chaqirish mumkin - <sched.h>.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 34
 * ============================================================================= */
#include "mashq.h"

void spin_lock(spinlock_t *l)
{
    /* TODO */
    (void)l;
}

void spin_unlock(spinlock_t *l)
{
    /* TODO */
    (void)l;
}

bool spin_trylock(spinlock_t *l)
{
    /* TODO */
    (void)l;
    return false;
}

void ticket_lock(ticketlock_t *l)
{
    /* TODO */
    (void)l;
}

void ticket_unlock(ticketlock_t *l)
{
    /* TODO */
    (void)l;
}
