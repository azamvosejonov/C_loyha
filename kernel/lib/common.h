/* =============================================================================
 *  lib/common.h - butun yadroda ishlatiladigan kichik makroslar
 * ============================================================================= */
#pragma once

#include <stddef.h>
#include <stdint.h>

/* Massiv elementlari soni. Faqat haqiqiy massiv uchun (ko'rsatkich uchun EMAS). */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* x ni "align" ga karrali qilib pastga/yuqoriga yaxlitlash. align - 2 ning darajasi
 * bo'lishi SHART. Hiyla: 2^n - 1 niqobidagi bitlarni tozalasak, 2^n ga karrali
 * son hosil bo'ladi. Masalan, ALIGN_DOWN(0x1234, 0x1000) = 0x1000. */
#define ALIGN_DOWN(x, align) ((x) & ~((__typeof__(x))(align) - 1))
#define ALIGN_UP(x, align)   ALIGN_DOWN((x) + (align) - 1, align)
#define IS_ALIGNED(x, align) (((x) & ((__typeof__(x))(align) - 1)) == 0)

#define KiB (1024ULL)
#define MiB (1024ULL * KiB)
#define GiB (1024ULL * MiB)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* Kompilyatorga ko'rsatma: bu shart odatda to'g'ri/noto'g'ri bo'ladi.
 * U "tez yo'l"ni kod ichida birinchi qo'yadi (branch prediction'ga yordam). */
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/* Struktura a'zosining manzilidan butun strukturaning manzilini topish.
 * Linux yadrosining mashhur makrosi. Masalan, list_node* dan process* ni olish. */
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
