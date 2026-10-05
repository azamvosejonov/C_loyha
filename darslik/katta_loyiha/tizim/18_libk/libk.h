/* libk.h - yadro uchun mini kutubxona: libc'siz (freestanding) satr/xotira funksiyalari, bitmap, halqa bufer */
#ifndef LIBK_H
#define LIBK_H

#include <stddef.h>
#include <stdint.h>

/* --- xotira va satrlar (libc nomlari bilan to'qnashmasligi uchun "k" prefiksi) --- */
void *kmemset(void *dst, int bayt, size_t n);
void *kmemcpy(void *dst, const void *src, size_t n);       /* ustma-ust tushmasligi shart */
void *kmemmove(void *dst, const void *src, size_t n);      /* ustma-ust tushsa ham to'g'ri */
int kmemcmp(const void *a, const void *b, size_t n);
size_t kstrlen(const char *s);
int kstrcmp(const char *a, const char *b);
size_t kstrlcpy(char *dst, const char *src, size_t hajm); /* dst ga ko'pi bilan hajm-1 belgi + '\0'; src uzunligini qaytaradi */

/* --- sonlarni matnga aylantirish: asos 2..16, bufer yetarlicha katta bo'lishi shart (kamida 65 bayt) --- */
char *kutoa(uint64_t son, char *bufer, int asos);

/* --- bitmap: 1 bit = 1 obyekt (16-bob). bitlar massivi uint8_t bayt qatorida --- */
void kbit_yoq(uint8_t *b, size_t i);
void kbit_och(uint8_t *b, size_t i);
int kbit_bor(const uint8_t *b, size_t i);
long kbit_bosh_top(const uint8_t *b, size_t n);            /* birinchi 0 bitning indeksi yoki -1 */

/* --- halqa bufer: bitta yozuvchi va bitta o'quvchi (klaviatura buferi kabi). sig'im 2 ning darajasi bo'lishi shart --- */
struct kring {
    uint8_t *ma_lumot;
    size_t sigim;                               /* 2 ning darajasi */
    size_t bosh, oxir;                          /* o'qish va yozish hisoblagichlari (cheksiz oshadi, mask bilan indeks olinadi) */
};

void kring_boshla(struct kring *r, uint8_t *xotira, size_t sigim);
int kring_yoz(struct kring *r, uint8_t bayt);              /* 0 - OK, -1 - to'la */
int kring_oqi(struct kring *r, uint8_t *bayt);             /* 0 - OK, -1 - bo'sh */
size_t kring_band(const struct kring *r);

#endif
