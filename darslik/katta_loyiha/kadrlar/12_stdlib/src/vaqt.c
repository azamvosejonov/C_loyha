#include "vaqt.h"

int daqiqaga(int hhmm)
{
    return hhmm / 100 * 60 + hhmm % 100;
}

int ish_daqiqalari(int kirish, int chiqish, int tanaffus)
{
    if (kirish < 0 || chiqish < 0 || kirish / 100 > 23 || chiqish / 100 > 23 || kirish % 100 > 59 || chiqish % 100 > 59)
        return -1;
    if (chiqish <= kirish)
        return -1;
    int sof = daqiqaga(chiqish) - daqiqaga(kirish) - tanaffus;
    return sof < 0 ? 0 : sof;
}
