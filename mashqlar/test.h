/* =============================================================================
 *  mashqlar/test.h - mashqlar uchun kichik test "freymvork"i
 * =============================================================================
 *
 *  Har bir mashqning test.c fayli shu makrolardan foydalanadi. Siz bu faylni
 *  o'zgartirmaysiz - faqat yechim.c ni yozasiz.
 *
 *  NEGA MAKRO (funksiya emas): makro __FILE__, __LINE__ va #shart (shartning
 *  MATNI) ni ishlatib, xato qaysi qatorda va qaysi tekshiruvda ekanini aniq
 *  ko'rsatadi. Funksiya buni bila olmaydi. (Yadrodagi CHECK ham xuddi shunday.)
 * ============================================================================= */
#pragma once

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int t_checks, t_fail;

/* stdout buferlanmasin: dastur xotira xatosi bilan yiqilsa ham, undan oldingi
 * hamma xabarlar ekranga chiqib ulgursin. */
#define TEST_BOSHLA() setvbuf(stdout, NULL, _IONBF, 0)

/* Test bo'limi nomi - yiqilish qaysi bo'limda bo'lganini ko'rish uchun. */
#define BOLIM(nom) printf("-- %s\n", nom)

#define CHECK(shart)                                                         \
    do {                                                                     \
        t_checks++;                                                          \
        if (!(shart)) {                                                      \
            t_fail++;                                                        \
            printf("  [XATO] %s:%d: %s\n", __FILE__, __LINE__, #shart);      \
        }                                                                    \
    } while (0)

/* Butun sonlarni solishtirish: xato bo'lsa, kutilgan va olingan qiymatni ko'rsatadi. */
#define CHECK_INT(olingan, kutilgan)                                         \
    do {                                                                     \
        long long o_ = (long long)(olingan), k_ = (long long)(kutilgan);     \
        t_checks++;                                                          \
        if (o_ != k_) {                                                      \
            t_fail++;                                                        \
            printf("  [XATO] %s:%d: %s\n         kutilgan: %lld, olingan: %lld\n", \
                   __FILE__, __LINE__, #olingan, k_, o_);                    \
        }                                                                    \
    } while (0)

/* Ishorasiz 64 bitli sonlar (bitlar, manzillar) - o'n oltilik ko'rinishda. */
#define CHECK_HEX(olingan, kutilgan)                                         \
    do {                                                                     \
        unsigned long long o_ = (unsigned long long)(olingan);               \
        unsigned long long k_ = (unsigned long long)(kutilgan);              \
        t_checks++;                                                          \
        if (o_ != k_) {                                                      \
            t_fail++;                                                        \
            printf("  [XATO] %s:%d: %s\n         kutilgan: 0x%llx, olingan: 0x%llx\n", \
                   __FILE__, __LINE__, #olingan, k_, o_);                    \
        }                                                                    \
    } while (0)

/* Satrlarni solishtirish (NULL ham xavfsiz). */
#define CHECK_STR(olingan, kutilgan)                                         \
    do {                                                                     \
        const char *o_ = (olingan), *k_ = (kutilgan);                        \
        t_checks++;                                                          \
        if (!o_ || strcmp(o_, k_) != 0) {                                    \
            t_fail++;                                                        \
            printf("  [XATO] %s:%d: %s\n         kutilgan: \"%s\", olingan: %s%s%s\n", \
                   __FILE__, __LINE__, #olingan, k_, o_ ? "\"" : "",         \
                   o_ ? o_ : "NULL", o_ ? "\"" : "");                        \
        }                                                                    \
    } while (0)

#define TEST_TUGADI()                                                        \
    do {                                                                     \
        if (t_fail == 0)                                                     \
            printf("NATIJA: %d/%d tekshiruv o'tdi\n", t_checks, t_checks);   \
        else                                                                 \
            printf("NATIJA: %d/%d tekshiruvda XATO\n", t_fail, t_checks);    \
        return t_fail != 0;                                                  \
    } while (0)
