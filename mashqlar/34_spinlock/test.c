#include <pthread.h>
#include <unistd.h>

#include "test.h"
#include "mashq.h"

/* Oqimlar soni = CPU yadrolari soni (2..8). Yadrodan ko'p oqim bo'lsa, ticket lock
 * juda sekinlashadi: navbatdagi chipta egasi CPU'siz qolsa, hamma uni kutadi
 * (yadroda bu muammo yo'q - spinlock ushlagan kod to'xtatilmaydi). */
static int OQIMLAR;
#define HAR_BIRI 30000

static spinlock_t sl = SPINLOCK_INIT;
static ticketlock_t tl = TICKETLOCK_INIT;
static volatile long hisob;

static void *spin_ish(void *arg)
{
    (void)arg;
    for (int i = 0; i < HAR_BIRI; i++) {
        spin_lock(&sl);
        long v = hisob;             /* ataylab "o'qi - kut - yoz": poyga bo'lsa, albatta ko'rinadi */
        __builtin_ia32_pause();
        hisob = v + 1;
        spin_unlock(&sl);
    }
    return NULL;
}

static void *ticket_ish(void *arg)
{
    (void)arg;
    for (int i = 0; i < HAR_BIRI; i++) {
        ticket_lock(&tl);
        long v = hisob;
        __builtin_ia32_pause();
        hisob = v + 1;
        ticket_unlock(&tl);
    }
    return NULL;
}

static long yugur(void *(*f)(void *))
{
    pthread_t t[8];
    hisob = 0;
    for (int i = 0; i < OQIMLAR; i++)
        pthread_create(&t[i], NULL, f, NULL);
    for (int i = 0; i < OQIMLAR; i++)
        pthread_join(t[i], NULL);
    return hisob;
}

int main(void)
{
    TEST_BOSHLA();
    long cpu = sysconf(_SC_NPROCESSORS_ONLN);
    OQIMLAR = cpu < 2 ? 2 : cpu > 8 ? 8 : (int)cpu;
    BOLIM("spin_trylock (bitta oqim)");
    spinlock_t l = SPINLOCK_INIT;
    CHECK(spin_trylock(&l));
    CHECK(!spin_trylock(&l));               /* band */
    spin_unlock(&l);
    CHECK_INT(l.band, 0);
    CHECK(spin_trylock(&l));
    spin_unlock(&l);
    spin_lock(&l);
    CHECK_INT(l.band, 1);
    CHECK(!spin_trylock(&l));
    spin_unlock(&l);

    printf("   (oqimlar: %d)\n", OQIMLAR);
    BOLIM("spinlock: har bir oqim 30000 marta");
    CHECK_INT(yugur(spin_ish), (long)OQIMLAR * HAR_BIRI);

    BOLIM("ticket lock: chiptalar");
    ticketlock_t t = TICKETLOCK_INIT;
    ticket_lock(&t);
    CHECK(t.keyingi == 1 && t.xizmatda == 0);
    ticket_unlock(&t);
    CHECK(t.keyingi == 1 && t.xizmatda == 1);

    BOLIM("ticket lock: har bir oqim 30000 marta");
    CHECK_INT(yugur(ticket_ish), (long)OQIMLAR * HAR_BIRI);
    CHECK(tl.keyingi == tl.xizmatda && tl.keyingi == (unsigned)(OQIMLAR * HAR_BIRI));
    TEST_TUGADI();
}
