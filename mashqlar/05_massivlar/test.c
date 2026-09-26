#include "test.h"
#include "mashq.h"

/* Aniq o'lchamli heap massiv: chegaradan chiqilsa - sanitizer darhol ushlaydi. */
static int *nusxa(const int *src, size_t n)
{
    int *p = malloc((n ? n : 1) * sizeof(int));
    memcpy(p, src, n * sizeof(int));
    return p;
}

int main(void)
{
    TEST_BOSHLA();
    BOLIM("eng_katta");
    int a1[] = { 3, 9, -2, 9, 7 };
    int *p = nusxa(a1, 5);
    CHECK_INT(eng_katta(p, 5), 9);
    free(p);
    int a2[] = { -5, -3, -8 };
    p = nusxa(a2, 3);
    CHECK_INT(eng_katta(p, 3), -3);
    free(p);
    int a3[] = { 42 };
    p = nusxa(a3, 1);
    CHECK_INT(eng_katta(p, 1), 42);
    free(p);

    BOLIM("teskari");
    int b1[] = { 1, 2, 3, 4, 5 };
    p = nusxa(b1, 5);
    teskari(p, 5);
    CHECK(p[0] == 5 && p[1] == 4 && p[2] == 3 && p[3] == 2 && p[4] == 1);
    free(p);
    int b2[] = { 1, 2 };
    p = nusxa(b2, 2);
    teskari(p, 2);
    CHECK(p[0] == 2 && p[1] == 1);
    free(p);
    p = nusxa(b2, 1);
    teskari(p, 1);
    CHECK_INT(p[0], 1);
    free(p);
    p = nusxa(b2, 0);
    teskari(p, 0);                  /* n = 0: hech narsa qilmasligi kerak */
    free(p);

    BOLIM("takrorlanmas");
    int c1[] = { 1, 1, 2, 3, 3, 3, 4 };
    p = nusxa(c1, 7);
    size_t k = takrorlanmas(p, 7);
    CHECK_INT(k, 4);
    CHECK(k == 4 && p[0] == 1 && p[1] == 2 && p[2] == 3 && p[3] == 4);
    free(p);
    int c2[] = { 7, 7, 7 };
    p = nusxa(c2, 3);
    CHECK_INT(takrorlanmas(p, 3), 1);
    CHECK_INT(p[0], 7);
    free(p);
    p = nusxa(c2, 0);
    CHECK_INT(takrorlanmas(p, 0), 0);
    free(p);
    TEST_TUGADI();
}
