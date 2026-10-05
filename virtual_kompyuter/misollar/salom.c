/*
 * salom.c — virtual kompyuterimiz uchun ENG KICHIK dastur: operatsion tizimsiz, libc'siz ("bare metal").
 *
 * printf yo'q, fayl yo'q, hech narsa yo'q — faqat XOTIRA manzillari. Ekranga harf chiqarish = UART qurilmasining
 * 0x10000000 manziliga bayt YOZISH. Kompyuterni o'chirish = 0x100000 manziliga 0x5555 yozish.
 * Qurilmaning "registri" — oddiy xotira manzili kabi ko'rinadi (MMIO, 8-bob).
 *
 * Yig'ish va ishga tushirish:  make salom
 */
#include <stdint.h>

#define UART_THR ((volatile uint8_t *)0x10000000)   /* yuborish registri: bu yerga yozilgan bayt — ekranga */
#define QUVVAT   ((volatile uint32_t *)0x00100000)  /* 0x5555 — o'chirish (muvaffaqiyat), 0x3333 — xato */

/* volatile: "bu yozuvni olib tashlama/birlashtirma" — kompilyatorga. Aks holda u "hech kim o'qimaydi" deb
   tsikldagi yozuvlarni bitta qilib qo'yishi mumkin. Qurilma bilan ishlaganda HAR DOIM volatile. */
static void harf(char c)
{
    *UART_THR = (uint8_t)c;
}

static void matn(const char *s)
{
    while (*s)
        harf(*s++);
}

/* son -> o'nlik matn. printf yo'q — o'zimiz yozamiz (raqamlar teskari tartibda chiqadi, shuning uchun bufer) */
static void son(uint32_t n)
{
    char b[10];
    int i = 0;
    do {
        b[i++] = (char)('0' + n % 10);
        n /= 10;
    } while (n);
    while (i)
        harf(b[--i]);
}

void asosiy(void)
{
    matn("Salom, RISC-V! Men operatsion tizimsiz ishlayapman.\n");
    uint32_t yigindi = 0;
    for (uint32_t i = 1; i <= 100; i++)
        yigindi += i;
    matn("1 + 2 + ... + 100 = ");
    son(yigindi);
    matn("\n");
    *QUVVAT = 0x5555;                           /* emulyator to'xtaydi, chiqish kodi 0 */
}

/* _start — protsessor birinchi bajaradigan joy. Stek yo'q — o'rnatamiz (sp), keyin C funksiyani chaqiramiz.
   __attribute__((naked)): kompilyator funksiyaga "kirish/chiqish" kodini qo'shmasin (stek hali yo'q!). */
__attribute__((naked, section(".text.boshlash"))) void _start(void)
{
    __asm__ volatile("li sp, 0x80100000\n"      /* RAM boshidan 1 MB yuqorida — stek pastga o'sadi */
                     "call asosiy\n"
                     "1: j 1b\n");
}
