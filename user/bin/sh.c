/* =============================================================================
 *  user/bin/sh.c - SHELL (buyruqlar interpretatori)
 * =============================================================================
 *
 *  Shell - oddiy user dasturi, hech qanday maxsus huquqi yo'q. U faqat:
 *    1) qatorni o'qiydi                         (read syscall)
 *    2) uni so'zlarga ajratadi                  ("cat README.txt" -> ["cat", "README.txt"])
 *    3) dasturni ishga tushiradi                (spawn syscall)
 *    4) tugashini kutadi                        (wait syscall)
 *
 *  Bash, zsh ham asosan shu tsiklni bajaradi (fork+exec+wait bilan).
 *
 *  Ichki (builtin) buyruqlar: help, exit, shutdown.
 *  Qolgan har bir so'z - initrd dagi dastur nomi.
 *  Qator oxirida '&' bo'lsa - dastur FONDA ishlaydi (shell kutmaydi).
 * ============================================================================= */
#include "ulib.h"

#define LINE_MAX 128
#define ARGS_MAX 16

static void print_help(void)
{
    puts("Ichki buyruqlar:");
    puts("  help            - shu yordam");
    puts("  exit            - shell'dan chiqish (init uni qayta ishga tushiradi)");
    puts("  shutdown        - tizimni o'chirish");
    puts("Dasturlar (initrd ichida, 'ls' bilan ko'ring):");
    puts("  hello [args]    - birinchi user dastur, argv ni ko'rsatadi");
    puts("  echo  [args]    - argumentlarni chiqarish");
    puts("  ls              - fayllar ro'yxati");
    puts("  cat <fayl>...   - fayl tarkibi");
    puts("  ps              - jarayonlar");
    puts("  free            - xotira statistikasi");
    puts("  kill <pid>      - jarayonni to'xtatish");
    puts("  memtest         - user malloc stress testi");
    puts("  spin <nom> [n]  - CPU'ni band qiluvchi dastur (multitasking uchun)");
    puts("  crash <tur>     - himoyani sinash: null | kernel | cli | div0 | stack");
    puts("  lspci           - PCI qurilmalari");
    puts("  dmesg           - yadro logi");
    puts("  uname           - tizim haqida");
    puts("  reboot          - qayta yuklash");
    puts("Oxirida '&' - fonda ishga tushirish:  spin a 20 &");
}

/* Qatorni bo'shliqlar bo'yicha so'zlarga ajratish (joyida - '\0' qo'yib). */
static int parse(char *line, char *argv[])
{
    int argc = 0;
    char *p = line;
    while (*p && argc < ARGS_MAX - 1) {
        while (isspace(*p))
            *p++ = '\0';                /* bo'shliqlarni satr oxiriga aylantiramiz */
        if (!*p)
            break;
        argv[argc++] = p;               /* so'z boshi */
        while (*p && !isspace(*p))
            p++;
    }
    argv[argc] = NULL;
    return argc;
}

/* Fonda tugagan jarayonlarni "yig'ib olish" - aks holda ular zombie bo'lib qoladi. */
static void reap_background(void)
{
    int status;
    int pid;
    while ((pid = wait(-1, &status, WAIT_NOHANG)) > 0)
        printf("[fon] pid %d tugadi (kod %d)\n", pid, status);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    char line[LINE_MAX];
    char *args[ARGS_MAX];

    printf("\nMyOS shell'iga xush kelibsiz! (pid %d). 'help' - yordam.\n", getpid());

    for (;;) {
        reap_background();
        printf("myos$ ");
        if (readline(line, sizeof(line)) < 0)
            return 1;                   /* kiritish uzildi (kill) */

        int n = parse(line, args);
        if (n == 0)
            continue;

        /* Oxirgi so'z '&' bo'lsa - fon rejimi. */
        int background = 0;
        if (strcmp(args[n - 1], "&") == 0) {
            background = 1;
            args[--n] = NULL;
            if (n == 0)
                continue;
        }

        /* --- Ichki buyruqlar --- */
        if (strcmp(args[0], "help") == 0) {
            print_help();
            continue;
        }
        if (strcmp(args[0], "exit") == 0)
            return 0;
        if (strcmp(args[0], "shutdown") == 0)
            shutdown();

        /* --- Tashqi dastur --- */
        int pid = spawn(args[0], args);
        if (pid < 0) {
            printf("sh: '%s' topilmadi yoki ishga tushmadi (xato %d)\n", args[0], pid);
            continue;
        }
        if (background) {
            printf("[fon] pid %d ishga tushdi\n", pid);
            continue;
        }
        int status;
        wait(pid, &status, 0);
        if (status != 0)
            printf("[sh] '%s' kod %d bilan tugadi\n", args[0], status);
    }
}
