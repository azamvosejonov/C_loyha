/* =============================================================================
 *  08_xotira.c - malloc, realloc, free: dinamik massiv       (darslik 8-bob)
 * =============================================================================
 *  Ishga tushirish (sanitizer bilan - xotira xatolari darhol ko'rinadi):
 *      gcc -Wall -Wextra -g -fsanitize=address 08_xotira.c -o xotira && ./xotira
 *
 *  Kutilgan natija:
 *      sig'im o'zgardi: 0 -> 4 -> 8 -> 16 -> 32
 *      20 ta element: 0 1 4 9 16 ... 361
 *      static hisoblagich: 1 2 3
 *      bitta printf ichida: ? ? ? (tartib KAFOLATLANMAGAN - GCC'da odatda 6 5 4)
 *      xotira ozod qilindi
 *
 *  Sinab ko'ring:
 *      1) free(a) qatorini o'chiring - AddressSanitizer "memory leak" deydi.
 *      2) a[20] = 1; qo'shing (chegaradan tashqari) - "heap-buffer-overflow".
 * ============================================================================= */
#include <stdio.h>
#include <stdlib.h>

static int keyingi_raqam(void)
{
    static int n = 0;           /* statik xotirada: chaqiruvlar orasida yashaydi */
    return ++n;
}

int main(void)
{
    int *a = NULL;
    size_t soni = 0, sigim = 0;
    printf("sig'im o'zgardi: %zu", sigim);
    for (int i = 0; i < 20; i++) {
        if (soni == sigim) {
            size_t yangi = sigim ? sigim * 2 : 4;
            int *t = realloc(a, yangi * sizeof(int));   /* avval vaqtinchalik o'zgaruvchiga! */
            if (!t) {
                free(a);
                return 1;
            }
            a = t;
            sigim = yangi;
            printf(" -> %zu", sigim);
        }
        a[soni++] = i * i;
    }
    printf("\n20 ta element: %d %d %d %d %d ... %d\n", a[0], a[1], a[2], a[3], a[4], a[19]);

    int x = keyingi_raqam(), y = keyingi_raqam(), z = keyingi_raqam();
    printf("static hisoblagich: %d %d %d\n", x, y, z);
    /* DIQQAT: funksiya argumentlarining hisoblanish TARTIBI C'da aniqlanmagan
     * (13-bob, 13.6). Quyidagi qator 4 5 6 emas, 6 5 4 chiqarishi mumkin! */
    printf("bitta printf ichida: %d %d %d\n", keyingi_raqam(), keyingi_raqam(), keyingi_raqam());

    free(a);                    /* har bir malloc/realloc ga - aniq bitta free */
    a = NULL;
    printf("xotira ozod qilindi\n");
    return 0;
}
