/* =============================================================================
 *  user/libc/stdio.c - BUFERLANGAN kiritish/chiqarish (FILE*)
 * =============================================================================
 *
 *  NEGA BUFER? Har bir write() - bu ring 3 -> ring 0 -> ring 3 o'tishi,
 *  yuzlab takt. printf("x") ni 1000 marta chaqirish 1000 ta syscall bo'lmasligi
 *  uchun belgilar avval buferga yig'iladi va keyin bitta write() bilan ketadi.
 *
 *  UCH REJIM (setvbuf):
 *    buferlanmagan  - stderr: xato xabari darhol ko'rinishi kerak (dastur
 *                     keyingi qatorda qulab tushishi mumkin).
 *    qatorli        - terminalga stdout: '\n' kelganda chiqariladi, foydalanuvchi
 *                     har bir qatorni o'z vaqtida ko'radi.
 *    to'liq         - fayl yoki pipe'ga stdout: bufer to'lganda chiqariladi
 *                     (eng tez). `ls | cat` da ls ning chiqishi shunday.
 *
 *  DIQQAT - klassik xatolar:
 *    * exit() buferlarni bo'shatadi, _exit() - YO'Q (fork'dan keyin bolada
 *      _exit ishlatiladi, aks holda otaning buferi ikki marta chiqadi).
 *    * stdin'dan o'qishdan oldin stdout bo'shatiladi - aks holda "Ismingiz: "
 *      degan so'rov ekranda ko'rinmay qoladi.
 * ============================================================================= */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "stdio_impl.h"

static unsigned char stdin_buf[BUFSIZ], stdout_buf[BUFSIZ];

static FILE std_err = { .fd = 2, .flags = F_WRITE, .mode = BUF_NONE };
static FILE std_out = { .fd = 1, .flags = F_WRITE, .mode = BUF_UNSET, .buf = stdout_buf,
                        .size = BUFSIZ, .next = &std_err };
static FILE std_in = { .fd = 0, .flags = F_READ, .mode = BUF_FULL, .buf = stdin_buf,
                       .size = BUFSIZ, .next = &std_out };

FILE *stdin = &std_in;
FILE *stdout = &std_out;
FILE *stderr = &std_err;

static FILE *all_files = &std_in;       /* ro'yxat boshi */

/* ---- Yozish ---- */

/* Buferdagi hamma narsani yadroga berish. write() qisman yozishi mumkin - takrorlaymiz. */
static int flush_write(FILE *f)
{
    size_t done = 0;
    while (done < f->wlen) {
        ssize_t n = write(f->fd, f->buf + done, f->wlen - done);
        if (n <= 0) {
            f->flags |= F_ERR;
            /* Yozilmagan qismni saqlab qolamiz (keyinroq qayta urinish mumkin). */
            memmove(f->buf, f->buf + done, f->wlen - done);
            f->wlen -= done;
            return EOF;
        }
        done += (size_t)n;
    }
    f->wlen = 0;
    return 0;
}

/* O'qish buferida ishlatilmay qolgan baytlarni "qaytarish": fayl pozitsiyasini
 * dastur haqiqatan o'qigan joyga suramiz. (Pipe/terminalda imkonsiz - tashlaymiz.) */
static void drop_read(FILE *f)
{
    if (f->rlen > f->rpos)
        lseek(f->fd, -(off_t)(f->rlen - f->rpos), SEEK_CUR);
    f->rpos = f->rlen = 0;
}

int fflush(FILE *f)
{
    if (!f) {                           /* fflush(NULL) - barcha ochiq oqimlar */
        int r = 0;
        for (FILE *p = all_files; p; p = p->next)
            if (p->wlen && fflush(p))
                r = EOF;
        return r;
    }
    if (f->rlen)
        drop_read(f);
    return f->wlen ? flush_write(f) : 0;
}

static void decide_mode(FILE *f)
{
    if (f->mode == BUF_UNSET)
        f->mode = isatty(f->fd) ? BUF_LINE : BUF_FULL;
}

static int put_byte(FILE *f, unsigned char c)
{
    decide_mode(f);
    if (f->rlen)
        drop_read(f);
    if (f->mode == BUF_NONE || !f->buf) {
        if (write(f->fd, &c, 1) != 1) {
            f->flags |= F_ERR;
            return EOF;
        }
        return c;
    }
    if (f->wlen == f->size && flush_write(f))
        return EOF;
    f->buf[f->wlen++] = c;
    if ((f->mode == BUF_LINE && c == '\n') || f->wlen == f->size)
        if (flush_write(f))
            return EOF;
    return c;
}

