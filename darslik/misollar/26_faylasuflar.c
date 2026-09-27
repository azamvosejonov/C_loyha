/* =============================================================================
 *  26_faylasuflar.c - ovqatlanayotgan faylasuflar: deadlock va uning yechimi (26-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -O2 -pthread 26_faylasuflar.c -o faylasuf
 *      ./faylasuf togri      - vilkalar GLOBAL TARTIBDA olinadi: hammasi ovqatlanadi
 *      ./faylasuf deadlock   - hamma avval chap vilkani oladi: deyarli darhol QOTADI
 *                              (2 soniyadan keyin dastur o'zi aniqlab, chiqib ketadi)
 *
 *  Kutilgan natija:
 *      togri:    har bir faylasuf 1000 marta ovqatlandi. Deadlock yo'q.
 *      deadlock: DEADLOCK! 2 soniyada hech kim ovqatlana olmadi (aylanma kutish: 0->1->2->3->4->0)
 *
 *  Sinab ko'ring:
 *      1) usleep(100) ni o'chirib `./faylasuf deadlock` ni bir necha marta ishga tushiring.
 *         Deadlock kamroq bo'ladi, lekin yo'qolmaydi - poyga holati "kamdan-kam" degani "hech qachon" emas.
 *      2) togri rejimda faqat 4-faylasuf tartibni almashtiradi (chap = 4 > ong = 0). Nega bitta
 *         faylasufning o'zi aylanma kutishni buzishga yetadi?
 *      3) Boshqa yechim: ikkinchi vilkani pthread_mutex_trylock bilan oling; ololmasa -
 *         birinchisini qo'yib yuborib, qaytadan urining.
 *      4) Qotgan dasturni gdb bilan ko'ring: `gdb -p $(pidof faylasuf)`, keyin
 *         `thread apply all bt` - har bir oqim qaysi lock'da kutyapti?
 * ============================================================================= */
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define F 5
static pthread_mutex_t vilka[F];
static int tartibli;
static long ovqat[F];

static void *faylasuf(void *arg)
{
    int i = (int)(long)arg;
    int chap = i, ong = (i + 1) % F;
    int birinchi = chap, ikkinchi = ong;
    if (tartibli && birinchi > ikkinchi) {      /* 4-shartni buzish: kichik raqamlisini AVVAL */
        birinchi = ong;
        ikkinchi = chap;
    }
    for (int k = 0; k < 1000; k++) {
        pthread_mutex_lock(&vilka[birinchi]);
        usleep(100);                            /* deadlock ehtimolini oshirish */
        pthread_mutex_lock(&vilka[ikkinchi]);
        __atomic_add_fetch(&ovqat[i], 1, __ATOMIC_RELAXED);
        pthread_mutex_unlock(&vilka[ikkinchi]);
        pthread_mutex_unlock(&vilka[birinchi]);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    tartibli = argc < 2 || strcmp(argv[1], "deadlock") != 0;
    pthread_t t[F];
    for (int i = 0; i < F; i++)
        pthread_mutex_init(&vilka[i], NULL);
    for (long i = 0; i < F; i++)
        pthread_create(&t[i], NULL, faylasuf, (void *)i);

    long oldingi = -1;
    for (;;) {
        sleep(2);
        long jami = 0;
        for (int i = 0; i < F; i++)
            jami += __atomic_load_n(&ovqat[i], __ATOMIC_RELAXED);
        if (jami == F * 1000L)
            break;
        if (jami == oldingi) {
            printf("DEADLOCK! 2 soniyada hech kim ovqatlana olmadi (aylanma kutish: 0->1->2->3->4->0)\n");
            return 1;                           /* qotgan oqimlar bilan chiqib ketamiz */
        }
        oldingi = jami;
    }
    for (int i = 0; i < F; i++)
        pthread_join(t[i], NULL);
    printf("har bir faylasuf 1000 marta ovqatlandi. Deadlock yo'q.\n");
    return 0;
}
