/* =============================================================================
 *  fs/pipe.h - anonim pipe'lar (ls | grep)
 * ============================================================================= */
#pragma once

#include "fs/vfs.h"

/* O'qish va yozish uchlarini yaratish. 0 yoki -errno. */
int pipe_create(struct file **read_end, struct file **write_end);
