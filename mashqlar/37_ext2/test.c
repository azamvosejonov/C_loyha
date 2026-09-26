#include <sys/stat.h>

#include "test.h"
#include "mashq.h"

static void yoz(const char *yol, const void *d, size_t n)
{
    FILE *f = fopen(yol, "wb");
    fwrite(d, 1, n, f);
    fclose(f);
}

static unsigned char naqsh(size_t i)
{
    return (unsigned char)(i * 7 + i / 1024);
}

/* Tasvir yaratish: t37_ildiz/ papkasidagi fayllar -> t37.img (1 KB bloklar). */
static int tasvir_yarat(void)
{
    if (system("rm -rf t37_ildiz t37.img") != 0)
        return -1;
    mkdir("t37_ildiz", 0755);
    mkdir("t37_ildiz/papka", 0755);
    mkdir("t37_ildiz/a", 0755);
    mkdir("t37_ildiz/a/b", 0755);
    mkdir("t37_ildiz/a/b/c", 0755);
    mkdir("t37_ildiz/kop", 0755);
    yoz("t37_ildiz/salom.txt", "Salom, ext2!\n", 13);
    yoz("t37_ildiz/papka/ichki.txt", "ichkarida\n", 10);
    yoz("t37_ildiz/a/b/c/chuqur.txt", "chuqur fayl", 11);
    yoz("t37_ildiz/bosh.txt", "", 0);
    size_t n = 300000;                          /* 293 blok: bevosita + bilvosita + ikki karra */
    unsigned char *d = malloc(n);
    for (size_t i = 0; i < n; i++)
        d[i] = naqsh(i);
    yoz("t37_ildiz/katta.bin", d, n);
    free(d);
    char nom[64], mazmun[32];
    for (int i = 0; i < 100; i++) {             /* papka bir necha blokni egallaydi */
        snprintf(nom, sizeof(nom), "t37_ildiz/kop/fayl_nomi_uzunroq_%03d.txt", i);
        int k = snprintf(mazmun, sizeof(mazmun), "fayl %d", i);
        yoz(nom, mazmun, (size_t)k);
    }
    return system("mke2fs -q -F -t ext2 -b 1024 -O ^dir_index -d t37_ildiz t37.img 2048 "
                  ">/dev/null 2>&1");
}

int main(void)
{
    TEST_BOSHLA();
    if (tasvir_yarat() != 0) {
        printf("  [XATO] mke2fs ishlamadi. O'rnating: sudo apt install e2fsprogs\n");
        return 1;
    }

    BOLIM("ext2_och");
    CHECK(ext2_och("t37_yoq.img") == NULL);
    CHECK(ext2_och("t37_ildiz/salom.txt") == NULL);   /* ext2 emas */
    struct ext2 *fs = ext2_och("t37.img");
    CHECK(fs != NULL);
    if (!fs)
        TEST_TUGADI();

    BOLIM("ext2_inode: ildiz papka");
    struct ext2_inode ino;
    memset(&ino, 0, sizeof(ino));
    CHECK_INT(ext2_inode(fs, EXT2_ILDIZ_INODE, &ino), 0);
    CHECK_HEX(ino.i_mode & EXT2_S_IFMT, EXT2_S_IFDIR);
    CHECK_INT(ext2_inode(fs, 0, &ino), -1);
    CHECK_INT(ext2_inode(fs, 1000000, &ino), -1);

    BOLIM("ext2_qidir");
    CHECK_INT(ext2_qidir(fs, "/"), 2);
    long s = ext2_qidir(fs, "/salom.txt");
    CHECK(s > 2);
    if (s > 2 && ext2_inode(fs, (uint32_t)s, &ino) == 0) {
        CHECK_HEX(ino.i_mode & EXT2_S_IFMT, EXT2_S_IFREG);
        CHECK_INT(ino.i_size, 13);
    }
    CHECK(ext2_qidir(fs, "/papka") > 2);
    CHECK(ext2_qidir(fs, "/a/b/c/chuqur.txt") > 2);
    CHECK_INT(ext2_qidir(fs, "/yoq.txt"), -1);
    CHECK_INT(ext2_qidir(fs, "/salom.txt/x"), -1);         /* fayl papka emas */
    CHECK_INT(ext2_qidir(fs, "/salom"), -1);               /* prefiks mos kelishi yetmaydi */
    CHECK(ext2_qidir(fs, "/kop/fayl_nomi_uzunroq_099.txt") > 2);

    BOLIM("ext2_oqi: kichik fayllar");
    char buf[128];
    memset(buf, 0, sizeof(buf));
    CHECK_INT(ext2_oqi(fs, "/salom.txt", buf, sizeof(buf)), 13);
    CHECK_STR(buf, "Salom, ext2!\n");
    memset(buf, 0, sizeof(buf));
    CHECK_INT(ext2_oqi(fs, "/papka/ichki.txt", buf, sizeof(buf)), 10);
    CHECK_STR(buf, "ichkarida\n");
    memset(buf, 0, sizeof(buf));
    CHECK_INT(ext2_oqi(fs, "/a/b/c/chuqur.txt", buf, sizeof(buf)), 11);
    CHECK_STR(buf, "chuqur fayl");
    CHECK_INT(ext2_oqi(fs, "/bosh.txt", buf, sizeof(buf)), 0);
    memset(buf, 0, sizeof(buf));
    CHECK_INT(ext2_oqi(fs, "/salom.txt", buf, 5), 5);       /* bufer kichik */
    CHECK_STR(buf, "Salom");
    CHECK_INT(ext2_oqi(fs, "/papka", buf, sizeof(buf)), -2);
    CHECK_INT(ext2_oqi(fs, "/yoq", buf, sizeof(buf)), -1);

    BOLIM("ko'p blokli papka");
    int ok = 1;
    for (int i = 0; i < 100; i += 9) {
        char yol[64], kut[32];
        snprintf(yol, sizeof(yol), "/kop/fayl_nomi_uzunroq_%03d.txt", i);
        int k = snprintf(kut, sizeof(kut), "fayl %d", i);
        memset(buf, 0, sizeof(buf));
        ok &= ext2_oqi(fs, yol, buf, sizeof(buf)) == k && strcmp(buf, kut) == 0;
    }
    CHECK(ok);

    BOLIM("katta fayl: 300000 bayt (ikki karra bilvosita bloklar)");
    size_t n = 300000;
    unsigned char *d = malloc(n + 100);
    CHECK_INT(ext2_oqi(fs, "/katta.bin", d, n + 100), 300000);
    ok = 1;
    size_t xato_joy = 0;
    for (size_t i = 0; i < n; i++)
        if (d[i] != naqsh(i)) {
            ok = 0;
            xato_joy = i;
            break;
        }
    CHECK(ok);
    if (!ok)
        printf("         birinchi farq: %zu-bayt (%zu-blok)\n", xato_joy, xato_joy / 1024);
    free(d);
    ext2_yop(fs);
    TEST_TUGADI();
}
