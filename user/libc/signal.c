/* =============================================================================
 *  user/libc/signal.c - signal(), sigaction(), raise() ...
 * ============================================================================= */
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#include "syscall.h"

extern void __restore_rt(void);         /* sigreturn.asm */

int sigaction(int sig, const struct sigaction *act, struct sigaction *old)
{
    struct sigaction k;
    if (act) {
        k = *act;
        /* Handler qaytishi uchun trampolin - dasturchi o'ylashi shart emas. */
        k.sa_flags |= SA_RESTORER;
        k.sa_restorer = __restore_rt;
    }
    return (int)__sysret(__syscall3(SYS_SIGACTION, sig, act ? (long)&k : 0, (long)old));
}

/* signal() - soddalashtirilgan interfeys. BSD semantikasi (glibc kabi):
 * handler doimiy qoladi va uzilgan syscall'lar qayta boshlanadi. */
sighandler_t signal(int sig, sighandler_t handler)
{
    struct sigaction act = { .sa_handler = handler, .sa_flags = SA_RESTART }, old;
    if (sigaction(sig, &act, &old) < 0)
        return SIG_ERR;
    return old.sa_handler;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *old)
{
    return (int)__sysret(__syscall3(SYS_SIGPROCMASK, how, (long)set, (long)old));
}

int raise(int sig)
{
    return kill(getpid(), sig);
}

int setpgid(int pid, int pgid)
{
    return (int)__sysret(__syscall2(SYS_SETPGID, pid, pgid));
}

int getpgid(int pid)
{
    return (int)__sysret(__syscall1(SYS_GETPGID, pid));
}

int getpgrp(void)
{
    return getpgid(0);
}

int setsid(void)
{
    return (int)__sysret(__syscall0(SYS_SETSID));
}

unsigned alarm(unsigned seconds)
{
    return (unsigned)__syscall1(SYS_ALARM, seconds);
}

int pause(void)
{
    return (int)__sysret(__syscall0(SYS_PAUSE));
}

int tcgetpgrp(int fd)
{
    int pg;
    return ioctl(fd, TIOCGPGRP, &pg) < 0 ? -1 : pg;
}

int tcsetpgrp(int fd, int pgid)
{
    int pg = pgid;
    return ioctl(fd, TIOCSPGRP, &pg);
}
