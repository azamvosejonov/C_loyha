/* =============================================================================
 *  06_satrlar.c - massivlar va satrlar                       (darslik 6-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 06_satrlar.c -o satrlar && ./satrlar
 *
 *  Kutilgan natija:
 *      massiv: 10 20 30 40 50 (5 ta element, 20 bayt)
 *      "salom": sizeof = 6, strlen = 5
 *      s[0..5]: 's' 'a' 'l' 'o' 'm' '\0'
 *      strcmp: teng
 *      == bilan solishtirish: manzillar boshqa (mazmun emas!)
 *      snprintf: "/home/ali/hujjatlar/fayl.txt" (28 belgi)
 *      kichik buferga: "/home/ali/" (kerak edi 28)
 *      katta harf: SALOM DUNYO
 *
 *  Sinab ko'ring:
 *      1) `char kichik[11]` ni `char kichik[5]` qiling. Qator nimaga qisqaradi? `k` o'zgaradimi?
 *      2) Massivni funksiyaga uzatib, funksiya ichida sizeof(a) ni chop eting: `void f(int a[])`.
 *         Kompilyator ogohlantirishini o'qing - nega 20 emas, 8?
 *      3) `char s[] = "salom";` ni `char *s = "salom";` qiling va -fsanitize=address bilan yig'ing.
 *         sizeof(s) endi nechaga teng? Pastdagi sikl nega chegaradan chiqib ketdi?
 *      4) `strcmp(buyruq, "exit") == 0` dan `== 0` ni o'chiring. Natija nega teskari bo'ldi?
 * ============================================================================= */
#include <ctype.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    int a[] = { 10, 20, 30, 40, 50 };
    size_t n = sizeof(a) / sizeof(a[0]);            /* faqat massiv e'lon qilingan joyda ishlaydi */
    printf("massiv:");
    for (size_t i = 0; i < n; i++)
        printf(" %d", a[i]);
    printf(" (%zu ta element, %zu bayt)\n", n, sizeof(a));

    char s[] = "salom";
    printf("\"salom\": sizeof = %zu, strlen = %zu\n", sizeof(s), strlen(s));
    printf("s[0..5]:");
    for (size_t i = 0; i < sizeof(s); i++)
        printf(s[i] ? " '%c'" : " '\\0'", s[i]);
    printf("\n");

    char buyruq[16] = "exit";
    printf("strcmp: %s\n", strcmp(buyruq, "exit") == 0 ? "teng" : "teng emas");
    printf("== bilan solishtirish: %s\n",
           (void *)buyruq == (void *)"exit" ? "teng" : "manzillar boshqa (mazmun emas!)");

    /* volatile - kompilyator satr uzunligini oldindan "ko'rib" ogohlantirmasligi uchun
     * (haqiqiy dasturda bu qiymatlar foydalanuvchidan keladi va oldindan noma'lum). */
    const char *volatile papka = "/home/ali/hujjatlar", *volatile fayl = "fayl.txt";
    char yol[64];
    int k = snprintf(yol, sizeof(yol), "%s/%s", papka, fayl);
    printf("snprintf: \"%s\" (%d belgi)\n", yol, k);

    char kichik[11];                                /* 10 belgi + '\0' */
    k = snprintf(kichik, sizeof(kichik), "%s/%s", papka, fayl);   /* sig'maydi - qirqiladi */
    printf("kichik buferga: \"%s\" (kerak edi %d)\n", kichik, k);

    char matn[] = "salom dunyo";
    for (char *p = matn; *p; p++)
        *p = (char)toupper((unsigned char)*p);      /* (unsigned char) - 6.7-bo'lim */
    printf("katta harf: %s\n", matn);
    return 0;
}
