/* =============================================================================
 *  user/lib/ulib.c - syscall o'ramlari va satr funksiyalari
 * =============================================================================
 *
 *  SYSCALL QANDAY CHAQIRILADI (myos/abi.h ga qarang):
 *    RAX = raqam, RDI/RSI/RDX/R10/R8 = argumentlar, `syscall`, natija RAX da.
 *    `syscall` instruksiyasi RCX (qaytish manzili) va R11 (RFLAGS) ni BUZADI -
 *    shuning uchun ular "clobber" ro'yxatida. (Eski usul `int 0x80` ham
 *    ishlaydi - yadro ikkalasini qo'llaydi, lekin syscall ancha tez.)
 *
 *  Inline asm cheklovlari:
 *    "a"(n)  - n ni RAX ga        "D"(a1) - RDI ga
 *    "S"(a2) - RSI ga             "d"(a3) - RDX ga
 *    "=a"(r) - natijani RAX dan
 *    "memory" - yadro xotiramizni o'qishi/yozishi mumkin (buf), kompilyator
 *               xotira operatsiyalarini syscall atrofida ko'chirmasin.
 *  Qolgan registrlarni yadro saqlab qaytaradi.
 * ============================================================================= */
#include "ulib.h"

static inline long syscall0(long n)
{
    long r;
    __asm__ volatile("syscall" : "=a"(r) : "a"(n) : "rcx", "r11", "memory");
    return r;
}

