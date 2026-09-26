#include "test.h"
#include "mashq.h"

static char *heap(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    memcpy(p, s, n);
    return p;
}

#define TESKARI(kirish, kutilgan)            \
    do {                                     \
        char *s_ = heap(kirish);             \
        satr_teskari(s_);                    \
        CHECK_STR(s_, kutilgan);             \
        free(s_);                            \
    } while (0)

int main(void)
{
    TEST_BOSHLA();
    BOLIM("satr_teskari");
    TESKARI("salom", "molas");
    TESKARI("ab", "ba");
    TESKARI("a", "a");
    TESKARI("", "");
    TESKARI("1234", "4321");
    BOLIM("palindrommi");
    char *s = heap("A man, a plan, a canal: Panama");
    CHECK(palindrommi(s));
    free(s);
    CHECK(palindrommi(""));
    CHECK(palindrommi("a"));
    CHECK(palindrommi("Kiyik"));
    CHECK(palindrommi(".,!"));
    CHECK(!palindrommi("salom"));
    CHECK(!palindrommi("ab"));
    CHECK(palindrommi("12321"));
    CHECK(!palindrommi("123 21x"));
    CHECK(palindrommi("\xe9" "a\xe9"));   /* 0x80 dan katta baytlar: (unsigned char) kerak */
    TEST_TUGADI();
}
