/* =============================================================================
 *  00_salom.c - birinchi dastur                              (darslik 0-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 00_salom.c -o salom && ./salom
 *
 *  Kutilgan natija:
 *      Salom, dunyo!
 *      Men C tilida yozilgan dasturman. 2 + 3 = 5
 *
 *  Sinab ko'ring:
 *      1) printf qatoridagi ; ni o'chirib, xatoni o'qing.
 *      2) return 0 ni return 7 qiling, keyin: ./salom; echo $?
 *      3) \n larni olib tashlab, natija qanday o'zgarishini ko'ring.
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
        printf("massiv[%d] = %d", i, massiv[i]);
    }
    return 0;
}
