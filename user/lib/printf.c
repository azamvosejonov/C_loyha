/* =============================================================================
 *  user/lib/printf.c - user rejimidagi printf
 * =============================================================================
 *
 *  Yadrodagi kprintf.c bilan bir xil g'oya (format dvigateli + "emit"
 *  funksiyasi), lekin muhim farq bor: BUFERLASH.
 *
 *  Har bir belgi uchun alohida write() syscall = har bir belgi uchun ring 3 ->
 *  ring 0 -> ring 3 o'tishi. Bu juda qimmat (yuzlab takt). Shuning uchun
 *  belgilarni 256 baytlik buferga yig'ib, bitta write() bilan yuboramiz.
 *  glibc'dagi stdio (FILE*, setvbuf) ham aynan shuni qiladi.
 *  Mashq: printf("x") ni 1000 marta chaqirib, buferli va buferli emas
 *  variantlarning tezligini uptime_ms() bilan solishtiring.
 * ============================================================================= */
#include <stdbool.h>

#include "ulib.h"

typedef void (*emit_fn)(char c, void *ctx);

static void emit_number(emit_fn emit, void *ctx, uint64_t v, bool neg, unsigned base,
                        bool upper, int width, bool zero, bool left, int *count)
{
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[24];
    int len = 0;
    do {
        tmp[len++] = digits[v % base];
        v /= base;
    } while (v);
    int total = len + (neg ? 1 : 0);
    int pad = width > total ? width - total : 0;
    if (!left && !zero)
        for (; pad > 0; pad--, (*count)++)
            emit(' ', ctx);
    if (neg)
        emit('-', ctx), (*count)++;
    if (!left && zero)
        for (; pad > 0; pad--, (*count)++)
            emit('0', ctx);
    while (len)
        emit(tmp[--len], ctx), (*count)++;
    for (; pad > 0; pad--, (*count)++)
        emit(' ', ctx);
}

static int format(emit_fn emit, void *ctx, const char *fmt, va_list ap)
{
    int count = 0;
    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            emit(*fmt, ctx);
            count++;
            continue;
        }
        fmt++;
        bool left = false, zero = false;
        for (;; fmt++) {
            if (*fmt == '-')
                left = true;
            else if (*fmt == '0')
                zero = true;
            else
                break;
        }
        int width = 0;
        while (*fmt >= '0' && *fmt <= '9')
            width = width * 10 + (*fmt++ - '0');
        bool is_long = false;
        while (*fmt == 'l' || *fmt == 'z') {
            is_long = true;
            fmt++;
        }
        switch (*fmt) {
        case 'd':
        case 'i': {
            int64_t v = is_long ? va_arg(ap, int64_t) : va_arg(ap, int);
            uint64_t mag = v < 0 ? (uint64_t)0 - (uint64_t)v : (uint64_t)v;
            emit_number(emit, ctx, mag, v < 0, 10, false, width, zero, left, &count);
            break;
        }
        case 'u':
        case 'x':
        case 'X': {
            uint64_t v = is_long ? va_arg(ap, uint64_t) : va_arg(ap, unsigned);
            emit_number(emit, ctx, v, false, *fmt == 'u' ? 10 : 16, *fmt == 'X', width,
                        zero, left, &count);
            break;
        }
        case 'p':
            emit('0', ctx);
            emit('x', ctx);
            count += 2;
            emit_number(emit, ctx, (uint64_t)va_arg(ap, void *), false, 16, false, 0, false,
                        false, &count);
            break;
        case 's': {
            const char *s = va_arg(ap, const char *);
            if (!s)
                s = "(null)";
            int len = (int)strlen(s);
            int pad = width > len ? width - len : 0;
            if (!left)
                for (; pad > 0; pad--, count++)
                    emit(' ', ctx);
            for (int i = 0; i < len; i++, count++)
                emit(s[i], ctx);
            for (; pad > 0; pad--, count++)
                emit(' ', ctx);
            break;
        }
        case 'c':
            emit((char)va_arg(ap, int), ctx);
            count++;
            break;
        case '%':
            emit('%', ctx);
            count++;
            break;
        case '\0':
            return count;
        default:
            emit('%', ctx);
            emit(*fmt, ctx);
            count += 2;
        }
    }
    return count;
}

/* ---- stdout buferi ---- */

struct out_buf {
    char data[256];
    size_t len;
};

static void flush(struct out_buf *b)
{
    if (b->len) {
        write(STDOUT, b->data, b->len);
        b->len = 0;
    }
}

static void emit_stdout(char c, void *ctx)
{
    struct out_buf *b = ctx;
    b->data[b->len++] = c;
    if (b->len == sizeof(b->data))
        flush(b);
}

int vprintf(const char *fmt, va_list ap)
{
    struct out_buf b;                   /* stekda: har bir printf o'z buferiga ega */
    b.len = 0;
    int n = format(emit_stdout, &b, fmt, ap);
    flush(&b);                          /* printf qaytgunicha hammasi chiqarilgan bo'lsin */
    return n;
}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = vprintf(fmt, ap);
    va_end(ap);
    return n;
}

/* ---- snprintf ---- */

struct str_buf {
    char *buf;
    size_t size;
    size_t pos;
};

static void emit_str(char c, void *ctx)
{
    struct str_buf *s = ctx;
    if (s->pos + 1 < s->size)
        s->buf[s->pos] = c;
    s->pos++;
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
    struct str_buf s = { buf, size, 0 };
    int n = format(emit_str, &s, fmt, ap);
    if (size)
        buf[s.pos < size ? s.pos : size - 1] = '\0';
    return n;
}

int snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return n;
}
