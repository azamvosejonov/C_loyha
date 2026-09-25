/* stdlib.h - xotira, dasturdan chiqish, sonlar */
#pragma once
#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
__attribute__((noreturn)) void exit(int code);
__attribute__((noreturn)) void abort(void);
int atoi(const char *s);
long strtol(const char *s, char **end, int base);
unsigned long strtoul(const char *s, char **end, int base);
int abs(int x);
void qsort(void *base, size_t n, size_t size, int (*cmp)(const void *, const void *));
int atexit(void (*fn)(void));
