/* =============================================================================
 *  drivers/vt.h - virtual terminal: ANSI/VT100 escape ketma-ketliklari
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct screen_ops;

/* Ekran drayveri tayyor bo'lganda: katakchalar buferini yaratib, tozalaydi. */
bool vt_attach(const struct screen_ops *ops);
/* Bitta bayt (UTF-8 va escape ketma-ketliklari tushuniladi). console_lock ushlangan. */
void vt_putc(char c);
/* Yadro ichki xabarlari uchun rang (panic - qizil fonda oq). */
void vt_set_color(uint8_t fg, uint8_t bg);
void vt_clear(void);
/* Terminal javobi (masalan, ESC[6n -> kursor pozitsiyasi) - kiritish navbatiga
 * qo'yilishi kerak bo'lgan baytlar. Qaytaradi: nechta bayt (0 - yo'q). */
size_t vt_take_response(char *buf, size_t size);
