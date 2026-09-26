#include "test.h"
#include "mashq.h"

int main(void)
{
    TEST_BOSHLA();
    BOLIM("1. nusxa");
    char *s = nusxa("salom");
    CHECK_STR(s, "salom");
    free(s);

    BOLIM("2. massiv_nusxa");
    int a[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    int *b = massiv_nusxa(a, 8);
    CHECK(b && memcmp(a, b, sizeof(a)) == 0);
    free(b);

    BOLIM("3. yigindi");
    int *c = malloc(4 * sizeof(int));
    c[0] = 1, c[1] = 2, c[2] = 3, c[3] = 4;
    CHECK_INT(yigindi(c, 4), 10);
    free(c);

    BOLIM("4. katta_harf");
    s = katta_harf("salom, Dunyo!");
    CHECK_STR(s, "SALOM, DUNYO!");
    free(s);

    BOLIM("5. royxat_ozod");
    struct tugun *r = NULL;
    for (int i = 0; i < 3; i++) {
        struct tugun *t = malloc(sizeof(*t));
        t->qiymat = i;
        t->keyingi = r;
        r = t;
    }
    royxat_ozod(r);
    printf("-- (dastur oxirida LeakSanitizer free qilinmagan xotirani tekshiradi)\n");
    TEST_TUGADI();
}
