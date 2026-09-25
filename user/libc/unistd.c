/* =============================================================================
 *  user/libc/unistd.c - POSIX tizim chaqiruvlari: har biri yupqa o'ram
 * =============================================================================
 *
 *  Har bir funksiya: argumentlarni registrlarga -> syscall -> __sysret()
 *  (xato bo'lsa errno ni o'rnatib -1 qaytaradi). glibc/musl'da ham bular
 *  deyarli shunday ko'rinadi.
 * ============================================================================= */
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "myos.h"
#include "syscall.h"

int errno;

/* ---- Fayllar ---- */

int open(const char *path, int flags, ...)
{
    unsigned mode = 0;
    if (flags & O_CREAT) {              /* mode faqat O_CREAT bilan beriladi (POSIX) */
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, unsigned);
        va_end(ap);
    }
    return (int)__sysret(__syscall3(SYS_OPEN, (long)path, flags, mode));
}

int creat(const char *path, unsigned mode)
{
    return open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

int close(int fd)
{
    return (int)__sysret(__syscall1(SYS_CLOSE, fd));
}

ssize_t read(int fd, void *buf, size_t len)
{
    return __sysret(__syscall3(SYS_READ, fd, (long)buf, (long)len));
}

ssize_t write(int fd, const void *buf, size_t len)
{
    return __sysret(__syscall3(SYS_WRITE, fd, (long)buf, (long)len));
}

off_t lseek(int fd, off_t off, int whence)
{
    return __sysret(__syscall3(SYS_LSEEK, fd, off, whence));
}

int dup(int fd)
{
    return (int)__sysret(__syscall1(SYS_DUP, fd));
}

int dup2(int oldfd, int newfd)
{
    return (int)__sysret(__syscall2(SYS_DUP2, oldfd, newfd));
}

int pipe(int fds[2])
{
    return (int)__sysret(__syscall1(SYS_PIPE, (long)fds));
}

int stat(const char *path, struct myos_stat *st)
{
    return (int)__sysret(__syscall2(SYS_STAT, (long)path, (long)st));
}

int fstat(int fd, struct myos_stat *st)
{
    return (int)__sysret(__syscall2(SYS_FSTAT, fd, (long)st));
}

int mkdir(const char *path, unsigned mode)
{
    return (int)__sysret(__syscall2(SYS_MKDIR, (long)path, mode));
}

int rmdir(const char *path)
{
    return (int)__sysret(__syscall1(SYS_RMDIR, (long)path));
}

int unlink(const char *path)
{
    return (int)__sysret(__syscall1(SYS_UNLINK, (long)path));
}

int rename(const char *from, const char *to)
{
    return (int)__sysret(__syscall2(SYS_RENAME, (long)from, (long)to));
}

int chdir(const char *path)
{
    return (int)__sysret(__syscall1(SYS_CHDIR, (long)path));
}

char *getcwd(char *buf, size_t size)
{
    return __sysret(__syscall2(SYS_GETCWD, (long)buf, (long)size)) < 0 ? NULL : buf;
}

int ftruncate(int fd, off_t size)
{
    return (int)__sysret(__syscall2(SYS_FTRUNCATE, fd, size));
}

void sync(void)
{
    __syscall0(SYS_SYNC);
}

int getdents(int fd, struct myos_dirent *buf, int max)
{
    return (int)__sysret(__syscall3(SYS_GETDENTS, fd, (long)buf, max));
}

int mount(const char *source, const char *target, const char *fstype)
{
    return (int)__sysret(__syscall3(SYS_MOUNT, (long)source, (long)target, (long)fstype));
}

int umount(const char *target)
{
    return (int)__sysret(__syscall1(SYS_UMOUNT, (long)target));
}

int ioctl(int fd, unsigned long cmd, ...)
{
    va_list ap;
    va_start(ap, cmd);
    long arg = va_arg(ap, long);
    va_end(ap);
    return (int)__sysret(__syscall3(SYS_IOCTL, fd, (long)cmd, arg));
}

/* ---- Terminal ---- */

int tcgetattr(int fd, struct myos_termios *t)
{
    return ioctl(fd, TCGETS, t);
}

int tcsetattr(int fd, int action, const struct myos_termios *t)
{
    (void)action;                       /* hozircha faqat TCSANOW */
    return ioctl(fd, TCSETS, t);
}

/* "Bu fd terminalmi?" - faqat terminal TCGETS ni tushunadi. */
int isatty(int fd)
{
    struct myos_termios t;
    int saved = errno;
    int r = tcgetattr(fd, &t) == 0;
    errno = saved;
    return r;
}

/* ---- Jarayonlar ---- */

pid_t fork(void)
{
    /* stdio buferidagi hali yozilmagan ma'lumot bolaga ham nusxalanadi va
     * IKKI MARTA chiqadi. Shuning uchun fork'dan oldin buferlarni bo'shatamiz.
     * (glibc buni qilmaydi - u yerda dasturchi o'zi fflush qilishi kerak.) */
    fflush(NULL);
    return (pid_t)__sysret(__syscall0(SYS_FORK));
}

int execv(const char *path, char *const argv[])
{
    fflush(NULL);
    return (int)__sysret(__syscall2(SYS_EXEC, (long)path, (long)argv));
}

int spawn(const char *path, char *const argv[])
{
    fflush(NULL);
    return (int)__sysret(__syscall2(SYS_SPAWN, (long)path, (long)argv));
}

int waitpid(int pid, int *status, int flags)
{
    return (int)__sysret(__syscall3(SYS_WAIT, pid, (long)status, flags));
}

int wait(int *status)
{
    return waitpid(-1, status, 0);
}

pid_t getpid(void)
{
    return (pid_t)__syscall0(SYS_GETPID);
}

pid_t getppid(void)
{
    return (pid_t)__syscall0(SYS_GETPPID);
}

int kill(int pid, int sig)
{
    return (int)__sysret(__syscall2(SYS_KILL, pid, sig));
}

void _exit(int code)
{
    __syscall1(SYS_EXIT, code);
    __builtin_unreachable();
}

/* ---- Xotira ---- */

void *sbrk(long increment)
{
    long r = __syscall1(SYS_SBRK, increment);
    if (r < 0 && r >= -4095) {          /* manzil manfiy bo'lolmaydi - demak xato */
        errno = (int)-r;
        return (void *)-1;
    }
    return (void *)r;
}

void *mmap(void *addr, size_t len, int prot, int flags, int fd, long off)
{
    if (fd != -1 || off != 0) {         /* fayl xaritalash hali yo'q */
        errno = ENODEV;
        return MAP_FAILED;
    }
    long r = __syscall4(SYS_MMAP, (long)addr, (long)len, prot, flags);
    if (r < 0 && r >= -4095) {
        errno = (int)-r;
        return MAP_FAILED;
    }
    return (void *)r;
}

int munmap(void *addr, size_t len)
{
    return (int)__sysret(__syscall2(SYS_MUNMAP, (long)addr, (long)len));
}

/* ---- Vaqt ---- */

time_t time(time_t *out)
{
    time_t t = __syscall0(SYS_TIME);
    if (out)
        *out = t;
    return t;
}

/* Qaytaradi: signal uzgan bo'lsa - qolgan millisekundlar, aks holda 0. */
uint64_t sleep_ms(uint64_t ms)
{
    long r = __syscall1(SYS_SLEEP, (long)ms);
    return r == -EINTR ? ms : 0;        /* (yadro qolganini argumentga yozadi, bizda aniq qiymat yo'q) */
}

unsigned sleep(unsigned seconds)
{
    return sleep_ms((uint64_t)seconds * 1000) ? 1 : 0;  /* POSIX: uzilsa - noldan katta */
}

int usleep(unsigned long usec)
{
    if (sleep_ms((usec + 999) / 1000)) {
        errno = EINTR;
        return -1;
    }
    return 0;
}

uint64_t uptime_ms(void)
{
    return (uint64_t)__syscall0(SYS_UPTIME);
}

void yield(void)
{
    __syscall0(SYS_YIELD);
}

/* ---- MyOS tizim ma'lumotlari ---- */

int meminfo(struct myos_meminfo *out)
{
    return (int)__sysret(__syscall1(SYS_MEMINFO, (long)out));
}

int ps(struct myos_proc_info *buf, int max)
{
    return (int)__sysret(__syscall2(SYS_PS, (long)buf, max));
}

int pciinfo(int index, struct myos_pci_info *out)
{
    return (int)__sysret(__syscall2(SYS_PCIINFO, index, (long)out));
}

long dmesg(char *buf, size_t size)
{
    return __sysret(__syscall2(SYS_DMESG, (long)buf, (long)size));
}

int sysinfo(struct myos_sysinfo *out)
{
    return (int)__sysret(__syscall1(SYS_SYSINFO, (long)out));
}

void shutdown(void)
{
    sync();
    __syscall0(SYS_SHUTDOWN);
    __builtin_unreachable();
}

void reboot(void)
{
    sync();
    __syscall0(SYS_REBOOT);
    __builtin_unreachable();
}
