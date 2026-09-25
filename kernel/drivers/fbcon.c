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
 *    Har bir belgi - 8x16 bitlik rasm (font8x16.c). 1-bit = harf rangi,
 *    0-bit = fon rangi. Ekran 1024x768 bo'lsa: 128 ustun x 48 qator matn.
 *
 *  TEZLIK VA TO'G'RILIK:
 *    * Framebuffer WRITE-COMBINING (WC) rejimida xaritalanadi (vmalloc.c +
 *      cpu.c dagi PAT) - yozish juda tez, lekin O'QISH juda sekin (keshlanmaydi).
 *    * Shuning uchun ekrandan HECH QACHON o'qimaymiz. Matnning "soya" nusxasini
 *      (har bir katakdagi belgi + rang) RAMda saqlaymiz. Scroll qilishda soya
 *      nusxani suramiz va ekranni undan qayta chizamiz.
 * ============================================================================= */
#include "drivers/fbcon.h"

#include <stdint.h>

#include "lib/string.h"
#include "mm/slab.h"
#include "mm/vmalloc.h"

extern const uint8_t font8x16[128][16];

#define GLYPH_W 8
#define GLYPH_H 16

struct cell {
    char ch;
    uint8_t attr;                       /* past 4 bit - harf rangi, yuqori 4 bit - fon */
};

static volatile uint8_t *fb;            /* framebuffer (WC xaritalangan) */
static uint32_t pitch, width, height, bytes_pp;
static uint32_t cols, rows;
static uint32_t cur_x, cur_y;
static uint8_t cur_attr = COLOR_LIGHT_GREY;
static struct cell *shadow;             /* cols * rows ta katak */
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

static inline void put_pixel(uint32_t x, uint32_t y, uint32_t color)
{
    volatile uint8_t *p = fb + (uint64_t)y * pitch + (uint64_t)x * bytes_pp;
    if (bytes_pp == 4) {
        *(volatile uint32_t *)p = color;
    } else {                            /* 24 bpp: 3 bayt */
        p[0] = color & 0xFF;
        p[1] = (color >> 8) & 0xFF;
        p[2] = (color >> 16) & 0xFF;
    }
}

/* (cx, cy) katakdagi belgini chizish. */
static void draw_cell(uint32_t cx, uint32_t cy, bool cursor)
{
    const struct cell *c = &shadow[cy * cols + cx];
    uint8_t ch = (uint8_t)c->ch < 128 ? (uint8_t)c->ch : '?';
    uint32_t fg = palette[c->attr & 0xF];
    uint32_t bg = palette[c->attr >> 4];
    if (cursor) {                       /* kursor: ranglarni almashtiramiz (inverse) */
        uint32_t t = fg;
        fg = bg;
        bg = t;
    }
    const uint8_t *glyph = font8x16[ch];
    uint32_t px = cx * GLYPH_W, py = cy * GLYPH_H;
    for (uint32_t y = 0; y < GLYPH_H; y++) {
        uint8_t bits = glyph[y];
        for (uint32_t x = 0; x < GLYPH_W; x++)
            put_pixel(px + x, py + y, (bits & (0x80 >> x)) ? fg : bg);
    }
}

static void redraw_all(void)
{
    for (uint32_t y = 0; y < rows; y++)
        for (uint32_t x = 0; x < cols; x++)
            draw_cell(x, y, false);
}

static void scroll_up(void)
{
    /* Soya nusxani bir qator yuqoriga surib, oxirgi qatorni tozalaymiz. */
    memmove(shadow, shadow + cols, (size_t)(rows - 1) * cols * sizeof(struct cell));
    for (uint32_t x = 0; x < cols; x++)
        shadow[(rows - 1) * cols + x] = (struct cell){ ' ', cur_attr };
    redraw_all();
}

static void fbcon_putc(char c)
{
    draw_cell(cur_x, cur_y, false);     /* eski kursorni o'chiramiz */
    switch (c) {
    case '\n':
        cur_x = 0;
        cur_y++;
        break;
    case '\r':
        cur_x = 0;
        break;
    case '\b':
        if (cur_x > 0)
            cur_x--;
        break;
    case '\t':
        cur_x = (cur_x + 8) & ~7u;
        break;
    default:
        shadow[cur_y * cols + cur_x] = (struct cell){ c, cur_attr };
        draw_cell(cur_x, cur_y, false);
        cur_x++;
        break;
    }
    if (cur_x >= cols) {
        cur_x = 0;
        cur_y++;
    }
    if (cur_y >= rows) {
        scroll_up();
        cur_y = rows - 1;
    }
    draw_cell(cur_x, cur_y, true);      /* yangi kursor */
}

static void fbcon_set_color(enum color fg, enum color bg)
{
    cur_attr = (uint8_t)(fg | (bg << 4));
}

static void fbcon_clear(void)
{
    for (uint32_t i = 0; i < cols * rows; i++)
        shadow[i] = (struct cell){ ' ', cur_attr };
    cur_x = cur_y = 0;
    redraw_all();
}

static void fbcon_get_size(unsigned *c, unsigned *r)
{
    *c = cols;
    *r = rows;
}

static const struct screen_ops fbcon_ops = {
    .name = "framebuffer",
    .putc = fbcon_putc,
    .set_color = fbcon_set_color,
    .clear = fbcon_clear,
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
    shadow = kmalloc((size_t)cols * rows * sizeof(struct cell));
    if (!fb || !shadow)
        return NULL;
    for (int i = 0; i < 16; i++)
        palette[i] = make_color(f, vga_rgb[i]);
    return &fbcon_ops;
}