int fputc(int c, FILE *f)
{
    return put_byte(f, (unsigned char)c);
}

int putchar(int c)
{
    return fputc(c, stdout);
}

size_t fwrite(const void *buf, size_t size, size_t n, FILE *f)
{
    size_t total = size * n;
    if (!total)
        return 0;
    decide_mode(f);
    const unsigned char *p = buf;
    /* Buferlanmagan oqim yoki bufer hajmidan katta blok: to'g'ridan-to'g'ri yozamiz
     * (avval buferdagini - tartib buzilmasin). */
    if (f->mode == BUF_NONE || !f->buf || total >= f->size) {
        if (fflush(f))
            return 0;
        size_t done = 0;
        while (done < total) {
            ssize_t w = write(f->fd, p + done, total - done);
            if (w <= 0) {
                f->flags |= F_ERR;
                break;
            }
            done += (size_t)w;
        }
        return done / size;
    }
    for (size_t i = 0; i < total; i++)
        if (put_byte(f, p[i]) == EOF)
            return i / size;
    return n;
}

int fputs(const char *s, FILE *f)
{
    size_t len = strlen(s);
    return fwrite(s, 1, len, f) == len ? 0 : EOF;
}

int puts(const char *s)
{
    if (fputs(s, stdout) == EOF)
        return EOF;
    return fputc('\n', stdout) == EOF ? EOF : 0;
}

/* ---- printf oilasi ---- */

static void emit_file(char c, void *ctx)
{
    put_byte(ctx, (unsigned char)c);
}

int vfprintf(FILE *f, const char *fmt, va_list ap)
{
    return __format(emit_file, f, fmt, ap);
}

int fprintf(FILE *f, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = vfprintf(f, fmt, ap);
    va_end(ap);
    return n;
}

int vprintf(const char *fmt, va_list ap)
{
    return vfprintf(stdout, fmt, ap);
}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = vfprintf(stdout, fmt, ap);
    va_end(ap);
    return n;
}

/* dprintf - FILE'siz, to'g'ridan-to'g'ri fd ga (kichik stek buferi orqali). */
struct fd_buf {
    int fd;
    size_t len;
    char data[256];
};

static void emit_fd(char c, void *ctx)
{
    struct fd_buf *b = ctx;
    b->data[b->len++] = c;
    if (b->len == sizeof(b->data)) {
        write(b->fd, b->data, b->len);
        b->len = 0;
    }
}

int dprintf(int fd, const char *fmt, ...)
{
    struct fd_buf b = { .fd = fd };
    va_list ap;
    va_start(ap, fmt);
    int n = __format(emit_fd, &b, fmt, ap);
    va_end(ap);
    if (b.len)
        write(fd, b.data, b.len);
    return n;
}

void perror(const char *msg)
{
    if (msg && *msg)
        fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    else
        fprintf(stderr, "%s\n", strerror(errno));
}

/* ---- O'qish ---- */

static int refill(FILE *f)
{
    if (f->flags & (F_EOF | F_ERR))
        return EOF;
    if (f == stdin)
        fflush(stdout);                 /* so'rov (prompt) ko'rinsin */
    if (f->wlen && flush_write(f))
        return EOF;
    unsigned char one;
    unsigned char *dst = f->buf ? f->buf : &one;
    ssize_t n = read(f->fd, dst, f->buf ? f->size : 1);
    if (n <= 0) {
        f->flags |= n == 0 ? F_EOF : F_ERR;
        return EOF;
    }
    if (!f->buf)
        return one;
    f->rpos = 0;
    f->rlen = (size_t)n;
    return 0;
}

int fgetc(FILE *f)
{
    if (f->rpos < f->rlen)
        return f->buf[f->rpos++];
    int r = refill(f);
    if (r == EOF)
        return EOF;
    if (!f->buf)
        return r;
    return f->buf[f->rpos++];
}

int getchar(void)
{
    return fgetc(stdin);
}

