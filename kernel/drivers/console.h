/* =============================================================================
 *  drivers/console.h - "konsol": ekran + serial port ustidagi umumiy qatlam
 * ============================================================================= */
#pragma once

#include <stddef.h>

void console_init(void);
/* Bitta belgini VGA ekranga ham, serial portga ham chiqaradi. */
void console_putc(char c);
void console_write(const char *s, size_t len);

/* ---- Kiritish (input) ----
 * Klaviatura va serial port uzilish handlerlari belgini shu yerga beradi. */
void console_input_char(char c);
/* Serial port orqali kiritishni yoqish (IRQ4). Uzilishlar sozlangandan keyin. */
void console_enable_serial_input(void);
/* Bitta belgi kelguncha KUTADI va uni qaytaradi. */
char console_getc(void);
