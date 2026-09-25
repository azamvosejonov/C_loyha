/* =============================================================================
 *  user/bin/sigtest.c - signallar testi
 * =============================================================================
 *
 *   1. handler chaqiriladi va dastur to'xtagan joyidan davom etadi
 *   2. sigprocmask: bloklangan signal kutib turadi, ochilganda yetkaziladi
 *   3. SIG_IGN, SIGKILL ni ushlab bo'lmasligi
 *   4. alarm + pause
 *   5. SIGSEGV handler (NULL ga yozish ushlanadi)
 *   6. standart amal: SIGTERM o'ldiradi -> WIFSIGNALED
 *   7. SIGSTOP / SIGCONT va waitpid(WUNTRACED)
 *   8. SIGPIPE
 *   9. EINTR va SA_RESTART
 *  10. butun guruhga kill(-pgid)
 *  11. SIGCHLD
 *  12. registrlar saqlanishi: hisob-kitob davomida 100 ta signal
 * ============================================================================= */
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "myos.h"

static int failures, checks;
static volatile sig_atomic_t hits, last_sig;

static void check(int ok, const char *what)
{
    checks++;
    printf("  [%s] %s\n", ok ? "OK" : "FAIL", what);
    if (!ok)
        failures++;
}

static void on_signal(int sig)
{
    hits++;
    last_sig = sig;
}

static void set_handler(int sig, sighandler_t h, unsigned long flags)
{
    struct sigaction sa = { .sa_handler = h, .sa_flags = flags };
    sigaction(sig, &sa, NULL);
}

static void on_segv(int sig)
{
    (void)sig;
    _exit(42);                          /* xato ushlandi - "chiroyli" chiqish */
}

/* Bolani ishga tushirib, holat so'zini qaytarish. */
static int child_status(void (*fn)(void))
{
    int pid = fork();
    if (pid == 0) {
        fn();
        _exit(0);
    }
    int st = -1;
    waitpid(pid, &st, 0);
    return st;
}

static void child_segv(void)
{
    set_handler(SIGSEGV, on_segv, 0);
    *(volatile int *)0 = 1;
}

static void child_term(void)
{
    raise(SIGTERM);
    _exit(1);                           /* bu yerga kelmasligi kerak */
}

static void child_pipe(void)
{
    int p[2];
    pipe(p);
    close(p[0]);
    write(p[1], "x", 1);                /* SIGPIPE -> o'ladi */
    _exit(1);
}

static volatile sig_atomic_t chld_hits;
static void on_chld(int sig)
{
    (void)sig;
    chld_hits++;
}

/* 12-test uchun: registrlarni ko'p ishlatadigan hisob-kitob. */
static uint64_t crunch(uint64_t n)
{
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    for (uint64_t i = 0; i < n; i++) {
        a = a * 6364136223846793005ULL + b;
        b ^= a >> 7;
        c += b * 3 + d;
        d = (d << 1) | (c >> 63);
        e += c ^ f;
        f -= e >> 3;
        g ^= f + h;
        h += g | 1;
    }
    return a ^ b ^ c ^ d ^ e ^ f ^ g ^ h;
}

