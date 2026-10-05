/*
 * init.c — Linux'dagi BIRINCHI foydalanuvchi dasturi (pid 1) va oddiy shell ("vksh").
 *
 * Linux yadrosi yuklangach, /init ni ishga tushiradi. Bu dastur libc'siz (nolibc — Linux manba kodidagi
 * kichik sarlavha: tizim chaqiruvlarini to'g'ridan-to'g'ri `ecall` bilan bajaradi) yozilgan.
 *   1) /proc va /sys ni ulaydi (mount);
 *   2) qatorni o'qiydi, so'zlarga bo'ladi va buyruqni bajaradi: ichki buyruqlar (ls, cat, ...) yoki
 *      tashqi dastur (fork + execve + waitid).
 */
#include <linux/utsname.h>
#include <linux/wait.h>
#include <asm-generic/siginfo.h>

#define MAKS_SOZ 16

static int bolish(char *q, char *soz[])
{
    int n = 0;
    while (*q && n < MAKS_SOZ - 1) {
        while (*q == ' ' || *q == '\t' || *q == '\n')
            *q++ = 0;
        if (!*q)
            break;
        soz[n++] = q;
        while (*q && *q != ' ' && *q != '\t' && *q != '\n')
            q++;
    }
    soz[n] = 0;
    return n;
}

static void fayl_chiqar(const char *yol)
{
    int fd = open(yol, O_RDONLY, 0);
    if (fd < 0) {
        printf("%s: ochilmadi (xato %d)\n", yol, errno);
        return;
    }
    char b[256];
    ssize_t n;
    while ((n = read(fd, b, sizeof(b))) > 0)
        write(1, b, n);
    close(fd);
}

static void royxat(const char *yol)
{
    int fd = open(yol, O_RDONLY | O_DIRECTORY, 0);
    if (fd < 0) {
        printf("%s: ochilmadi (xato %d)\n", yol, errno);
        return;
    }
    char b[1024];
    long n;
    while ((n = sys_getdents64(fd, (struct linux_dirent64 *)b, sizeof(b))) > 0) {
        for (long i = 0; i < n;) {
            struct linux_dirent64 *d = (struct linux_dirent64 *)(b + i);
            if (d->d_name[0] != '.')
                printf("%s%s  ", d->d_name, d->d_type == DT_DIR ? "/" : "");
            i += d->d_reclen;
        }
    }
    printf("\n");
    close(fd);
}

/* bolani kutish. DIQQAT: rv32 Linux'da eski wait4 tizim chaqiruvi YO'Q (64 bitli vaqtga o'tishda olib
   tashlangan) — faqat waitid bor. Chiqish kodi siginfo ichida (si_status). */
static int kut(pid_t p)
{
    siginfo_t s;
    memset(&s, 0, sizeof(s));
    if (my_syscall5(__NR_waitid, P_PID, p, &s, WEXITED, 0) < 0)
        return -1;
    return s.si_status;
}

static void yordam(void)
{
    printf("Buyruqlar: help ls [papka] cat FAYL echo ... mkdir PAPKA uname free uptime ps\n"
           "           run DASTUR [arg] (fork+execve)  poweroff\n");
}

static void ps(void)
{
    int fd = open("/proc", O_RDONLY | O_DIRECTORY, 0);
    char b[1024];
    long n;
    printf("  PID  NOM\n");
    while (fd >= 0 && (n = sys_getdents64(fd, (struct linux_dirent64 *)b, sizeof(b))) > 0) {
        for (long i = 0; i < n;) {
            struct linux_dirent64 *d = (struct linux_dirent64 *)(b + i);
            if (d->d_name[0] >= '1' && d->d_name[0] <= '9') {
                char yol[64] = "/proc/", nom[64] = { 0 };
                size_t u = strlen(yol);         /* nolibc da snprintf/strcat yo'q: satrlarni qo'lda yig'amiz */
                strcpy(yol + u, d->d_name);
                strcpy(yol + strlen(yol), "/comm");
                int f = open(yol, O_RDONLY, 0);
                if (f >= 0) {
                    ssize_t k = read(f, nom, sizeof(nom) - 1);
                    if (k > 0 && nom[k - 1] == '\n')
                        nom[k - 1] = 0;
                    close(f);
                }
                printf("%5s  %s\n", d->d_name, nom);
            }
            i += d->d_reclen;
        }
    }
    if (fd >= 0)
        close(fd);
}

int main(void)
{
    mount("proc", "/proc", "proc", 0, 0);
    mount("sysfs", "/sys", "sysfs", 0, 0);
    printf("\nSalom! Linux ishga tushdi. Men — init (pid %d), oddiy shell. 'help' — buyruqlar.\n", getpid());

    char qator[256];
    for (;;) {
        printf("vk:/# ");
        fflush(stdout);
        ssize_t n = read(0, qator, sizeof(qator) - 1);
        if (n <= 0) {                           /* kirish tugadi (masalan test fayli): kutib turamiz */
            sleep(1);
            continue;
        }
        qator[n] = 0;
        char *s[MAKS_SOZ];
        int k = bolish(qator, s);
        if (k == 0)
            continue;
        if (!strcmp(s[0], "help")) {
            yordam();
        } else if (!strcmp(s[0], "echo")) {
            for (int i = 1; i < k; i++)
                printf("%s%s", s[i], i + 1 < k ? " " : "");
            printf("\n");
        } else if (!strcmp(s[0], "ls")) {
            royxat(k > 1 ? s[1] : "/");
        } else if (!strcmp(s[0], "cat") && k > 1) {
            fayl_chiqar(s[1]);
        } else if (!strcmp(s[0], "mkdir") && k > 1) {
            if (mkdir(s[1], 0755) < 0)
                printf("mkdir: xato %d\n", errno);
        } else if (!strcmp(s[0], "uname")) {
            struct new_utsname u;           /* yadroning o'z tuzilmasi (linux/utsname.h) */
            my_syscall1(__NR_uname, &u);
            printf("%s %s %s %s\n", u.sysname, u.release, u.version, u.machine);
        } else if (!strcmp(s[0], "free")) {
            fayl_chiqar("/proc/meminfo");
        } else if (!strcmp(s[0], "uptime")) {
            fayl_chiqar("/proc/uptime");
        } else if (!strcmp(s[0], "ps")) {
            ps();
        } else if (!strcmp(s[0], "run") && k > 1) {
            pid_t p = fork();
            if (p == 0) {
                char *muhit[] = { 0 };
                execve(s[1], &s[1], muhit);
                printf("run: %s ishga tushmadi (xato %d)\n", s[1], errno);
                exit(127);
            }
            printf("[pid %d tugadi, kod %d]\n", p, kut(p));
        } else if (!strcmp(s[0], "poweroff")) {
            printf("O'chiryapman...\n");
            reboot(LINUX_REBOOT_CMD_POWER_OFF);
        } else {
            printf("%s: noma'lum buyruq ('help')\n", s[0]);
        }
    }
    return 0;
}
