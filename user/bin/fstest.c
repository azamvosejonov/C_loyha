/* =============================================================================
 *  user/bin/fstest.c - VFS, fayllar, papkalar, pipe va dup2 testlari
 * =============================================================================
 *
 *  Har bir tekshiruv POSIX xatti-harakatini sinaydi - xuddi shu dastur Linux'da
 *  ham (kichik o'zgartirishlar bilan) o'tishi kerak. Yadroning fayl qismini
 *  o'zgartirsangiz - shu testni ishga tushiring.
 *
 *    1. Fayl: yaratish, yozish, o'qish, lseek, O_APPEND, O_TRUNC, ftruncate, "teshik"
 *    2. Papkalar: mkdir, getdents, rmdir (bo'sh emas -> ENOTEMPTY), rename
 *    3. unlink ochiq faylni: nom yo'qoladi, ma'lumot fd orqali hali o'qiladi
 *    4. Xato kodlari: ENOENT, EEXIST, ENOTDIR, EISDIR, EBADF
 *    5. pipe + fork: 200 KB uzatish, EOF, o'quvchisiz yozish -> EPIPE
 *    6. dup2: stdout ni faylga yo'naltirish (shell'dagi `>` aynan shu)
 *    7. /dev/null, /dev/zero
 * ============================================================================= */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static int failures, checks;

static void check(int ok, const char *what)
{
    checks++;
    if (!ok) {
        failures++;
        printf("  [FAIL] %s (errno=%d %s)\n", what, errno, strerror(errno));
    }
}

static void test_files(void)
{
    puts("1. fayllar");
    unlink("/tmp/fst/a.txt");
    int fd = open("/tmp/fst/a.txt", O_RDWR | O_CREAT | O_EXCL, 0644);
    check(fd >= 0, "O_CREAT|O_EXCL yangi fayl");
    check(open("/tmp/fst/a.txt", O_RDWR | O_CREAT | O_EXCL, 0644) < 0 && errno == EEXIST,
          "O_EXCL mavjud faylga -> EEXIST");
    check(write(fd, "salom dunyo", 11) == 11, "write 11 bayt");
    check(lseek(fd, 0, SEEK_CUR) == 11, "pozitsiya 11");
    check(lseek(fd, 6, SEEK_SET) == 6, "lseek SEEK_SET");
    char buf[64] = { 0 };
    check(read(fd, buf, sizeof(buf)) == 5 && memcmp(buf, "dunyo", 5) == 0, "read qolgan qism");
    check(read(fd, buf, sizeof(buf)) == 0, "fayl oxiri: read = 0");

    /* "Teshik": oxiridan uzoqqa lseek + write - oraliq nollar bilan to'ladi. */
    check(lseek(fd, 100, SEEK_SET) == 100 && write(fd, "X", 1) == 1, "teshikdan keyin yozish");
    struct stat st;
    check(fstat(fd, &st) == 0 && st.st_size == 101 && S_ISREG(st.st_mode), "fstat: hajm 101");
    lseek(fd, 50, SEEK_SET);
    check(read(fd, buf, 4) == 4 && buf[0] == 0 && buf[3] == 0, "teshik nollar bilan o'qiladi");
    check(ftruncate(fd, 5) == 0 && fstat(fd, &st) == 0 && st.st_size == 5, "ftruncate 5");
    close(fd);

    fd = open("/tmp/fst/a.txt", O_WRONLY | O_APPEND);
    check(fd >= 0 && write(fd, "!!", 2) == 2, "O_APPEND yozish");
    lseek(fd, 0, SEEK_SET);             /* O_APPEND: baribir oxiriga yoziladi */
    write(fd, "?", 1);
    close(fd);
    fd = open("/tmp/fst/a.txt", O_RDONLY);
    memset(buf, 0, sizeof(buf));
    check(read(fd, buf, sizeof(buf)) == 8 && strcmp(buf, "salom!!?") == 0,
          "O_APPEND lseek'dan qat'i nazar oxiriga yozadi");
    check(write(fd, "x", 1) < 0 && errno == EBADF, "O_RDONLY faylga yozish -> EBADF");
    close(fd);
    fd = open("/tmp/fst/a.txt", O_WRONLY | O_TRUNC);
    check(fd >= 0 && fstat(fd, &st) == 0 && st.st_size == 0, "O_TRUNC hajmni 0 qiladi");
    close(fd);

    /* Katta fayl: 300 KB yozib, qayta o'qib solishtiramiz. */
    fd = open("/tmp/fst/katta", O_RDWR | O_CREAT | O_TRUNC, 0644);
    static char big[300 * 1024];
    for (size_t i = 0; i < sizeof(big); i++)
        big[i] = (char)(i * 7 + i / 4096);
    check(write(fd, big, sizeof(big)) == (ssize_t)sizeof(big), "300 KB yozish");
    lseek(fd, 0, SEEK_SET);
    static char back[300 * 1024];
    ssize_t got = 0, n;
    while ((n = read(fd, back + got, sizeof(back) - (size_t)got)) > 0)
        got += n;
    check(got == (ssize_t)sizeof(big) && memcmp(big, back, sizeof(big)) == 0,
          "300 KB qayta o'qildi va mos keldi");
    close(fd);
    check(unlink("/tmp/fst/katta") == 0, "unlink");
}