static inline long syscall1(long n, long a1)
{
    long r;
    __asm__ volatile("syscall" : "=a"(r) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
    return r;
}

static inline long syscall2(long n, long a1, long a2)
{
    long r;
    __asm__ volatile("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2) : "rcx", "r11", "memory");
    return r;
}

static inline long syscall3(long n, long a1, long a2, long a3)
{
    long r;
    __asm__ volatile("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2), "d"(a3)
                     : "rcx", "r11", "memory");
    return r;
}

/* 4-argument R10 da (RCX emas - uni syscall instruksiyasi buzadi). */
static inline long syscall4(long n, long a1, long a2, long a3, long a4)
{
    long r;
    register long r10 __asm__("r10") = a4;
    __asm__ volatile("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
                     : "rcx", "r11", "memory");
    return r;
}

/* ---- Syscall'lar ------------------------------------------------------------ */

int fork(void)
{
    return (int)syscall0(SYS_FORK);
}

int exec(const char *path, char *const argv[])
{
    return (int)syscall2(SYS_EXEC, (long)path, (long)argv);
}

void *mmap(void *addr, size_t len, int prot, int flags)
{
    long r = syscall4(SYS_MMAP, (long)addr, (long)len, prot, flags);
    return r < 0 ? MAP_FAILED : (void *)r;
}

int munmap(void *addr, size_t len)
{
    return (int)syscall2(SYS_MUNMAP, (long)addr, (long)len);
}

int getppid(void)
{
    return (int)syscall0(SYS_GETPPID);
}

void exit(int code)
{
    syscall1(SYS_EXIT, code);
    for (;;)
        ;                               /* bu yerga hech qachon kelmaymiz */
}

long write(int fd, const void *buf, size_t len)
{
    return syscall3(SYS_WRITE, fd, (long)buf, (long)len);
}

long read(int fd, void *buf, size_t len)
{
    return syscall3(SYS_READ, fd, (long)buf, (long)len);
}

int open(const char *path)
{
    return (int)syscall1(SYS_OPEN, (long)path);
}

int close(int fd)
{
    return (int)syscall1(SYS_CLOSE, fd);
}

int spawn(const char *path, char *const argv[])
{
    return (int)syscall2(SYS_SPAWN, (long)path, (long)argv);
}

int wait(int pid, int *status, int flags)
{
    return (int)syscall3(SYS_WAIT, pid, (long)status, flags);
}

int getpid(void)
{
    return (int)syscall0(SYS_GETPID);
}

void yield(void)
{
    syscall0(SYS_YIELD);
}

void sleep_ms(uint64_t ms)
{
    syscall1(SYS_SLEEP, (long)ms);
}

void *sbrk(long increment)
{
    return (void *)syscall1(SYS_SBRK, increment);
}

int readdir(int index, struct myos_dirent *out)
{
    return (int)syscall2(SYS_READDIR, index, (long)out);
}

int meminfo(struct myos_meminfo *out)
{
    return (int)syscall1(SYS_MEMINFO, (long)out);
}

int ps(struct myos_proc_info *buf, int max)
{
    return (int)syscall2(SYS_PS, (long)buf, max);
}

int kill(int pid)
{
    return (int)syscall1(SYS_KILL, pid);
}

uint64_t uptime_ms(void)
{
    return (uint64_t)syscall0(SYS_UPTIME);
}

void shutdown(void)
{
    syscall0(SYS_SHUTDOWN);
    for (;;)
        ;
}

void reboot(void)
{
    syscall0(SYS_REBOOT);
    for (;;)
        ;
}

int pciinfo(int index, struct myos_pci_info *out)
{
    return (int)syscall2(SYS_PCIINFO, index, (long)out);
}

long dmesg(char *buf, size_t size)
{
    return syscall2(SYS_DMESG, (long)buf, (long)size);
}

int sysinfo(struct myos_sysinfo *out)
{
    return (int)syscall1(SYS_SYSINFO, (long)out);
}

/* ---- Satr va xotira funksiyalari -------------------------------------------- */

size_t strlen(const char *s)
{
    size_t n = 0;
    while (s[n])
        n++;
    return n;
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n)
{
    while (n && *a && *a == *b) {
        a++;
        b++;
        n--;
    }
    return n ? (unsigned char)*a - (unsigned char)*b : 0;
}

char *strcpy(char *dst, const char *src)
{
    char *d = dst;
    while ((*d++ = *src++))
        ;
    return dst;
}

char *strchr(const char *s, int c)
{
    for (;; s++) {
        if (*s == (char)c)
            return (char *)s;
        if (!*s)
            return NULL;
    }
}

void *memset(void *dst, int c, size_t n)
{
    unsigned char *d = dst;
    while (n--)
        *d++ = (unsigned char)c;
    return dst;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--)
        *d++ = *s++;
    return dst;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a, *y = b;
    for (size_t i = 0; i < n; i++)
        if (x[i] != y[i])
            return x[i] - y[i];
    return 0;
}

int isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int isdigit(int c)
{
    return c >= '0' && c <= '9';
}

int atoi(const char *s)
{
    int sign = 1, v = 0;
    while (isspace(*s))
        s++;
    if (*s == '-') {
        sign = -1;
        s++;
    }
    while (isdigit(*s))
        v = v * 10 + (*s++ - '0');
    return sign * v;
}

/* ---- Oddiy kiritish/chiqarish ----------------------------------------------- */

int putchar(int c)
{
    char ch = (char)c;
    return write(STDOUT, &ch, 1) == 1 ? c : -1;
}

int puts(const char *s)
{
    write(STDOUT, s, strlen(s));
    return putchar('\n');
}

int getchar(void)
{
    unsigned char c;
    return read(STDIN, &c, 1) == 1 ? c : -1;
}

int readline(char *buf, int max)
{
    int len = 0;
    for (;;) {
        int c = getchar();
        if (c < 0)
            return -1;
        if (c == '\n' || c == '\r') {
            putchar('\n');
            break;
        }
        if (c == '\b' || c == 0x7F) {   /* backspace: ekrandan ham o'chiramiz */
            if (len > 0) {
                len--;
                write(STDOUT, "\b \b", 3);
            }
            continue;
        }
        if (c < 32 || len >= max - 1)   /* boshqaruv belgilari va to'lib ketish */
            continue;
        buf[len++] = (char)c;
        putchar(c);                     /* echo: terminal biz yozganni ko'rsin */
    }
    buf[len] = '\0';
    return len;
}
