/* msh.c - mini shell: buyruqlar, quvurlar (|), yo'naltirish (< > >>), cd/pwd/exit/durum */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAKS_ARG 32
#define MAKS_BUYRUQ 8

struct buyruq {
    char *argv[MAKS_ARG + 1];                   /* execvp uchun: oxiri NULL */
    char *kirish;                               /* < fayl */
    char *chiqish;                              /* > yoki >> fayl */
    int qoshib;                                 /* 1 - >>, 0 - > */
};

static int oxirgi_holat;                        /* oxirgi buyruqning chiqish kodi ("durum") */

/* qatorni bo'sh joylar bo'yicha so'zlarga ajratadi va "|" bo'yicha buyruqlarga bo'ladi.
   Qaytaradi: buyruqlar soni, xato bo'lsa -1. Qatorni o'zgartiradi (so'zlar uning ichida qoladi). */
static int tahlil(char *qator, struct buyruq *b)
{
    int n = 0;
    memset(&b[0], 0, sizeof(b[0]));
    int arg = 0;
    char *saqla, *so_z = strtok_r(qator, " \t\n", &saqla);
    while (so_z) {
        if (strcmp(so_z, "|") == 0) {
            if (arg == 0 || ++n == MAKS_BUYRUQ)
                return -1;                      /* bo'sh buyruq yoki juda ko'p quvur */
            memset(&b[n], 0, sizeof(b[n]));
            arg = 0;
        } else if (strcmp(so_z, "<") == 0 || strcmp(so_z, ">") == 0 || strcmp(so_z, ">>") == 0) {
            char *fayl = strtok_r(NULL, " \t\n", &saqla);
            if (!fayl)
                return -1;                      /* yo'naltirishdan keyin fayl nomi kerak */
            if (so_z[0] == '<') {
                b[n].kirish = fayl;
            } else {
                b[n].chiqish = fayl;
                b[n].qoshib = so_z[1] == '>';
            }
        } else {
            if (arg == MAKS_ARG)
                return -1;
            b[n].argv[arg++] = so_z;
            b[n].argv[arg] = NULL;
        }
        so_z = strtok_r(NULL, " \t\n", &saqla);
    }
    if (arg == 0)
        return n == 0 ? 0 : -1;                 /* butunlay bo'sh qator OK, "a |" - xato */
    return n + 1;
}

/* bola jarayon ichida: yo'naltirishlarni o'rnatib, buyruqni ishga tushiradi */
static void bola(struct buyruq *b, int kirish_fd, int chiqish_fd)
{
    signal(SIGINT, SIG_DFL);                    /* shell Ctrl-C ni e'tiborsiz qoldirdi, bola esa emas */
    if (kirish_fd != 0) {
        dup2(kirish_fd, 0);
        close(kirish_fd);
    }
    if (chiqish_fd != 1) {
        dup2(chiqish_fd, 1);
        close(chiqish_fd);
    }
    if (b->kirish) {
        int fd = open(b->kirish, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "msh: %s: %s\n", b->kirish, strerror(errno));
            _exit(1);
        }
        dup2(fd, 0);
        close(fd);
    }
    if (b->chiqish) {
        int fd = open(b->chiqish, O_WRONLY | O_CREAT | (b->qoshib ? O_APPEND : O_TRUNC), 0644);
        if (fd < 0) {
            fprintf(stderr, "msh: %s: %s\n", b->chiqish, strerror(errno));
            _exit(1);
        }
        dup2(fd, 1);
        close(fd);
    }
    execvp(b->argv[0], b->argv);
    fprintf(stderr, "msh: %s: %s\n", b->argv[0], errno == ENOENT ? "topilmadi" : strerror(errno));
    _exit(127);                                 /* exec muvaffaqiyatsiz: odatiy kod 127 */
}

/* "ichki" buyruqlar: shell o'zi bajaradi (bola jarayon emas) - cd ni boshqa jarayon qila olmaydi! */
static int ichki(struct buyruq *b)
{
    char *k = b->argv[0];
    if (strcmp(k, "exit") == 0)
        exit(b->argv[1] ? atoi(b->argv[1]) : oxirgi_holat);
    if (strcmp(k, "cd") == 0) {
        const char *yol = b->argv[1] ? b->argv[1] : getenv("HOME");
        if (!yol || chdir(yol) != 0) {
            fprintf(stderr, "msh: cd: %s: %s\n", yol ? yol : "(HOME yo'q)", strerror(errno));
            oxirgi_holat = 1;
        } else {
            oxirgi_holat = 0;
        }
        return 1;
    }
    if (strcmp(k, "pwd") == 0) {
        char bufer[256];
        if (getcwd(bufer, sizeof(bufer)))
            printf("%s\n", bufer);
        oxirgi_holat = 0;
        return 1;
    }
    if (strcmp(k, "durum") == 0) {
        printf("%d\n", oxirgi_holat);
        return 1;
    }
    return 0;
}

static void bajar(struct buyruq *b, int n)
{
    if (n == 1 && ichki(&b[0]))
        return;

    pid_t pid[MAKS_BUYRUQ];
    int oldingi_oqim = 0;                       /* oldingi buyruqning chiqishi - bu buyruqning kirishi */
    for (int i = 0; i < n; i++) {
        int quvur[2] = { 0, 1 };
        if (i < n - 1 && pipe(quvur) != 0) {
            perror("msh: pipe");
            return;
        }
        pid[i] = fork();
        if (pid[i] < 0) {
            perror("msh: fork");
            return;
        }
        if (pid[i] == 0) {
            if (i < n - 1)
                close(quvur[0]);                /* bola quvurning o'qish uchini ishlatmaydi */
            bola(&b[i], oldingi_oqim, quvur[1]);
        }
        if (oldingi_oqim != 0)
            close(oldingi_oqim);                /* ota: ishlatib bo'lingan uchlarni yopamiz */
        if (i < n - 1) {
            close(quvur[1]);
            oldingi_oqim = quvur[0];
        }
    }
    for (int i = 0; i < n; i++) {
        int holat;
        waitpid(pid[i], &holat, 0);
        if (i == n - 1)                         /* quvurning holati - oxirgi buyruqniki */
            oxirgi_holat = WIFEXITED(holat) ? WEXITSTATUS(holat) : 128 + WTERMSIG(holat);
    }
}

int main(void)
{
    signal(SIGINT, SIG_IGN);                    /* Ctrl-C shellni o'ldirmasin */
    int interaktiv = isatty(0);
    char qator[512];
    while (1) {
        if (interaktiv)
            printf("msh> ");
        fflush(stdout);                         /* bolalar chiqishidan OLDIN bizniki chiqib ketsin */
        if (!fgets(qator, sizeof(qator), stdin))
            break;
        struct buyruq b[MAKS_BUYRUQ];
        int n = tahlil(qator, b);
        if (n < 0)
            fprintf(stderr, "msh: sintaksis xatosi\n");
        else if (n > 0)
            bajar(b, n);
        fflush(stdout);
    }
    return oxirgi_holat;
}
