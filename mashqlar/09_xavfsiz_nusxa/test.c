#include "test.h"
#include "mashq.h"

/* Bufer ichida '\0' bormi? Bo'lmasa, strcmp bufer tashqarisini o'qib ketadi -
 * shuning uchun avval shuni tekshiramiz. */
#define CHECK_BUF(b, hajm, kutilgan)                                        \
    do {                                                                    \
        if (memchr(b, '\0', hajm) == NULL) {                                \
            t_checks++;                                                     \
            t_fail++;                                                       \
            printf("  [XATO] %s:%d: bufer '\\0' bilan tugamagan\n", __FILE__, __LINE__); \
        } else {                                                            \
            CHECK_STR(b, kutilgan);                                         \
        }                                                                   \
    } while (0)

int main(void)
{
    TEST_BOSHLA();
    BOLIM("xavfsiz_nusxa: sig'adi");
    char *b = malloc(10);
    memset(b, 'X', 10);                /* ataylab \0 siz "axlat" */
    CHECK_INT(xavfsiz_nusxa(b, "salom", 10), 5);
    CHECK_BUF(b, 10, "salom");
    free(b);

    BOLIM("xavfsiz_nusxa: aniq sig'adi (hajm = uzunlik + 1)");
    b = malloc(6);
    CHECK_INT(xavfsiz_nusxa(b, "salom", 6), 5);
    CHECK_BUF(b, 6, "salom");
    free(b);

    BOLIM("xavfsiz_nusxa: sig'maydi - qisqartiriladi");
    b = malloc(4);
    CHECK_INT(xavfsiz_nusxa(b, "salom", 4), 5);
    CHECK_BUF(b, 4, "sal");
    free(b);
    b = malloc(1);
    CHECK_INT(xavfsiz_nusxa(b, "salom", 1), 5);
    CHECK_BUF(b, 1, "");
    free(b);
    b = malloc(1);
    b[0] = 'Z';
    CHECK_INT(xavfsiz_nusxa(b, "salom", 0), 5);   /* hajm 0 - hech narsa yozilmaydi */
    CHECK_INT(b[0], 'Z');
    free(b);

    BOLIM("xavfsiz_ulash");
    b = malloc(12);
    strcpy(b, "salom");
    CHECK_INT(xavfsiz_ulash(b, " dunyo", 12), 11);
    CHECK_BUF(b, 12, "salom dunyo");
    free(b);
    b = malloc(8);
    strcpy(b, "salom");
    CHECK_INT(xavfsiz_ulash(b, " dunyo", 8), 11);
    CHECK_BUF(b, 8, "salom d");
    free(b);
    b = malloc(6);
    strcpy(b, "salom");
    CHECK_INT(xavfsiz_ulash(b, "!!!", 6), 8);
    CHECK_BUF(b, 6, "salom");
    free(b);
    TEST_TUGADI();
}
