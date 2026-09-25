/* =============================================================================
 *  user/libc/dirent.c - opendir/readdir/closedir (getdents ustida)
 * =============================================================================
 *
 *  Yadroda "papkani o'qish" uchun bitta syscall bor: getdents(fd, buf, max) -
 *  bir chaqiruvda bir NECHTA yozuv qaytaradi (syscall'lar soni kamayadi).
 *  readdir() esa dasturchiga bittadan beradi: yozuvlarni DIR ichidagi
 *  buferda saqlab, tugaganda yana getdents qiladi. Linux'da ham xuddi shunday.
 * ============================================================================= */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "myos.h"

#define BATCH 16

struct DIR {
    int fd;
    int count;                          /* buferdagi yozuvlar */
    int pos;                            /* keyingi beriladigani */
    struct myos_dirent ents[BATCH];
};

DIR *opendir(const char *path)
{
    int fd = open(path, O_RDONLY | O_DIRECTORY);
    if (fd < 0)
        return NULL;
    DIR *d = malloc(sizeof(*d));
    if (!d) {
        close(fd);
        errno = ENOMEM;
        return NULL;
    }
    d->fd = fd;
    d->count = d->pos = 0;
    return d;
}

struct dirent *readdir(DIR *d)
{
    if (d->pos == d->count) {
        int n = getdents(d->fd, d->ents, BATCH);
        if (n <= 0)
            return NULL;                /* 0 = tugadi, -1 = xato (errno o'rnatilgan) */
        d->count = n;
        d->pos = 0;
    }
    return &d->ents[d->pos++];
}

int closedir(DIR *d)
{
    int r = close(d->fd);
    free(d);
    return r;
}
