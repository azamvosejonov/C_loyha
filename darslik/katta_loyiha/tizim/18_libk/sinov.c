/* sinov.c - libk ni (hosted rejimda) libc bilan solishtirib sinaydi: bir xil kirishda bir xil natija */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libk.h"

static unsigned long holat = 1;

static unsigned tasodif(void)
{
    holat = holat * 6364136223846793005UL + 1442695040888963407UL;
    return (unsigned)(holat >> 33);
}

static int xatolar;

#define TEKSHIR(shart, xabar)                                                      \
    do {                                                                           \
        if (!(shart)) {                                                            \
            xatolar++;                                                             \
            if (xatolar <= 5)                                                      \
                printf("  XATO %s:%d: %s\n", __FILE__, __LINE__, xabar);           \
        }                                                                          \
    } while (0)

static void xotira_sinovi(void)
{
    uint8_t a[64], b[64], c[64];
    for (int t = 0; t < 20000; t++) {
        size_t n = tasodif() % 40;
        size_t i = tasodif() % 16, j = tasodif() % 16;         /* ustma-ust tushadigan siljishlar */
        for (int k = 0; k < 64; k++)
            a[k] = b[k] = (uint8_t)tasodif();
        memmove(a + i, a + j, n);
        kmemmove(b + i, b + j, n);
        TEKSHIR(memcmp(a, b, 64) == 0, "kmemmove libc memmove dan farq qiladi");

        int bayt = (int)(tasodif() & 0xFF);
        memset(a, bayt, n);
        kmemset(b, bayt, n);
        TEKSHIR(memcmp(a, b, n) == 0, "kmemset");
        memcpy(c, a, n);
        kmemcpy(a, c, n);
        TEKSHIR(kmemcmp(a, b, n) == 0 && (memcmp(a, b, n) == 0), "kmemcmp/kmemcpy");
    }
}

static void satr_sinovi(void)
{
    char x[32], y[32];
    for (int t = 0; t < 20000; t++) {
        size_t n = tasodif() % 20;
        for (size_t k = 0; k < n; k++)
            x[k] = (char)('a' + tasodif() % 3);                 /* kichik alifbo: teng satrlar ko'p uchraydi */
        x[n] = '\0';
        size_t m = tasodif() % 20;
        for (size_t k = 0; k < m; k++)
            y[k] = (char)('a' + tasodif() % 3);
        y[m] = '\0';
        TEKSHIR(kstrlen(x) == strlen(x), "kstrlen");
        int c1 = strcmp(x, y), c2 = kstrcmp(x, y);
        TEKSHIR((c1 < 0) == (c2 < 0) && (c1 > 0) == (c2 > 0), "kstrcmp ishorasi");

        char d1[8], d2[8];
        size_t u1 = strlen(x);
        size_t u2 = kstrlcpy(d1, x, sizeof(d1));
        size_t k = u1 < sizeof(d2) - 1 ? u1 : sizeof(d2) - 1;   /* etalon: qo'lda chegarali nusxa */
        memcpy(d2, x, k);
        d2[k] = '\0';
        TEKSHIR(u1 == u2 && strcmp(d1, d2) == 0, "kstrlcpy");
    }
}

static void son_sinovi(void)
{
    char b[65], s[65];
    for (int t = 0; t < 20000; t++) {
        uint64_t v = ((uint64_t)tasodif() << 32) | tasodif();
        v >>= tasodif() % 64;                                   /* har xil uzunlikdagi sonlar */
        kutoa(v, b, 10);
        snprintf(s, sizeof(s), "%llu", (unsigned long long)v);
        TEKSHIR(strcmp(b, s) == 0, "kutoa o'nlik");
        kutoa(v, b, 16);
        snprintf(s, sizeof(s), "%llx", (unsigned long long)v);
        TEKSHIR(strcmp(b, s) == 0, "kutoa hex");
    }
    char ikki[65];
    kutoa(11, ikki, 2);
    TEKSHIR(strcmp(ikki, "1011") == 0, "kutoa ikkilik");
}

static void bitmap_sinovi(void)
{
    uint8_t bm[16] = { 0 };                                    /* 128 bit */
    TEKSHIR(kbit_bosh_top(bm, 128) == 0, "bo'sh bitmapda birinchi nol 0 da");
    for (int i = 0; i < 20; i++)
        kbit_yoq(bm, (size_t)i);
    TEKSHIR(kbit_bosh_top(bm, 128) == 20, "20 ta band: birinchi bo'sh 20");
    kbit_och(bm, 7);
    TEKSHIR(kbit_bosh_top(bm, 128) == 7, "7-bit bo'shatildi");
    TEKSHIR(kbit_bor(bm, 3) == 1 && kbit_bor(bm, 7) == 0, "kbit_bor");
    for (int i = 0; i < 128; i++)
        kbit_yoq(bm, (size_t)i);
    TEKSHIR(kbit_bosh_top(bm, 128) == -1, "to'la bitmap: -1");
}

static void halqa_sinovi(void)
{
    uint8_t xotira[8];
    struct kring r;
    kring_boshla(&r, xotira, 8);
    unsigned yozilgan = 0, oqilgan = 0;
    uint8_t kutilgan = 0, yoz = 0;
    for (int t = 0; t < 100000; t++) {
        if (tasodif() % 2) {
            if (kring_yoz(&r, yoz) == 0) {
                yoz++;
                yozilgan++;
            }
        } else {
            uint8_t b;
            if (kring_oqi(&r, &b) == 0) {
                TEKSHIR(b == kutilgan, "halqa: FIFO tartibi buzildi");
                kutilgan++;
                oqilgan++;
            }
        }
        TEKSHIR(kring_band(&r) == yozilgan - oqilgan && kring_band(&r) <= 8, "halqa: band soni");
    }
}

int main(void)
{
    xotira_sinovi();
    satr_sinovi();
    son_sinovi();
    bitmap_sinovi();
    halqa_sinovi();
    printf("libk sinovi: %s\n", xatolar ? "XATO BOR" : "hammasi to'g'ri (5 ta guruh, ~200000 tekshiruv)");
    return xatolar != 0;
}
