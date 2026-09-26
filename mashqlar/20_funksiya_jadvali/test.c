#include "test.h"
#include "mashq.h"

#define OK(nom, a, b, kutilgan)                         \
    do {                                                \
        int r_ = 12345;                                 \
        CHECK_INT(hisobla(nom, a, b, &r_), 0);          \
        CHECK_INT(r_, kutilgan);                        \
    } while (0)

int main(void)
{
    TEST_BOSHLA();
    BOLIM("jadval");
    int n = 0;
    while (amallar[n].nom)
        n++;
    CHECK_INT(n, 5);
    BOLIM("oddiy amallar");
    OK("+", 2, 3, 5);
    OK("-", 2, 3, -1);
    OK("*", -4, 3, -12);
    OK("/", 17, 5, 3);
    OK("/", -17, 5, -3);
    OK("%", 17, 5, 2);
    OK("%", -17, 5, -2);
    BOLIM("nom topilmadi -> -2");
    int r = 0;
    CHECK_INT(hisobla("^", 1, 2, &r), -2);
    CHECK_INT(hisobla("", 1, 2, &r), -2);
    char nom[] = "+";                   /* boshqa manzildagi "+" - strcmp kerak */
    CHECK_INT(hisobla(nom, 1, 2, &r), 0);
    BOLIM("hisoblab bo'lmaydi -> -1");
    CHECK_INT(hisobla("/", 1, 0, &r), -1);
    CHECK_INT(hisobla("%", 1, 0, &r), -1);
    CHECK_INT(hisobla("/", INT_MIN, -1, &r), -1);
    CHECK_INT(hisobla("%", INT_MIN, -1, &r), -1);
    CHECK_INT(hisobla("+", INT_MAX, 1, &r), -1);
    CHECK_INT(hisobla("-", INT_MIN, 1, &r), -1);
    CHECK_INT(hisobla("*", 65536, 65536, &r), -1);
    TEST_TUGADI();
}
