/* =============================================================================
 *  14_jarayonlar.c - fork, exec, wait, pipe, dup2, signal    (darslik 14-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 14_jarayonlar.c -o jarayonlar && ./jarayonlar
 *
 *  Kutilgan natija (pid raqamlari boshqa bo'ladi; ota va bolaning birinchi qatorlari
 *  tartibi har safar boshqacha bo'lishi mumkin - ikkala jarayon PARALLEL ishlaydi):
 *      ota: pid 1234, bola yaratildi: 1235
 *      bola: men 1235, otam 1234
 *      ota: bola 7 kodi bilan tugadi
 *      "ls /" | "wc -l" natijasi (quvur orqali): <papkalar soni>
 *      Ctrl-C ni bosing (yoki 3 soniya kuting)...
 *      tugadi (signal keldimi: yo'q/ha)
 *
 *  Sinab ko'ring: strace -f ./jarayonlar 2>&1 | grep -E "clone|execve|pipe|dup2|wait"
 * ============================================================================= */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t signal_keldi = 0;

static void ishlovchi(int sig)
{
    (void)sig;
    signal_keldi = 1;                           /* handler ichida faqat eng oddiy ish */
}

int main(void)
{
    /* 1) fork + wait */
    fflush(stdout);                             /* bufer ikki marta chiqmasligi uchun */
    pid_t pid = fork();
    if (pid == 0) {
        printf("bola: men %d, otam %d\n", getpid(), getppid());
        exit(7);
    }
    printf("ota: pid %d, bola yaratildi: %d\n", getpid(), pid);
    int st;
    waitpid(pid, &st, 0);
    if (WIFEXITED(st))
        printf("ota: bola %d kodi bilan tugadi\n", WEXITSTATUS(st));

    /* 2) pipe + dup2 + exec: "ls / | wc -l" */
    int fds[2];
    if (pipe(fds) < 0) {                        /* tizim chaqiruvi natijasini DOIM tekshiring */
        perror("pipe");
        return 1;
    }
    fflush(stdout);
    if (fork() == 0) {
        dup2(fds[1], 1);                        /* stdout -> quvurning yozish uchi */
        close(fds[0]);
        close(fds[1]);
        execlp("ls", "ls", "/", (char *)NULL);
        _exit(127);
    }
    close(fds[1]);                              /* OTA yozish uchini yopmasa - EOF kelmaydi! */
    char buf[4096];
    ssize_t n, jami = 0;
    int qatorlar = 0;
    while ((n = read(fds[0], buf, sizeof(buf))) > 0)
        for (ssize_t i = 0; i < n; i++, jami++)
            qatorlar += buf[i] == '\n';
    close(fds[0]);
    wait(NULL);
    printf("\"ls /\" | \"wc -l\" natijasi (quvur orqali): %d\n", qatorlar);

    /* 3) signal */
    struct sigaction sa = { 0 };
    sa.sa_handler = ishlovchi;
    sigaction(SIGINT, &sa, NULL);
    printf("Ctrl-C ni bosing (yoki 3 soniya kuting)...\n");
    fflush(stdout);
    for (int i = 0; i < 30 && !signal_keldi; i++)
        usleep(100000);
    printf("tugadi (signal keldimi: %s)\n", signal_keldi ? "ha" : "yo'q");
    return 0;
}
