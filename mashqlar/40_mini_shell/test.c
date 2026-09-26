#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
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

static const char *fayl(const char *yol)
{
    static char buf[4096];
    memset(buf, 0, sizeof(buf));
    FILE *f = fopen(yol, "r");
    if (!f)
        return "(fayl yo'q)";
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

int main(void)
{
    TEST_BOSHLA();
    long fd0 = ochiq_fdlar();
    FILE *k = fopen("t40_kirish.txt", "w");
    for (int i = 0; i < 12345; i++)
        fputc('a' + i % 26, k);
    fclose(k);

    BOLIM("bo'sh qator");
    CHECK_INT(shell(""), 0);
    CHECK_INT(shell("   \t "), 0);

    BOLIM("oddiy buyruqlar va chiqish kodlari");
    CHECK_INT(shell("true"), 0);
    CHECK_INT(shell("false"), 1);
    CHECK_INT(shell("test 3 -gt 2"), 0);
    CHECK_INT(shell("bunday_dastur_yoq_xyz"), 127);

    BOLIM("> yo'naltirish");
    unlink("t40.txt");
    CHECK_INT(shell("echo salom   dunyo > t40.txt"), 0);
    CHECK_STR(fayl("t40.txt"), "salom dunyo\n");
    CHECK_INT(shell("echo qisqa > t40.txt"), 0);               /* O_TRUNC */
    CHECK_STR(fayl("t40.txt"), "qisqa\n");

    BOLIM("pipe");
    CHECK_INT(shell("printf abc | tr a-z A-Z > t40.txt"), 0);
    CHECK_STR(fayl("t40.txt"), "ABC");
    CHECK_INT(shell("seq 1 100 | grep 7 | wc -l > t40.txt"), 0);
    CHECK_STR(fayl("t40.txt"), "19\n");
    CHECK_INT(shell("echo a | cat | cat | cat | cat | cat | cat | cat > t40.txt"), 0);
    CHECK_STR(fayl("t40.txt"), "a\n");
    CHECK_INT(shell("seq 1 200000 | head -n 1 > t40.txt"), 0);  /* seq SIGPIPE oladi */
    CHECK_STR(fayl("t40.txt"), "1\n");

    BOLIM("< yo'naltirish");
    CHECK_INT(shell("wc -c < t40_kirish.txt > t40.txt"), 0);
    CHECK_STR(fayl("t40.txt"), "12345\n");
    CHECK_INT(shell("< t40_kirish.txt head -c 5 > t40.txt"), 0);   /* istalgan joyda */
    CHECK_STR(fayl("t40.txt"), "abcde");
    CHECK_INT(shell("cat < t40_yoq_fayl.txt"), 1);

    BOLIM("holat - OXIRGI buyruqniki");
    CHECK_INT(shell("false | true"), 0);
    CHECK_INT(shell("true | false"), 1);

    BOLIM("sintaksis xatolari -> -1");
    CHECK_INT(shell("| ls"), -1);
    CHECK_INT(shell("ls |"), -1);
    CHECK_INT(shell("ls | | wc"), -1);
    CHECK_INT(shell("ls >"), -1);
    CHECK_INT(shell("cat <"), -1);
    CHECK_INT(shell("true | true | true | true | true | true | true | true | true"), -1);   /* 9 ta */

    BOLIM("zombi va fd sizib chiqishi yo'q");
    errno = 0;
    CHECK(waitpid(-1, NULL, WNOHANG) == -1 && errno == ECHILD);
    CHECK_INT(ochiq_fdlar(), fd0);
    unlink("t40.txt");
    unlink("t40_kirish.txt");
    TEST_TUGADI();
}
