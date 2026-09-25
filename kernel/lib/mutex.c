/* =============================================================================
 *  lib/mutex.c - mutex amalga oshirilishi (proc_sleep ustiga qurilgan)
 * ============================================================================= */
#include "lib/mutex.h"

#include "lib/panic.h"
#include "proc/process.h"

void mutex_init(struct mutex *m, const char *name)
{
    m->lock = (spinlock_t)SPINLOCK_INIT(name);
    m->locked = false;
    m->owner = 0;
    m->name = name;
}

void mutex_lock(struct mutex *m)
{
    if (this_cpu()->ncli > 0)
        panic("mutex_lock(%s): spinlock ushlab turib yoki uzilishda uxlash mumkin emas", m->name);
    spin_lock(&m->lock);
    if (m->locked && m->owner == current)
        panic("mutex_lock(%s): jarayon o'zi ushlab turgan mutex'ni qayta olmoqchi (deadlock)",
              m->name);
    while (m->locked)
        proc_sleep(m, &m->lock);        /* bo'shaguncha uxlaymiz */
    m->locked = true;
    m->owner = current;
    spin_unlock(&m->lock);
}

void mutex_unlock(struct mutex *m)
{
    spin_lock(&m->lock);
    if (!m->locked || m->owner != current)
        panic("mutex_unlock(%s): bu jarayon ushlamagan", m->name);
    m->locked = false;
    m->owner = 0;
    proc_wakeup(m);                     /* kutayotganlardan birortasi oladi */
    spin_unlock(&m->lock);
}

bool mutex_held(struct mutex *m)
{
    return m->locked && m->owner == current;
}