/* Qator o'qish ('\n' bilan birga). NULL - hech narsa o'qilmadi (EOF yoki xato). */
char *fgets(char *buf, int size, FILE *f)
{
    if (size <= 0)
        return NULL;
    int i = 0;
    while (i < size - 1) {
        int c = fgetc(f);
        if (c == EOF)
            break;
        buf[i++] = (char)c;
        if (c == '\n')
            break;
    }
    if (i == 0)
        return NULL;                    /* C standarti: hech narsa o'qilmasa bufer O'ZGARMAYDI */
    buf[i] = '\0';
    return buf;
}

size_t fread(void *buf, size_t size, size_t n, FILE *f)
{
    size_t total = size * n, done = 0;
    unsigned char *p = buf;
    while (done < total) {
        if (f->rpos < f->rlen) {
            size_t k = f->rlen - f->rpos;
            if (k > total - done)
                k = total - done;
            memcpy(p + done, f->buf + f->rpos, k);
            f->rpos += k;
            done += k;
            continue;
        }
        if (total - done >= f->size || !f->buf) {
            /* Katta blok - buferni chetlab o'tamiz. */
            ssize_t r = read(f->fd, p + done, total - done);
            if (r <= 0) {
                f->flags |= r == 0 ? F_EOF : F_ERR;
                break;
            }
            done += (size_t)r;
            continue;
        }
        if (refill(f) == EOF)
            break;
    }
    return size ? done / size : 0;
}

int feof(FILE *f)
{
    return (f->flags & F_EOF) != 0;
}

int ferror(FILE *f)
{
    return (f->flags & F_ERR) != 0;
}

void clearerr(FILE *f)
{
    f->flags &= ~(F_EOF | F_ERR);
}

int fileno(FILE *f)
{
    return f->fd;
}

/* ---- Ochish/yopish ---- */

/* "r" "w" "a" "r+" "w+" "a+" -> open() bayroqlari. */
static int parse_mode(const char *mode, int *oflags, int *fflags)
{
    int plus = strchr(mode, '+') != NULL;
    switch (mode[0]) {
    case 'r':
        *oflags = plus ? O_RDWR : O_RDONLY;
        *fflags = F_READ | (plus ? F_WRITE : 0);
        return 0;
    case 'w':
        *oflags = (plus ? O_RDWR : O_WRONLY) | O_CREAT | O_TRUNC;
        *fflags = F_WRITE | (plus ? F_READ : 0);
        return 0;
    case 'a':
        *oflags = (plus ? O_RDWR : O_WRONLY) | O_CREAT | O_APPEND;
        *fflags = F_WRITE | F_APPEND | (plus ? F_READ : 0);
        return 0;
    }
    errno = EINVAL;
    return -1;
}

FILE *fdopen(int fd, const char *mode)
{
    int oflags, fflags;
    if (parse_mode(mode, &oflags, &fflags))
        return NULL;
    FILE *f = calloc(1, sizeof(*f));
    unsigned char *buf = malloc(BUFSIZ);
    if (!f || !buf) {
        free(f);
        free(buf);
        errno = ENOMEM;
        return NULL;
    }
    f->fd = fd;
    f->flags = fflags;
    f->mode = BUF_UNSET;
    f->buf = buf;
    f->size = BUFSIZ;
    f->owns_buf = 1;
    f->next = all_files;
    all_files = f;
    return f;
}

FILE *fopen(const char *path, const char *mode)
{
    int oflags, fflags;
    if (parse_mode(mode, &oflags, &fflags))
        return NULL;
    int fd = open(path, oflags, 0644);
    if (fd < 0)
        return NULL;
    FILE *f = fdopen(fd, mode);
    if (!f)
        close(fd);
    return f;
}

int fclose(FILE *f)
{
    int r = fflush(f);
    if (close(f->fd) < 0)
        r = EOF;
    for (FILE **pp = &all_files; *pp; pp = &(*pp)->next) {
        if (*pp == f) {
            *pp = f->next;
            break;
        }
    }
    if (f == stdin || f == stdout || f == stderr)
        return r;                       /* statik - bo'shatilmaydi */
    if (f->owns_buf)
        free(f->buf);
    free(f);
    return r;
}

int setvbuf(FILE *f, char *buf, int mode, size_t size)
{
    fflush(f);
    if (buf) {
        if (f->owns_buf)
            free(f->buf);
        f->buf = (unsigned char *)buf;
        f->size = size;
        f->owns_buf = 0;
    }
    f->mode = mode;
    return 0;
}

int remove(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
        return rmdir(path);
    return unlink(path);
}
