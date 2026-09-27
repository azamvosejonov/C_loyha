/* =============================================================================
 *  03_bitlar.c - bitli amallar va maskalar                   (darslik 3-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 03_bitlar.c -o bitlar && ./bitlar
 *
 *  Kutilgan natija:
 *      5 & 3 = 1, 5 | 3 = 7, 5 ^ 3 = 6, 1 << 4 = 16
 *      bayroqlar = 0x3 (BOR + YOZISH)
 *      yozish mumkinmi? ha
 *      YOZISH o'chirildi: 0x1, yozish mumkinmi? yo'q
 *      0x12345 sahifa boshi: 0x12000, sahifa ichidagi siljish: 0x345
 *      0x12345 ni 4096 ga yuqoriga tekislash: 0x13000
 *      0b1011 da 1-bitlar soni: 3
 *
 *  Sinab ko'ring: BOR, YOZISH, USER bayroqlarining boshqa kombinatsiyalarini yasang.
 * ============================================================================= */
#include <stdint.h>
#include <stdio.h>

/* Sahifa jadvali yozuvidagi kabi bayroqlar (MyOS: kernel/mm/vmm.h) */
#define BOR     (1u << 0)
#define YOZISH  (1u << 1)
#define USER    (1u << 2)

int main(void)
{
    printf("5 & 3 = %d, 5 | 3 = %d, 5 ^ 3 = %d, 1 << 4 = %d\n", 5 & 3, 5 | 3, 5 ^ 3, 1 << 4);

    unsigned bayroqlar = BOR | YOZISH;                      /* bitlarni yoqish */
    printf("bayroqlar = 0x%x (BOR + YOZISH)\n", bayroqlar);
    printf("yozish mumkinmi? %s\n", (bayroqlar & YOZISH) ? "ha" : "yo'q");   /* tekshirish */

    bayroqlar &= ~YOZISH;                                   /* bitni o'chirish */
    printf("YOZISH o'chirildi: 0x%x, yozish mumkinmi? %s\n", bayroqlar,
           (bayroqlar & YOZISH) ? "ha" : "yo'q");

    uint64_t manzil = 0x12345;
    printf("0x%llx sahifa boshi: 0x%llx, sahifa ichidagi siljish: 0x%llx\n",
           (unsigned long long)manzil, (unsigned long long)(manzil & ~0xFFFull),
           (unsigned long long)(manzil & 0xFFF));
    printf("0x%llx ni 4096 ga yuqoriga tekislash: 0x%llx\n", (unsigned long long)manzil,
           (unsigned long long)((manzil + 4095) & ~4095ull));

    unsigned x = 0xB, soni = 0;                             /* 0b1011 */
    while (x) {
        x &= x - 1;                                         /* eng pastki 1-bitni o'chirish */
        soni++;
    }
    printf("0b1011 da 1-bitlar soni: %u\n", soni);
    return 0;
}
