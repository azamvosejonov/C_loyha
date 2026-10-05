/* sinov_dastur.c - ataylab sizib chiqish bor dastur. trace.c uni KOD O'ZGARTIRMASDAN kuzatadi.
   Funksiyalar static EMAS: dladdr faqat tashqariga ochiq (global) belgilarning nomini topa oladi (-rdynamic bilan) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *yaratish_a(void)
{
    char *p = malloc(100);                      /* free YO'Q: sizib chiqadi */
    strcpy(p, "birinchi");
    return p;
}

void yaratish_b(int n)
{
    for (int i = 0; i < n; i++) {
        char *p = malloc(24);                   /* har aylanishda 24 bayt, hech biri free qilinmaydi */
        snprintf(p, 24, "blok %d", i);
    }
}

void toza_ish(void)
{
    char *p = malloc(4000);
    memset(p, 1, 4000);
    p = realloc(p, 8000);                       /* realloc: eski bloklar hisobdan chiqadi, yangisi yoziladi */
    free(p);
}

int main(void)
{
    char *a = yaratish_a();
    printf("a = \"%s\"\n", a);
    yaratish_b(3);
    toza_ish();
    char *q = calloc(10, 16);
    free(q);                                    /* bu to'g'ri qaytarildi */
    puts("dastur tugadi");
    fflush(stdout);                             /* stdout buferi hisobotdan OLDIN chiqsin */
    return 0;
}
