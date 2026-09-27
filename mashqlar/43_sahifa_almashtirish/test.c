#include "test.h"
#include "mashq.h"

#define N(a) (sizeof(a) / sizeof((a)[0]))

int main(void)
{
    TEST_BOSHLA();
    /* Darsliklardagi klassik ketma-ketlik. */
    int a[] = { 7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1 };
    BOLIM("klassik ketma-ketlik, 3 freym");
    CHECK_INT(fifo_xatolar(a, N(a), 3), 15);
    CHECK_INT(lru_xatolar(a, N(a), 3), 12);
    CHECK_INT(opt_xatolar(a, N(a), 3), 9);
    CHECK_INT(clock_xatolar(a, N(a), 3), 14);

    BOLIM("Belady anomaliyasi (FIFO)");
    int b[] = { 1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5 };
    CHECK_INT(fifo_xatolar(b, N(b), 3), 9);
    CHECK_INT(fifo_xatolar(b, N(b), 4), 10);            /* ko'proq freym - ko'proq xato! */
    CHECK_INT(lru_xatolar(b, N(b), 3), 10);
    CHECK_INT(lru_xatolar(b, N(b), 4), 8);              /* LRU'da anomaliya yo'q */

    BOLIM("chegaraviy holatlar");
    int c[] = { 5, 5, 5, 5 };
    CHECK_INT(fifo_xatolar(c, N(c), 1), 1);
    CHECK_INT(lru_xatolar(c, N(c), 1), 1);
    CHECK_INT(opt_xatolar(c, N(c), 1), 1);
    CHECK_INT(clock_xatolar(c, N(c), 1), 1);
    int d[] = { 1, 2, 1, 2, 1, 2 };
    CHECK_INT(fifo_xatolar(d, N(d), 1), 6);
    CHECK_INT(clock_xatolar(d, N(d), 2), 2);
    CHECK_INT(opt_xatolar(a, N(a), 10), 6);             /* hammasi sig'adi - faqat birinchi murojaatlar */
    CHECK_INT(lru_xatolar(a, 0, 3), 0);

    BOLIM("katta ketma-ketlik: OPT <= LRU, OPT <= FIFO, OPT <= Clock");
    static int e[20000];
    unsigned long rng = 11;
    for (int i = 0; i < 20000; i++) {
        rng = rng * 6364136223846793005UL + 1442695040888963407UL;
        int bazasi = (i / 500) * 7;                     /* ishchi to'plam vaqt o'tishi bilan siljiydi */
        e[i] = bazasi + (int)((rng >> 33) % 12);
    }
    int f = fifo_xatolar(e, 20000, 8), l = lru_xatolar(e, 20000, 8);
    int o = opt_xatolar(e, 20000, 8), k = clock_xatolar(e, 20000, 8);
    printf("   (FIFO %d, LRU %d, Clock %d, OPT %d)\n", f, l, k, o);
    CHECK(o > 0 && o <= l && o <= f && o <= k);
    TEST_TUGADI();
}
