/* =============================================================================
 *  user/libc/string.c - satr va xotira funksiyalari, strerror
 * =============================================================================
 *
 *  DIQQAT: -O2 bilan GCC oddiy tsikllarni memset/memcpy chaqiruviga
 *  "optimallashtirishi" mumkin - memset ichida esa bu cheksiz rekursiya!
 *  Makefile'dagi -ffreestanding va -fno-builtin buni oldini oladi.
 * ============================================================================= */
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

size_t strlen(const char *s)
{
    size_t n = 0;
    while (s[n])
        n++;
    return n;
}

size_t strnlen(const char *s, size_t max)
{
    size_t n = 0;
    while (n < max && s[n])
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

/* strncpy - klassik tuzoq: src uzun bo'lsa dst '\0' bilan TUGAMAYDI.
 * Xavfsiz nusxalash uchun strlcpy ishlating (BSD'dan). */
char *strncpy(char *dst, const char *src, size_t n)
{
    size_t i = 0;
    for (; i < n && src[i]; i++)
        dst[i] = src[i];
    for (; i < n; i++)
        dst[i] = '\0';
    return dst;
}

/* Doim '\0' bilan tugatadi. Qaytaradi: strlen(src) - >= size bo'lsa, qirqilgan. */
size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t len = strlen(src);
    if (size) {
        size_t n = len < size - 1 ? len : size - 1;
        memcpy(dst, src, n);
        dst[n] = '\0';
    }
    return len;
}

char *strcat(char *dst, const char *src)
{
    strcpy(dst + strlen(dst), src);
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

char *strrchr(const char *s, int c)
{
    const char *last = NULL;
    for (;; s++) {
        if (*s == (char)c)
            last = s;
        if (!*s)
            return (char *)last;
    }
}

char *strstr(const char *s, const char *sub)
{
    size_t n = strlen(sub);
    for (; *s; s++)
        if (strncmp(s, sub, n) == 0)
            return (char *)s;
    return n ? NULL : (char *)s;
}

char *strdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p)
        memcpy(p, s, n);
    return p;
}

size_t strspn(const char *s, const char *accept)
{
    size_t n = 0;
    while (s[n] && strchr(accept, s[n]))
        n++;
    return n;
}

size_t strcspn(const char *s, const char *reject)
{
    size_t n = 0;
    while (s[n] && !strchr(reject, s[n]))
        n++;
    return n;
}

/* Qayta kiriluvchan (re-entrant) strtok: holat *save da, global o'zgaruvchida emas. */
char *strtok_r(char *s, const char *delim, char **save)
{
    if (!s)
        s = *save;
    s += strspn(s, delim);
    if (!*s) {
        *save = s;
        return NULL;
    }
    char *end = s + strcspn(s, delim);
    if (*end)
        *end++ = '\0';
    *save = end;
    return s;
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

/* memmove - bloklar ustma-ust tushsa ham to'g'ri: kerak bo'lsa ORQADAN nusxalaydi. */
void *memmove(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    if (d < s || d >= s + n) {
        while (n--)
            *d++ = *s++;
    } else {
        while (n--)
            d[n] = s[n];
    }
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

void *memchr(const void *s, int c, size_t n)
{
    const unsigned char *p = s;
    for (size_t i = 0; i < n; i++)
        if (p[i] == (unsigned char)c)
            return (void *)(p + i);
    return NULL;
}

/* ---- Xato matnlari ---- */

static const char *const messages[] = {
    [0] = "Muvaffaqiyat",
    [EPERM] = "Amalga ruxsat yo'q",
    [ENOENT] = "Bunday fayl yoki papka yo'q",
    [ESRCH] = "Bunday jarayon yo'q",
    [EINTR] = "Uzildi",
    [EIO] = "Kiritish/chiqarish xatosi",
    [ENXIO] = "Bunday qurilma yoki manzil yo'q",
    [E2BIG] = "Argumentlar ro'yxati juda uzun",
    [ENOEXEC] = "Bajariladigan fayl formati noto'g'ri",
    [EBADF] = "Noto'g'ri fayl deskriptori",
    [ECHILD] = "Bola jarayonlar yo'q",
    [EAGAIN] = "Resurs vaqtincha mavjud emas",
    [ENOMEM] = "Xotira yetmadi",
    [EACCES] = "Ruxsat berilmagan",
    [EFAULT] = "Noto'g'ri manzil",
    [EBUSY] = "Qurilma yoki resurs band",
    [EEXIST] = "Fayl allaqachon mavjud",
    [EXDEV] = "Qurilmalararo havola",
    [ENODEV] = "Bunday qurilma yo'q",
    [ENOTDIR] = "Papka emas",
    [EISDIR] = "Bu papka",
    [EINVAL] = "Noto'g'ri argument",
    [ENFILE] = "Tizimda ochiq fayllar juda ko'p",
    [EMFILE] = "Ochiq fayllar juda ko'p",
    [ENOTTY] = "Terminal emas",
    [EFBIG] = "Fayl juda katta",
    [ENOSPC] = "Qurilmada joy qolmadi",
    [ESPIPE] = "Noto'g'ri siljitish (pipe)",
    [EROFS] = "Faqat o'qish uchun fayl tizimi",
    [EMLINK] = "Havolalar juda ko'p",
    [EPIPE] = "Pipe uzildi",
    [ERANGE] = "Natija juda katta",
    [ENAMETOOLONG] = "Fayl nomi juda uzun",
    [ENOSYS] = "Funksiya amalga oshirilmagan",
    [ENOTEMPTY] = "Papka bo'sh emas",
};

char *strerror(int err)
{
    if (err >= 0 && err < (int)(sizeof(messages) / sizeof(messages[0])) && messages[err])
        return (char *)messages[err];
    return "Noma'lum xato";
}

/* ---- Signal nomlari (bash xabarlari bilan bir xil ma'noda) ---- */

static const char *const signames[NSIG] = {
    [SIGHUP] = "Terminal uzildi",
    [SIGINT] = "Uzildi",
    [SIGQUIT] = "Chiqish",
    [SIGILL] = "Noto'g'ri instruksiya",
    [SIGTRAP] = "Trace/breakpoint",
    [SIGABRT] = "Abort qilindi",
    [SIGBUS] = "Shina xatosi",
    [SIGFPE] = "Suzuvchi nuqta xatosi",
    [SIGKILL] = "O'ldirildi",
    [SIGUSR1] = "Foydalanuvchi signali 1",
    [SIGSEGV] = "Segmentation fault",
    [SIGUSR2] = "Foydalanuvchi signali 2",
    [SIGPIPE] = "Pipe uzildi",
    [SIGALRM] = "Budilnik",
    [SIGTERM] = "Tugatildi",
    [SIGCHLD] = "Bola holati o'zgardi",
    [SIGCONT] = "Davom etdi",
    [SIGSTOP] = "To'xtatildi (signal)",
    [SIGTSTP] = "To'xtatildi",
    [SIGTTIN] = "To'xtatildi (terminal kiritish)",
    [SIGTTOU] = "To'xtatildi (terminal chiqarish)",
    [SIGWINCH] = "Oyna o'lchami o'zgardi",
};

char *strsignal(int sig)
{
    if (sig > 0 && sig < NSIG && signames[sig])
        return (char *)signames[sig];
    return "Noma'lum signal";
}
