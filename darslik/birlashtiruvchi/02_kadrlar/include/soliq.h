/* soliq.h - soliq va ustamalar. Chegaralar va foizlar config.h da */
#ifndef SOLIQ_H
#define SOLIQ_H

#include <stdint.h>

/* [TODO T8 ★★ 4-bob, switch] xodim toifasiga qarab bonus foizi (asosiy ish haqiga):
   toifa 1 -> 0, toifa 2 -> 5, toifa 3 -> 10, toifa 4 -> 20; boshqa har qanday qiymat -> 0.
   Maslahat: switch yoki if/else if. */
int toifa_bonusi(int toifa);

/* [TODO T9 ★★ 4-bob] PROGRESSIV soliq (tiyinda). Brutto bo'laklarga bo'linadi va har bo'lak o'z foizi bilan soliqqa tortiladi:
     - birinchi SOLIQ_CHEGARA1 gacha bo'lgan qism   -> SOLIQ_FOIZ1 foiz
     - CHEGARA1 dan CHEGARA2 gacha bo'lgan qism     -> SOLIQ_FOIZ2 foiz
     - CHEGARA2 dan oshgan qism                      -> SOLIQ_FOIZ3 foiz
   Har bo'lakning soliqi foiz() bilan alohida hisoblanadi (yaxlitlash bo'lak bo'yicha), keyin qo'shiladi. Manfiy yoki nol brutto -> 0.
   Misol (so'mda aytsak): brutto 2 000 000 -> 240 000; brutto 4 000 000 -> 360 000 + 150 000 = 510 000;
   brutto 10 000 000 -> 360 000 + 750 000 + 400 000 = 1 510 000. Eslatma: funksiya TIYIN bilan ishlaydi (foiz() ni ishlating). */
int64_t soliq_hisobla(int64_t brutto);

#endif
