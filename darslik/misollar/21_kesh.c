/* =============================================================================
 *  21_kesh.c - kesh ta'siri: qatorma-qator va ustunma-ustun, soxta bo'lishish (21-bob)
 * =============================================================================
 *  Ishga tushirish (optimallashtirish bilan!):
 *      gcc -Wall -Wextra -O2 -pthread 21_kesh.c -o kesh && ./kesh
 *
 *  Kutilgan natija (vaqtlar kompyuterga qarab; nisbatiga qarang):
 *      qatorma-qator:   0.02 s
 *      ustunma-ustun:   0.20 s   <- bir xil ish, ~5-10 marta sekin!
 *      soxta bo'lishish (bitta kesh qatori):   0.30 s
 *      alohida kesh qatorlari:                  0.05 s
 *
 *  Sinab ko'ring: perf stat -e cache-misses,cache-references ./kesh   (29-bob)
 * ============================================================================= */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 4096
#define OQIMLAR 4
#define MARTA 50000000L

static double hozir(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

struct birga { volatile long v; };                              /* 8 bayt - 4 tasi bitta 64 baytli qatorda */
struct alohida { volatile long v; } __attribute__((aligned(64))); /* har biri o'z qatorida */
static struct birga b[OQIMLAR];
static struct alohida al[OQIMLAR];

static void *birga_ish(void *arg)
{
    long i = (long)arg;
    for (long k = 0; k < MARTA; k++)
        b[i].v++;
    return NULL;
}

static void *alohida_ish(void *arg)
{
    long i = (long)arg;
    for (long k = 0; k < MARTA; k++)
        al[i].v++;
    return NULL;
}

static double yugur(void *(*f)(void *))
{
    pthread_t t[OQIMLAR];
    double t0 = hozir();
    for (long i = 0; i < OQIMLAR; i++)
        pthread_create(&t[i], NULL, f, (void *)i);
    for (int i = 0; i < OQIMLAR; i++)
        pthread_join(t[i], NULL);
    return hozir() - t0;
}

int main(void)
{
    int (*m)[N] = malloc(sizeof(int[N][N]));    /* 64 MB matritsa */
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            m[i][j] = i ^ j;

    volatile long s = 0;
    double t0 = hozir();
    for (int i = 0; i < N; i++)                 /* ichki sikl - oxirgi indeks: xotirada ketma-ket */
        for (int j = 0; j < N; j++)
            s += m[i][j];
    double qator = hozir() - t0;

    t0 = hozir();
    for (int j = 0; j < N; j++)                 /* har bir murojaat - yangi kesh qatori */
        for (int i = 0; i < N; i++)
            s += m[i][j];
    double ustun = hozir() - t0;
    printf("qatorma-qator:   %.2f s\nustunma-ustun:   %.2f s   <- bir xil ish!\n", qator, ustun);
    free(m);

    printf("soxta bo'lishish (bitta kesh qatori):   %.2f s\n", yugur(birga_ish));
    printf("alohida kesh qatorlari:                  %.2f s\n", yugur(alohida_ish));
    return 0;
}
