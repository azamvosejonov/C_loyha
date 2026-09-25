/* =============================================================================
 *  drivers/keyboard.c - PS/2 klaviatura
 * =============================================================================
 *
 *  QANDAY ISHLAYDI:
 *    Tugma bosilganda yoki qo'yib yuborilganda klaviatura kontrolleri IRQ1
 *    yuboradi va 0x60 portiga "scancode" qo'yadi. Scancode - tugmaning FIZIK
 *    raqami (harf emas!). Masalan, 'A' tugmasi = 0x1E. Uni harfga aylantirish
 *    (va Shift, Caps Lock ni hisobga olish) - drayverning ishi.
 *
 *    "Scancode set 1" da:
 *      bosish     = kod        (masalan 0x1E)
 *      qo'yish    = kod | 0x80 (masalan 0x9E)
 *      0xE0       = kengaytirilgan tugma prefiksi (strelkalar va h.k.)
 *
 *  Tayyor belgi console_input_char() ga uzatiladi - konsol uni navbatga qo'yadi
 *  va o'qishni kutayotgan jarayonni uyg'otadi.
 * ============================================================================= */
#include "drivers/keyboard.h"

#include <stdbool.h>

#include "arch/interrupts.h"
#include "arch/io.h"
#include "drivers/console.h"

#define KBD_DATA_PORT 0x60

/* US QWERTY klaviatura uchun scancode -> ASCII jadvali (Shift'siz). 0 = belgi yo'q. */
static const char keymap_normal[128] = {
    0,    27,   '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']',  '\n', 0,    'a',  's',
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',
    'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',  0,    ' ',
};

/* Shift bosilgan holatdagi jadval. */
static const char keymap_shift[128] = {
    0,    27,   '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}',  '\n', 0,    'A',  'S',
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',
    'B',  'N',  'M',  '<',  '>',  '?',  0,    '*',  0,    ' ',
};

#define SC_LSHIFT 0x2A
#define SC_RSHIFT 0x36
#define SC_CTRL   0x1D
#define SC_CAPS   0x3A

static bool shift_down;
static bool ctrl_down;
static bool caps_lock;
static bool extended;                   /* oldingi bayt 0xE0 edi */

static void keyboard_irq(struct interrupt_frame *frame)
{
    (void)frame;
    uint8_t sc = inb(KBD_DATA_PORT);    /* scancodeni o'qish SHART - aks holda yangi kelmaydi */

    if (sc == 0xE0) {                   /* kengaytirilgan tugma: keyingi baytni kutamiz */
        extended = true;
        return;
    }
    bool released = sc & 0x80;         /* 7-bit = tugma qo'yib yuborildi */
    uint8_t code = sc & 0x7F;

    if (extended) {                     /* strelkalar va h.k. - hozircha e'tiborsiz */
        extended = false;
        if (code == SC_CTRL)            /* o'ng Ctrl */
            ctrl_down = !released;
        return;
    }

    switch (code) {                     /* modifikator tugmalar holatini kuzatamiz */
    case SC_LSHIFT:
    case SC_RSHIFT:
        shift_down = !released;
        return;
    case SC_CTRL:
        ctrl_down = !released;
        return;
    case SC_CAPS:
        if (!released)
            caps_lock = !caps_lock;
        return;
    }

    if (released)                       /* oddiy tugmani qo'yish - bizga kerak emas */
        return;

    char c = shift_down ? keymap_shift[code] : keymap_normal[code];
    if (c == 0)
        return;
    /* Caps Lock faqat harflarga ta'sir qiladi va Shift bilan teskari ishlaydi. */
    if (caps_lock && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
        c ^= 0x20;                      /* ASCII hiylasi: katta/kichik harf 0x20 bit bilan farq qiladi */
    if (ctrl_down && c >= 'a' && c <= 'z')
        c = (char)(c - 'a' + 1);        /* Ctrl+A = 1, Ctrl+C = 3 ... (ASCII boshqaruv belgilari) */

    console_input_char(c);
}

void keyboard_init(void)
{
    /* Kontroller buferida qolib ketgan eski baytlarni tozalaymiz. */
    while (inb(0x64) & 1)
        inb(KBD_DATA_PORT);
    irq_register(IRQ_KEYBOARD, keyboard_irq);
}
