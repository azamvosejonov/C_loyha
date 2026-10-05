/*
 * elf.c — yadroni ELF fayldan RAM ga yuklash ("bootloader"ning eng sodda varianti).
 *
 * ELF — Linux va RISC-V dasturlari formati (22-bob). Bizga kerakli qismi:
 *   ELF sarlavhasi (52 bayt): sehrli baytlar 0x7F 'E' 'L' 'F', 32/64 bit, baytlar tartibi, arxitektura,
 *                             KIRISH NUQTASI (e_entry), dastur sarlavhalari jadvali qayerda (e_phoff) ...
 *   Dastur sarlavhalari (har biri 32 bayt): PT_LOAD turi — "fayldagi shu baytlarni xotiraning shu
 *                             manziliga qo'y". p_filesz dan p_memsz gacha qolgan qism — nollar (.bss).
 * Biz sarlavhalarni struct ga memcpy qilmaymiz: maydonlarni little-endian baytlardan o'zimiz yig'amiz
 * (to'ldirish va baytlar tartibiga bog'liq bo'lmaslik uchun — 16-bobdagi qoida).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mashina.h"

static uint32_t u16(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8; }
static uint32_t u32(const uint8_t *p) { return u16(p) | u16(p + 2) << 16; }

int elf_yukla(struct mashina *m, const char *yol, uint32_t *kirish_nuqtasi)
{
    FILE *f = fopen(yol, "rb");
    if (!f) {
        perror(yol);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    long hajm = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *b = malloc(hajm > 0 ? (size_t)hajm : 1);
    if (!b || fread(b, 1, (size_t)hajm, f) != (size_t)hajm) {
        fprintf(stderr, "%s: o'qib bo'lmadi\n", yol);
        free(b);
        fclose(f);
        return -1;
    }
    fclose(f);

    int xato = 0;
    if (hajm < 52 || memcmp(b, "\x7f" "ELF", 4) != 0) {
        fprintf(stderr, "%s: ELF fayl emas\n", yol);
        xato = 1;
    } else if (b[4] != 1 || b[5] != 1 || u16(b + 18) != 0xF3) {
        fprintf(stderr, "%s: 32 bitli, little-endian RISC-V ELF kerak\n", yol);
        xato = 1;
    }

    uint32_t phoff = xato ? 0 : u32(b + 28), phsoni = xato ? 0 : u16(b + 44), phhajm = xato ? 0 : u16(b + 42);
    for (uint32_t i = 0; !xato && i < phsoni; i++) {
        uint64_t s = (uint64_t)phoff + (uint64_t)i * phhajm;
        if (s + 32 > (uint64_t)hajm) {
            xato = 1;
            break;
        }
        const uint8_t *ph = b + s;
        if (u32(ph) != 1)                       /* PT_LOAD emas — o'tkazamiz */
            continue;
        uint32_t offset = u32(ph + 4), manzil = u32(ph + 12), fhajm = u32(ph + 16), mhajm = u32(ph + 20);
        /* fizik manzil (p_paddr) bo'yicha yuklaymiz: yadro shu manzillar bilan bog'langan (0x8000_0000 dan) */
        if (manzil < RAM_BOSH || (uint64_t)manzil - RAM_BOSH + mhajm > m->ram_hajm || fhajm > mhajm ||
            (uint64_t)offset + fhajm > (uint64_t)hajm) {
            fprintf(stderr, "%s: segment RAM ga sig'maydi (0x%08x, %u bayt)\n", yol, manzil, mhajm);
            xato = 1;
            break;
        }
        memcpy(m->ram + (manzil - RAM_BOSH), b + offset, fhajm);
        memset(m->ram + (manzil - RAM_BOSH) + fhajm, 0, mhajm - fhajm);       /* .bss — nollar */
    }
    if (!xato)
        *kirish_nuqtasi = u32(b + 24);
    free(b);
    return xato ? -1 : 0;
}

/* xom (raw) tasvir: fayl baytlari aynan shu manzilga ko'chiriladi (sarlavha yo'q) */
int xom_yukla(struct mashina *m, const char *yol, uint32_t manzil)
{
    FILE *f = fopen(yol, "rb");
    if (!f) {
        perror(yol);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    long hajm = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (hajm <= 0 || (uint64_t)manzil - RAM_BOSH + (uint64_t)hajm > m->ram_hajm ||
        fread(m->ram + (manzil - RAM_BOSH), 1, (size_t)hajm, f) != (size_t)hajm) {
        fprintf(stderr, "%s: RAM ga sig'maydi yoki o'qib bo'lmadi\n", yol);
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}
