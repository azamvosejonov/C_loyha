/* =============================================================================
 *  lib/mutex.h - MUTEX: uxlaydigan qulf
 * =============================================================================
 *
 *  SPINLOCK vs MUTEX:
 *    spinlock - band bo'lsa AYLANIB kutadi (CPU band). Faqat juda QISQA
 *               bo'limlar uchun va uxlash taqiqlangan joylarda (uzilishlar).
 *    mutex    - band bo'lsa jarayon UXLAYDI, CPU boshqalarga beriladi. Uzoq
 *               davom etishi mumkin bo'lgan ishlar uchun: diskdan o'qish,
 *               fayl tizimi operatsiyalari. Ichida uxlash MUMKIN.
 *
 *  Mutex'ni uzilish handleri ichida yoki spinlock ushlab turib olish TAQIQLANADI
 *  (uxlab bo'lmaydigan joyda uxlash = deadlock yoki panic).
 * ============================================================================= */
#pragma once

#include <stdbool.h>

#include "lib/spinlock.h"

struct process;

struct mutex {
    spinlock_t lock;                    /* mutex'ning o'z holatini himoya qiladi */
    bool locked;
    struct process *owner;
    const char *name;
};

#define MUTEX_INIT(n) { SPINLOCK_INIT(n), false, 0, n }

void mutex_init(struct mutex *m, const char *name);
void mutex_lock(struct mutex *m);
void mutex_unlock(struct mutex *m);
bool mutex_held(struct mutex *m);       /* joriy jarayon ushlab turibdimi */
