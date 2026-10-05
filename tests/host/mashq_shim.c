/* tests/host/mashq_shim.c - host testi uchun "yadro" o'rinbosarlari: sbrk (statik maydon ustida), abort,
 * fprintf/stderr (malloc xabari uchun), memset/memcpy, console_write (kprintf uchun). */
#include <setjmp.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

jmp_buf abort_nuqta;
int abort_kutilmoqda;

static _Alignas(4096) unsigned char maydon[64 << 20];   /* 64 MB "heap" */
static size_t brk_joy;

void *lab_sbrk(long n)
{
    if ((n > 0 && brk_joy + (size_t)n > sizeof(maydon)) || (n < 0 && (size_t)-n > brk_joy))
        return (void *)-1;
    void *eski = maydon + brk_joy;
    brk_joy += (size_t)n;
    return eski;
}

void lab_abort(void)
{
    if (abort_kutilmoqda)
        longjmp(abort_nuqta, 1);
    abort();
}

void *lab_stderr;
int lab_fprintf(void *f, const char *fmt, ...)
{
    (void)f;
    (void)fmt;
    return 0;
}

void *lab_memset(void *d, int c, size_t n) { return memset(d, c, n); }
void *lab_memcpy(void *d, const void *s, size_t n) { return memcpy(d, s, n); }
void lab_console_write(const char *s, size_t n) { (void)s; (void)n; }
