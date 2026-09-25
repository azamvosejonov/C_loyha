/* =============================================================================
 *  user/bin/libctest.c - libc funksiyalari uchun unit testlar
 * =============================================================================
 *
 *  Lab tizimi uchun (labs/README.md): libc funksiyasini o'zingiz qayta
 *  yozganingizda, xato AYNAN qaysi funksiyada ekanini shu test aytadi.
 *  Har bir tekshiruv chegaraviy holatlarni sinaydi - xatolar odatda aynan
 *  o'sha yerda yashiringan bo'ladi (bo'sh satr, ustma-ust bloklar, to'lish).
 * ============================================================================= */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int failures, checks;

#define CHECK(cond, what)                                                        \
    do {                                                                         \
        checks++;                                                                \
        if (!(cond)) {                                                           \
            failures++;                                                          \
            printf("  [FAIL] %s: %s  (%s:%d)\n", what, #cond, __FILE__, __LINE__); \
        }                                                                        \
    } while (0)

static void test_strlen(void)
{
    CHECK(strlen("") == 0, "strlen");
    CHECK(strlen("a") == 1, "strlen");
    CHECK(strlen("salom dunyo") == 11, "strlen");
    char big[1000];
    memset(big, 'x', sizeof(big) - 1);
    big[999] = '\0';
    CHECK(strlen(big) == 999, "strlen");
    CHECK(strlen("ab\0cd") == 2, "strlen: birinchi \\0 da to'xtaydi");
}

static void test_memmove(void)
{
    char a[16] = "0123456789";
    memmove(a + 2, a, 5);               /* o'ngga, ustma-ust: orqadan nusxalash kerak */
    CHECK(memcmp(a, "0101234789", 10) == 0, "memmove o'ngga (ustma-ust)");
    char b[16] = "0123456789";
    memmove(b, b + 3, 5);               /* chapga, ustma-ust: oldindan nusxalash kerak */
    CHECK(memcmp(b, "3456756789", 10) == 0, "memmove chapga (ustma-ust)");
    char c[16] = "abcdef";
    CHECK(memmove(c, c, 6) == c && strcmp(c, "abcdef") == 0, "memmove o'ziga");
    char d[8] = "xyz", e[8] = "";
    CHECK(memmove(e, d, 4) == e && strcmp(e, "xyz") == 0, "memmove qaytish qiymati = dst");
    CHECK(memmove(e, d, 0) == e, "memmove n=0");
}

static void test_strtok(void)
{
    char s[] = "  ls -l ;; /bin  ";
    char *save, *t;
    const char *want[] = { "ls", "-l", "/bin" };
    int n = 0, ok = 1;
    for (t = strtok_r(s, " ;", &save); t; t = strtok_r(NULL, " ;", &save)) {
        if (n >= 3 || strcmp(t, want[n]) != 0)
            ok = 0;
        n++;
    }
    CHECK(ok && n == 3, "strtok_r: bo'shliq va ; bilan ajratish");
    char e[] = "   ";
    CHECK(strtok_r(e, " ", &save) == NULL, "strtok_r: faqat ajratuvchilar -> NULL");
    char one[] = "bitta";
    CHECK(strcmp(strtok_r(one, ",", &save), "bitta") == 0 && strtok_r(NULL, ",", &save) == NULL,
          "strtok_r: ajratuvchisiz satr");
}

static void test_strtoul(void)
{
    char *end;
    CHECK(strtoul("123", &end, 10) == 123 && *end == '\0', "strtoul o'nlik");
    CHECK(strtoul("  42xyz", &end, 10) == 42 && strcmp(end, "xyz") == 0, "strtoul: bo'shliq va oxiri");
    CHECK(strtoul("0x1F", &end, 0) == 31, "strtoul base 0: 0x -> 16");
    CHECK(strtoul("017", &end, 0) == 15, "strtoul base 0: 0 -> 8");
    CHECK(strtoul("ff", &end, 16) == 255, "strtoul base 16");
    CHECK(strtoul("zz", &end, 36) == 35 * 36 + 35, "strtoul base 36");
    const char *s = "abc";
    CHECK(strtoul(s, &end, 10) == 0 && end == s, "strtoul: raqam yo'q -> end = boshi");
    errno = 0;
    CHECK(strtoul("99999999999999999999999", &end, 10) == 0xFFFFFFFFFFFFFFFFUL && errno == ERANGE,
          "strtoul: to'lish -> ULONG_MAX, ERANGE");
    CHECK(strtol("-17", &end, 10) == -17, "strtol manfiy");
    CHECK(atoi("  -250") == -250, "atoi");
}

static void test_gmtime(void)
{
    struct tm tm;
    time_t t = 0;
    gmtime_r(&t, &tm);
    CHECK(tm.tm_year == 70 && tm.tm_mon == 0 && tm.tm_mday == 1 && tm.tm_hour == 0 &&
          tm.tm_wday == 4, "gmtime_r(0) = 1970-01-01, payshanba");
    t = 951782400;                      /* 2000-02-29 00:00:00 - kabisa kuni */
    gmtime_r(&t, &tm);
    CHECK(tm.tm_year == 100 && tm.tm_mon == 1 && tm.tm_mday == 29, "gmtime_r: 2000-02-29 (kabisa)");
    t = 4107542399;                     /* 2100-02-28 23:59:59 - 2100 kabisa EMAS */
    gmtime_r(&t, &tm);
    CHECK(tm.tm_year == 200 && tm.tm_mon == 1 && tm.tm_mday == 28 && tm.tm_hour == 23 &&
          tm.tm_min == 59 && tm.tm_sec == 59, "gmtime_r: 2100-02-28 23:59:59");
    t += 1;
    gmtime_r(&t, &tm);
    CHECK(tm.tm_mon == 2 && tm.tm_mday == 1, "gmtime_r: 2100 - 29-fevral yo'q");
    t = 1790208000;                     /* 2026-09-24 */
    gmtime_r(&t, &tm);
    CHECK(tm.tm_year == 126 && tm.tm_mon == 8 && tm.tm_mday == 24 && tm.tm_wday == 4 &&
          tm.tm_yday == 266, "gmtime_r: 2026-09-24, payshanba, yilning 267-kuni");
    t = -86400;
    gmtime_r(&t, &tm);
    CHECK(tm.tm_year == 69 && tm.tm_mon == 11 && tm.tm_mday == 31, "gmtime_r: 1969-12-31 (manfiy)");
}

#ifndef HOST_TEST
static void test_malloc(void)
{
    char *p[64];
    int ok = 1;
    for (int i = 0; i < 64; i++) {
        p[i] = malloc((size_t)(i * 37 + 1));
        if (!p[i] || ((unsigned long)p[i] & 15))
            ok = 0;                     /* 16 baytga tekislangan bo'lishi kerak */
        else
            memset(p[i], i, (size_t)(i * 37 + 1));
    }
    CHECK(ok, "malloc: 64 blok, 16 ga tekislangan");
    for (int i = 0; i < 64 && ok; i++)
        for (int k = 0; k < i * 37 + 1; k++)
            if ((unsigned char)p[i][k] != (unsigned char)i)
                ok = 0;
    CHECK(ok, "malloc: bloklar bir-birini bosib ketmagan");
    for (int i = 0; i < 64; i += 2)
        free(p[i]);
    for (int i = 1; i < 64; i += 2)
        free(p[i]);
    CHECK(malloc(0) == NULL, "malloc(0) = NULL");
    void *big = malloc(1 << 20);
    CHECK(big != NULL, "malloc 1 MB");
    free(big);
    char *r = malloc(10);
    strcpy(r, "salom");
    r = realloc(r, 5000);
    CHECK(r && strcmp(r, "salom") == 0, "realloc mazmunni saqlaydi");
    free(r);
}
#endif

static void test_snprintf(void)
{
    char b[64];
    snprintf(b, sizeof(b), "[%5d|%-5d|%05d]", 42, 42, 42);
    CHECK(strcmp(b, "[   42|42   |00042]") == 0, "snprintf kenglik");
    snprintf(b, sizeof(b), "%x %X %o %lu", 255, 255, 8, 18446744073709551615UL);
    CHECK(strcmp(b, "ff FF 10 18446744073709551615") == 0, "snprintf asoslar");
    /* NULL satr "(null)" bo'lib chiqishi kerak. volatile - GCC buni kompilyatsiya
     * paytida "ko'rib" ogohlantirmasligi uchun (biz aynan shu holatni sinaymiz). */
    const char *volatile null_str = NULL;
    snprintf(b, sizeof(b), "%.3s|%c|%%|%s", "salom", 'Z', null_str);
    CHECK(strcmp(b, "sal|Z|%|(null)") == 0, "snprintf satrlar");
    CHECK(snprintf(b, 4, "abcdef") == 6 && strcmp(b, "abc") == 0, "snprintf qirqish");
}

static int cmp_int(const void *a, const void *b)
{
    return *(const int *)a - *(const int *)b;
}

static void test_qsort(void)
{
    int a[100];
    for (int i = 0; i < 100; i++)
        a[i] = (i * 7919) % 101;
    qsort(a, 100, sizeof(int), cmp_int);
    int ok = 1;
    for (int i = 1; i < 100; i++)
        ok &= a[i - 1] <= a[i];
    CHECK(ok, "qsort 100 ta son");
}

int main(void)
{
    puts("libctest: libc funksiyalari");
    test_strlen();
    test_memmove();
    test_strtok();
    test_strtoul();
    test_gmtime();
#ifndef HOST_TEST                       /* host testida malloc yo'q (sbrk kerak) */
    test_malloc();
#endif
    test_snprintf();
    test_qsort();
    if (failures == 0)
        printf("libctest: PASSED (%d tekshiruv)\n", checks);
    else
        printf("libctest: %d/%d FAIL\n", failures, checks);
    return failures != 0;
}
