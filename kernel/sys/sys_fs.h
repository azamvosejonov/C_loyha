/* =============================================================================
 *  sys/sys_fs.h - fayl tizimi syscall'lari
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stdint.h>

struct file;

int64_t sys_fs_dispatch(uint64_t nr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                        bool *handled);
/* Bo'sh fd raqamini topib, f ni o'rnatish. -EMFILE yoki fd. */
int fd_install(struct file *f);
