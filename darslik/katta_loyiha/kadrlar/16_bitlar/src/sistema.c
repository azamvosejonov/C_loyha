#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sistema.h"

int yoz_hammasi(int fd, const void *malumot, size_t uzunlik)
{
    const char *p = malumot;
    while (uzunlik > 0) {
        ssize_t n = write(fd, p, uzunlik);
        if (n < 0) {
            if (errno == EINTR)
                continue;                       /* signal uzdi: qayta urinamiz */
            return -1;
        }
        p += n;                                 /* n bayt yozildi: qolganini yozamiz */
        uzunlik -= (size_t)n;
    }
    return 0;
}

int fayl_yarat_yoz(const char *yol, const void *malumot, size_t uzunlik)
{
    int fd = open(yol, O_WRONLY | O_CREAT | O_EXCL, 0644);      /* 0644: egasi o'qiy/yoza oladi, boshqalar faqat o'qiydi */
    if (fd < 0)
        return -1;
    if (yoz_hammasi(fd, malumot, uzunlik) != 0) {
        int xato = errno;                       /* close errno ni buzmasin */
        close(fd);
        errno = xato;
        return -1;
    }
    return close(fd);                           /* close ham xato berishi mumkin (masalan, NFS da) */
}

/* fork dan keyin printf ishlatmaymiz (buferlar nusxalangan): faqat write. Xato bo'lsa ham qiladigan ish yo'q */
static void xabar_yoz(const char *s)
{
    if (write(STDERR_FILENO, s, strlen(s)) < 0)
        return;
}

int dastur_ishlat(char *const argv[], const void *kirish, size_t uzunlik)
{
    int quvur[2];                               /* quvur[0] - o'qish uchi, quvur[1] - yozish uchi */
    if (pipe(quvur) != 0)
        return -1;

    fflush(stdout);                             /* bufer bola jarayonga NUSXALANIB, ikki marta chiqib ketmasin */
    signal(SIGPIPE, SIG_IGN);                   /* bola kirishni o'qimay chiqib ketsa, write() SIGPIPE bilan o'ldirmasin: EPIPE xatosi qaytsin */

    pid_t pid = fork();
    if (pid < 0) {
        int xato = errno;
        close(quvur[0]);
        close(quvur[1]);
        errno = xato;
        return -1;
    }
    if (pid == 0) {                             /* ---- BOLA ---- */
        close(quvur[1]);                        /* yozish uchi bolaga kerak emas */
        dup2(quvur[0], STDIN_FILENO);           /* endi fayl deskriptor 0 (stdin) quvurning o'qish uchi */
        close(quvur[0]);
        execvp(argv[0], argv);                  /* muvaffaqiyatli bo'lsa, bu yerga QAYTMAYDI */
        xabar_yoz("kadrlar: dastur topilmadi yoki ishga tushmadi: ");
        xabar_yoz(argv[0]);
        xabar_yoz("\n");
        _exit(127);                             /* exit EMAS: ota jarayonning buferlarini bola yopib yubormasin */
    }

    /* ---- OTA ---- */
    close(quvur[0]);
    int yozish = yoz_hammasi(quvur[1], kirish, uzunlik);
    int yozish_xato = errno;
    close(quvur[1]);                            /* bola "fayl tugadi" (EOF) ni ko'rishi uchun YOPISH SHART */

    int holat;
    while (waitpid(pid, &holat, 0) < 0)
        if (errno != EINTR)
            return -1;
    if (yozish != 0 && yozish_xato != EPIPE) {  /* EPIPE - bola kirishni to'liq o'qimadi: bu normal bo'lishi mumkin */
        errno = yozish_xato;
        return -1;
    }
    if (WIFEXITED(holat))
        return WEXITSTATUS(holat);
    return 128 + WTERMSIG(holat);
}
