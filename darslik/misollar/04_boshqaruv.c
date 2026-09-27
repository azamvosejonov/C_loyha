/* =============================================================================
 *  04_boshqaruv.c - if, for, while, switch, break/continue   (darslik 4-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 04_boshqaruv.c -o boshqaruv && ./boshqaruv
 *
 *  Kutilgan natija:
 *      FizzBuzz: 1 2 Fizz 4 Buzz Fizz 7 8 Fizz Buzz 11 Fizz 13 14 FizzBuzz
 *      12345 raqamlari yig'indisi: 15
 *      teskari (size_t bilan): 4 3 2 1 0
 *      '+' -> qo'shish, '*' -> ko'paytirish, '?' -> noma'lum
 *      birinchi manfiy: indeks 3
 *
 *  Sinab ko'ring:
 *      1) FizzBuzz'da `i % 15 == 0` tekshiruvini zanjirning ENG OXIRIGA ko'chiring. Nega endi
 *         "FizzBuzz" hech qachon chiqmaydi? (15 ni qaysi shart birinchi "ushlab qoladi"?)
 *      2) Teskari siklni `for (size_t i = 4; i >= 0; i--)` ga almashtiring. -Wextra nima deydi
 *         ("always true")? Dastur nega to'xtamaydi? (Ctrl+C bilan to'xtating.)
 *      3) amal_nomi() da `return "qo'shish";` qatorini o'chiring. '+' uchun endi nima chiqadi?
 *         (case'lar "pastga oqadi" - break/return bo'lmasa keyingi case bajariladi.)
 *      4) switch'ga '/' -> "bo'lish" qo'shing.
 * ============================================================================= */
#include <stdio.h>

static const char *amal_nomi(char c)
{
    switch (c) {
    case '+':
        return "qo'shish";
    case '-':
        return "ayirish";
    case '*':
        return "ko'paytirish";
    default:
        return "noma'lum";
    }
}

int main(void)
{
    printf("FizzBuzz:");
    for (int i = 1; i <= 15; i++) {
        if (i % 15 == 0)
            printf(" FizzBuzz");
        else if (i % 3 == 0)
            printf(" Fizz");
        else if (i % 5 == 0)
            printf(" Buzz");
        else
            printf(" %d", i);
    }
    printf("\n");

    int n = 12345, yigindi = 0;
    while (n > 0) {
        yigindi += n % 10;          /* oxirgi raqam */
        n /= 10;                    /* oxirgi raqamni olib tashlash */
    }
    printf("12345 raqamlari yig'indisi: %d\n", yigindi);

    printf("teskari (size_t bilan):");
    for (size_t i = 5; i-- > 0;)    /* size_t bilan to'g'ri teskari sikl */
        printf(" %zu", i);
    printf("\n");

    printf("'+' -> %s, '*' -> %s, '?' -> %s\n", amal_nomi('+'), amal_nomi('*'), amal_nomi('?'));

    int a[] = { 4, 7, 2, -5, 8, -1 };
    for (int i = 0; i < 6; i++) {
        if (a[i] >= 0)
            continue;               /* musbatlarni o'tkazib yuborish */
        printf("birinchi manfiy: indeks %d\n", i);
        break;                      /* topdik - sikldan chiqish */
    }
    return 0;
}
