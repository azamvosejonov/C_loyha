/* =============================================================================
 *  drivers/console.h - konsol: log + serial + ekran (VGA matn yoki framebuffer)
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* CGA/VGA 16 rangli palitra - ikkala ekran turi ham shu raqamlarni tushunadi. */
enum color {
    COLOR_BLACK = 0, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN, COLOR_RED, COLOR_MAGENTA,
    COLOR_BROWN, COLOR_LIGHT_GREY, COLOR_DARK_GREY, COLOR_LIGHT_BLUE, COLOR_LIGHT_GREEN,
    COLOR_LIGHT_CYAN, COLOR_LIGHT_RED, COLOR_LIGHT_MAGENTA, COLOR_YELLOW, COLOR_WHITE,
};

/* Ekran drayveri interfeysi ("backend"). Har bir ekran turi shu funksiyalarni beradi.
 * Bu C tilida "polimorfizm": funksiya ko'rsatkichlari jadvali (C++ dagi vtable).
 * Terminal mantig'i (kursor, escape ketma-ketliklari, aylantirish) - drivers/vt.c da;
 * drayver faqat katakchani chizishni biladi. */
struct screen_ops {
    const char *name;
    /* (x, y) katakka Unicode belgini attr ranglarida chizish (past 4 bit - harf, yuqori - fon). */
    void (*draw)(unsigned x, unsigned y, uint16_t cp, uint8_t attr);
    /* Apparat kursori (VGA). NULL - vt kursorni o'zi chizadi (teskari rangli katak). */
    void (*cursor)(unsigned x, unsigned y, bool visible);
    void (*get_size)(unsigned *cols, unsigned *rows);  /* belgilarda */
};

/* Faqat serial port bilan (xotira menejeri hali yo'q paytda). */
void console_init_early(void);
/* Ekran tayyor: log tarixini unga qayta chiqaramiz va ulaymiz. */
void console_attach_screen(const struct screen_ops *ops);

void console_putc(char c);
void console_write(const char *s, size_t len);
/* Terminal (tty) chiqishi: ekran + serial, lekin yadro logiga (dmesg) YOZILMAYDI -
 * dmesg faqat yadro xabarlari uchun (Linux kabi). */
void console_write_tty(const char *s, size_t len);
void console_set_color(enum color fg, enum color bg);
void console_clear(void);
/* Yadro logining (dmesg) oxirgi qismini nusxalash. */
size_t console_read_log(char *buf, size_t size);
/* Ekran o'lchami (belgilarda). Ekran yo'q bo'lsa - 80x25 (serial terminal). */
void console_get_size(unsigned *cols, unsigned *rows);

/* ---- Kiritish (input) ---- */
void console_input_char(char c);
/* Kiritish navbatini tozalash (Ctrl-C: oldindan terilganlar bekor qilinadi). */
void console_input_flush(void);
void console_enable_serial_input(void);
int console_getc(void);
bool console_input_available(void);
