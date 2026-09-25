/* =============================================================================
 *  user/libc/syscall.h - yadroga murojaat qilishning eng past darajasi (ichki)
 * =============================================================================
 *
 *  SYSCALL QANDAY CHAQIRILADI (myos/abi.h ga qarang):
 *    RAX = raqam, RDI/RSI/RDX/R10/R8 = argumentlar, `syscall`, natija RAX da.
 *    `syscall` instruksiyasi RCX (qaytish manzili) va R11 (RFLAGS) ni BUZADI -
 *    shuning uchun ular "clobber" ro'yxatida.
 *
 *  Inline asm cheklovlari:
 *    "a"(n)  - n ni RAX ga        "D"(a1) - RDI ga
 *    "S"(a2) - RSI ga             "d"(a3) - RDX ga
 *    "=a"(r) - natijani RAX dan
 *    "memory" - yadro xotiramizni o'qishi/yozishi mumkin (buf), kompilyator
 *               xotira operatsiyalarini syscall atrofida ko'chirmasin.
 *    R10/R8 uchun alohida harf yo'q - "register ... asm("r10")" o'zgaruvchisi.
 *
 *  XATOLAR: yadro -errno qaytaradi (masalan, -2 = -ENOENT). POSIX an'anasi
 *  boshqacha: funksiya -1 qaytaradi va sababni global `errno` ga yozadi.
 *  __sysret() shu tarjimani qiladi. Linux'da ham xuddi shunday: -4095..-1
 *  oralig'i xato, qolgan hamma narsa (hatto katta manzillar) - natija.
 * ============================================================================= */
#pragma once

#include <errno.h>

static inline long __syscall0(long n)
{
    long r;
    __asm__ volatile("syscall" : "=a"(r) : "a"(n) : "rcx", "r11", "memory");
    return r;
}

static inline long __syscall1(long n, long a1)
{
    long r;
    __asm__ volatile("syscall" : "=a"(r) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
    return r;
}

static inline long __syscall2(long n, long a1, long a2)
{
    long r;
    __asm__ volatile("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2) : "rcx", "r11", "memory");
    return r;
}

static inline long __syscall3(long n, long a1, long a2, long a3)
{
    long r;
    __asm__ volatile("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2), "d"(a3)
                     : "rcx", "r11", "memory");
    return r;
}

static inline long __syscall4(long n, long a1, long a2, long a3, long a4)
{
    long r;
    register long r10 __asm__("r10") = a4;
    __asm__ volatile("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
                     : "rcx", "r11", "memory");
    return r;
}

/* Yadro natijasini POSIX ko'rinishiga: xato bo'lsa errno = kod, qaytish = -1. */
static inline long __sysret(long r)
{
    if (r < 0 && r >= -4095) {
        errno = (int)-r;
        return -1;
    }
    return r;
}
