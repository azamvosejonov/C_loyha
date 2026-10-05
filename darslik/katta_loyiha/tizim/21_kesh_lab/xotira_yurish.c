/* xotira_yurish.c - bitta massivni turlicha aylanish: kesh xatolari soni valgrind (cachegrind) bilan o'lchanadi */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 512                                   /* N x N int = 1 MB: 32 KB li D1 keshga sig'maydi */
#define M 128                                   /* matritsa ko'paytirish uchun kichikroq (3 ta M x M massiv) */

static int a[N][N];
static int p[M][M], q[M][M], r[M][M];

static long satr_boyicha(void)                  /* a[i][0], a[i][1], ...: xotirada KETMA-KET */
{
    long s = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            s += a[i][j];
    return s;
}

static long ustun_boyicha(void)                 /* a[0][j], a[1][j], ...: har qadam N*4 = 2048 bayt SAKRAYDI */
{
    long s = 0;
    for (int j = 0; j < N; j++)
        for (int i = 0; i < N; i++)
            s += a[i][j];
    return s;
}

static long kopaytir_ijk(void)                  /* klassik tartib: q[k][j] ustun bo'ylab o'qiladi */
{
    memset(r, 0, sizeof(r));
    for (int i = 0; i < M; i++)
        for (int j = 0; j < M; j++)
            for (int k = 0; k < M; k++)
                r[i][j] += p[i][k] * q[k][j];
    return r[M - 1][M - 1];
}

static long kopaytir_ikj(void)                  /* j ichkarida: q[k][*] va r[i][*] qator bo'ylab o'qiladi */
{
    memset(r, 0, sizeof(r));
    for (int i = 0; i < M; i++)
        for (int k = 0; k < M; k++) {
            int pik = p[i][k];
            for (int j = 0; j < M; j++)
                r[i][j] += pik * q[k][j];
        }
    return r[M - 1][M - 1];
}

int main(int argc, char **argv)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            a[i][j] = (i + j) % 7;
    for (int i = 0; i < M; i++)
        for (int j = 0; j < M; j++) {
            p[i][j] = (i * 3 + j) % 5;
            q[i][j] = (i + j * 2) % 5;
        }

    const char *rejim = argc > 1 ? argv[1] : "satr";
    long natija;
    if (strcmp(rejim, "satr") == 0)
        natija = satr_boyicha();
    else if (strcmp(rejim, "ustun") == 0)
        natija = ustun_boyicha();
    else if (strcmp(rejim, "ijk") == 0)
        natija = kopaytir_ijk();
    else if (strcmp(rejim, "ikj") == 0)
        natija = kopaytir_ikj();
    else
        return 2;
    printf("%s: natija %ld\n", rejim, natija);
    return 0;
}
