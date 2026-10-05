/* ombor.h - xodimlar ombori. struct ombor ning ICHI YASHIRIN (opaque tur, 11-bob): foydalanuvchi faqat funksiyalar orqali ishlaydi.
   Shuning uchun ichki tuzilishni (masalan massiv o'rniga ro'yxat) o'zgartirsak, boshqa fayllar buzilmaydi. */
#ifndef OMBOR_H
#define OMBOR_H

#include <stddef.h>

#include "xodim.h"

struct ombor;

struct ombor *ombor_yarat(void);                /* NULL - xotira yo'q */
void ombor_yoq_qil(struct ombor *o);            /* free: har ombor_yarat uchun BIR marta */
int ombor_qosh(struct ombor *o, const struct xodim *x);        /* 0 - OK, -1 - xotira yo'q, -2 - ID takror */
size_t ombor_soni(const struct ombor *o);
const struct xodim *ombor_ol(const struct ombor *o, size_t i);          /* i-xodim (0..soni-1) */
const struct xodim *ombor_top(const struct ombor *o, int id);           /* ID bo'yicha; yo'q bo'lsa NULL */
struct xodim *ombor_top_yoz(struct ombor *o, int id);                   /* xuddi shunday, lekin o'zgartirish mumkin */

/* xodimlarni qsort bilan saralaydi. taqqoslash: const struct xodim * larni solishtiradigan funksiya (manfiy / 0 / musbat) */
void ombor_saralash(struct ombor *o, int (*taqqoslash)(const void *, const void *));

#endif
