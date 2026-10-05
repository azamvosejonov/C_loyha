#include "soliq.h"
#include "test.h"

int main(void)
{
    BOSHLA("T8 toifa_bonusi");
    TEKSHIR(toifa_bonusi(1), 0);
    TEKSHIR(toifa_bonusi(2), 5);
    TEKSHIR(toifa_bonusi(3), 10);
    TEKSHIR(toifa_bonusi(4), 20);
    TEKSHIR(toifa_bonusi(0), 0);
    TEKSHIR(toifa_bonusi(99), 0);
    TUGAT();

    BOSHLA("T9 soliq_hisobla");
    TEKSHIR(soliq_hisobla(200000000LL), 24000000LL);                    /* 2 000 000 so'm -> 240 000 */
    TEKSHIR(soliq_hisobla(400000000LL), 51000000LL);                    /* 4 000 000 -> 360 000 + 150 000 */
    TEKSHIR(soliq_hisobla(1000000000LL), 151000000LL);                  /* 10 000 000 -> 1 510 000 */
    TEKSHIR(soliq_hisobla(300000000LL), 36000000LL);                    /* aynan 1-chegarada */
    TEKSHIR(soliq_hisobla(800000000LL), 36000000LL + 75000000LL);       /* aynan 2-chegarada */
    TEKSHIR(soliq_hisobla(0), 0);
    TEKSHIR(soliq_hisobla(-100), 0);
    TEKSHIR(soliq_hisobla(445008900LL), 36000000LL + 21751335LL);       /* 1 450 089.00 * 15% = 217 513.35 */
    TUGAT();
    YAKUN();
}
