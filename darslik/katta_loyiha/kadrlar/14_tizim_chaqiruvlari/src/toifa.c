#include "toifa.h"

static const char *const matnlar[] = {
#define X(nom, matn, bonus) [T_##nom] = matn,
    TOIFALAR(X)
#undef X
};

static const int bonuslar[] = {
#define X(nom, matn, bonus) [T_##nom] = bonus,
    TOIFALAR(X)
#undef X
};

const char *toifa_matni(enum toifa t)
{
    return (t >= 0 && t < T_SONI) ? matnlar[t] : "?";
}

int toifa_bonusi(enum toifa t)
{
    return (t >= 0 && t < T_SONI) ? bonuslar[t] : 0;
}
