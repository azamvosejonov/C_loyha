/* =============================================================================
 *  user/bin/cksum.c - POSIX CRC-32 nazorat yig'indisi:  cksum [fayl]...
 * =============================================================================
 *
 *  Faylning "barmoq izi": bitta bayt o'zgarsa ham natija butunlay boshqacha
 *  bo'ladi. Natija Linux'dagi `cksum` bilan AYNAN bir xil - diskdan o'qilgan
 *  yoki yozilgan fayl to'g'riligini kompyuterdagi asl nusxa bilan solishtirish
 *  mumkin.
 *
 *  CRC (Cyclic Redundancy Check) - ma'lumotni GF(2) ustidagi ko'phad deb qarab,
 *  uni 0x04C11DB7 generator ko'phadga bo'lishdan qolgan qoldiq. Tezlik uchun
 *  256 ta qiymatli jadval oldindan hisoblanadi (har bir baytga 1 ta qidiruv).
 *  POSIX varianti: MSB-first, oxirida fayl UZUNLIGI ham qo'shiladi, natija
 *  inversiyalanadi.
 * ============================================================================= */
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static uint32_t table[256];

static void make_table(void)
{
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i << 24;
        for (int k = 0; k < 8; k++)
            c = (c & 0x80000000u) ? (c << 1) ^ 0x04C11DB7u : c << 1;
        table[i] = c;
    }
}

static uint32_t step(uint32_t crc, uint8_t byte)
{
    return (crc << 8) ^ table[(crc >> 24) ^ byte];
}

static int cksum_fd(int fd, const char *name)
{
    static uint8_t buf[16384];
    uint32_t crc = 0;
    uint64_t len = 0;
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++)
            crc = step(crc, buf[i]);
        len += (uint64_t)n;
    }
    if (n < 0) {
        fprintf(stderr, "cksum: %s: %s\n", name ? name : "-", strerror(errno));
        return 1;
    }
    for (uint64_t l = len; l; l >>= 8)  /* uzunlik: kichik baytdan boshlab, nolsiz */
        crc = step(crc, (uint8_t)l);
    printf("%u %lu%s%s\n", ~crc, len, name ? " " : "", name ? name : "");
    return 0;
}

int main(int argc, char **argv)
{
    make_table();
    if (argc < 2)
        return cksum_fd(STDIN_FILENO, NULL);
    int status = 0;
    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "cksum: %s: %s\n", argv[i], strerror(errno));
            status = 1;
            continue;
        }
        status |= cksum_fd(fd, argv[i]);
        close(fd);
    }
    return status;
}
