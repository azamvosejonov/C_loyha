/* pul.h - pul bilan ishlash: hamma summa TIYINDA (1 so'm = 100 tiyin), turi int64_t */
#ifndef PUL_H
#define PUL_H

#include <stdint.h>

/* [TODO T1 ★ 2-bob] summaning p foizi, tiyinga yaxlitlangan (.5 va undan yuqori - yuqoriga).
   Faqat musbat summa uchun. Misol: foiz(1000, 12) = 120; foiz(1050, 1) = 11 (10.5 -> 11); foiz(1049, 1) = 10 */
int64_t foiz(int64_t summa, int p);

/* tarif (tiyin/SOAT) va daqiqalar bo'yicha haq, tiyinga yaxlitlangan: tarif * daqiqa / 60 */
int64_t pul_vaqt_haqi(int64_t tarif, int daqiqa);

/* "yorliq:   1234.56 so'm" ko'rinishida chiqaradi */
void pul_chiqar(const char *yorliq, int64_t tiyin);

#endif
