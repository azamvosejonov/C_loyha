/* =============================================================================
 *  lib/string.c - string.h dagi funksiyalarning amalga oshirilishi
 * =============================================================================
 *
 *  Bu yerda soddalik tanlangan: bayt-baytdan ishlaymiz. Haqiqiy kutubxonalar
 *  (glibc, musl) 8 yoki 32 baytdan nusxalaydi, SIMD ishlatadi. Mashq sifatida
 *  memcpy ni 8 baytlik bo'laklar bilan tezlashtirib ko'ring (docs/mashqlar.md).
 * ============================================================================= */
#include "lib/string.h"

#include <stdint.h>

void *memset(void *dest, int value, size_t count)
{
    uint8_t *d = dest;                  /* void* ni bayt ko'rsatkichiga aylantiramiz */
    while (count--)                     /* count marta takrorlash */
        *d++ = (uint8_t)value;          /* baytni yozib, keyingisiga o'tish */
    return dest;                        /* standart bo'yicha dest qaytariladi */
}

void *memcpy(void *dest, const void *src, size_t count)
{
    uint8_t *d = dest;
    const uint8_t *s = src;
    while (count--)
        *d++ = *s++;
    return dest;
}

/* memmove - memcpy dan farqi: manba va maqsad BIR-BIRIGA KIRISHIB ketgan
 * (overlap) bo'lsa ham to'g'ri ishlaydi. Agar dest > src bo'lsa, oxiridan
 * boshlab nusxalaymiz - aks holda hali o'qilmagan baytlarni ustidan yozib
 * yuboramiz. */
void *memmove(void *dest, const void *src, size_t count)
{
    uint8_t *d = dest;
    const uint8_t *s = src;
    if (d == s || count == 0)
        return dest;
    if (d < s) {
        while (count--)                 /* oldinga qarab: xavfsiz */
            *d++ = *s++;
    } else {
        d += count;                     /* orqaga qarab: oxiridan boshlaymiz */
        s += count;
        while (count--)
            *--d = *--s;
    }
    return dest;
}

int memcmp(const void *a, const void *b, size_t count)
{
    const uint8_t *x = a;
    const uint8_t *y = b;
    for (size_t i = 0; i < count; i++) {
        if (x[i] != y[i])
            return x[i] < y[i] ? -1 : 1;  /* birinchi farq qilgan bayt hal qiladi */
    }
    return 0;                           /* hammasi teng */
}

size_t strlen(const char *s)
{
    size_t n = 0;
    while (s[n])                        /* '\0' (nol bayt) gacha sanaymiz */
        n++;
    return n;
}

size_t strnlen(const char *s, size_t max)
{
    size_t n = 0;
    while (n < max && s[n])             /* max dan oshmaymiz - buzilgan satrdan himoya */
        n++;
    return n;
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) {            /* teng va tugamagan bo'lsa davom etamiz */
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n)
{
    while (n && *a && *a == *b) {
        a++;
        b++;
        n--;
    }
    if (n == 0)
        return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    size_t i = 0;
    for (; i < n && src[i]; i++)
        dest[i] = src[i];
    for (; i < n; i++)                  /* standart bo'yicha qolgan joy nollanadi */
        dest[i] = '\0';
    return dest;
}

size_t strlcpy(char *dest, const char *src, size_t size)
{
    size_t len = strlen(src);
    if (size) {
        size_t n = len < size - 1 ? len : size - 1;   /* '\0' uchun joy qoldiramiz */
        memcpy(dest, src, n);
        dest[n] = '\0';
    }
    return len;                         /* qaytgan qiymat >= size bo'lsa - satr kesildi */
}

char *strchr(const char *s, int c)
{
    for (;; s++) {
        if (*s == (char)c)
            return (char *)s;
        if (*s == '\0')
            return NULL;
    }
}

char *strstr(const char *s, const char *sub)
{
    size_t n = strlen(sub);
    if (n == 0)
        return (char *)s;
    for (; *s; s++) {
        if (*s == *sub && strncmp(s, sub, n) == 0)
            return (char *)s;
    }
    return NULL;
}
