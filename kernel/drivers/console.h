/* =============================================================================
 *  drivers/console.h - "konsol": ekran + serial port ustidagi umumiy qatlam
 * ============================================================================= */
#pragma once

#include <stddef.h>

void console_init(void);
/* Bitta belgini VGA ekranga ham, serial portga ham chiqaradi. */
void console_putc(char c);
void console_write(const char *s, size_t len);
