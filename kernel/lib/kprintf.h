/* =============================================================================
 *  lib/kprintf.h - yadro uchun printf
 * ============================================================================= */
#pragma once

#include <stdarg.h>
#include <stddef.h>

/* __attribute__((format(printf, 1, 2))) - gcc'ga "bu printf kabi funksiya,
 * 1-argument format satri, 2-dan boshlab qiymatlar" deydi. Shunda gcc
 * kprintf("%d", "salom") kabi xatolarni KOMPILYATSIYA paytida topadi. */
int kprintf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int kvprintf(const char *fmt, va_list ap);

/* Buferga yozish. Har doim '\0' bilan tugatadi, size dan oshmaydi.
 * Qaytaradi: to'liq natija uzunligi (kesilgan bo'lsa ham). */
int ksnprintf(char *buf, size_t size, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
int kvsnprintf(char *buf, size_t size, const char *fmt, va_list ap);
