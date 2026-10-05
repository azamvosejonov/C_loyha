/* vaqt.h - vaqt "HHMM" butun son sifatida: 9:30 -> 930 */
#ifndef VAQT_H
#define VAQT_H

int daqiqaga(int hhmm);                         /* 930 -> 570 */
int ish_daqiqalari(int kirish, int chiqish, int tanaffus);      /* sof ish daqiqalari; noto'g'ri vaqt yoki chiqish <= kirish -> -1 */

#endif
