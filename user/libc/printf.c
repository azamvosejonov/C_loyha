/* =============================================================================
 *  user/libc/printf.c - printf oilasining FORMAT DVIGATELI
 * =============================================================================
 *
 *  printf, fprintf, snprintf, dprintf - hammasi bitta __format() ni
 *  chaqiradi, faqat "belgini qayerga qo'yish" funksiyasi (emit) har xil:
 *      printf/fprintf -> FILE buferiga (stdio.c)
 *      snprintf       -> satrga (quyida)
 *  Yadrodagi kprintf.c ham shu g'oyada qurilgan.
 *
 *  Format: %[bayroqlar][kenglik][.aniqlik][uzunlik]tur
 *      bayroqlar: '-' chapga, '0' nol bilan to'ldirish, '+' doim ishora, ' '
 *      kenglik:   son yoki '*' (argumentdan)
 *      aniqlik:   %.3s - ko'pi bilan 3 belgi; %.5d - kamida 5 raqam
 *      uzunlik:   l, ll, z (hammasi 64 bit - x86-64 da long = 64 bit)
 *      tur:       d i u x X o p s c %
 * ============================================================================= */
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stdio_impl.h"

struct spec {
    bool left, zero, plus, space;
    int width;
    int prec;                           /* -1 = berilmagan */
};

static void pad(emit_fn emit, void *ctx, char c, int n, int *count)
{
    for (; n > 0; n--, (*count)++)
        emit(c, ctx);
}

static void emit_number(emit_fn emit, void *ctx, uint64_t v, bool neg, unsigned base,
                        bool upper, const struct spec *sp, int *count)
{
    /* >>> LAB emit_number - vazifa: labs/README.md */
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[24];
    int len = 0;
    if (v || sp->prec != 0) {           /* printf("%.0d", 0) - hech narsa chiqarmaydi (C standarti) */
        do {
            tmp[len++] = digits[v % base];
            v /= base;
        } while (v);
    }
    char sign = neg ? '-' : sp->plus ? '+' : sp->space ? ' ' : 0;
    int zeros = sp->prec > len ? sp->prec - len : 0;
    int total = len + zeros + (sign ? 1 : 0);
    int padn = sp->width > total ? sp->width - total : 0;
    bool zero_pad = sp->zero && !sp->left && sp->prec < 0;

    if (!sp->left && !zero_pad)
        pad(emit, ctx, ' ', padn, count);
    if (sign)
        emit(sign, ctx), (*count)++;
    if (zero_pad)
        pad(emit, ctx, '0', padn, count);
    pad(emit, ctx, '0', zeros, count);
    while (len)
        emit(tmp[--len], ctx), (*count)++;
    if (sp->left)
        pad(emit, ctx, ' ', padn, count);
    /* <<< LAB emit_number */
}

int __format(emit_fn emit, void *ctx, const char *fmt, va_list ap)
{
    int count = 0;
    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            emit(*fmt, ctx);
            count++;
            continue;
        }
        fmt++;
        struct spec sp = { .prec = -1 };
        for (;; fmt++) {
            if (*fmt == '-')
                sp.left = true;
            else if (*fmt == '0')
                sp.zero = true;
            else if (*fmt == '+')
                sp.plus = true;
            else if (*fmt == ' ')
                sp.space = true;
            else
                break;
        }
        if (*fmt == '*') {
            sp.width = va_arg(ap, int);
            if (sp.width < 0) {
                sp.left = true;
                sp.width = -sp.width;
            }
            fmt++;
        } else {
            while (*fmt >= '0' && *fmt <= '9')
                sp.width = sp.width * 10 + (*fmt++ - '0');
        }
        if (*fmt == '.') {
            fmt++;
            sp.prec = 0;
            if (*fmt == '*') {
                sp.prec = va_arg(ap, int);
                fmt++;
            } else {
                while (*fmt >= '0' && *fmt <= '9')
                    sp.prec = sp.prec * 10 + (*fmt++ - '0');
            }
        }
        bool is_long = false;
        while (*fmt == 'l' || *fmt == 'z' || *fmt == 'j' || *fmt == 't') {
            is_long = true;
            fmt++;
        }
        switch (*fmt) {
        case 'd':
        case 'i': {
            int64_t v = is_long ? va_arg(ap, int64_t) : va_arg(ap, int);
            uint64_t mag = v < 0 ? (uint64_t)0 - (uint64_t)v : (uint64_t)v;
            emit_number(emit, ctx, mag, v < 0, 10, false, &sp, &count);
            break;
        }
        case 'u':
        case 'x':
        case 'X':
        case 'o': {
            uint64_t v = is_long ? va_arg(ap, uint64_t) : va_arg(ap, unsigned);
            unsigned base = *fmt == 'u' ? 10 : *fmt == 'o' ? 8 : 16;
            sp.plus = sp.space = false;
            emit_number(emit, ctx, v, false, base, *fmt == 'X', &sp, &count);
            break;
        }
        case 'p':
            emit('0', ctx);
            emit('x', ctx);
            count += 2;
            sp = (struct spec){ .prec = -1 };
            emit_number(emit, ctx, (uint64_t)va_arg(ap, void *), false, 16, false, &sp, &count);
            break;
        case 's': {
            const char *s = va_arg(ap, const char *);
            if (!s)
                s = "(null)";
            int len = (int)(sp.prec >= 0 ? strnlen(s, (size_t)sp.prec) : strlen(s));
            int padn = sp.width > len ? sp.width - len : 0;
            if (!sp.left)
                pad(emit, ctx, ' ', padn, &count);
            for (int i = 0; i < len; i++, count++)
                emit(s[i], ctx);
            if (sp.left)
                pad(emit, ctx, ' ', padn, &count);
            break;
        }
        case 'c': {
            int padn = sp.width > 1 ? sp.width - 1 : 0;
            if (!sp.left)
                pad(emit, ctx, ' ', padn, &count);
            emit((char)va_arg(ap, int), ctx);
            count++;
            if (sp.left)
                pad(emit, ctx, ' ', padn, &count);
            break;
        }
        case '%':
            emit('%', ctx);
            count++;
            break;
        case '\0':
            return count;
        default:                        /* noma'lum - o'zini chiqaramiz, xatoni ko'rish oson */
            emit('%', ctx);
            emit(*fmt, ctx);
            count += 2;
        }
    }
    return count;
}

/* ---- snprintf: satrga yozish ---- */

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
    s->pos++;                           /* sig'masa ham sanaymiz: C standarti shuni talab qiladi */
}

/* Qaytaradi: bufer CHEKSIZ bo'lganda yoziladigan uzunlik. >= size bo'lsa - qirqilgan. */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
    struct str_buf s = { buf, size, 0 };
    int n = __format(emit_str, &s, fmt, ap);
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

/* sprintf - bufer hajmini bilmaydi: XAVFLI (buffer overflow manbai). Faqat moslik uchun. */
int sprintf(char *buf, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, SIZE_MAX, fmt, ap);
    va_end(ap);
    return n;
}
