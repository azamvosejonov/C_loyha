/* =============================================================================
 *  44 - Semafor                                          [7-modul: nazariya amalda]
 * =============================================================================
 *
 *  VAZIFA (darslik 26-bob):
 *    Hisoblovchi semaforni pthread mutex + shart o'zgaruvchisi bilan yozing
 *    (<semaphore.h> dagi tayyor sem_* funksiyalarini ISHLATMANG):
 *      semafor_yarat(n)  - qiymati n (>= 0)
 *      semafor_kut(s)    - qiymat 0 bo'lsa - uxlab kutish; keyin qiymat--
 *      semafor_ber(s)    - qiymat++; kutayotgan bo'lsa - bittasini uyg'otish
 *      semafor_urin(s)   - qiymat > 0 bo'lsa: qiymat--, 1; aks holda darhol 0
 *      semafor_yoq(s)
 *
 *  UCH XIL ISHLATILISHI (test uchalasini ham tekshiradi):
 *    * n = 1  -> qulf (o'zaro istisno)
 *    * n = 0  -> tartiblash: "boshqa oqim ber() qilmaguncha kut"
 *    * n = N  -> N ta resurs: bir vaqtda ko'pi bilan N ta oqim kiradi
 *
 *  ESLATMA: 38-mashqdagi qoidalar shu yerda ham - shartni mutex ostida tekshirish va
 *  `while` bilan kutish.
 *
 *  NEGA:
 *    Dijkstra'ning (1965) semafori - sinxronlashning eng qadimgi va universal vositasi.
 *    Linux yadrosida `struct semaphore` (kernel/locking/semaphore.c) aynan shunday: hisoblagich
 *    + spinlock + kutish navbati.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 44
 * ============================================================================= */
#include <pthread.h>
#include <stdlib.h>

#include "mashq.h"

struct semafor {
    int hali_bosh;      /* TODO: qiymat, mutex, shart o'zgaruvchisi */
};

struct semafor *semafor_yarat(int boshlangich)
{
    /* TODO */
    (void)boshlangich;
    return NULL;
}

void semafor_kut(struct semafor *s)
{
    /* TODO */
    (void)s;
}

void semafor_ber(struct semafor *s)
{
    /* TODO */
    (void)s;
}

int semafor_urin(struct semafor *s)
{
    /* TODO */
    (void)s;
    return 0;
}

void semafor_yoq(struct semafor *s)
{
    /* TODO */
    (void)s;
}
