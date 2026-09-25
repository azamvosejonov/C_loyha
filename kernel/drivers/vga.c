/* =============================================================================
 *  drivers/vga.c - VGA matn rejimi (80x25 belgi)
 * =============================================================================
 *
 *  QANDAY ISHLAYDI:
 *    BIOS ekranni "matn rejimi"da qoldiradi. Videokarta 0xB8000 fizik manzildagi
 *    xotirani doimiy ravishda o'qib, ekranga chiqaradi. Bu MMIO (memory-mapped I/O)
 *    ga misol: oddiy xotiraga yozgandek yozamiz, lekin natija ekranda ko'rinadi.
 *
 *    Har bir belgi 2 bayt (uint16_t):
 *        bitlar 0..7   - ASCII kod
 *        bitlar 8..11  - harf rangi
 *        bitlar 12..15 - fon rangi
 *    Ekran 80 ustun x 25 qator = 2000 belgi = 4000 bayt.
 *
 *  NEGA volatile:
 *    Kompilyator "bu xotiraga yozilgan qiymat hech qachon o'qilmaydi" deb yozishni
 *    optimizatsiya qilib o'chirib yuborishi mumkin. volatile unga "bu xotiraning
 *    har bir o'qish/yozishi muhim, tegma" deydi. MMIO uchun doim volatile!
 *
 *  Fizik 0xB8000 ga direct map (phys_to_virt) orqali murojaat qilamiz.
 *  Zamonaviy UEFI kompyuterlarda bu rejim UMUMAN YO'Q - shuning uchun
 *  asosiy ekran drayverimiz fbcon.c (framebuffer), bu esa zaxira variant.
 * ============================================================================= */
#include "drivers/vga.h"

#include "arch/io.h"
#include "mm/layout.h"

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t *)phys_to_virt(0xB8000))

/* Kursorni boshqarish portlari (CRT Controller). */
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5

/* Unicode -> VGA'ning ichki shrifti (IBM kod sahifasi 437). VGA matn rejimida
 * har bir katak 1 baytli belgi, shuning uchun faqat CP437 dagi belgilar
 * ko'rsatiladi: ASCII, chegara chiziqlari, bloklar, ba'zi lotin harflari. */
static const struct {
    uint16_t cp;
    uint8_t code;
} cp437[] = {
    { 0x00C7, 0x80 }, { 0x00FC, 0x81 }, { 0x00E9, 0x82 }, { 0x00E2, 0x83 }, { 0x00E4, 0x84 },
    { 0x00E0, 0x85 }, { 0x00E7, 0x87 }, { 0x00EA, 0x88 }, { 0x00EB, 0x89 }, { 0x00E8, 0x8A },
    { 0x00EF, 0x8B }, { 0x00EE, 0x8C }, { 0x00C4, 0x8E }, { 0x00C9, 0x90 }, { 0x00F4, 0x93 },
    { 0x00F6, 0x94 }, { 0x00FB, 0x96 }, { 0x00F9, 0x97 }, { 0x00D6, 0x99 }, { 0x00DC, 0x9A },
    { 0x00E1, 0xA0 }, { 0x00ED, 0xA1 }, { 0x00F3, 0xA2 }, { 0x00FA, 0xA3 }, { 0x00F1, 0xA4 },
    { 0x00AB, 0xAE }, { 0x00BB, 0xAF }, { 0x2591, 0xB0 }, { 0x2592, 0xB1 }, { 0x2593, 0xB2 },
    { 0x2502, 0xB3 }, { 0x2524, 0xB4 }, { 0x2551, 0xBA }, { 0x2557, 0xBB }, { 0x255D, 0xBC },
    { 0x2510, 0xBF }, { 0x2514, 0xC0 }, { 0x2534, 0xC1 }, { 0x252C, 0xC2 }, { 0x251C, 0xC3 },
    { 0x2500, 0xC4 }, { 0x253C, 0xC5 }, { 0x255A, 0xC8 }, { 0x2554, 0xC9 }, { 0x2569, 0xCA },
    { 0x2566, 0xCB }, { 0x2560, 0xCC }, { 0x2550, 0xCD }, { 0x256C, 0xCE }, { 0x2518, 0xD9 },
    { 0x250C, 0xDA }, { 0x2588, 0xDB }, { 0x2584, 0xDC }, { 0x258C, 0xDD }, { 0x2590, 0xDE },
    { 0x2580, 0xDF }, { 0x00DF, 0xE1 }, { 0x00B1, 0xF1 }, { 0x2265, 0xF2 }, { 0x2264, 0xF3 },
    { 0x00F7, 0xF6 }, { 0x2248, 0xF7 }, { 0x00B0, 0xF8 }, { 0x2022, 0xF9 }, { 0x00B7, 0xFA },
    { 0x221A, 0xFB }, { 0x00B2, 0xFD }, { 0x25A0, 0xFE }, { 0x2192, 0x1A }, { 0x2190, 0x1B },
    { 0x2191, 0x18 }, { 0x2193, 0x19 }, { 0x25B6, 0x10 }, { 0x25C0, 0x11 },
};

static uint8_t to_cp437(uint16_t cp)
{
    if (cp >= 32 && cp < 127)
        return (uint8_t)cp;
    for (unsigned i = 0; i < sizeof(cp437) / sizeof(cp437[0]); i++)
        if (cp437[i].cp == cp)
            return cp437[i].code;
    return '?';
}

static void vga_draw(unsigned x, unsigned y, uint16_t cp, uint8_t attr)
{
    if (x < VGA_WIDTH && y < VGA_HEIGHT)
        VGA_MEMORY[y * VGA_WIDTH + x] = (uint16_t)(to_cp437(cp) | ((uint16_t)attr << 8));
}

/* Apparat kursori. CRTC registrlari: 0x0E/0x0F - pozitsiya, 0x0A - kursor
 * boshlanish qatori (5-bit = kursorni yashirish). */
static void vga_cursor(unsigned x, unsigned y, bool visible)
{
    uint16_t pos = (uint16_t)(y * VGA_WIDTH + x);
    outb(VGA_CRTC_INDEX, 0x0F);
    outb(VGA_CRTC_DATA, (uint8_t)(pos & 0xFF));
    outb(VGA_CRTC_INDEX, 0x0E);
    outb(VGA_CRTC_DATA, (uint8_t)(pos >> 8));
    outb(VGA_CRTC_INDEX, 0x0A);
    outb(VGA_CRTC_DATA, visible ? 0x0E : 0x20);    /* 14-qatordan boshlanuvchi pastki chiziq */
}

static void vga_get_size(unsigned *c, unsigned *r)
{
    *c = VGA_WIDTH;
    *r = VGA_HEIGHT;
}

static const struct screen_ops vga_ops = {
    .name = "VGA matn 80x25",
    .draw = vga_draw,
    .cursor = vga_cursor,
    .get_size = vga_get_size,
};

const struct screen_ops *vga_text_init(void)
{
    return &vga_ops;
}
