#include "pul.h"
#include "test.h"

int main(void)
{
    BOSHLA("T1 foiz");
    TEKSHIR(foiz(1000, 12), 120);
    TEKSHIR(foiz(1050, 1), 11);                 /* 10.5 -> 11 */
    TEKSHIR(foiz(1049, 1), 10);                 /* 10.49 -> 10 */
    TEKSHIR(foiz(0, 15), 0);
    TEKSHIR(foiz(445008900LL, 12), 53401068LL);
    TEKSHIR(foiz(10000000000LL, 20), 2000000000LL);     /* katta summa: int toshmasin */
    TUGAT();
    YAKUN();
}
