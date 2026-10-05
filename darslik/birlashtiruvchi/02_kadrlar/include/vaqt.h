/* vaqt.h - vaqt ko'rinishlari. Vaqt "HHMM" butun son sifatida beriladi: 9:30 -> 930, 18:05 -> 1805 */
#ifndef VAQT_H
#define VAQT_H

/* [TODO T2 ★ 2-bob] HHMM -> kun boshidan o'tgan daqiqalar. Misol: daqiqaga(930) = 570; daqiqaga(1805) = 1085; daqiqaga(0) = 0.
   Maslahat: soat = hhmm / 100, daqiqa = hhmm % 100. */
int daqiqaga(int hhmm);

/* [TODO T3 ★★ 4-bob] sof ish daqiqalari: (chiqish - kirish) - tanaffus.
   Qoidalar: HHMM noto'g'ri bo'lsa (soat > 23 yoki daqiqa > 59) yoki chiqish <= kirish bo'lsa -> -1.
   Natija manfiy chiqib qolsa (tanaffus ish vaqtidan uzun) -> 0.
   Misol: (900, 1800, 60) = 480; (830, 1900, 60) = 570; (1800, 900, 60) = -1; (900, 930, 60) = 0; (975, 1800, 0) = -1.
   Maslahat: daqiqaga() dan foydalaning. */
int ish_daqiqalari(int kirish, int chiqish, int tanaffus);

/* [TODO T4 ★ 2-bob] daqiqalarni "soatning yuzdan bir ulushi"ga aylantiradi, eng yaqin butunga yaxlitlab.
   Misol: soat_yuzdan(60) = 100; soat_yuzdan(570) = 950; soat_yuzdan(45) = 75; soat_yuzdan(1) = 2 (1.67 -> 2); soat_yuzdan(0) = 0. */
int soat_yuzdan(int daq);

#endif
