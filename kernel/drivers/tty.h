/* =============================================================================
 *  drivers/tty.h - terminal (TTY) va uning "line discipline"i
 * ============================================================================= */
#pragma once

#include <stdbool.h>

#include "fs/vfs.h"

/* /dev/console ning file_ops jadvali (devfs uni MAJOR_TTY ga ro'yxatdan o'tkazadi). */
extern const struct file_ops tty_fops;

/* Klaviatura uzilishidan (console_input_char): Ctrl-C/Ctrl-\\/Ctrl-Z bo'lsa -
 * oldingi plan guruhiga signal. Qaytaradi: true - belgini buferga qo'ymaslik
 * (xom rejim); kanonik rejimda belgi qatorni bekor qilish uchun buferga tushadi. */
bool tty_input_signal(char c);
