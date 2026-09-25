/* =============================================================================
 *  drivers/serial.c - COM1 serial port drayveri
 * =============================================================================
 *
 *  NEGA SERIAL PORT - OS DASTURCHISINING ENG YAQIN DO'STI:
 *    * QEMU uni `-serial stdio` orqali to'g'ridan-to'g'ri terminalingizga ulaydi.
 *      Yadro yozgan hamma narsa terminalda chiqadi va uni faylga saqlash,
 *      grep qilish, avtomatik test qilish mumkin (tools/test.sh shunday ishlaydi).
 *    * VGA ekrani 25 qatordan keyin yo'qoladi, serial log esa saqlanib qoladi.
 *    * Juda sodda: bir nechta port, hech qanday xotira xaritasi kerak emas.
 *
 *  UART 16550 REGISTRLARI (COM1 bazasi 0x3F8 ga nisbatan):
 *    +0  DATA  - o'qish: qabul qilingan bayt; yozish: yuboriladigan bayt
 *              (DLAB=1 bo'lsa: bo'luvchining past bayti)
 *    +1  IER   - qaysi hodisalarda uzilish berish (DLAB=1: bo'luvchi yuqori bayti)
 *    +2  FCR   - FIFO (navbat) sozlamalari
 *    +3  LCR   - chiziq formati: bitlar soni, stop bit, juftlik; 7-bit = DLAB
 *    +4  MCR   - modem boshqaruvi
 *    +5  LSR   - holat: 0-bit = ma'lumot keldi, 5-bit = yuborish buferi bo'sh
 * ============================================================================= */
#include "drivers/serial.h"

#include "arch/io.h"

#define COM1 0x3F8

#define UART_DATA 0
#define UART_IER  1
#define UART_FCR  2
#define UART_LCR  3
#define UART_MCR  4
#define UART_LSR  5

#define LSR_DATA_READY    0x01          /* qabul qilingan bayt bor */
#define LSR_TX_EMPTY      0x20          /* yangi bayt yuborish mumkin */

void serial_init(void)
{
    outb(COM1 + UART_IER, 0x00);        /* Hozircha barcha uzilishlarni o'chiramiz */
    outb(COM1 + UART_LCR, 0x80);        /* DLAB=1: keyingi ikki yozuv tezlik bo'luvchisi */
    outb(COM1 + UART_DATA, 0x03);       /* bo'luvchi = 3 -> 115200 / 3 = 38400 bod */
    outb(COM1 + UART_IER, 0x00);        /*   (bo'luvchining yuqori bayti = 0) */
    outb(COM1 + UART_LCR, 0x03);        /* DLAB=0; 8 bit, juftliksiz, 1 stop bit ("8N1") */
    outb(COM1 + UART_FCR, 0xC7);        /* FIFO yoqish, navbatlarni tozalash, 14 baytlik chegara */
    outb(COM1 + UART_MCR, 0x0B);        /* DTR + RTS + OUT2. OUT2 - UART uzilishlarini PIC ga
                                         * ulovchi "kalit"; usiz IRQ4 hech qachon kelmaydi */
}

void serial_enable_rx_interrupt(void)
{
    outb(COM1 + UART_IER, 0x01);        /* 0-bit: "ma'lumot qabul qilindi" uzilishi */
}

void serial_putc(char c)
{
    if (c == '\n')                      /* Terminallar yangi qator uchun "\r\n" kutadi */
        serial_putc('\r');
    while ((inb(COM1 + UART_LSR) & LSR_TX_EMPTY) == 0)
        ;                               /* Yuborish buferi bo'shaguncha kutamiz ("polling") */
    outb(COM1 + UART_DATA, (uint8_t)c);
}

bool serial_has_data(void)
{
    return (inb(COM1 + UART_LSR) & LSR_DATA_READY) != 0;
}

char serial_read_byte(void)
{
    return (char)inb(COM1 + UART_DATA);
}
