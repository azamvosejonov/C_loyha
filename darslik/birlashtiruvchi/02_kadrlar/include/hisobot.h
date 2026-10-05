/* hisobot.h - umumiy hisobotlar */
#ifndef HISOBOT_H
#define HISOBOT_H

#include "ombor.h"

/* barcha xodimlar jadvali (TAYYOR) */
void hisobot_royxat(const struct ombor *o);

/* [TODO T14 ★★★ 9-bob] hamma xodimning natijalarini (xodim_hisobla bilan) yig'ib, *j ga yozadi: j->asosiy, ustama, bonus, brutto,
   soliq, kasaba, sof - har biri barcha xodimlar yig'indisi. Ombor bo'sh bo'lsa hammasi 0. Avval *j ni o'zingiz nollang. */
void hisobot_jami(const struct ombor *o, struct natija *j);

/* jami hisobotini chiqaradi (TAYYOR; hisobot_jami dan foydalanadi) */
void hisobot_jami_chiqar(const struct ombor *o);

#endif
