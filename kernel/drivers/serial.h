/* =============================================================================
 *  drivers/serial.h - COM1 serial port (UART 16550) drayveri
 * ============================================================================= */
#pragma once

#include <stdbool.h>

void serial_init(void);
void serial_putc(char c);
/* Qabul qilingan bayt bormi? */
bool serial_has_data(void);
/* Bitta baytni o'qish (faqat serial_has_data() true bo'lsa chaqiring). */
char serial_read_byte(void);
/* Ma'lumot kelganda IRQ4 uzilishini yoqish (2-bosqichdan keyin chaqiriladi). */
void serial_enable_rx_interrupt(void);
