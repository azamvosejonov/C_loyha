/* =============================================================================
 *  28_algoritmlar.c - ikkilik qidiruv, heapsort, murakkablikni o'lchash  (28-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -O2 28_algoritmlar.c -o algo && ./algo
 *
 *  Kutilgan natija (vaqtlar kompyuterga qarab):
 *      heapsort 1000000 ta son: saralandi, 0.1 s
 *      chiziqli qidiruv x 2000: 1.0 s    <- O(n) har biri
 *      ikkilik qidiruv  x 2000: 0.0001 s <- O(log n) har biri: ~20 qadam
 *      topildi: ikkalasida ham bir xil (2000 tadan 2000)
 *
 *  Sinab ko'ring:
 *      1) n ni 20000 qilib, pufakcha saralash (bubble sort) yozing va vaqtini o'lchang. Keyin
 *         n = 40000: vaqt ~4 barobar o'sadi (O(n^2)). heapsort esa ~2 barobardan sal ko'proq.
 *      2) heapsort ni qsort() bilan solishtiring (12-bob).
 *      3) ikkilik() ga saralanMAGAN massiv bering (heapsort dan oldin). Nima bo'ladi?
 *      4) `orta = (chap + ong) / 2` qachon xato beradi? (int bo'lsa - 2^30 dan katta massivda.)
 * ============================================================================= */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double hozir(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

static void chokt(int *a, size_t n, size_t i)   /* max-heap: pastga cho'ktirish */
{
    for (;;) {
        size_t l = 2 * i + 1, r = l + 1, m = i;
        if (l < n && a[l] > a[m]) m = l;
        if (r < n && a[r] > a[m]) m = r;
        if (m == i) return;
        int t = a[i]; a[i] = a[m]; a[m] = t;
        i = m;
    }
}

static void heapsort(int *a, size_t n)          /* Linux lib/sort.c ham heapsort */
{
    for (size_t i = n / 2; i-- > 0;)
        chokt(a, n, i);                         /* O(n) da heap qurish */
    for (size_t k = n; k-- > 1;) {
        int t = a[0]; a[0] = a[k]; a[k] = t;    /* eng kattasi oxiriga */
        chokt(a, k, 0);
    }
}

static long ikkilik(const int *a, size_t n, int x)
{
    size_t chap = 0, ong = n;
    while (chap < ong) {
        size_t orta = chap + (ong - chap) / 2;  /* (chap + ong) / 2 toshishi mumkin */
        if (a[orta] < x) chap = orta + 1;
        else if (a[orta] > x) ong = orta;
        else return (long)orta;
    }
    return -1;
}

static long chiziqli(const int *a, size_t n, int x)
{
    for (size_t i = 0; i < n; i++)
        if (a[i] == x) return (long)i;
    return -1;
}

int main(void)
{
    size_t n = 1000000;
    int *a = malloc(n * sizeof(int));
    srand(1);
    for (size_t i = 0; i < n; i++)
        a[i] = rand();
    double t0 = hozir();
    heapsort(a, n);
    double ts = hozir() - t0;
    int ok = 1;
    for (size_t i = 1; i < n; i++)
        ok &= a[i - 1] <= a[i];
    printf("heapsort %zu ta son: %s, %.2f s\n", n, ok ? "saralandi" : "XATO", ts);

    int t1 = 0, t2 = 0;
    t0 = hozir();
    for (int k = 0; k < 2000; k++)
        t1 += chiziqli(a, n, a[(size_t)k * 499]) >= 0;
    double tc = hozir() - t0;
    t0 = hozir();
    for (int k = 0; k < 2000; k++)
        t2 += ikkilik(a, n, a[(size_t)k * 499]) >= 0;
    double tb = hozir() - t0;
    printf("chiziqli qidiruv x 2000: %.4f s    <- O(n) har biri\n", tc);
    printf("ikkilik qidiruv  x 2000: %.4f s <- O(log n) har biri: ~20 qadam\n", tb);
    printf("topildi: %s (2000 tadan %d)\n", t1 == t2 ? "ikkalasida ham bir xil" : "FARQ!", t2);
    free(a);
    return 0;
}
