/*
 * konsol.c — UART (16550) orqali eng oddiy chiqarish. printf yo'q: firmware'da kutubxona yo'q,
 * hamma narsani o'zimiz yozamiz. Sonni o'nlik yoki o'n oltilikka aylantirish — klassik mashq.
 */
#include "fw.h"

#define LSR 5
#define LSR_THRE 0x20
#define LSR_DR 0x01

void konsol_belgi(int c)
{
    if (c == '\n')
        konsol_belgi('\r');                     /* terminal uchun: yangi qator = karetka qaytishi + qator */
    while (!(mmio_oqi8(UART + LSR) & LSR_THRE))
        ;                                       /* yuborish registri bo'shaguncha kutamiz */
    mmio_yoz8(UART, (uint8_t)c);
}

int konsol_oqi(void)
{
    if (!(mmio_oqi8(UART + LSR) & LSR_DR))
        return -1;
    return mmio_oqi8(UART);
}

void konsol_matn(const char *s)
{
    while (*s)
        konsol_belgi(*s++);
}

/* 0x0000ABCD ko'rinishida: har 4 bit (nibble) — bitta hex raqam, yuqoridan pastga */
void konsol_hex(uint32_t q)
{
    konsol_matn("0x");
    for (int siljish = 28; siljish >= 0; siljish -= 4)
        konsol_belgi("0123456789abcdef"[(q >> siljish) & 0xF]);
}

/* o'nlik: raqamlar oxiridan hosil bo'ladi (q % 10), shuning uchun buferga teskari yozib, keyin chiqaramiz */
void konsol_son(uint32_t q)
{
    char b[10];
    int n = 0;
    do {
        b[n++] = (char)('0' + q % 10);
        q /= 10;
    } while (q);
    while (n)
        konsol_belgi(b[--n]);
}
