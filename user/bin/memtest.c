/* =============================================================================
 *  user/bin/memtest.c - user malloc/free va sbrk stress testi
 * =============================================================================
 *
 *  1) Ko'p tasodifiy o'lchamdagi bloklar ajratiladi, har biriga naqsh yoziladi.
 *  2) Naqshlar tekshiriladi (bloklar bir-birining ustiga tushmaganmi?).
 *  3) Yarmi bo'shatilib, qayta ajratiladi (fragmentatsiya va split/coalesce).
 *  4) Hammasi bo'shatiladi: bo'sh bloklar BITTAGA birlashishi va heap
 *     yadroga qaytarilishi (trim) kerak.
 *  5) Katta blok (1 MB) - sbrk orqali heap o'sishi.
 * ============================================================================= */
#include "ulib.h"

#define N 300

static uint32_t seed = 42;
static uint32_t rnd(void)
{
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}

static void *ptrs[N];
static size_t sizes[N];
static int failures;

static void check(int ok, const char *what)
{
    if (!ok) {
        printf("  [FAIL] %s\n", what);
        failures++;
    }
}

static void fill(int i)
{
    memset(ptrs[i], (unsigned char)(i * 7 + 1), sizes[i]);
}

static int verify(int i)
{
    unsigned char *p = ptrs[i];
    for (size_t j = 0; j < sizes[i]; j++)
        if (p[j] != (unsigned char)(i * 7 + 1))
            return 0;
    return 1;
}

static void print_stats(const char *when)
{
    struct malloc_stats s;
    malloc_get_stats(&s);
    printf("  %-22s heap=%6lu band=%6lu (%3lu blok) bo'sh=%6lu (%lu blok)\n", when,
           s.heap_bytes, s.used_bytes, s.used_blocks, s.free_bytes, s.free_blocks);
}

int main(void)
{
    printf("memtest: user heap stress testi (%d blok)\n", N);
    print_stats("boshida:");

    for (int i = 0; i < N; i++) {
        sizes[i] = 1 + rnd() % 2000;
        ptrs[i] = malloc(sizes[i]);
        check(ptrs[i] != NULL, "malloc NULL qaytardi");
        check(((uintptr_t)ptrs[i] & 15) == 0, "16 ga tekislanmagan");
        fill(i);
    }
    print_stats("ajratilgandan keyin:");

    int ok = 1;
    for (int i = 0; i < N; i++)
        ok &= verify(i);
    check(ok, "naqsh buzilgan - bloklar ustma-ust tushgan!");

    for (int i = 0; i < N; i += 2) {        /* juftlarini bo'shatamiz -> "teshiklar" */
        free(ptrs[i]);
        ptrs[i] = NULL;
    }
    print_stats("yarmi bo'shatilgach:");

    for (int i = 0; i < N; i += 2) {        /* teshiklarni qayta to'ldiramiz */
        sizes[i] = 1 + rnd() % 1000;
        ptrs[i] = malloc(sizes[i]);
        check(ptrs[i] != NULL, "qayta malloc NULL");
        fill(i);
    }
    ok = 1;
    for (int i = 0; i < N; i++)
        ok &= verify(i);
    check(ok, "qayta ajratishdan keyin naqsh buzilgan");

    for (int i = 0; i < N; i++)
        free(ptrs[i]);
    print_stats("hammasi bo'shatilgach:");

    struct malloc_stats s;
    malloc_get_stats(&s);
    check(s.used_blocks == 0 && s.used_bytes == 0, "band bloklar qolib ketdi (leak)");
    check(s.free_blocks == 1, "bo'sh bloklar birlashmadi (coalesce ishlamadi)");

    /* Katta blok: heap sbrk orqali o'sadi, bo'shatilgach yana kichrayadi. */
    char *big = malloc(1024 * 1024);
    check(big != NULL, "1 MB ajratib bo'lmadi");
    if (big) {
        memset(big, 0x5A, 1024 * 1024);
        check(big[0] == 0x5A && big[1024 * 1024 - 1] == 0x5A, "katta blok buzilgan");
        print_stats("1 MB ajratilgach:");
        free(big);
        print_stats("1 MB bo'shatilgach:");
    }

    /* calloc to'lib ketishni ushlashi kerak. */
    check(calloc((size_t)1 << 62, 16) == NULL, "calloc overflow tekshiruvi ishlamadi");

    if (failures == 0)
        printf("memtest: PASSED\n");
    else
        printf("memtest: %d ta FAIL\n", failures);
    return failures ? 1 : 0;
}
