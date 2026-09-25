/* =============================================================================
 *  fs/file.h - ochiq fayl obyektlari (fayl deskriptorlari ortidagi narsa)
 * ============================================================================= */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "fs/tarfs.h"

enum file_type {
    FILE_CONSOLE,                       /* klaviatura/ekran (fd 0, 1, 2) */
    FILE_TAR,                           /* initrd dagi fayl */
};

struct file {
    enum file_type type;
    int refcount;                       /* nechta fd bu obyektga ko'rsatadi (fork, dup) */
    const struct tar_file *tar;         /* FILE_TAR uchun */
    size_t offset;                      /* keyingi read() qayerdan o'qiydi */
};

/* Konsol - bitta umumiy statik obyekt (yopilmaydi). */
struct file *file_console(void);
/* initrd dan faylni ochish. Topilmasa NULL. */
struct file *file_open(const char *path);
void file_close(struct file *f);
/* Yana bitta ko'rsatkich (fork/dup): refcount++ */
struct file *file_dup(struct file *f);

/* buf - chaqiruvchi tekshirgan (yadro yoki tasdiqlangan user) bufer.
 * Qaytaradi: baytlar soni, 0 = fayl oxiri, manfiy = xato. */
long file_read(struct file *f, void *buf, size_t len);
long file_write(struct file *f, const void *buf, size_t len);
