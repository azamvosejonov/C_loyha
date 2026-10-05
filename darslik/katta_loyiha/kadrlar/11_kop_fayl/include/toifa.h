/* toifa.h - toifalar jadvali (X-makro, 10-bob). BIR joyda yoziladi; enum shu yerda, matn va bonus toifa.c da hosil bo'ladi */
#ifndef TOIFA_H
#define TOIFA_H

#define TOIFALAR(X)                             \
    X(BOSHLOVCHI, "Boshlovchi", 0)              \
    X(MUTAXASSIS, "Mutaxassis", 5)              \
    X(YETAKCHI, "Yetakchi", 10)                 \
    X(RAHBAR, "Rahbar", 20)

enum toifa {
#define X(nom, matn, bonus) T_##nom,
    TOIFALAR(X)
#undef X
    T_SONI
};

const char *toifa_matni(enum toifa t);          /* "Boshlovchi" ... yoki "?" (noto'g'ri toifa) */
int toifa_bonusi(enum toifa t);                 /* asosiy ish haqiga foiz */

#endif
