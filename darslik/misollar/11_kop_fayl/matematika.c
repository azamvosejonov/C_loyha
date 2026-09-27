/* matematika.c - AMALGA OSHIRISH */
#include "matematika.h"                 /* o'z interfeysi bilan mosligini kompilyator tekshiradi */

int chaqiruvlar_soni = 0;               /* TA'RIF: xotira shu yerda */

static long yordamchi(long x)           /* static - faqat shu faylda ko'rinadi */
{
    chaqiruvlar_soni++;
    return x;
}

long kvadrat(int x)
{
    return yordamchi((long)x * x);
}

long yigindi(const int *a, int n)
{
    long s = 0;
    for (int i = 0; i < n; i++)
        s += a[i];
    return yordamchi(s);
}
