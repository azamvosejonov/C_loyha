/* =============================================================================
 *  user/libc/stdlib.c - exit, sonlarni o'qish, saralash
 * ============================================================================= */
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ULONG_MAX_ 0xFFFFFFFFFFFFFFFFUL
#define ATEXIT_MAX 32

static void (*atexit_fns[ATEXIT_MAX])(void);
static int atexit_count;

int atexit(void (*fn)(void))
{
    if (atexit_count == ATEXIT_MAX)
        return -1;
    atexit_fns[atexit_count++] = fn;
    return 0;
}

/* exit = atexit funksiyalari (teskari tartibda) + stdio buferlari + _exit. */
void exit(int code)
{
    while (atexit_count > 0)
        atexit_fns[--atexit_count]();
    fflush(NULL);
    _exit(code);
}

void abort(void)
{
    fflush(NULL);
    fputs("abort()\n", stderr);
    _exit(134);                         /* 128 + SIGABRT(6) - Linux shell shunday ko'rsatadi */
}

int abs(int x)
{
    return x < 0 ? -x : x;
}

/* strtoul - asosiy son o'qish funksiyasi. base = 0: "0x.." -> 16, "0.." -> 8, aks holda 10. */
unsigned long strtoul(const char *s, char **end, int base)
{
    const char *p = s;
    while (isspace(*p))
        p++;
    bool neg = false;
    if (*p == '+' || *p == '-')
        neg = *p++ == '-';
    if ((base == 0 || base == 16) && p[0] == '0' && (p[1] == 'x' || p[1] == 'X') &&
        isxdigit(p[2])) {
        p += 2;
        base = 16;
    } else if (base == 0) {
        base = *p == '0' ? 8 : 10;
    }
    unsigned long v = 0;
    const char *start = p;
    bool overflow = false;
    for (;; p++) {
        int d;
        if (isdigit(*p))
            d = *p - '0';
        else if (isalpha(*p))
            d = tolower(*p) - 'a' + 10;
        else
            break;
        if (d >= base)
            break;
        if (v > (ULONG_MAX_ - (unsigned long)d) / (unsigned long)base)
            overflow = true;
        v = v * (unsigned long)base + (unsigned long)d;
    }
    if (end)
        *end = (char *)(p == start ? s : p);   /* raqam yo'q - end = boshi (C standarti) */
    if (overflow) {
        errno = ERANGE;
        return ULONG_MAX_;
    }
    return neg ? -v : v;
}

long strtol(const char *s, char **end, int base)
{
    const char *p = s;
    while (isspace(*p))
        p++;
    bool neg = *p == '-';
    unsigned long u = strtoul(s, end, base);
    unsigned long mag = neg ? -u : u;
    if (!neg && mag > (unsigned long)INT64_MAX) {
        errno = ERANGE;
        return INT64_MAX;
    }
    if (neg && mag > (unsigned long)INT64_MAX + 1) {
        errno = ERANGE;
        return INT64_MIN;
    }
    return neg ? -(long)(mag - 1) - 1 : (long)mag;
}

int atoi(const char *s)
{
    return (int)strtol(s, NULL, 10);
}

/* ---- qsort ----
 * Oddiy va ishonchli variant: SHELL SORT (O(n^1.3) atrofida, rekursiyasiz,
 * qo'shimcha xotirasiz). Haqiqiy libc'lar introsort yoki merge sort ishlatadi.
 * Mashq: uni quicksort + insertion sort (introsort) bilan almashtiring. */
static void swap_bytes(unsigned char *a, unsigned char *b, size_t size)
{
    while (size--) {
        unsigned char t = *a;
        *a++ = *b;
        *b++ = t;
    }
}

void qsort(void *base, size_t n, size_t size, int (*cmp)(const void *, const void *))
{
    unsigned char *arr = base;
    for (size_t gap = n / 2; gap > 0; gap /= 2) {
        for (size_t i = gap; i < n; i++) {
            for (size_t j = i; j >= gap && cmp(arr + (j - gap) * size, arr + j * size) > 0;
                 j -= gap)
                swap_bytes(arr + (j - gap) * size, arr + j * size, size);
        }
    }
}
