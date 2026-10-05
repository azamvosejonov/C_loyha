#include "test.h"
#include "vaqt.h"

int main(void)
{
    BOSHLA("T2 daqiqaga");
    TEKSHIR(daqiqaga(930), 570);
    TEKSHIR(daqiqaga(1805), 1085);
    TEKSHIR(daqiqaga(0), 0);
    TEKSHIR(daqiqaga(2359), 1439);
    TUGAT();

    BOSHLA("T3 ish_daqiqalari");
    TEKSHIR(ish_daqiqalari(900, 1800, 60), 480);
    TEKSHIR(ish_daqiqalari(830, 1900, 60), 570);
    TEKSHIR(ish_daqiqalari(1800, 900, 60), -1);         /* chiqish kirishdan oldin */
    TEKSHIR(ish_daqiqalari(900, 900, 0), -1);           /* bir xil vaqt */
    TEKSHIR(ish_daqiqalari(900, 930, 60), 0);           /* tanaffus ishdan uzun */
    TEKSHIR(ish_daqiqalari(975, 1800, 0), -1);          /* 75 daqiqa bo'lmaydi */
    TEKSHIR(ish_daqiqalari(900, 2460, 0), -1);          /* 24 soat bo'lmaydi */
    TEKSHIR(ish_daqiqalari(0, 2359, 0), 1439);
    TUGAT();

    BOSHLA("T4 soat_yuzdan");
    TEKSHIR(soat_yuzdan(60), 100);
    TEKSHIR(soat_yuzdan(570), 950);
    TEKSHIR(soat_yuzdan(45), 75);
    TEKSHIR(soat_yuzdan(1), 2);
    TEKSHIR(soat_yuzdan(0), 0);
    TUGAT();
    YAKUN();
}
