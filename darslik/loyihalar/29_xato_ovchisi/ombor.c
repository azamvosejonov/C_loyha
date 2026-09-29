/* ombor.c - mahsulotlar ombori. DIQQAT: ichida 5 ta xato bor! */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mahsulot {
    char nom[16];
    int narx;
    int soni;
};

static struct mahsulot *royxat;
static int n;

static void qosh(const char *nom, int narx, int soni)
{
    royxat = realloc(royxat, (n + 1) * sizeof(*royxat));
    strcpy(royxat[n].nom, nom);
    royxat[n].narx = narx;
    royxat[n].soni = soni;
    n++;
}

static long jami_qiymat(void)
{
    long jami = 0;
    for (int i = 0; i <= n; i++)
        jami += (long)royxat[i].narx * royxat[i].soni;
    return jami;
}

static struct mahsulot *eng_qimmat(void)
{
    struct mahsulot *eng = &royxat[0];
    for (int i = 1; i < n; i++)
        if (royxat[i].narx > eng->narx)
            eng = &royxat[i];
    return eng;
}

static int chegirmali(int narx, int foiz)
{
    return narx * (100 - foiz) / 100;
}

static void hisobot(void)
{
    char *satr = malloc(64);
    snprintf(satr, 64, "Mahsulot turlari: %d", n);
    puts(satr);
}

int main(void)
{
    qosh("Non", 4000, 10);
    qosh("Sut", 12000, 5);
    qosh("Guruch", 18000, 3);
    qosh("Juda uzun nomli mahsulot", 50000, 1);
    printf("Jami qiymat: %ld\n", jami_qiymat());

    struct mahsulot *qimmat = eng_qimmat();
    qosh("Choy", 25000, 4);
    printf("Eng qimmat: %s (%d so'm)\n", qimmat->nom, qimmat->narx);

    printf("Chegirmali narx (30000000 so'm, 10%%): %d\n", chegirmali(30000000, 10));
    hisobot();
    free(royxat);
    return 0;
}
