/* =============================================================================
 *  lib/spinlock.h - SPINLOCK: ko'p yadroli tizimda eng oddiy qulf
 * =============================================================================
 *
 *  MUAMMO: 2 ta CPU bir vaqtda bitta ro'yxatni o'zgartirsa - ro'yxat buziladi.
 *  Bitta CPU'da uzilishlarni o'chirish yetardi. Ko'p CPU'da esa boshqa CPU
 *  uzilishsiz ham PARALLEL ishlaydi - haqiqiy QULF kerak.
 *
 *  SPINLOCK: "qulf bo'sh bo'lguncha aylanib (spin) kutaman".
 *    lock:   while (atomic_xchg(&locked, 1) == 1) kut;
 *    unlock: locked = 0;
 *  xchg - ATOMAR instruksiya: "eski qiymatni o'qib, yangisini yozish" bitta
 *  bo'linmas amal. Ikki CPU bir vaqtda xchg qilsa, faqat bittasi 0 ni ko'radi.
 *
 *  NEGA QULF USHLANGANDA UZILISHLAR O'CHIQ:
 *    CPU qulfni ushlab turganda uzilish kelsa va handler ham shu qulfni olmoqchi
 *    bo'lsa - handler abadiy aylanadi (qulf egasi handler tugashini kutyapti) =
 *    DEADLOCK. Shuning uchun spin_lock() push_off() qiladi (xv6 uslubi).
 *
 *  QOIDALAR:
 *    1. Spinlock ichida UXLASH (sleep) taqiqlanadi (proc_sleep bundan mustasno -
 *       u qulfni o'zi to'g'ri qo'yib yuboradi).
 *    2. Qulflar DOIM BIR XIL TARTIBDA olinadi, aks holda: CPU0 A ni olib B ni
 *       kutadi, CPU1 B ni olib A ni kutadi = deadlock. Tartib docs/10-smp.md da.
 *    3. Kod qisqa bo'lsin - boshqa CPU'lar shu vaqt bekorga aylanyapti.
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "arch/cpu.h"
#include "arch/percpu.h"

typedef struct {
    volatile uint32_t locked;
    const char *name;                   /* debug uchun */
    struct cpu *owner;                  /* qaysi CPU ushlab turibdi (deadlock aniqlash) */
} spinlock_t;

#define SPINLOCK_INIT(n) { 0, n, 0 }

void spin_lock(spinlock_t *l);
void spin_unlock(spinlock_t *l);
/* Joriy CPU shu qulfni ushlab turibdimi? */
bool spin_holding(spinlock_t *l);
/* Panic paytida: qulflarni e'tiborsiz qoldirish (aks holda panic ham qotadi). */
extern volatile bool spinlocks_busted;

/* Eski API bilan moslik: flags qaytarilmaydi - push_off o'zi eslab qoladi. */
static inline uint64_t spin_lock_irqsave(spinlock_t *l)
{
    spin_lock(l);
    return 0;
}

static inline void spin_unlock_irqrestore(spinlock_t *l, uint64_t flags)
{
    (void)flags;
    spin_unlock(l);
}
