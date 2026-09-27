/* =============================================================================
 *  11_kop_fayl/ - ko'p faylli dastur va Makefile            (darslik 1, 11-boblar)
 * =============================================================================
 *  Ishga tushirish:
 *      make            (yoki qo'lda: gcc -Wall -Wextra -c matematika.c && gcc -Wall -Wextra -c main.c
 *                       && gcc main.o matematika.o -o dastur)
 *      ./dastur
 *
 *  Kutilgan natija:
 *      kvadrat(12) = 144
 *      yigindi = 15
 *      chaqiruvlar soni: 2
 *
 *  Sinab ko'ring:
 *      1) nm matematika.o      - T kvadrat, t yordamchi (kichik t - static!),
 *                                B chaqiruvlar_soni (0 bilan boshlangan global - .bss da)
 *      2) nm main.o            - U kvadrat (aniqlanmagan: linker topadi)
 *      3) matematika.c ga tegmasdan main.c ni o'zgartirib, make ni qayta bajaring -
 *         faqat main.c qayta kompilyatsiya qilinadi.
 *      4) Makefile'dan matematika.o ni olib tashlang - "undefined reference" xatosi.
 *      5) main.c da yordamchi() ni chaqirib ko'ring - static bo'lgani uchun topilmaydi.
 * ============================================================================= */
#include <stdio.h>

#include "matematika.h"

int main(void)
{
    int a[] = { 1, 2, 3, 4, 5 };
    printf("kvadrat(12) = %ld\n", kvadrat(12));
    printf("yigindi = %ld\n", yigindi(a, 5));
    printf("chaqiruvlar soni: %d\n", chaqiruvlar_soni);
    return 0;
}