static void test_dirs(void)
{
    puts("2. papkalar");
    check(mkdir("/tmp/fst/d", 0755) == 0, "mkdir");
    check(mkdir("/tmp/fst/d", 0755) < 0 && errno == EEXIST, "mkdir mavjud -> EEXIST");
    close(open("/tmp/fst/d/f1", O_WRONLY | O_CREAT, 0644));
    close(open("/tmp/fst/d/f2", O_WRONLY | O_CREAT, 0644));
    check(mkdir("/tmp/fst/d/ichki", 0755) == 0, "ichki papka");

    DIR *d = opendir("/tmp/fst/d");
    int seen = 0, dots = 0;
    struct dirent *de;
    while (d && (de = readdir(d)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            dots++;
        else if (strcmp(de->d_name, "f1") == 0 || strcmp(de->d_name, "f2") == 0)
            seen++;
        else if (strcmp(de->d_name, "ichki") == 0 && de->d_type == S_IFDIR)
            seen++;
    }
    if (d)
        closedir(d);
    check(seen == 3 && dots == 2, "readdir: . .. f1 f2 ichki/");

    check(rmdir("/tmp/fst/d") < 0 && errno == ENOTEMPTY, "bo'sh bo'lmagan papka -> ENOTEMPTY");
    check(rename("/tmp/fst/d/f1", "/tmp/fst/d/ichki/g") == 0, "rename boshqa papkaga");
    struct stat st;
    check(stat("/tmp/fst/d/f1", &st) < 0 && errno == ENOENT, "eski nom yo'q");
    check(stat("/tmp/fst/d/ichki/g", &st) == 0, "yangi nom bor");
    check(rename("/tmp/fst/d", "/tmp/fst/d/ichki/x") < 0 && errno == EINVAL,
          "papkani o'z ichiga ko'chirish -> EINVAL");

    /* Nisbiy yo'llar va chdir. */
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    check(chdir("/tmp/fst/d/ichki") == 0, "chdir");
    char now[PATH_MAX];
    check(getcwd(now, sizeof(now)) && strcmp(now, "/tmp/fst/d/ichki") == 0, "getcwd");
    check(stat("g", &st) == 0 && stat("../f2", &st) == 0 && stat("./../../d/f2", &st) == 0,
          "nisbiy yo'llar: g, ../f2, ./../../d/f2");
    check(chdir("..") == 0 && getcwd(now, sizeof(now)) && strcmp(now, "/tmp/fst/d") == 0,
          "chdir ..");
    chdir(cwd);

    unlink("/tmp/fst/d/ichki/g");
    unlink("/tmp/fst/d/f2");
    check(rmdir("/tmp/fst/d/ichki") == 0 && rmdir("/tmp/fst/d") == 0, "rmdir bo'sh papkalar");
}

static void test_unlink_open(void)
{
    puts("3. ochiq faylni unlink");
    int fd = open("/tmp/fst/vaqtincha", O_RDWR | O_CREAT | O_TRUNC, 0644);
    write(fd, "hali shu yerda", 14);
    check(unlink("/tmp/fst/vaqtincha") == 0, "unlink ochiq fayl");
    struct stat st;
    check(stat("/tmp/fst/vaqtincha", &st) < 0, "nom yo'qoldi");
    char buf[32] = { 0 };
    lseek(fd, 0, SEEK_SET);
    check(read(fd, buf, sizeof(buf)) == 14 && strcmp(buf, "hali shu yerda") == 0,
          "ma'lumot fd orqali hali o'qiladi (inode tirik)");
    check(fstat(fd, &st) == 0 && st.st_nlink == 0, "nlink = 0");
    close(fd);                          /* endi inode haqiqatan bo'shaydi */
}

static void test_errors(void)
{
    puts("4. xato kodlari");
    check(open("/yoq/fayl", O_RDONLY) < 0 && errno == ENOENT, "ENOENT");
    check(open("/bin/sh/x", O_RDONLY) < 0 && errno == ENOTDIR, "fayl ichida yo'l -> ENOTDIR");
    check(open("/bin", O_WRONLY) < 0 && errno == EISDIR, "papkaga yozish -> EISDIR");
    check(close(55) < 0 && errno == EBADF, "yopiq fd -> EBADF");
    check(unlink("/tmp") < 0 && errno == EISDIR, "papkani unlink -> EISDIR");
    check(rmdir("/bin/sh") < 0 && errno == ENOTDIR, "faylni rmdir -> ENOTDIR");
    check(rmdir("/") < 0, "ildizni o'chirib bo'lmaydi");
}

static void test_pipe(void)
{
    puts("5. pipe");
    int p[2];
    check(pipe(p) == 0, "pipe()");
    int pid = fork();
    if (pid == 0) {
        close(p[0]);
        /* 200 KB - pipe buferidan (16 KB) ancha katta: yozuvchi to'lganda uxlaydi. */
        static char chunk[1000];
        for (int i = 0; i < 200; i++) {
            memset(chunk, 'a' + i % 26, sizeof(chunk));
            for (size_t done = 0; done < sizeof(chunk);) {
                ssize_t w = write(p[1], chunk + done, sizeof(chunk) - done);
                if (w <= 0)
                    _exit(1);
                done += (size_t)w;
            }
        }
        _exit(0);                       /* yozish uchi yopiladi -> o'quvchi EOF oladi */
    }
    close(p[1]);                        /* MUHIM: aks holda EOF hech qachon kelmaydi */
    static char buf[4096];
    long total = 0, bad = 0;
    ssize_t n;
    while ((n = read(p[0], buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++)
            if (buf[i] != 'a' + (int)((total + i) / 1000) % 26)
                bad++;
        total += n;
    }
    close(p[0]);
    int st = -1;
    waitpid(pid, &st, 0);
    check(total == 200000 && bad == 0 && st == 0, "200 KB tartib bilan uzatildi, keyin EOF");

    check(pipe(p) == 0, "ikkinchi pipe");
    close(p[0]);                        /* o'quvchi yo'q */
    check(write(p[1], "x", 1) < 0 && errno == EPIPE, "o'quvchisiz pipe'ga yozish -> EPIPE");
    close(p[1]);
}

static void test_dup2(void)
{
    puts("6. dup2 (shell yo'naltirishi)");
    fflush(stdout);
    int saved = dup(STDOUT_FILENO);
    int fd = open("/tmp/fst/chiqish", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    check(dup2(fd, STDOUT_FILENO) == STDOUT_FILENO, "dup2(fd, 1)");
    close(fd);
    write(STDOUT_FILENO, "faylga ketdi\n", 13);
    dup2(saved, STDOUT_FILENO);         /* stdout ni qaytaramiz */
    close(saved);
    char buf[32] = { 0 };
    fd = open("/tmp/fst/chiqish", O_RDONLY);
    check(read(fd, buf, sizeof(buf)) == 13 && strcmp(buf, "faylga ketdi\n") == 0,
          "stdout faylga yo'naltirildi");
    close(fd);
    unlink("/tmp/fst/chiqish");

    int a = dup(STDIN_FILENO);
    check(a >= 3, "dup eng kichik bo'sh raqamni beradi");
    close(a);
}

static void test_dev(void)
{
    puts("7. qurilmalar");
    int fd = open("/dev/null", O_RDWR);
    char buf[16];
    check(fd >= 0 && write(fd, "yo'qoladi", 9) == 9 && read(fd, buf, sizeof(buf)) == 0,
          "/dev/null");
    close(fd);
    fd = open("/dev/zero", O_RDONLY);
    memset(buf, 1, sizeof(buf));
    check(fd >= 0 && read(fd, buf, sizeof(buf)) == 16 && buf[0] == 0 && buf[15] == 0,
          "/dev/zero");
    close(fd);
    struct stat st;
    check(stat("/dev/console", &st) == 0 && S_ISCHR(st.st_mode), "/dev/console - belgili qurilma");
}

int main(void)
{
    mkdir("/tmp", 0755);
    mkdir("/tmp/fst", 0755);
    test_files();
    test_dirs();
    test_unlink_open();
    test_errors();
    test_pipe();
    test_dup2();
    test_dev();
    rmdir("/tmp/fst/d");
    unlink("/tmp/fst/a.txt");
    rmdir("/tmp/fst");
    if (failures == 0)
        printf("fstest: PASSED (%d tekshiruv)\n", checks);
    else
        printf("fstest: %d/%d FAIL\n", failures, checks);
    return failures != 0;
}
