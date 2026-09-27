/* =============================================================================
 *  02_turlar.c - turlar, hajmlar va toshish                  (darslik 2-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 02_turlar.c -o turlar && ./turlar
 *
 *  Kutilgan natija (x86-64 Linux):
 *      char: 1 bayt, int: 4 bayt, long: 8 bayt, double: 8 bayt, uint16_t: 2 bayt
 *      INT_MAX = 2147483647
 *      unsigned: 4294967295 + 1 = 0
 *      7 / 2 = 3,  7.0 / 2 = 3.500000,  (double)7 / 2 = 3.500000
 *      '7' - '0' = 7
 *      010 = 8 (sakkizlik!), 0x10 = 16
 *      noto'g'ri: 50000 * 50000 -> int'ga sig'maydi; to'g'ri: 2500000000
 *
 *  Sinab ko'ring:
 *      1) -fsanitize=undefined bilan yig'ib, xavfli qatorni izohdan chiqaring
 *         (pastda ko'rsatilgan) - sanitizer nima deydi?
 * ============================================================================= */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    printf("char: %zu bayt, int: %zu bayt, long: %zu bayt, double: %zu bayt, uint16_t: %zu bayt\n",
           sizeof(char), sizeof(int), sizeof(long), sizeof(double), sizeof(uint16_t));
    printf("INT_MAX = %d\n", INT_MAX);

    unsigned int u = UINT_MAX;
    printf("unsigned: %u + 1 = %u\n", u, u + 1);   /* ishorasiz toshish - aniqlangan: 0 */

    printf("7 / 2 = %d,  7.0 / 2 = %f,  (double)7 / 2 = %f\n", 7 / 2, 7.0 / 2, (double)7 / 2);
    printf("'7' - '0' = %d\n", '7' - '0');
    printf("010 = %d (sakkizlik!), 0x10 = %d\n", 010, 0x10);

    /* long x = 50000 * 50000;    <- XATO: int * int toshadi (UB). Izohdan chiqarib ko'ring. */
    long y = 50000L * 50000;      /* L - long literal: ko'paytirish long'da */
    printf("noto'g'ri: 50000 * 50000 -> int'ga sig'maydi; to'g'ri: %ld\n", y);
    return 0;
}
