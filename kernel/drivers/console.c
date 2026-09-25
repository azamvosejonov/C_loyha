/* =============================================================================
 *  drivers/console.c - konsol qatlami
 * =============================================================================
 *
 *  NEGA ALOHIDA QATLAM:
 *    Yadroning qolgan qismi "qayerga chiqarish kerak" deb o'ylamasligi kerak.
 *    kprintf faqat console_putc() ni biladi. Ertaga grafik ekran yoki tarmoq
 *    logi qo'shsak - faqat shu faylni o'zgartiramiz. Bu "abstraktsiya qatlami"
 *    tamoyili: har bir qatlam faqat pastdagi qatlamning INTERFEYSINI biladi.
 * ============================================================================= */
#include "drivers/console.h"

#include "drivers/serial.h"
#include "drivers/vga.h"

void console_init(void)
{
    serial_init();                      /* avval serial - xato bo'lsa log qolsin */
    vga_init();
}

void console_putc(char c)
{
    vga_putc(c);
    serial_putc(c);
}

void console_write(const char *s, size_t len)
{
    for (size_t i = 0; i < len; i++)
        console_putc(s[i]);
}
