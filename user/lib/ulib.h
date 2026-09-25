/* =============================================================================
 *  user/lib/ulib.h - user dasturlar uchun kichik "standart kutubxona"
 * =============================================================================
 *
 *  Bu bizning mini "libc"imiz. Haqiqiy tizimda bu glibc yoki musl bo'lardi.
 *  Uch qismdan iborat:
 *    1) syscall o'ramlari   (ulib.c)   - yadroga murojaat
 *    2) satr funksiyalari   (ulib.c)   - strlen, strcmp, ...
 *    3) printf              (printf.c) - formatlangan chiqarish
 *    4) malloc / free       (malloc.c) - sbrk ustiga qurilgan heap
 * ============================================================================= */
#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "myos/abi.h"

#define NULL_FD -1
#define STDIN   0
#define STDOUT  1
#define STDERR  2

/* ---- Syscall o'ramlari ---- */
int fork(void);                         /* otaga: bola pid, bolaga: 0, xato: -1 */
int exec(const char *path, char *const argv[]);    /* faqat xatoda qaytadi */
void *mmap(void *addr, size_t len, int prot, int flags);
int munmap(void *addr, size_t len);
int getppid(void);
__attribute__((noreturn)) void exit(int code);
long write(int fd, const void *buf, size_t len);
long read(int fd, void *buf, size_t len);
int open(const char *path);
int close(int fd);
int spawn(const char *path, char *const argv[]);
int wait(int pid, int *status, int flags);
int getpid(void);
void yield(void);
void sleep_ms(uint64_t ms);
void *sbrk(long increment);
int readdir(int index, struct myos_dirent *out);
int meminfo(struct myos_meminfo *out);
int ps(struct myos_proc_info *buf, int max);
int kill(int pid);
uint64_t uptime_ms(void);
__attribute__((noreturn)) void shutdown(void);
__attribute__((noreturn)) void reboot(void);
int pciinfo(int index, struct myos_pci_info *out);
long dmesg(char *buf, size_t size);
int sysinfo(struct myos_sysinfo *out);

/* ---- Satrlar va xotira ---- */
size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strcpy(char *dst, const char *src);
char *strchr(const char *s, int c);
void *memset(void *dst, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
int memcmp(const void *a, const void *b, size_t n);
int atoi(const char *s);
int isspace(int c);
int isdigit(int c);

/* ---- Kiritish/chiqarish ---- */
int putchar(int c);
int puts(const char *s);                /* satr + '\n' */
int getchar(void);                      /* -1 = xato */
int printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int vprintf(const char *fmt, va_list ap);
int snprintf(char *buf, size_t size, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);
/* Qatorni o'qish (echo va backspace bilan). Qaytaradi: uzunlik yoki -1. */
int readline(char *buf, int max);

/* ---- Dinamik xotira (malloc.c) ---- */
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);

struct malloc_stats {
    size_t heap_bytes;                  /* sbrk orqali olingan jami */
    size_t used_bytes;                  /* band bloklar (foydali yuk) */
    size_t free_bytes;                  /* bo'sh bloklar */
    size_t free_blocks;                 /* bo'sh bloklar soni (fragmentatsiya o'lchovi) */
    size_t used_blocks;
};
void malloc_get_stats(struct malloc_stats *out);
