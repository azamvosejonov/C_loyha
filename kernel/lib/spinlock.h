/* =============================================================================
 *  lib/spinlock.h - SPINLOCK: ko'p yadroli tizimda eng oddiy qulf
 * =============================================================================
 *
 *  MUAMMO: 2 ta CPU bir vaqtda bitta ro'yxatni o'zgartirsa - ro'yxat buziladi.
 *  Bitta CPU'da uzilishlarni o'chirish yetardi (6-bosqich). Ko'p CPU'da esa
 *  boshqa CPU uzilishsiz ham parallel ishlaydi - haqiqiy QULF kerak.
 *
 *  SPINLOCK: "qulf bo'sh bo'lguncha aylanib (spin) kutaman".
 *    lock:   while (atomic_xchg(&locked, 1) == 1) kut;
 *    unlock: locked = 0;
 *  xchg - ATOMAR instruksiya: "eski qiymatni o'qib, yangisini yozish" bitta
 *  bo'linmas amal. Ikki CPU bir vaqtda xchg qilsa, faqat bittasi 0 ni ko'radi.
 *
 *  NEGA UZILISHLARNI HAM O'CHIRAMIZ (spin_lock_irqsave):
 *    CPU qulfni ushlab turganda uzilish kelsa va handler ham shu qulfni olmoqchi
 *    bo'lsa - handler abadiy aylanadi (qulf egasi handler tugashini kutyapti) =
 *    DEADLOCK. Shuning uchun qulf ushlanganda o'sha CPU'da uzilishlar o'chiq.
 *
 *  QOIDA: spinlock ichida UXLASH (schedule) taqiqlanadi va kod QISQA bo'lishi
 *  kerak - boshqa CPU'lar shu vaqt davomida bekorga aylanyapti.
 * ============================================================================= */
#pragma once

#include <stdint.h>

#include "arch/cpu.h"

typedef struct {
    volatile uint32_t locked;
    const char *name;                   /* debug uchun */
} spinlock_t;

#define SPINLOCK_INIT(n) { 0, n }

static inline void spin_lock(spinlock_t *l)
{
    for (;;) {
        /* __atomic_exchange_n -> `xchg` instruksiyasi (LOCK prefiksi o'zida bor).
         * ACQUIRE tartibi: qulfdan keyingi o'qishlar undan OLDIN bajarilib
         * ketmasligi kafolatlanadi. */
        if (__atomic_exchange_n(&l->locked, 1, __ATOMIC_ACQUIRE) == 0)
            return;
        /* "Test-and-test-and-set": qulf bo'shaguncha faqat O'QIYMIZ. Yozish
         * kesh qatorini boshqa CPU'lardan tortib oladi (kesh "ping-pong") -
         * o'qish esa har bir CPU'ning o'z keshida bo'ladi. */
        while (__atomic_load_n(&l->locked, __ATOMIC_RELAXED))
            cpu_pause();
    }
}

static inline void spin_unlock(spinlock_t *l)
{
    /* RELEASE: qulf ichidagi barcha yozuvlar boshqa CPU'larga qulf
     * bo'shashidan OLDIN ko'rinadi. */
    __atomic_store_n(&l->locked, 0, __ATOMIC_RELEASE);
}

static inline uint64_t spin_lock_irqsave(spinlock_t *l)
{
    uint64_t flags = irq_save();
    spin_lock(l);
    return flags;
}

static inline void spin_unlock_irqrestore(spinlock_t *l, uint64_t flags)
{
    spin_unlock(l);
    irq_restore(flags);
}

static inline int spin_is_locked(spinlock_t *l)
{
    return __atomic_load_n(&l->locked, __ATOMIC_RELAXED) != 0;
}
