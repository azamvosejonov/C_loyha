/* =============================================================================
 *  user/bin/kill.c - jarayonga signal yuborish
 *
 *    kill <pid>...            SIGTERM ("iltimos, tugat")
 *    kill -9 <pid>            SIGKILL (ushlab bo'lmaydi)
 *    kill -STOP <pid>         to'xtatish; kill -CONT <pid> - davom ettirish
 *    kill -- -<pgid>          butun guruhga (job)
 *    kill -l                  signallar ro'yxati
 * ============================================================================= */
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const struct {
    const char *name;
    int sig;
} names[] = {
    { "HUP", SIGHUP },   { "INT", SIGINT },   { "QUIT", SIGQUIT }, { "ILL", SIGILL },
    { "TRAP", SIGTRAP }, { "ABRT", SIGABRT }, { "BUS", SIGBUS },   { "FPE", SIGFPE },
    { "KILL", SIGKILL }, { "USR1", SIGUSR1 }, { "SEGV", SIGSEGV }, { "USR2", SIGUSR2 },
    { "PIPE", SIGPIPE }, { "ALRM", SIGALRM }, { "TERM", SIGTERM }, { "CHLD", SIGCHLD },
    { "CONT", SIGCONT }, { "STOP", SIGSTOP }, { "TSTP", SIGTSTP }, { "TTIN", SIGTTIN },
    { "TTOU", SIGTTOU }, { "WINCH", SIGWINCH },
};
#define NNAMES (int)(sizeof(names) / sizeof(names[0]))

/* "9", "KILL", "SIGKILL" -> 9. -1 - noma'lum. */
static int parse_signal(const char *s)
{
    if (isdigit((unsigned char)s[0]))
        return atoi(s);
    if (strncmp(s, "SIG", 3) == 0)
        s += 3;
    for (int i = 0; i < NNAMES; i++)
        if (strcmp(s, names[i].name) == 0)
            return names[i].sig;
    return -1;
}

int main(int argc, char **argv)
{
    int sig = SIGTERM, i = 1;
    if (argc > 1 && strcmp(argv[1], "-l") == 0) {
        for (int k = 0; k < NNAMES; k++)
            printf("%2d) SIG%-6s %s\n", names[k].sig, names[k].name, strsignal(names[k].sig));
        return 0;
    }
    if (i < argc && argv[i][0] == '-' && strcmp(argv[i], "--") != 0 &&
        !isdigit((unsigned char)argv[i][1])) {
        sig = parse_signal(argv[i] + 1);
        i++;
    } else if (i < argc && argv[i][0] == '-' && isdigit((unsigned char)argv[i][1]) &&
               i + 1 < argc) {
        sig = atoi(argv[i] + 1);        /* kill -9 123 */
        i++;
    }
    if (i < argc && strcmp(argv[i], "--") == 0)
        i++;
    if (sig < 0 || i >= argc) {
        fprintf(stderr, "ishlatish: kill [-SIGNAL] <pid>...   (kill -l - ro'yxat)\n");
        return 1;
    }
    int status = 0;
    for (; i < argc; i++) {
        int pid = atoi(argv[i]);
        if (kill(pid, sig) < 0) {
            fprintf(stderr, "kill: (%d): %s\n", pid, strerror(errno));
            status = 1;
        }
    }
    return status;
}
