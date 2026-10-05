/* ombor.c - Ombor, 1-bosqich: asosiy dastur chiqarishni boshqa fayldan oladi */
#include "ombor_chop.h"

int main(void)
{
    int non_narx = 4000, non_soni = 120;
    int sut_narx = 12000, sut_soni = 45;
    int guruch_narx = 18000, guruch_soni = 8;

    int jami = non_narx * non_soni + sut_narx * sut_soni + guruch_narx * guruch_soni;

    chop_sarlavha();
    chop_qator("Non", non_narx, non_soni);
    chop_qator("Sut", sut_narx, sut_soni);
    chop_qator("Guruch", guruch_narx, guruch_soni);
    chop_jami(jami, 12);
    return 0;
}
