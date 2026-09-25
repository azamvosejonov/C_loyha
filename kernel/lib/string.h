/* =============================================================================
 *  lib/string.h - xotira va satrlar bilan ishlash funksiyalari
 * =============================================================================
 *
 *  Yadroda standart C kutubxonasi (glibc) YO'Q. memcpy, strlen kabi eng oddiy
 *  funksiyalarni ham o'zimiz yozishimiz kerak.
 *
 *  MUHIM: gcc -ffreestanding rejimida ham struktura nusxalash (a = b) yoki katta
 *  massivni nollash uchun memcpy/memset ga AVTOMATIK chaqiruv yaratishi mumkin.
 *  Shuning uchun bu funksiyalar aynan shu nomlar va imzolar bilan bo'lishi SHART.
 * ============================================================================= */
#pragma once

#include <stddef.h>

void *memset(void *dest, int value, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
void *memmove(void *dest, const void *src, size_t count);
int memcmp(const void *a, const void *b, size_t count);

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t max);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strncpy(char *dest, const char *src, size_t n);
/* strlcpy - xavfsiz nusxalash: har doim '\0' bilan tugatadi, buferdan oshmaydi. */
size_t strlcpy(char *dest, const char *src, size_t size);
char *strchr(const char *s, int c);
/* s ichida sub bormi? (oddiy qidiruv) */
char *strstr(const char *s, const char *sub);
