/* =============================================================================
 *  fs/tarfs.h - initrd.tar dan o'qiladigan oddiy, faqat-o'qish fayl tizimi
 * ============================================================================= */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "boot/multiboot.h"

struct tar_file {
    char name[100];                     /* fayl nomi ('\0' bilan tugagan) */
    const uint8_t *data;                /* ma'lumot boshi (xotirada, initrd ichida) */
    size_t size;                        /* hajmi (baytda) */
};

void tarfs_init(const struct multiboot_info *mbi);
const struct tar_file *tarfs_find(const char *name);
/* index-chi fayl (ls uchun). Yo'q bo'lsa NULL. */
const struct tar_file *tarfs_get(size_t index);
size_t tarfs_count(void);
