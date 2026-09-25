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

static int cursor_row;                  /* joriy qator (0..24) */
static int cursor_col;                  /* joriy ustun (0..79) */
static uint8_t current_color;           /* joriy rang bayti */

/* Belgi va rangni bitta 16-bitli qiymatga birlashtirish. */
static inline uint16_t vga_entry(char c, uint8_t color)
{
    return (uint16_t)(uint8_t)c | ((uint16_t)color << 8);
}

/* Ekrandagi miltillovchi kursorni (row, col) ga ko'chirish.
 * CRTC ichki registrlari: 0x0E - pozitsiyaning yuqori bayti, 0x0F - past bayti. */
static void update_hw_cursor(void)
{
    uint16_t pos = (uint16_t)(cursor_row * VGA_WIDTH + cursor_col);
    outb(VGA_CRTC_INDEX, 0x0F);
    outb(VGA_CRTC_DATA, (uint8_t)(pos & 0xFF));
    outb(VGA_CRTC_INDEX, 0x0E);
    outb(VGA_CRTC_DATA, (uint8_t)(pos >> 8));
}

static void vga_set_color(enum color fg, enum color bg)
{
    current_color = (uint8_t)(fg | (bg << 4));
}

static void vga_clear(void)
{
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        VGA_MEMORY[i] = vga_entry(' ', current_color);
    cursor_row = 0;
    cursor_col = 0;
    update_hw_cursor();
}

/* Ekranni bir qator yuqoriga surish: 1..24 qatorlarni 0..23 ga ko'chiramiz,
 * oxirgi qatorni tozalaymiz. */
static void scroll(void)
{
    for (int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++)
        VGA_MEMORY[i] = VGA_MEMORY[i + VGA_WIDTH];
    for (int i = VGA_WIDTH * (VGA_HEIGHT - 1); i < VGA_WIDTH * VGA_HEIGHT; i++)
        VGA_MEMORY[i] = vga_entry(' ', current_color);
    cursor_row = VGA_HEIGHT - 1;
}

static void vga_putc(char c)
{
    switch (c) {
    case '\n':                          /* yangi qator: keyingi qator boshiga */
        cursor_col = 0;
        cursor_row++;
        break;
    case '\r':                          /* karetka qaytishi: qator boshiga */
        cursor_col = 0;
        break;
    case '\b':                          /* backspace: bitta orqaga (belgini o'chirmaydi) */
        if (cursor_col > 0)
            cursor_col--;
        break;
    case '\t':                          /* tab: keyingi 8 ga karrali ustunga */
        cursor_col = (cursor_col + 8) & ~7;
        break;
    default:
        VGA_MEMORY[cursor_row * VGA_WIDTH + cursor_col] = vga_entry(c, current_color);
        cursor_col++;
        break;
    }

    if (cursor_col >= VGA_WIDTH) {      /* qator to'ldi - keyingisiga o'tamiz */
        cursor_col = 0;
        cursor_row++;
    }
    if (cursor_row >= VGA_HEIGHT)       /* ekran to'ldi - yuqoriga suramiz */
        scroll();

    update_hw_cursor();
}

static const struct screen_ops vga_ops = {
    .name = "VGA matn 80x25",
    .putc = vga_putc,
    .set_color = vga_set_color,
    .clear = vga_clear,
};

const struct screen_ops *vga_text_init(void)
{
    vga_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    return &vga_ops;
}
