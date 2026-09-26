#include "test.h"
#include "mashq.h"

#define TEKSHIR(ifoda, kutilgan)                    \
    do {                                            \
        char *r_ = (ifoda);                         \
        CHECK_STR(r_, kutilgan);                    \
        free(r_);                                   \
    } while (0)

int main(void)
{
    TEST_BOSHLA();
    BOLIM("birlashtir");
    TEKSHIR(birlashtir("sa", "lom"), "salom");
    TEKSHIR(birlashtir("", "abc"), "abc");
    TEKSHIR(birlashtir("abc", ""), "abc");
    TEKSHIR(birlashtir("", ""), "");
    BOLIM("takrorla");
    TEKSHIR(takrorla("ab", 3), "ababab");
    TEKSHIR(takrorla("x", 1), "x");
    TEKSHIR(takrorla("abc", 0), "");
    TEKSHIR(takrorla("", 5), "");
    char *katta = takrorla("0123456789", 10000);
    CHECK(katta && strlen(katta) == 100000 && katta[99999] == '9');
    free(katta);
    BOLIM("qoshib_yoz");
    const char *q[] = { "a", "b", "c" };
    TEKSHIR(qoshib_yoz(q, 3, ", "), "a, b, c");
    TEKSHIR(qoshib_yoz(q, 1, ", "), "a");
    TEKSHIR(qoshib_yoz(q, 0, ", "), "");
    TEKSHIR(qoshib_yoz(q, 3, ""), "abc");
    const char *q2[] = { "", "x", "" };
    TEKSHIR(qoshib_yoz(q2, 3, "/"), "/x/");
    TEST_TUGADI();
}
