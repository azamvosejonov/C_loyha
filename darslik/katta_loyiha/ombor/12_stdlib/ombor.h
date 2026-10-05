/* ombor.h - ombor kutubxonasining OCHIQ interfeysi (12-bosqich: saralash, faylga saqlash) */
#ifndef OMBOR_H
#define OMBOR_H

#include <stdint.h>

/* toifalar ro'yxati bir joyda: enum ham, nomlar jadvali ham undan hosil bo'ladi */
#define TOIFALAR(X) \
    X(OZIQ_OVQAT, "oziq-ovqat") \
    X(ICHIMLIK, "ichimlik") \
    X(UY_RUZGOR, "uy-ro'zg'or")

#define X_ENUM(id, nom) id,
enum toifa { TOIFALAR(X_ENUM) TOIFA_SONI };
#undef X_ENUM

/* funksiyalar natija kodlari: 0 - OK, manfiy - xato */
enum { OK = 0, XOTIRA_YOQ = -1, NOM_BAND = -2, TOPILMADI = -3, NOTOGRI_MIQDOR = -4, YETARLI_EMAS = -5, NOTOGRI_TOIFA = -6 };

typedef struct ombor Ombor;                     /* faqat nom: ichi ombor.c da */

enum saralash { SARALASH_NOM = 1, SARALASH_NARX = 2 };

Ombor *ombor_yarat(void);
void ombor_yoq(Ombor *o);
int ombor_qosh(Ombor *o, const char *nom, long narx, uint16_t soni, enum toifa toifa);
int ombor_sot(Ombor *o, const char *nom, int miqdor, uint16_t *qoldi);
int ombor_ochir(Ombor *o, const char *nom);
int ombor_soni(const Ombor *o);
void ombor_royxat(const Ombor *o);
void ombor_hisobot(const Ombor *o);
void ombor_saralash(Ombor *o, enum saralash mezon);
int ombor_saqla(const Ombor *o, const char *fayl);      /* 0 yoki -1 (errno qo'yiladi) */
int ombor_yukla(Ombor *o, const char *fayl);            /* nechta mahsulot yuklandi yoki -1 (errno) */
const char *toifa_nomi(enum toifa t);
const char *ombor_xato(int kod);

#endif
