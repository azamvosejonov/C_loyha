/* =============================================================================
 *  lib/kprintf.c - formatlangan chiqarish (printf) ni noldan yozish
 * =============================================================================
 *
 *  QO'LLAB-QUVVATLANADIGAN FORMATLAR:
 *    %d %i  - ishorali butun son          %u  - ishorasiz butun son
 *    %x %X  - o'n oltilik (hex)           %p  - ko'rsatkich (0x... ko'rinishida)
 *    %s     - satr                        %c  - bitta belgi
 *    %%     - '%' belgisining o'zi
 *  Modifikatorlar:
 *    l, ll  - long / long long (64-bit)   z   - size_t
 *    -      - chapga tekislash            0   - bo'sh joyni nol bilan to'ldirish
 *    raqam  - minimal kenglik (masalan %08x -> 0000beef)
 *
 *  ARXITEKTURA:
 *    Bitta "dvigatel" (format_core) bor, u har bir tayyor belgini "chiqarish
 *    funksiyasi"ga (emit) beradi. kprintf uchun emit = console_putc,
 *    ksnprintf uchun emit = buferga yozish. Kod takrorlanmaydi.
 *
 *  VA_LIST QANDAY ISHLAYDI:
 *    Funksiya o'zgaruvchan sonli argument (...) olsa, ular registrlar va stekda
 *    keladi. <stdarg.h> (kompilyator bilan keladi, glibc emas) va_arg() orqali
 *    ularni ketma-ket, BIZ AYTGAN TUR bilan olib beradi. Shuning uchun formatda
 *    %d yozib long bersangiz - noto'g'ri qiymat olasiz.
 * ============================================================================= */
#include "lib/kprintf.h"

#include <stdbool.h>
#include <stdint.h>

#include "drivers/console.h"

/* "Chiqarish" funksiyasi turi: bitta belgini qayergadir yozadi. */
typedef void (*emit_fn)(char c, void *ctx);

/* Sonni matnga aylantirib, kenglik/to'ldirish bilan chiqarish. */
static int emit_number(emit_fn emit, void *ctx, uint64_t value, bool negative,
                       unsigned base, bool upper, int width, bool zero_pad, bool left)
{
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[24];                       /* 2^64 o'nlikda 20 raqam - 24 yetarli */
    int len = 0;

    /* Raqamlarni teskari tartibda hosil qilamiz: 1234 -> "4321". */
    do {
        tmp[len++] = digits[value % base];
        value /= base;
    } while (value);

    int total = len + (negative ? 1 : 0);   /* manfiy bo'lsa '-' uchun joy */
    int pad = width > total ? width - total : 0;
    int count = 0;

    if (!left && !zero_pad)             /* "   -42": bo'shliqlar ishoradan OLDIN */
        for (; pad > 0; pad--, count++)
            emit(' ', ctx);
    if (negative) {
        emit('-', ctx);
        count++;
    }
    if (!left && zero_pad)              /* "-00042": nollar ishoradan KEYIN */
        for (; pad > 0; pad--, count++)
            emit('0', ctx);
    while (len)                         /* raqamlarni to'g'ri tartibda chiqaramiz */
        emit(tmp[--len], ctx), count++;
    for (; pad > 0; pad--, count++)     /* chapga tekislangan bo'lsa - o'ngda bo'shliq */
        emit(' ', ctx);
    return count;
}

