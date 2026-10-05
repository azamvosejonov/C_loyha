/* hisobot.h - jadval va umumiy yig'indi */
#ifndef HISOBOT_H
#define HISOBOT_H

#include <stdio.h>

#include "ombor.h"

/* tartib: 0 - yuklangan tartibda, 1 - ism bo'yicha (A-Z), 2 - qo'lga tegadigan maosh bo'yicha (kamayish). Ombor tartibini O'ZGARTIRADI */
enum tartib { TARTIB_YUKLANGAN, TARTIB_ISM, TARTIB_MAOSH };

void hisobot_royxat(struct ombor *o, enum tartib t, FILE *chiqish);        /* jadval chiqishga yoziladi (stdout yoki fayl) */
void hisobot_jami(const struct ombor *o, FILE *chiqish);

/* hisobotni faylga saqlaydi. 0 - OK, -1 - xato (sabab stderr ga). Yozish xatosi fclose da ham chiqishi mumkin: tekshiriladi */
int hisobot_saqla(struct ombor *o, const char *fayl);

#endif
