#include <unistd.h>

#include "test.h"
#include "mashq.h"

static void yoz(const char *yol, const char *d, size_t n)
{
    FILE *f = fopen(yol, "wb");
    fwrite(d, 1, n, f);
    fclose(f);
}

#define SANA(matn, q, s, b)                                             \
    do {                                                                \
        yoz("t26.txt", matn, sizeof(matn) - 1);                         \
        struct wc w_ = { -1, -1, -1 };                                  \
        CHECK_INT(sana("t26.txt", &w_), 0);                             \
        CHECK_INT(w_.qatorlar, q);                                      \
        CHECK_INT(w_.sozlar, s);                                        \
        CHECK_INT(w_.baytlar, b);                                       \
    } while (0)

int main(void)
{
    TEST_BOSHLA();
    BOLIM("kichik fayllar");
    SANA("", 0, 0, 0);
    SANA("salom\n", 1, 1, 6);
    SANA("salom dunyo", 0, 2, 11);
    SANA("  bir\t ikki\n\nuch  \n", 3, 3, 19);
    SANA("\n\n\n", 3, 0, 3);

    BOLIM("so'z 4096 bayt chegarasida");
    size_t n = 10000;
    char *d = malloc(n);
    memset(d, ' ', n);
    memcpy(d + 4093, "SALOM", 5);           /* 4093..4097 - chegaradan o'tadi */
    memcpy(d + 8190, "AB\nCD", 5);
    d[n - 1] = '\n';
    yoz("t26.txt", d, n);
    struct wc w = { -1, -1, -1 };
    CHECK_INT(sana("t26.txt", &w), 0);
    CHECK_INT(w.qatorlar, 2);
    CHECK_INT(w.sozlar, 3);
    CHECK_INT(w.baytlar, 10000);
    free(d);

    BOLIM("fayl yo'q");
    CHECK_INT(sana("t26_yoq.txt", &w), -1);
    unlink("t26.txt");
    TEST_TUGADI();
}
