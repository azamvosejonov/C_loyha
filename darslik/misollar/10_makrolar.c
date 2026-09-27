/* =============================================================================
 *  10_makrolar.c - #define, makro tuzoqlari, #x, __LINE__, #ifdef  (10-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 10_makrolar.c -o makrolar && ./makrolar
 *      gcc -Wall -Wextra -g -DDEBUG 10_makrolar.c -o makrolar && ./makrolar   (DEBUG bilan)
 *
 *  Makrolar qanday "ochilganini" ko'rish:
 *      gcc -E 10_makrolar.c | tail -40
 *
 *  Kutilgan natija (DEBUG'siz):
 *      N = 10, massiv: 0 1 4 9 16 25 36 49 64 81
 *      YOMON_KVADRAT(2 + 3) = 11 (25 kutilgandi!)
 *      KVADRAT(2 + 3) = 25
 *      MAX(i++, 4) = 6, lekin i = 7 (i++ ikki marta bajarildi!)
 *      TEKSHIR: [XATO] 10_makrolar.c:65: 2 + 2 == 5
 *      ARRAY_SIZE(massiv) = 10
 *  DEBUG bilan qo'shimcha:
 *      [debug] DEBUG rejimi yoqilgan
 *
 *  Sinab ko'ring:
 *      1) `gcc -E 10_makrolar.c | tail -40` - YOMON_KVADRAT(2 + 3) nimaga ochilganini toping.
 *      2) N ni 20 qiling: faqat BITTA qator o'zgardi, lekin massiv, ikkala sikl va ARRAY_SIZE
 *         hammasi moslashdi. #define N ning asosiy foydasi - shu.
 *      3) `#define N 10` ni `#ifndef N` / `#define N 10` / `#endif` ichiga oling va
 *         `gcc -DN=5 ...` bilan yig'ing - N ni buyruq qatoridan berdingiz.
 *      4) TEKSHIR ichidagi `do { ... } while (0)` ni oddiy `{ ... }` bilan almashtiring.
 *         Nega endi `else` da xato chiqdi ("'else' without a previous 'if'")?
 *      5) MAX o'rniga `static inline int max_f(int a, int b)` yozing - i++ endi necha marta
 *         bajariladi?
 * ============================================================================= */
#include <stdio.h>

#define N 10                                    /* o'zgarmas: hamma N -> 10 */
#define YOMON_KVADRAT(x) x * x                  /* qavssiz - XATO */
#define KVADRAT(x) ((x) * (x))                  /* to'g'ri: har bir parametr va butun ifoda qavsda */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* Ko'p buyruqli makro: do { ... } while (0) ichida (10.5-bo'lim) */
#define TEKSHIR(shart)                                                          \
    do {                                                                        \
        if (!(shart))                                                           \
            printf("TEKSHIR: [XATO] %s:%d: %s\n", __FILE__, __LINE__, #shart);  \
    } while (0)

int main(void)
{
    int massiv[N];
    for (int i = 0; i < N; i++)
        massiv[i] = KVADRAT(i);
    printf("N = %d, massiv:", N);
    for (int i = 0; i < N; i++)
        printf(" %d", massiv[i]);
    printf("\n");

    printf("YOMON_KVADRAT(2 + 3) = %d (25 kutilgandi!)\n", YOMON_KVADRAT(2 + 3));  /* 2 + 3 * 2 + 3 */
    printf("KVADRAT(2 + 3) = %d\n", KVADRAT(2 + 3));

    int i = 5;
    int m = MAX(i++, 4);                        /* ((i++) > (4) ? (i++) : (4)) */
    printf("MAX(i++, 4) = %d, lekin i = %d (i++ ikki marta bajarildi!)\n", m, i);

    if (1)
        TEKSHIR(2 + 2 == 5);                    /* do-while(0) tufayli if ichida to'g'ri ishlaydi */
    else
        printf("bu chiqmaydi\n");

    printf("ARRAY_SIZE(massiv) = %zu\n", ARRAY_SIZE(massiv));

#ifdef DEBUG
    printf("[debug] DEBUG rejimi yoqilgan\n");
#endif
    return 0;
}
