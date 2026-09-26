#include <fcntl.h>
#include <unistd.h>

#include "test.h"
#include "mashq.h"

#define ARGV(...) ((char *const[]){ __VA_ARGS__, NULL })

static long ochiq_fdlar(void)
{
    long n = 0;
    for (int fd = 0; fd < 256; fd++)
        n += fcntl(fd, F_GETFD) != -1;
    return n;
}

int main(void)
{
    TEST_BOSHLA();
    long fd0 = ochiq_fdlar();
    char *buf = calloc(200000, 1);

    BOLIM("oddiy chiqish");
    CHECK_INT(chiqishni_ol(ARGV("echo", "salom"), buf, 100), 6);
    CHECK_STR(buf, "salom\n");
    CHECK_INT(chiqishni_ol(ARGV("printf", "a\\nb"), buf, 100), 3);
    CHECK_STR(buf, "a\nb");
    CHECK_INT(chiqishni_ol(ARGV("true"), buf, 100), 0);
    CHECK_STR(buf, "");

    BOLIM("katta chiqish (pipe buferidan katta: 64 KB+)");
    long n = chiqishni_ol(ARGV("seq", "1", "30000"), buf, 200000);
    CHECK_INT(n, 168894);
    CHECK(n > 0 && strncmp(buf, "1\n2\n3\n", 6) == 0 && strcmp(buf + n - 6, "30000\n") == 0);

    BOLIM("bufer kichik - qolgani tashlab yuboriladi (osilib qolmasin!)");
    char *kichik = calloc(10, 1);
    CHECK_INT(chiqishni_ol(ARGV("seq", "1", "30000"), kichik, 10), 9);
    CHECK_STR(kichik, "1\n2\n3\n4\n5");
    free(kichik);

    BOLIM("deskriptorlar sizib chiqmadimi");
    CHECK_INT(ochiq_fdlar(), fd0);
    free(buf);
    TEST_TUGADI();
}
