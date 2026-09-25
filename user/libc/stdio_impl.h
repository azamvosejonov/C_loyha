/* user/libc/stdio_impl.h - stdio ichki tuzilmalari (dasturlar ko'rmaydi) */
#pragma once
#include <stdarg.h>
#include <stddef.h>

typedef void (*emit_fn)(char c, void *ctx);
int __format(emit_fn emit, void *ctx, const char *fmt, va_list ap);

#define F_READ   1                      /* o'qish uchun ochilgan */
#define F_WRITE  2                      /* yozish uchun ochilgan */
#define F_EOF    4                      /* fayl oxiriga yetildi */
#define F_ERR    8                      /* xato yuz berdi */
#define F_APPEND 16

#define BUF_NONE 0                      /* buferlanmagan (stderr) */
#define BUF_LINE 1                      /* qator bo'yicha (terminalga stdout) */
#define BUF_FULL 2                      /* to'liq (fayl, pipe) */
#define BUF_UNSET 3                     /* birinchi yozishda aniqlanadi */

struct FILE {
    int fd;
    int flags;
    int mode;                           /* BUF_* */
    unsigned char *buf;
    size_t size;
    size_t wlen;                        /* buferdagi YOZILMAGAN baytlar */
    size_t rpos, rlen;                  /* o'qish buferi: [rpos, rlen) hali berilmagan */
    int owns_buf;                       /* buf malloc bilan olingan - fclose bo'shatadi */
    struct FILE *next;                  /* barcha ochiq FILE'lar ro'yxati (fflush(NULL)) */
};
