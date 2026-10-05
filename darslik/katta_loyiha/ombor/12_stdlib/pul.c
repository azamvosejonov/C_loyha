/* pul.c - pul formatlash (butun sonlarda: kasr xatosi yo'q) */
#include <stdio.h>

#include "pul.h"

void pul_matn(char *bufer, size_t hajm, long tiyin)
{
    snprintf(bufer, hajm, "%ld.%02ld", tiyin / 100, tiyin % 100);
}

void pul_chiqar(long tiyin)
{
    char b[32];
    pul_matn(b, sizeof(b), tiyin);
    printf("%s", b);
}
