/* =============================================================================
 *  00_massiv_N.c - #define N bilan massivni to'ldirish va chiqarish
 * =============================================================================
 *  Bu fayl o'quvchining o'zi yozgan variant: u GitHub'da 00_salom.c o'rniga tahrirlangan edi.
 *  Asl "Salom, dunyo" misoli 00_salom.c da qoldi (0- va 1-boblar shunga tayanadi),
 *  bu esa alohida saqlandi. 10-bobdagi #define mavzusiga ham mos keladi.
 *
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 00_massiv_N.c -o massiv_n && ./massiv_n
 *
 *  Kutilgan natija:
 *      massiv[0] = 0
 *      massiv[1] = 1
 *      massiv[2] = 4
 *      ...
 *      massiv[9] = 81
 *
 *  Sinab ko'ring:
 *      1) N ni 5 qiling - faqat BITTA qatorni o'zgartirdingiz, lekin ikkala sikl ham moslashdi.
 *      2) massiv[i] = i * i ni i * 2 ga almashtiring.
 * ============================================================================= */
#include <stdio.h>
#define N 10

int main(void)
{
    int massiv[N];
    for (int i = 0; i < N; i++) {
        massiv[i] = i * i;

    }

    for (int i = 0; i < N; i++) {
        printf("massiv[%d] = %d\n", i, massiv[i]);
    }
    return 0;
}
