/* =============================================================================
 *  drivers/console.c - konsol qatlami (SMP uchun qulflar bilan)
 * =============================================================================
 *
 *  CHIQISH YO'LI:  kprintf -> console_write -> (1) klog buferi
 *                                             (2) serial port
 *                                             (3) ekran (agar ulangan bo'lsa)
 *
 *  QULFLAR:
 *    console_lock - chiqish. Ikki CPU bir vaqtda yozsa, harflar aralashib
 *                   ketmasligi uchun BUTUN XABAR bitta qulf ostida yoziladi.
 *    input_lock   - kiritish buferi. Klaviatura IRQ (odatda CPU0 da) yozadi,
 *                   o'quvchi jarayon (istalgan CPU da) o'qiydi.
 *
 *  KIRITISH: HALQALI BUFER (ring buffer)
 *        [ . . a b c . . . ]
 *              ^     ^
 *            tail   head      bo'sh: head == tail,  to'la: head+1 == tail
 * ============================================================================= */
#include "drivers/console.h"

#include <stdbool.h>
#include <stdint.h>

#include "arch/interrupts.h"
#include "drivers/serial.h"
#include "drivers/tty.h"
#include "drivers/vt.h"
#include "lib/klog.h"
#include "lib/spinlock.h"
#include "proc/process.h"
#include "proc/signal.h"

#define INPUT_BUFFER_SIZE 1024          /* 2 ning darajasi bo'lishi SHART */

static spinlock_t console_lock = SPINLOCK_INIT("console");
static spinlock_t input_lock = SPINLOCK_INIT("console-input");

static char input_buffer[INPUT_BUFFER_SIZE];
static uint32_t input_head;
static uint32_t input_tail;

static const struct screen_ops *screen; /* NULL - hali ekran yo'q */

void console_init_early(void)
{
    serial_init();
}

static void screen_putc(char c)
{
    vt_putc(c);
}

void console_attach_screen(const struct screen_ops *ops)
{
    spin_lock(&console_lock);
    if (vt_attach(ops)) {
        screen = ops;
        klog_replay(screen_putc);       /* ilk boot xabarlarini ham ekranda ko'ramiz */
    }
    spin_unlock(&console_lock);
}

void console_get_size(unsigned *cols, unsigned *rows)
{
    *cols = 80;
    *rows = 25;
    const struct screen_ops *s = __atomic_load_n(&screen, __ATOMIC_ACQUIRE);
    if (s && s->get_size)
        s->get_size(cols, rows);
}

/* console_lock ushlangan holda. */
static void putc_locked(char c, bool log)
{
    if (log)
        klog_putc(c);
    serial_putc(c);
    if (screen)
        vt_putc(c);
}

/* Terminal so'rovlariga javob (ESC[6n -> ESC[qator;ustunR) - dastur uni
 * klaviaturadan kelgandek o'qiydi. Qulfdan TASHQARIDA (input_lock boshqa qulf). */
static void flush_vt_response(void)
{
    char buf[32];
    size_t n;
    spin_lock(&console_lock);
    n = vt_take_response(buf, sizeof(buf));
    spin_unlock(&console_lock);
    for (size_t i = 0; i < n; i++)
        console_input_char(buf[i]);
}

void console_putc(char c)
{
    spin_lock(&console_lock);
    putc_locked(c, true);
    spin_unlock(&console_lock);
}

void console_write(const char *s, size_t len)
{
    spin_lock(&console_lock);
    for (size_t i = 0; i < len; i++)
        putc_locked(s[i], true);
    spin_unlock(&console_lock);
}

void console_write_tty(const char *s, size_t len)
{
    spin_lock(&console_lock);
    for (size_t i = 0; i < len; i++)
        putc_locked(s[i], false);
    spin_unlock(&console_lock);
    flush_vt_response();
}

void console_set_color(enum color fg, enum color bg)
{
    spin_lock(&console_lock);
    vt_set_color((uint8_t)fg, (uint8_t)bg);
    spin_unlock(&console_lock);
}

void console_clear(void)
{
    spin_lock(&console_lock);
    vt_clear();
    spin_unlock(&console_lock);
}

size_t console_read_log(char *buf, size_t size)
{
    spin_lock(&console_lock);
    size_t n = klog_read(buf, size);
    spin_unlock(&console_lock);
    return n;
}

/* ---- Kiritish ---- */

/* Uzilish kontekstidan chaqiriladi (klaviatura yoki serial IRQ). */
void console_input_flush(void)
{
    spin_lock(&input_lock);
    input_tail = input_head;            /* hali o'qilmagan hamma narsa tashlanadi */
    spin_unlock(&input_lock);
}

void console_input_char(char c)
{
    if (tty_input_signal(c))            /* Ctrl-C -> SIGINT (uzilish kontekstida, darhol) */
        return;
    spin_lock(&input_lock);
    uint32_t next = (input_head + 1) & (INPUT_BUFFER_SIZE - 1);
    if (next != input_tail) {           /* bufer to'la bo'lsa - belgi tashlab yuboriladi */
        input_buffer[input_head] = c;
        input_head = next;
        proc_wakeup(&input_head);       /* o'qishni kutayotgan jarayonni uyg'otamiz */
    }
    spin_unlock(&input_lock);
}

/* Serial port IRQ4 handleri: kelgan barcha baytlarni navbatga qo'yamiz. */
static void serial_irq(struct interrupt_frame *frame)
{
    (void)frame;
    while (serial_has_data()) {
        char c = serial_read_byte();
        if (c == '\r')                  /* terminallar Enter uchun '\r' yuboradi */
            c = '\n';
        else if (c == 0x7F)             /* ko'p terminallar Backspace uchun DEL yuboradi */
            c = '\b';
        console_input_char(c);
    }
}

void console_enable_serial_input(void)
{
    irq_register(IRQ_COM1, serial_irq);
    serial_enable_rx_interrupt();
}

bool console_input_available(void)
{
    return __atomic_load_n(&input_tail, __ATOMIC_RELAXED) !=
           __atomic_load_n(&input_head, __ATOMIC_RELAXED);
}

int console_getc(void)
{
    spin_lock(&input_lock);
    while (input_tail == input_head) {
        if (signal_interrupted(current)) {  /* signal keldi - kutishni to'xtatamiz (-EINTR) */
            spin_unlock(&input_lock);
            return -1;
        }
        /* Bufer bo'sh: uxlaymiz. proc_sleep input_lock ni ATOMAR qo'yib yuboradi
         * - belgi "tekshirdim" va "uxladim" orasida kelsa ham yo'qolmaydi. */
        proc_sleep(&input_head, &input_lock);
    }
    char c = input_buffer[input_tail];
    input_tail = (input_tail + 1) & (INPUT_BUFFER_SIZE - 1);
    spin_unlock(&input_lock);
    return (unsigned char)c;
}
