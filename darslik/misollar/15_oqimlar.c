/* =============================================================================
 *  15_oqimlar.c - poyga holati o'z ko'zingiz bilan: qulfsiz, mutex, atomik (15-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -O2 -pthread 15_oqimlar.c -o oqimlar && ./oqimlar
 *
 *  Kutilgan natija (birinchi raqam har safar boshqacha):
 *      qulfsiz:  1234567  (kutilgan 4000000 - oshirishlar YO'QOLDI)
 *      mutex:    4000000
 *      atomik:   4000000
 *
 *  Sinab ko'ring: gcc -g -fsanitize=thread -pthread 15_oqimlar.c -o t && ./t
 *                 - ThreadSanitizer "data race" ni aniq qatori bilan ko'rsatadi.
 * ============================================================================= */
#include <pthread.h>
#include <stdio.h>

#define OQIMLAR 4
#define MARTA 1000000

static volatile long qulfsiz_hisob;         /* volatile - kompilyator siklni qisqartirmasin (qulf EMAS!) */
static long mutex_hisob;
static long atomik_hisob;
static pthread_mutex_t qulf = PTHREAD_MUTEX_INITIALIZER;

static void *ish(void *arg)
{
    (void)arg;
    for (int i = 0; i < MARTA; i++) {
        qulfsiz_hisob++;                                    /* o'qish-oshirish-yozish: 3 amal */

        pthread_mutex_lock(&qulf);
        mutex_hisob++;                                      /* kritik seksiya */
        pthread_mutex_unlock(&qulf);

        __atomic_add_fetch(&atomik_hisob, 1, __ATOMIC_RELAXED);   /* bitta bo'linmas amal */
    }
    return NULL;
}

int main(void)
{
    pthread_t t[OQIMLAR];
    for (int i = 0; i < OQIMLAR; i++)
        pthread_create(&t[i], NULL, ish, NULL);
    for (int i = 0; i < OQIMLAR; i++)
        pthread_join(t[i], NULL);
    printf("qulfsiz:  %ld  (kutilgan %d)\n", qulfsiz_hisob, OQIMLAR * MARTA);
    printf("mutex:    %ld\n", mutex_hisob);
    printf("atomik:   %ld\n", atomik_hisob);
    return 0;
}
