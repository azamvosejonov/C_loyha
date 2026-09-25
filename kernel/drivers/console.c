/* =============================================================================
 *  drivers/console.c - konsol qatlami
 * =============================================================================
 *
 *  NEGA ALOHIDA QATLAM:
 *    Yadroning qolgan qismi "qayerga chiqarish kerak" deb o'ylamasligi kerak.
 *    kprintf faqat console_putc() ni biladi. Ertaga grafik ekran yoki tarmoq
 *    logi qo'shsak - faqat shu faylni o'zgartiramiz. Bu "abstraktsiya qatlami"
 *    tamoyili: har bir qatlam faqat pastdagi qatlamning INTERFEYSINI biladi.
 *
 *  KIRITISH: HALQALI BUFER (ring buffer)
 *    Uzilish handleri (ishlab chiqaruvchi) belgini buferga YOZADI, o'qiyotgan
 *    kod (iste'molchi) esa undan OLADI. Ikkita indeks: head (keyingi yoziladigan
 *    joy) va tail (keyingi o'qiladigan joy). Oxiriga yetganda boshiga qaytamiz -
 *    shuning uchun "halqali". Hajm 2 ning darajasi bo'lsa, "% SIZE" o'rniga
 *    tezkor "& (SIZE-1)" ishlatish mumkin.
 *
 *        [ . . a b c . . . ]
 *              ^     ^
 *            tail   head      bo'sh: head == tail,  to'la: head+1 == tail
 * ============================================================================= */
#include "drivers/console.h"

#include <stdint.h>

#include "arch/cpu.h"
#include "arch/interrupts.h"
#include "drivers/serial.h"
#include "drivers/vga.h"

#define INPUT_BUFFER_SIZE 256           /* 2 ning darajasi bo'lishi SHART */

static char input_buffer[INPUT_BUFFER_SIZE];
static volatile uint32_t input_head;    /* uzilish handleri o'zgartiradi -> volatile */
static volatile uint32_t input_tail;

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

/* Uzilish kontekstidan chaqiriladi (IF=0), shuning uchun qulf kerak emas. */
void console_input_char(char c)
{
    uint32_t next = (input_head + 1) & (INPUT_BUFFER_SIZE - 1);
    if (next == input_tail)             /* bufer to'la - belgini tashlab yuboramiz */
        return;
    input_buffer[input_head] = c;
    input_head = next;
}

/* Serial port IRQ4 handleri: kelgan barcha baytlarni navbatga qo'yamiz. */
static void serial_irq(struct interrupt_frame *frame)
{
    (void)frame;
    while (serial_has_data()) {
        char c = serial_read_byte();
        if (c == '\r')                  /* terminallar Enter uchun '\r' yuboradi */
            c = '\n';
        else if (c == 0x7F)             /* ko'p terminallar Backspace uchun DEL (0x7F) yuboradi */
            c = '\b';
        console_input_char(c);
    }
}

void console_enable_serial_input(void)
{
    irq_register(IRQ_COM1, serial_irq);
    serial_enable_rx_interrupt();
}

char console_getc(void)
{
    for (;;) {
        cpu_cli();                      /* tekshirish va uxlash orasida uzilish "yo'qolmasin" */
        if (input_tail != input_head) {
            char c = input_buffer[input_tail];
            input_tail = (input_tail + 1) & (INPUT_BUFFER_SIZE - 1);
            cpu_sti();
            return c;
        }
        /* "sti; hlt" - atomar juftlik: sti ning ta'siri BITTA instruksiyadan keyin
         * boshlanadi. Demak uzilish sti va hlt ORASIDA kela olmaydi - agar kelsa,
         * u hlt ni uyg'otadi. Aks holda belgi kelib, biz uni sezmay uxlab qolishimiz
         * mumkin edi ("lost wakeup" muammosi). 6-bosqichda bu yerni jarayonni
         * uxlatish bilan almashtiramiz. */
        __asm__ volatile("sti; hlt" ::: "memory");
    }
}
