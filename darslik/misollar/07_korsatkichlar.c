/* =============================================================================
 *  07_korsatkichlar.c - &, *, ->, arifmetika, void *, funksiya ko'rsatkichi (7-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 07_korsatkichlar.c -o korsatkich && ./korsatkich
 *
 *  Kutilgan natija (manzillar har safar boshqacha bo'ladi):
 *      x = 42, &x = 0x7ffe...(manzil), p = (xuddi o'sha manzil), *p = 42
 *      *p = 100 dan keyin x = 100
 *      a[1] = 20, *(a+1) = 20, p2[2] = 40, p2 - a = 1
 *      int* + 1 -> +4 bayt, char* + 1 -> +1 bayt
 *      nuqta: (3, 4) -> p->x = 10 dan keyin (10, 4)
 *      qoshish(5, 3) = 8, ayirish(5, 3) = 2
 *      argv[0] = ./korsatkich
 *
 *  Sinab ko'ring:
 *      1) `*p = 100;` dan keyin `p++; printf("%d\n", *p);` qo'shing va -fsanitize=address bilan
 *         yig'ing. ASan nima deydi? (p endi x dan KEYINGI joyni ko'rsatadi.)
 *      2) `int *q; *q = 5;` yozing. -Wall nima deydi? Ishga tushirsangiz-chi?
 *      3) `(void)argc;` ni o'chirib, barcha argumentlarni chiqaradigan sikl yozing va
 *         `./korsatkich bir ikki uch` bilan sinang.
 *      4) Funksiya ko'rsatkichlari massivi: `int (*amallar[])(int, int) = { qoshish, ayirish };`
 *         va sikl bilan ikkalasini chaqiring.
 * ============================================================================= */
#include <stdio.h>

struct nuqta {
    int x, y;
};

static int qoshish(int a, int b) { return a + b; }
static int ayirish(int a, int b) { return a - b; }

int main(int argc, char **argv)
{
    (void)argc;
    int x = 42;
    int *p = &x;                                    /* p - x ning manzili */
    printf("x = %d, &x = %p, p = %p, *p = %d\n", x, (void *)&x, (void *)p, *p);
    *p = 100;                                       /* p orqali x ni o'zgartirish */
    printf("*p = 100 dan keyin x = %d\n", x);

    int a[] = { 10, 20, 30, 40 };
    int *p2 = a + 1;                                /* a[1] ning manzili */
    printf("a[1] = %d, *(a+1) = %d, p2[2] = %d, p2 - a = %td\n", a[1], *(a + 1), p2[2], p2 - a);

    char c[4];
    printf("int* + 1 -> +%td bayt, char* + 1 -> +%td bayt\n",
           (char *)(a + 1) - (char *)a, (c + 1) - c);

    struct nuqta n = { 3, 4 };
    struct nuqta *np = &n;
    printf("nuqta: (%d, %d) -> ", n.x, n.y);
    np->x = 10;                                     /* (*np).x = 10 */
    printf("p->x = 10 dan keyin (%d, %d)\n", n.x, n.y);

    int (*amal)(int, int) = qoshish;                /* funksiya ko'rsatkichi */
    printf("qoshish(5, 3) = %d, ", amal(5, 3));
    amal = ayirish;
    printf("ayirish(5, 3) = %d\n", amal(5, 3));

    printf("argv[0] = %s\n", argv[0]);              /* char ** - satrlar massivi */
    return 0;
}
