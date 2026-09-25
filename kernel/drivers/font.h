/* =============================================================================
 *  drivers/font.h - ekran shrifti (framebuffer konsoli uchun)
 * ============================================================================= */
#pragma once

#include <stdint.h>

struct font_map {
    uint16_t cp;                        /* Unicode kod nuqtasi (BMP) */
    uint16_t glyph;
};

extern const unsigned font8x16_count;
extern const unsigned font8x16_map_len;
extern const uint16_t font8x16_fallback;
extern const uint8_t font8x16_glyphs[][16];
extern const uint16_t font8x16_ascii[128];
extern const struct font_map font8x16_map[];

/* Unicode belgining glif raqami (yo'q bo'lsa - '?'). */
static inline uint16_t font_glyph(uint32_t cp)
{
    if (cp < 128)
        return font8x16_ascii[cp];
    unsigned lo = 0, hi = font8x16_map_len;
    while (lo < hi) {                   /* ikkilik qidiruv: ~10 qadam */
        unsigned mid = (lo + hi) / 2;
        if (font8x16_map[mid].cp == cp)
            return font8x16_map[mid].glyph;
        if (font8x16_map[mid].cp < cp)
            lo = mid + 1;
        else
            hi = mid;
    }
    return font8x16_fallback;
}
