#include "test.h"
#include "mashq.h"

static void tekshir(const char *s, char c, const char *const *kutilgan, size_t k)
{
    size_t n = 12345;
    char **q = ajrat(s, c, &n);
    CHECK(q != NULL);
    if (!q)
        return;
    CHECK_INT(n, k);
    if (n == k) {
        for (size_t i = 0; i < k; i++)
            CHECK_STR(q[i], kutilgan[i]);
        CHECK(q[k] == NULL);            /* oxirida NULL */
    }
    ajrat_ozod(q);
}

int main(void)
{
    TEST_BOSHLA();
    BOLIM("oddiy");
    const char *k1[] = { "a", "b", "c" };
    tekshir("a,b,c", ',', k1, 3);
    const char *k2[] = { "ls", "-l", "/bin" };
    tekshir("ls -l /bin", ' ', k2, 3);
    const char *k3[] = { "salom" };
    tekshir("salom", ',', k3, 1);
    BOLIM("bo'sh qismlar");
    const char *k4[] = { "a", "", "b" };
    tekshir("a,,b", ',', k4, 3);
    const char *k5[] = { "" };
    tekshir("", ',', k5, 1);
    const char *k6[] = { "", "a", "" };
    tekshir(",a,", ',', k6, 3);
    const char *k7[] = { "", "", "" };
    tekshir("::", ':', k7, 3);
    BOLIM("PATH");
    const char *k8[] = { "/bin", "/usr/bin", "/sbin" };
    tekshir("/bin:/usr/bin:/sbin", ':', k8, 3);
    TEST_TUGADI();
}
