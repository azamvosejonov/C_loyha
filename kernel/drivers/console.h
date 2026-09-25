/* =============================================================================
 *  drivers/console.h - konsol: log + serial + ekran (VGA matn yoki framebuffer)
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stddef.h>

/* CGA/VGA 16 rangli palitra - ikkala ekran turi ham shu raqamlarni tushunadi. */
enum color {
    COLOR_BLACK = 0, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN, COLOR_RED, COLOR_MAGENTA,
    COLOR_BROWN, COLOR_LIGHT_GREY, COLOR_DARK_GREY, COLOR_LIGHT_BLUE, COLOR_LIGHT_GREEN,
    COLOR_LIGHT_CYAN, COLOR_LIGHT_RED, COLOR_LIGHT_MAGENTA, COLOR_YELLOW, COLOR_WHITE,
};

/* Ekran drayveri interfeysi ("backend"). Har bir ekran turi shu funksiyalarni beradi.
 * Bu C tilida "polimorfizm": funksiya ko'rsatkichlari jadvali (C++ dagi vtable). */
struct screen_ops {
    const char *name;
    void (*putc)(char c);
    void (*set_color)(enum color fg, enum color bg);
    void (*clear)(void);
};

/* Faqat serial port bilan (xotira menejeri hali yo'q paytda). */
void console_init_early(void);
/* Ekran tayyor: log tarixini unga qayta chiqaramiz va ulaymiz. */
void console_attach_screen(const struct screen_ops *ops);

void console_putc(char c);
void console_write(const char *s, size_t len);
void console_set_color(enum color fg, enum color bg);
void console_clear(void);

/* ---- Kiritish (input) ---- */
void console_input_char(char c);
void console_enable_serial_input(void);
int console_getc(void);
bool console_input_available(void);
