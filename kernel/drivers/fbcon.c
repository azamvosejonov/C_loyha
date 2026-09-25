/* =============================================================================
 *  drivers/fbcon.c - FRAMEBUFFER KONSOLI: piksellar bilan matn chizish
 * =============================================================================
 *
 *  FRAMEBUFFER NIMA:
 *    Ekran - piksellar massivi. Videokarta (UEFI GOP yoki VESA) uni oddiy xotira
 *    sifatida beradi: (x, y) piksel manzili = baza + y * pitch + x * (bpp/8).
 *    DIQQAT: pitch (qator hajmi baytda) width * 4 dan KATTA bo'lishi mumkin -
 *    videokarta qatorlarni tekislash uchun "to'ldiruvchi" baytlar qo'shadi.
 *    Ko'p boshlang'ich OS'lar shu yerda xato qiladi va ekran "qiyshayib" chiqadi.
 *
 *  MATN QANDAY CHIZILADI:
 *    Har bir belgi - 8x16 bitlik rasm (font8x16.c, Spleen shrifti: ~650 ta
 *    Unicode belgi - lotin, chegara chiziqlari, bloklar, strelkalar).
 *    1-bit = harf rangi, 0-bit = fon rangi. 1024x768 ekran = 128 x 48 katak.
 *
 *  BU DRAYVER NIMA QILMAYDI:
 *    Kursor, aylantirish, escape ketma-ketliklari - bular terminal mantig'i
 *    (drivers/vt.c). Bu yerda faqat bitta ish: "(x, y) katakka shu belgini
 *    shu ranglarda chiz". vt.c katakchalar nusxasini RAMda saqlaydi, shuning
 *    uchun framebuffer'dan hech qachon O'QILMAYDI - u write-combining rejimida
 *    xaritalangan: yozish juda tez, o'qish esa juda sekin (keshlanmaydi).
 * ============================================================================= */
#include "drivers/fbcon.h"

#include <stdint.h>

#include "drivers/font.h"
#include "mm/vmalloc.h"

#define GLYPH_W 8
#define GLYPH_H 16

static volatile uint8_t *fb;            /* framebuffer (WC xaritalangan) */
static uint32_t pitch, width, height, bytes_pp;
static uint32_t cols, rows;
static uint32_t palette[16];            /* 16 rangning framebuffer formatidagi qiymati */

/* Standart VGA palitrasi (R, G, B). */
static const uint8_t vga_rgb[16][3] = {
    {0, 0, 0},       {0, 0, 170},     {0, 170, 0},     {0, 170, 170},
    {170, 0, 0},     {170, 0, 170},   {170, 85, 0},    {170, 170, 170},
    {85, 85, 85},    {85, 85, 255},   {85, 255, 85},   {85, 255, 255},
    {255, 85, 85},   {255, 85, 255},  {255, 255, 85},  {255, 255, 255},
};

/* 8 bitli rang komponentini framebuffer'dagi o'rniga joylash. Masalan, odatiy
 * 32-bit formatda: red_pos=16, green_pos=8, blue_pos=0 -> 0x00RRGGBB. Lekin
 * ba'zi videokartalarda BGR tartib - shuning uchun pozitsiyalarni yuklovchidan olamiz. */
static uint32_t make_color(const struct boot_framebuffer *f, const uint8_t rgb[3])
{
    uint32_t r = rgb[0] >> (8 - f->red_size);
    uint32_t g = rgb[1] >> (8 - f->green_size);
    uint32_t b = rgb[2] >> (8 - f->blue_size);
    return (r << f->red_pos) | (g << f->green_pos) | (b << f->blue_pos);
}

static void fbcon_draw(unsigned cx, unsigned cy, uint16_t cp, uint8_t attr)
{
    if (cx >= cols || cy >= rows)
        return;
    const uint8_t *glyph = font8x16_glyphs[font_glyph(cp)];
    uint32_t fg = palette[attr & 0xF];
    uint32_t bg = palette[attr >> 4];
    volatile uint8_t *line = fb + (uint64_t)cy * GLYPH_H * pitch + (uint64_t)cx * GLYPH_W * bytes_pp;
    for (uint32_t y = 0; y < GLYPH_H; y++, line += pitch) {
        uint8_t bits = glyph[y];
        if (bytes_pp == 4) {
            volatile uint32_t *px = (volatile uint32_t *)line;
            for (uint32_t x = 0; x < GLYPH_W; x++)
                px[x] = (bits & (0x80 >> x)) ? fg : bg;
        } else {                        /* 24 bpp: har bir piksel 3 bayt */
            for (uint32_t x = 0; x < GLYPH_W; x++) {
                uint32_t c = (bits & (0x80 >> x)) ? fg : bg;
                line[x * 3] = c & 0xFF;
                line[x * 3 + 1] = (c >> 8) & 0xFF;
                line[x * 3 + 2] = (c >> 16) & 0xFF;
            }
        }
    }
}

static void fbcon_get_size(unsigned *c, unsigned *r)
{
    *c = cols;
    *r = rows;
}

static const struct screen_ops fbcon_ops = {
    .name = "framebuffer",
    .draw = fbcon_draw,
    .cursor = NULL,                     /* apparat kursori yo'q - vt.c o'zi chizadi */
    .get_size = fbcon_get_size,
};

const struct screen_ops *fbcon_init(const struct boot_framebuffer *f)
{
    if (!f->present || f->text_mode || (f->bpp != 32 && f->bpp != 24))
        return NULL;
    pitch = f->pitch;
    width = f->width;
    height = f->height;
    bytes_pp = f->bpp / 8;
    cols = width / GLYPH_W;
    rows = height / GLYPH_H;
    if (cols == 0 || rows == 0)
        return NULL;

    /* Framebuffer - qurilma xotirasi, RAM emas: direct map'da yo'q. Uni
     * vmalloc hududiga write-combining rejimida xaritalaymiz. */
    fb = ioremap_wc(f->phys, (uint64_t)pitch * height);
    if (!fb)
        return NULL;
    for (int i = 0; i < 16; i++)
        palette[i] = make_color(f, vga_rgb[i]);
    return &fbcon_ops;
}
