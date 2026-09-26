/* =============================================================================
 *  24 - Stek bilan kalkulyator (RPN)                   [4-modul: yadro uslubidagi C]
 * =============================================================================
 *
 *  VAZIFA:
 *    rpn("3 4 + 2 *", &r) -> r = 14, return 0
 *    Teskari Polsha yozuvi (Reverse Polish Notation): amal o'z sonlaridan KEYIN
 *    keladi. Qavs kerak emas: "5 1 2 + 4 * + 3 -" = 5 + (1 + 2) * 4 - 3 = 14.
 *    * Tokenlar bitta yoki bir nechta bo'shliq bilan ajratilgan.
 *    * Son: o'nlik, ixtiyoriy '-' bilan ("-3"). Yolg'iz "-" - bu ayirish amali.
 *    * Amallar: + - * /  (butun sonli bo'lish)
 *    * Xato -> -1 (va *natija ga tegmang): noma'lum token, stekda son yetmasa,
 *      oxirida stekda aniq 1 ta son qolmasa, nolga bo'lish, bo'sh ifoda.
 *    Stek chuqurligi 64 dan oshmaydi deb hisoblang (oshsa - xato).
 *
 *  PYTHON'DA:
 *    stek = []; for t in s.split(): ... stek.append(...) / stek.pop()
 *
 *  NEGA:
 *    Satrni tokenlarga ajratish (parsing) + stek - shell (MyOS: user/bin/sh.c),
 *    kompilyatorlar, konfiguratsiya fayllari, hatto ACPI AML'ni o'qish shunga
 *    tayanadi. Bu mashq - 12-mashq (satr -> son) va 7-mashq (xato kodlari)ni
 *    birlashtiradi.
 *
 *  MASLAHAT:
 *    * `long stek[64]; int top = 0;`  - push: stek[top++] = x;  pop: stek[--top].
 *    * Tokenni ajratish: ko'rsatkich bilan yurish - bo'shliqlarni o'tkazish,
 *      token boshi, token oxiri. strtok ishlatmang (u satrni o'zgartiradi, bizniki const).
 *    * Son o'qish uchun strtol(p, &end, 10) ishlatish mumkin - end tokenning
 *      oxiriga to'g'ri kelishini tekshiring ("12a" - xato).
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 24
 * ============================================================================= */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "mashq.h"

int rpn(const char *ifoda, long *natija)
{
    /* TODO */
    (void)ifoda; (void)natija;
    return -1;
}
