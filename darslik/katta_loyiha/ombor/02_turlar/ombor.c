/* ombor.c - Ombor, 2-bosqich: pul tiyinda (long), soni aniq 16 bit (uint16_t), toshishdan himoya */
#include <stdint.h>
#include <stdio.h>

#include "ombor_chop.h"

/* sonni xavfsiz oshirish: uint16_t ga sig'maydigan bo'lsa, eng kattasida to'xtaymiz */
static uint16_t qosh_soni(uint16_t hozirgi, uint16_t qoshiladi)
{
    uint32_t yigindi = (uint32_t)hozirgi + qoshiladi;   /* kattaroq qutida hisoblaymiz */
    if (yigindi > UINT16_MAX) {
        printf("  OGOHLANTIRISH: %u + %u = %u, 16 bitga sig'maydi -> %u da to'xtadi\n",
               hozirgi, qoshiladi, yigindi, UINT16_MAX);
        return UINT16_MAX;
    }
    return (uint16_t)yigindi;
}

int main(void)
{
    long non_narx = 400000;                             /* 4000.00 so'm = 400000 tiyin */
    long sut_narx = 1200000;
    long guruch_narx = 1800000;
    uint16_t non_soni = 120, sut_soni = 45, guruch_soni = 8;

    long jami = non_narx * non_soni + sut_narx * sut_soni + guruch_narx * guruch_soni;

    chop_sarlavha();
    chop_qator("Non", non_narx, non_soni);
    chop_qator("Sut", sut_narx, sut_soni);
    chop_qator("Guruch", guruch_narx, guruch_soni);
    chop_jami(jami, 12);

    printf("\nOmborga 500 dona non keldi:\n");
    non_soni = qosh_soni(non_soni, 500);
    printf("  non endi: %u dona\n", non_soni);

    printf("Yana 65000 dona keldi (juda ko'p):\n");
    uint16_t xato = non_soni + 65000;                   /* HIMOYASIZ: aylanib ketadi */
    printf("  himoyasiz qo'shsak: %u (noto'g'ri! aylanib ketdi)\n", xato);
    non_soni = qosh_soni(non_soni, 65000);
    printf("  himoyali qo'shsak: %u dona\n", non_soni);
    return 0;
}