static int format_core(emit_fn emit, void *ctx, const char *fmt, va_list ap)
{
    int count = 0;                      /* nechta belgi chiqarildi */

    for (; *fmt; fmt++) {
        if (*fmt != '%') {              /* oddiy belgi - shundayligicha */
            emit(*fmt, ctx);
            count++;
            continue;
        }
        fmt++;                          /* '%' dan keyingi belgiga o'tamiz */

        /* --- Bayroqlar --- */
        bool left = false, zero_pad = false;
        for (;; fmt++) {
            if (*fmt == '-')
                left = true;
            else if (*fmt == '0')
                zero_pad = true;
            else
                break;
        }

        /* --- Kenglik --- */
        int width = 0;
        while (*fmt >= '0' && *fmt <= '9')
            width = width * 10 + (*fmt++ - '0');

        /* --- Uzunlik modifikatori --- */
        int longness = 0;               /* 0 = int, 1 = long, 2 = long long */
        bool is_size = false;
        while (*fmt == 'l') {
            longness++;
            fmt++;
        }
        if (*fmt == 'z') {
            is_size = true;
            fmt++;
        }

        /* --- Konversiya --- */
        switch (*fmt) {
        case 'd':
        case 'i': {
            int64_t v;
            if (is_size || longness >= 1)
                v = va_arg(ap, int64_t);        /* x86-64 da long = long long = 64 bit */
            else
                v = va_arg(ap, int);            /* int kichikroq turlar ham int ga ko'tariladi */
            /* Eng kichik manfiy sonni (-2^63) musbatga aylantirib bo'lmaydi,
             * shuning uchun ishorasiz turda hisoblaymiz: 0 - v. */
            uint64_t mag = v < 0 ? (uint64_t)0 - (uint64_t)v : (uint64_t)v;
            count += emit_number(emit, ctx, mag, v < 0, 10, false, width, zero_pad, left);
            break;
        }
        case 'u':
        case 'x':
        case 'X': {
            uint64_t v;
            if (is_size || longness >= 1)
                v = va_arg(ap, uint64_t);
            else
                v = va_arg(ap, unsigned int);
            unsigned base = (*fmt == 'u') ? 10 : 16;
            count += emit_number(emit, ctx, v, false, base, *fmt == 'X', width, zero_pad, left);
            break;
        }
        case 'p': {
            uint64_t v = (uint64_t)(uintptr_t)va_arg(ap, void *);
            emit('0', ctx);
            emit('x', ctx);
            count += 2;
            count += emit_number(emit, ctx, v, false, 16, false, 16, true, false);
            break;
        }
        case 's': {
            const char *s = va_arg(ap, const char *);
            if (!s)
                s = "(null)";           /* NULL ko'rsatkichdan himoya */
            int len = 0;
            while (s[len])
                len++;
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
            emit((char)va_arg(ap, int), ctx);   /* char ham int bo'lib keladi */
            count++;
            break;
        case '%':
            emit('%', ctx);
            count++;
            break;
        case '\0':                      /* satr '%' bilan tugab qolgan */
            return count;
        default:                        /* noma'lum format - o'zini chiqaramiz */
            emit('%', ctx);
            emit(*fmt, ctx);
            count += 2;
            break;
        }
    }
    return count;
}

/* ---- kprintf: konsolga ------------------------------------------------------- */

static void emit_console(char c, void *ctx)
{
    (void)ctx;                          /* ishlatilmaydi - ogohlantirishni o'chiramiz */
    console_putc(c);
}

int kvprintf(const char *fmt, va_list ap)
{
    return format_core(emit_console, NULL, fmt, ap);
}

int kprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);                  /* fmt dan keyingi argumentlarni olishni boshlash */
    int n = kvprintf(fmt, ap);
    va_end(ap);
    return n;
}

/* ---- ksnprintf: buferga ------------------------------------------------------ */

struct buf_ctx {
    char *buf;
    size_t size;                        /* bufer hajmi */
    size_t pos;                         /* hozirgacha "yozilgan" belgilar (kesilganlari ham) */
};

static void emit_buffer(char c, void *ctx)
{
    struct buf_ctx *b = ctx;
    if (b->pos + 1 < b->size)           /* oxirgi bayt '\0' uchun qoladi */
        b->buf[b->pos] = c;
    b->pos++;
}

int kvsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
    struct buf_ctx b = { .buf = buf, .size = size, .pos = 0 };
    int n = format_core(emit_buffer, &b, fmt, ap);
    if (size)
        buf[b.pos < size ? b.pos : size - 1] = '\0';
    return n;
}

int ksnprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = kvsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return n;
}
