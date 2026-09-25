/* =============================================================================
 *  lib/spinlock.c - spinlock amalga oshirilishi
 * ============================================================================= */
#include "lib/spinlock.h"

#include "lib/panic.h"

volatile bool spinlocks_busted;

bool spin_holding(spinlock_t *l)
{
    return __atomic_load_n(&l->locked, __ATOMIC_RELAXED) && l->owner == this_cpu();
}

void spin_lock(spinlock_t *l)
{
    push_off();
    if (spinlocks_busted)
        return;
    /* O'zimiz ushlab turgan qulfni qayta olish = abadiy kutish. Darhol aytamiz. */
    if (spin_holding(l))
        panic("spin_lock: '%s' qulfini shu CPU allaqachon ushlab turibdi (DEADLOCK)", l->name);
    uint64_t spins = 0;
    for (;;) {
        /* __atomic_exchange_n -> `xchg` (LOCK o'zida bor). ACQUIRE: qulfdan
         * keyingi xotira amallari undan OLDIN bajarilib ketmasligi kafolati. */
        if (__atomic_exchange_n(&l->locked, 1, __ATOMIC_ACQUIRE) == 0)
            break;
        /* Test-and-test-and-set: bo'shaguncha faqat O'QIYMIZ - kesh qatori
         * CPU'lar orasida "ping-pong" bo'lmaydi. pause - CPU'ga "spin tsikl". */
        while (__atomic_load_n(&l->locked, __ATOMIC_RELAXED)) {
            cpu_pause();
            if (++spins == 2000000000ULL)
                panic("spin_lock: '%s' qulfi juda uzoq band (ehtimol deadlock)", l->name);
        }
    }
    l->owner = this_cpu();
}

void spin_unlock(spinlock_t *l)
{
    if (!spinlocks_busted) {
        if (!spin_holding(l))
            panic("spin_unlock: '%s' qulfi bu CPU'da ushlanmagan", l->name);
        l->owner = 0;
        /* RELEASE: qulf ichidagi barcha yozuvlar boshqa CPU'larga qulf
         * bo'shashidan OLDIN ko'rinadi. */
        __atomic_store_n(&l->locked, 0, __ATOMIC_RELEASE);
    }
    pop_off();
}
