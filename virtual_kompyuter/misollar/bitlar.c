/*
 * bitlar.c — 1-bob uchun: "bitlar bilan gaplashish" amallarini kompyuteringizda (x86/ARM) sinab ko'rish.
 * Emulyatorning turlar.h faylidagi makrolarni ishlatadi.
 *
 *   gcc -Wall -Wextra -I emu misollar/bitlar.c -o build/bitlar && ./build/bitlar
 */
#include <stdio.h>
#include <string.h>

#include "turlar.h"

/* 32 bitli sonni ikkilikda, 4 bitlik guruhlar bilan chiqarish */
static void ikkilik(const char *nom, uint32_t x)
{
    printf("%-34s 0x%08x = ", nom, x);
    for (int i = 31; i >= 0; i--) {
        putchar(BIT(x, i) ? '1' : '0');
        if (i % 4 == 0 && i)
            putchar('_');
    }
    putchar('\n');
}

int main(void)
{
    /* 1) buyruqni maydonlarga ajratish: addi a0, a0, -1 */
    uint32_t b = 0xFFF50513;
    ikkilik("addi a0, a0, -1", b);
    printf("  opcode (6..0)  = 0x%02x\n", BITLAR(b, 6, 0));
    printf("  rd     (11..7) = %u (a0 = x10)\n", BITLAR(b, 11, 7));
    printf("  rs1    (19..15)= %u\n", BITLAR(b, 19, 15));
    printf("  imm    (31..20)= 0x%03x -> ishora bilan: %d\n", BITLAR(b, 31, 20),
           (int32_t)ishora_kengaytir(BITLAR(b, 31, 20), 12));

    /* 2) ishora kengaytirish qadamlari */
    uint32_t q = 0xFFF;
    ikkilik("12 bit: 0xFFF", q);
    ikkilik("<< 20", q << 20);
    ikkilik("(int32_t) >> 20", (uint32_t)((int32_t)(q << 20) >> 20));
    ikkilik("(uint32_t) >> 20 (xato!)", (q << 20) >> 20);

    /* 3) maska bilan yozish: faqat 1-bit va 5-bitni o'zgartirish (1-bit yoqiladi, 5-bit o'chadi) */
    uint32_t eski = 0x0000A0A0, yangi = 0x00000002, maska = (1u << 1) | (1u << 5);
    ikkilik("eski", eski);
    ikkilik("yangi", yangi);
    ikkilik("maska", maska);
    ikkilik("(eski & ~maska) | (yangi & maska)", (eski & ~maska) | (yangi & maska));

    /* 4) little-endian: 0x12345678 xotirada qanday turadi */
    uint32_t son = 0x12345678;
    unsigned char bayt[4];
    memcpy(bayt, &son, 4);
    printf("0x12345678 xotirada: [%02x] [%02x] [%02x] [%02x]  (kichik manzildan)\n", bayt[0], bayt[1], bayt[2], bayt[3]);
    return 0;
}
