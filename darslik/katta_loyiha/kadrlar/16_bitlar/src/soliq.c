#include "config.h"
#include "pul.h"
#include "soliq.h"

static int64_t eng_kichik(int64_t a, int64_t b)
{
    return a < b ? a : b;
}

int64_t soliq_hisobla(int64_t brutto)
{
    if (brutto <= 0)
        return 0;
    int64_t q1 = eng_kichik(brutto, SOLIQ_CHEGARA1);
    int64_t q2 = brutto > SOLIQ_CHEGARA1 ? eng_kichik(brutto, SOLIQ_CHEGARA2) - SOLIQ_CHEGARA1 : 0;
    int64_t q3 = brutto > SOLIQ_CHEGARA2 ? brutto - SOLIQ_CHEGARA2 : 0;
    return foiz(q1, SOLIQ_FOIZ1) + foiz(q2, SOLIQ_FOIZ2) + foiz(q3, SOLIQ_FOIZ3);
}
