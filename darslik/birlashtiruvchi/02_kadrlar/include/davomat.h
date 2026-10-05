/* davomat.h - davomat faylini o'qib, xodimlarning ish daqiqalarini yig'adi (TAYYOR kod: o'qib, namuna oling) */
#ifndef DAVOMAT_H
#define DAVOMAT_H

#include "ombor.h"

/* Fayl qatori: id kun kirish chiqish  (vaqtlar HHMM). Har yozuv uchun ish daqiqalari hisoblanib, xodimga qo'shiladi:
   NORMA_KUN_DAQ gacha - oddiy, undan ortig'i - qo'shimcha. Yaroqsiz yozuv (noma'lum ID, noto'g'ri vaqt) *rad ni oshiradi.
   Qaytaradi: qabul qilingan yozuvlar soni; fayl ochilmasa -1. */
int davomat_yukla(struct ombor *o, const char *fayl, int *rad);

#endif
