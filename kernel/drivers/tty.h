/* =============================================================================
 *  drivers/tty.h - terminal (TTY) va uning "line discipline"i
 * ============================================================================= */
#pragma once

#include "fs/vfs.h"

/* /dev/console ning file_ops jadvali (devfs uni MAJOR_TTY ga ro'yxatdan o'tkazadi). */
extern const struct file_ops tty_fops;