int main(void)
{
    puts("sigtest: signallar");

    /* 1 */
    set_handler(SIGUSR1, on_signal, 0);
    hits = 0;
    raise(SIGUSR1);
    check(hits == 1 && last_sig == SIGUSR1, "handler chaqirildi va dastur davom etdi");

    /* 2 */
    sigset_t s, old;
    sigemptyset(&s);
    sigaddset(&s, SIGUSR1);
    sigprocmask(SIG_BLOCK, &s, &old);
    hits = 0;
    raise(SIGUSR1);
    int blocked_ok = hits == 0;
    sigprocmask(SIG_SETMASK, &old, NULL);
    check(blocked_ok && hits == 1, "bloklangan signal kutib turdi, ochilganda yetkazildi");

    /* 3 */
    signal(SIGUSR2, SIG_IGN);
    raise(SIGUSR2);
    check(1, "SIG_IGN: e'tiborsiz qoldirildi (tirikmiz)");
    struct sigaction sa = { .sa_handler = on_signal };
    check(sigaction(SIGKILL, &sa, NULL) < 0 && errno == EINVAL, "SIGKILL ni ushlab bo'lmaydi");

    /* 4 */
    set_handler(SIGALRM, on_signal, 0);
    hits = 0;
    uint64_t t0 = uptime_ms();
    alarm(1);
    int pr = pause();
    uint64_t dt = uptime_ms() - t0;
    check(pr < 0 && errno == EINTR && hits == 1 && last_sig == SIGALRM && dt >= 900 && dt < 2000,
          "alarm(1) + pause(): ~1 soniyadan keyin SIGALRM");

    /* 5, 6, 8 */
    int st = child_status(child_segv);
    check(WIFEXITED(st) && WEXITSTATUS(st) == 42, "SIGSEGV handler NULL ga yozishni ushladi");
    st = child_status(child_term);
    check(WIFSIGNALED(st) && WTERMSIG(st) == SIGTERM, "standart amal: SIGTERM o'ldiradi");
    st = child_status(child_pipe);
    check(WIFSIGNALED(st) && WTERMSIG(st) == SIGPIPE, "o'quvchisiz pipe -> SIGPIPE");
    signal(SIGPIPE, SIG_IGN);
    int p[2];
    pipe(p);
    close(p[0]);
    check(write(p[1], "x", 1) < 0 && errno == EPIPE, "SIGPIPE e'tiborsiz: write -> EPIPE");
    close(p[1]);
    signal(SIGPIPE, SIG_DFL);

    /* 7 */
    int pid = fork();
    if (pid == 0) {
        for (;;)
            pause();
    }
    kill(pid, SIGSTOP);
    st = 0;
    int r = waitpid(pid, &st, WUNTRACED);
    check(r == pid && WIFSTOPPED(st) && WSTOPSIG(st) == SIGSTOP, "SIGSTOP: WIFSTOPPED");
    kill(pid, SIGCONT);
    kill(pid, SIGTERM);
    waitpid(pid, &st, 0);
    check(WIFSIGNALED(st) && WTERMSIG(st) == SIGTERM, "SIGCONT + SIGTERM: davom etib, o'ldi");

    /* 9: EINTR (SA_RESTART siz) va avtomatik qayta boshlash (SA_RESTART bilan).
     * EINTR holatida bola chiqib ketadi va otaning write() i o'quvchisiz pipe'ga
     * tushadi - SIGPIPE bizni o'ldirmasligi uchun uni vaqtincha e'tiborsiz qoldiramiz. */
    signal(SIGPIPE, SIG_IGN);
    for (int restart = 0; restart <= 1; restart++) {
        int in[2];
        pipe(in);
        pid = fork();
        if (pid == 0) {
            close(in[1]);
            set_handler(SIGUSR1, on_signal, restart ? SA_RESTART : 0);
            char c;
            ssize_t n = read(in[0], &c, 1);
            if (n < 0 && errno == EINTR)
                _exit(10);
            _exit(n == 1 && c == 'z' ? 20 : 30);
        }
        close(in[0]);
        sleep_ms(200);                  /* bola read() da uxlab qolsin */
        kill(pid, SIGUSR1);
        sleep_ms(200);
        write(in[1], "z", 1);
        close(in[1]);
        waitpid(pid, &st, 0);
        check(WIFEXITED(st) && WEXITSTATUS(st) == (restart ? 20 : 10),
              restart ? "SA_RESTART: read() davom etdi va ma'lumotni oldi"
                      : "SA_RESTART siz: read() -> EINTR");
    }
    signal(SIGPIPE, SIG_DFL);

    /* 10 */
    int kids[3], pg = 0;
    for (int i = 0; i < 3; i++) {
        kids[i] = fork();
        if (kids[i] == 0) {
            for (;;)
                pause();
        }
        if (!pg)
            pg = kids[i];
        setpgid(kids[i], pg);
    }
    check(kill(-pg, SIGTERM) == 0, "kill(-pgid): guruhga yuborildi");
    int dead = 0;
    for (int i = 0; i < 3; i++)
        if (waitpid(kids[i], &st, 0) == kids[i] && WIFSIGNALED(st))
            dead++;
    check(dead == 3, "guruhdagi 3 jarayon ham tugadi");

    /* 11 */
    chld_hits = 0;
    set_handler(SIGCHLD, on_chld, SA_RESTART);
    pid = fork();
    if (pid == 0)
        _exit(0);
    waitpid(pid, &st, 0);
    check(chld_hits == 1, "SIGCHLD: bola tugaganini xabar qildi");
    signal(SIGCHLD, SIG_DFL);

    /* 12: bola hisoblaydi, ota esa hisob DAVOMIDA unga signallar yuboradi (har
     * 10 ms da). Bola kamida 20 ta signal olguncha bo'laklab hisoblaydi. Keyin ota
     * xuddi shuncha bo'lakni signalsiz hisoblab, natijalarni solishtiradi: har bir
     * handler'dan keyin sigreturn BARCHA registrlarni tiklagan bo'lsa - mos keladi. */
    int res[2];
    pipe(res);
    pid = fork();
    if (pid == 0) {
        close(res[0]);
        set_handler(SIGUSR1, on_signal, SA_RESTART);
        hits = 0;
        write(res[1], "r", 1);          /* "tayyorman" */
        uint64_t acc = 0, chunks = 0;
        while (hits < 20 && chunks < 100000) {
            acc ^= crunch(100000 + chunks);
            chunks++;
        }
        uint64_t out[2] = { acc, chunks };
        write(res[1], out, sizeof(out));
        _exit(0);
    }
    close(res[1]);
    char ready;
    read(res[0], &ready, 1);
    for (int i = 0; i < 500 && waitpid(pid, &st, WNOHANG) == 0; i++) {
        kill(pid, SIGUSR1);
        sleep_ms(10);
    }
    uint64_t out[2] = { 0, 0 };
    read(res[0], out, sizeof(out));
    close(res[0]);
    waitpid(pid, &st, 0);
    uint64_t expect = 0;
    for (uint64_t i = 0; i < out[1]; i++)
        expect ^= crunch(100000 + i);
    printf("  %lu bo'lak, kamida 20 ta signal\n", out[1]);
    check(out[1] > 0 && out[1] < 100000 && out[0] == expect,
          "hisob-kitob o'rtasida signallar: registrlar buzilmadi");

    if (failures == 0)
        printf("sigtest: PASSED (%d tekshiruv)\n", checks);
    else
        printf("sigtest: %d/%d FAIL\n", failures, checks);
    return failures != 0;
}
