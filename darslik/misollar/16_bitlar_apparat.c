/* =============================================================================
 *  16_bitlar_apparat.c - endianness, volatile, bitmap, registr maydonlari (16-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 16_bitlar_apparat.c -o apparat && ./apparat
 *
 *  Kutilgan natija:
 *      0x11223344 xotirada: 44 33 22 11 (little-endian)
 *      big-endian baytlardan o'qilgan son: 0x11223344
 *      registr = 0xa5, 4..7-bitlar maydoni = 0xa -> 0x3 yozildi: 0x35
 *      bitmap: 0, 63, 64, 130 yoqildi; birinchi bo'sh: 1; 64-bit bormi: ha
 *      __builtin_ctz(0x28) = 3, __builtin_popcount(0xff) = 8
 *
 *  Sinab ko'ring:
 *      1) x = 0xdeadbeef qiling. Xotiradagi baytlar tartibini AVVAL qog'ozda yozing.
 *      2) Maydonga 0x3 o'rniga 0x13 yozing (4 bitga sig'maydi). Mask bilan: faqat 0x3 yoziladi.
 *         `& MAYDON_MASK` ni o'chirsangiz - registr 0x135: qo'shni 8-bit buzildi!
 *      3) Teskari funksiya yozing: `void yoz_be32(uint8_t *p, uint32_t x)`.
 *      4) yoq[] ga 1 va 2 ni qo'shing (sikldagi 4 ni ham o'zgartiring). Birinchi bo'sh bit qaysi?
 *      5) __builtin_clz(1) nechaga teng? Nega?
 * ============================================================================= */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAYDON_SHIFT 4
#define MAYDON_MASK (0xFu << MAYDON_SHIFT)

static uint32_t oqi_be32(const uint8_t *p)      /* har qanday CPU'da to'g'ri ishlaydi */
{
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3];
}

int main(void)
{
    uint32_t x = 0x11223344;
    uint8_t b[4];
    memcpy(b, &x, 4);
    printf("0x%x xotirada: %02x %02x %02x %02x (little-endian)\n", x, b[0], b[1], b[2], b[3]);
    uint8_t tarmoq[4] = { 0x11, 0x22, 0x33, 0x44 };
    printf("big-endian baytlardan o'qilgan son: 0x%x\n", oqi_be32(tarmoq));

    /* Qurilma registri o'rnida oddiy o'zgaruvchi. Haqiqiy registr: volatile ko'rsatkich. */
    volatile uint32_t registr = 0xA5;
    uint32_t eski = (registr & MAYDON_MASK) >> MAYDON_SHIFT;
    registr = (registr & ~MAYDON_MASK) | ((0x3u << MAYDON_SHIFT) & MAYDON_MASK);   /* o'qi-o'zgartir-yoz */
    printf("registr = 0xa5, 4..7-bitlar maydoni = 0x%x -> 0x3 yozildi: 0x%x\n", eski, registr);

    uint64_t bm[4] = { 0 };                     /* 256 bitlik bitmap */
    size_t yoq[] = { 0, 63, 64, 130 };
    for (int i = 0; i < 4; i++)
        bm[yoq[i] / 64] |= 1ull << (yoq[i] % 64);
    size_t bosh = 0;
    while ((bm[bosh / 64] >> (bosh % 64)) & 1)
        bosh++;
    printf("bitmap: 0, 63, 64, 130 yoqildi; birinchi bo'sh: %zu; 64-bit bormi: %s\n", bosh,
           (bm[1] & 1) ? "ha" : "yo'q");

    printf("__builtin_ctz(0x28) = %d, __builtin_popcount(0xff) = %d\n",
           __builtin_ctz(0x28), __builtin_popcount(0xff));
    return 0;
}
