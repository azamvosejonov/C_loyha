/* stdio.h - buferlangan kiritish/chiqarish */
#pragma once
#include <stdarg.h>
#include <stddef.h>

#define EOF (-1)
#define BUFSIZ 4096

typedef struct FILE FILE;
extern FILE *stdin, *stdout, *stderr;

FILE *fopen(const char *path, const char *mode);
FILE *fdopen(int fd, const char *mode);
int fclose(FILE *f);
int fflush(FILE *f);
size_t fread(void *buf, size_t size, size_t n, FILE *f);
size_t fwrite(const void *buf, size_t size, size_t n, FILE *f);
int fgetc(FILE *f);
char *fgets(char *buf, int size, FILE *f);
int fputc(int c, FILE *f);
int fputs(const char *s, FILE *f);
int feof(FILE *f);
int fileno(FILE *f);

int printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int fprintf(FILE *f, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int dprintf(int fd, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int sprintf(char *buf, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
int snprintf(char *buf, size_t size, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
int vprintf(const char *fmt, va_list ap);
int vfprintf(FILE *f, const char *fmt, va_list ap);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);
int rename(const char *from, const char *to);
int remove(const char *path);
int putchar(int c);
int puts(const char *s);
int getchar(void);
void perror(const char *msg);
int ferror(FILE *f);
void clearerr(FILE *f);

#define _IONBF 0                        /* buferlanmagan */
#define _IOLBF 1                        /* qator bo'yicha */
#define _IOFBF 2                        /* to'liq */
int setvbuf(FILE *f, char *buf, int mode, size_t size);
