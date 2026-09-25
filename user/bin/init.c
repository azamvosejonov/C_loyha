/* =============================================================================
 *  user/bin/init.c - birinchi user dastur (Unix'dagi PID 1 ning vazifasi)
 * =============================================================================
 *
 *  Yadro boot tugagach faqat BITTA dasturni ishga tushiradi - /bin/init.
 *  Tizimni "tirik" qilish uning ishi:
 *    1. /etc/rc skriptini bajarish - disklarni ulash, sozlamalar (bir marta)
 *    2. Terminalga shell ochish; shell tugasa (exit, Ctrl-D) - yangisini ochish
 *
 *  Linux'da bu vazifani systemd, SysV init yoki busybox init bajaradi va ular
 *  yana ko'p narsa qiladi: xizmatlarni (daemon) boshqarish, bir nechta
 *  terminal (getty), yetim jarayonlarni yig'ish. init tugasa - Linux yadrosi
 *  panic qiladi ("Attempted to kill init!"); bizning yadro esa uni qayta
 *  ishga tushiradi.
 * ============================================================================= */
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "myos.h"

/* Dasturni ishga tushirib, tugashini kutish. */
static int run(char *const argv[])
{
    int pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0) {
        execv(argv[0], argv);
        perror(argv[0]);
        _exit(127);
    }
    int status = 0;
    while (waitpid(pid, &status, 0) < 0)
        ;
    return status;
}

int main(void)
{
    struct stat st;
    if (stat("/etc/rc", &st) == 0) {
        char *rc[] = { "/bin/sh", "/etc/rc", NULL };
        run(rc);
    }
    uint64_t last_start = 0;
    for (;;) {
        /* Shell darhol yiqilsa (masalan, buzilgan), tizimni sikl bilan
         * to'ldirmaslik uchun biroz kutamiz. */
        if (last_start && uptime_ms() - last_start < 1000)
            sleep_ms(1000);
        last_start = uptime_ms();
        char *sh[] = { "/bin/sh", NULL };
        int status = run(sh);
        printf("[init] shell tugadi (kod %d) - yangisi ochilmoqda\n", status);
    }
}
