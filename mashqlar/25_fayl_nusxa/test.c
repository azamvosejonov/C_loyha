#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "test.h"
#include "mashq.h"

static long ochiq_fdlar(void)
{
    long n = 0;
    for (int fd = 0; fd < 256; fd++)
        n += fcntl(fd, F_GETFD) != -1;
    return n;
}

static int fayl_yoz(const char *yol, const unsigned char *d, size_t n)
{
    FILE *f = fopen(yol, "wb");
    if (!f)
        return -1;
    fwrite(d, 1, n, f);
    fclose(f);
    return 0;
}

static unsigned char *fayl_oqi(const char *yol, size_t *n)
{
    FILE *f = fopen(yol, "rb");
    if (!f)
        return NULL;
    unsigned char *d = malloc(4 << 20);
    *n = fread(d, 1, 4 << 20, f);
    fclose(f);
    return d;
}

int main(void)
{
    TEST_BOSHLA();
    long fd0 = ochiq_fdlar();
    size_t n = 1000003;                         /* 4096 ga karrali emas */
    unsigned char *d = malloc(n);
    for (size_t i = 0; i < n; i++)
        d[i] = (unsigned char)(i * 7 + i / 4096);
    fayl_yoz("t25_manba.bin", d, n);
    fayl_yoz("t25_bosh.bin", d, 0);
    unlink("t25_nishon.bin");

    BOLIM("katta fayl (1 MB)");
    CHECK_INT(nusxala("t25_manba.bin", "t25_nishon.bin"), 0);
    size_t m = 0;
    unsigned char *e = fayl_oqi("t25_nishon.bin", &m);
    CHECK(e != NULL);
    CHECK_INT(m, n);
    CHECK(e && m == n && memcmp(d, e, n) == 0);
    free(e);

    BOLIM("mavjud faylning ustidan yozish (O_TRUNC)");
    fayl_yoz("t25_kichik.bin", (const unsigned char *)"abc", 3);
    CHECK_INT(nusxala("t25_kichik.bin", "t25_nishon.bin"), 0);
    e = fayl_oqi("t25_nishon.bin", &m);
    CHECK_INT(m, 3);
    free(e);

    BOLIM("bo'sh fayl");
    CHECK_INT(nusxala("t25_bosh.bin", "t25_nishon.bin"), 0);
    e = fayl_oqi("t25_nishon.bin", &m);
    CHECK_INT(m, 0);
    free(e);

    BOLIM("manba yo'q");
    CHECK_INT(nusxala("t25_yoq_fayl.bin", "t25_nishon.bin"), -1);
    CHECK_INT(nusxala("t25_manba.bin", "/yoq_papka/x.bin"), -1);

    BOLIM("fayl deskriptorlari yopilganmi");
    CHECK_INT(ochiq_fdlar(), fd0);
    unlink("t25_manba.bin");
    unlink("t25_bosh.bin");
    unlink("t25_kichik.bin");
    unlink("t25_nishon.bin");
    free(d);
    TEST_TUGADI();
}
