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
 *
 *  MAXSUS TUGMALAR (strelkalar, Home, Delete ...) ASCII'da yo'q. Ular xuddi
 *  haqiqiy terminaldagi kabi ESCAPE KETMA-KETLIKLARIGA aylanadi:
 *      yuqori = ESC [ A    pastga = ESC [ B    o'ng = ESC [ C    chap = ESC [ D
 *      Home = ESC [ H      End = ESC [ F       Delete = ESC [ 3 ~
 *  Shunda dastur (shell, muharrir) klaviatura PS/2 mi, USB mi yoki serial
 *  port orqali ulangan Linux terminalimi - farqini bilmaydi.
 * ============================================================================= */
#include "drivers/keyboard.h"

#include <stdbool.h>

#include "arch/interrupts.h"
#include "arch/io.h"
#include "drivers/console.h"
#include "lib/kprintf.h"

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

    if (extended) {                     /* strelkalar va h.k. */
        extended = false;
        if (code == SC_CTRL) {          /* o'ng Ctrl */
            ctrl_down = !released;
            return;
        }
        if (code == 0x2A || code == 0x36 || released)
            return;                     /* "soxta Shift" (NumLock bilan) va qo'yib yuborish */
        const char *seq = NULL;
        switch (code) {
        case 0x48: seq = "\033[A"; break;   /* yuqori */
        case 0x50: seq = "\033[B"; break;   /* pastga */
        case 0x4D: seq = "\033[C"; break;   /* o'ngga */
        case 0x4B: seq = "\033[D"; break;   /* chapga */
        case 0x47: seq = "\033[H"; break;   /* Home */
        case 0x4F: seq = "\033[F"; break;   /* End */
        case 0x52: seq = "\033[2~"; break;  /* Insert */
        case 0x53: seq = "\033[3~"; break;  /* Delete */
        case 0x49: seq = "\033[5~"; break;  /* Page Up */
        case 0x51: seq = "\033[6~"; break;  /* Page Down */
        case 0x1C: seq = "\n"; break;       /* raqamli klaviaturadagi Enter */
        case 0x35: seq = "/"; break;         /* raqamli klaviaturadagi / */
        }
        for (; seq && *seq; seq++)
            console_input_char(*seq);
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
    /* Kontroller buferida qolib ketgan eski baytlarni tozalaymiz.
     * HAQIQIY APPARAT: PS/2 kontrolleri umuman YO'Q kompyuterlarda (ko'plab
     * zamonaviy UEFI mashinalar) mavjud bo'lmagan port 0xFF qaytaradi - 0-bit
     * doim 1. Chegarasiz `while` bo'lsa, yadro shu yerda abadiy qotardi.
     * Bufer 16 baytdan oshmaydi, shuning uchun 64 marta o'qish yetarli. */
    if (inb(0x64) == 0xFF)
        kprintf("[kbd]  PS/2 kontroller topilmadi (USB klaviatura hali qo'llab-quvvatlanmaydi)\n");
    for (int i = 0; i < 64 && (inb(0x64) & 1); i++)
        inb(KBD_DATA_PORT);
    irq_register(IRQ_KEYBOARD, keyboard_irq);
}
