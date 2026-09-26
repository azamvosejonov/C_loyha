#include "test.h"
#include "mashq.h"

static char *heap(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    memcpy(p, s, n);
    return p;
}

int main(void)
{
    TEST_BOSHLA();
    BOLIM("mening_strlen");
    char *s = heap("salom");
    CHECK_INT(mening_strlen(s), 5);
    free(s);
    s = heap("");
    CHECK_INT(mening_strlen(s), 0);
    free(s);
    CHECK_INT(mening_strlen("ab\0cd"), 2);

    BOLIM("mening_strchr");
    s = heap("dasturlash");
    CHECK(mening_strchr(s, 'd') == s);
    CHECK(mening_strchr(s, 's') == s + 2);
    CHECK(mening_strchr(s, 'h') == s + 9);
    CHECK(mening_strchr(s, 'z') == NULL);
    CHECK(mening_strchr(s, '\0') == s + 10);
    free(s);

    BOLIM("mening_strcmp");
    CHECK(mening_strcmp("abc", "abc") == 0);
    CHECK(mening_strcmp("abc", "abd") < 0);
    CHECK(mening_strcmp("abd", "abc") > 0);
    CHECK(mening_strcmp("ab", "abc") < 0);
    CHECK(mening_strcmp("abc", "ab") > 0);
    CHECK(mening_strcmp("", "") == 0);
    CHECK(mening_strcmp("\xff", "a") > 0);    /* unsigned char! */
    TEST_TUGADI();
}
