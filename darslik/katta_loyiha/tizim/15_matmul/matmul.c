/* matmul.c - parallel matritsa ko'paytirish: qatorlarni oqimlar orasida bo'lish, natijani ketma-ket variant bilan solishtirish */
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 240                                   /* N x N matritsalar */

static int a[N][N], b[N][N];
static long c_ketma[N][N], c_par[N][N];

struct ish {
    int bosh, oxir;                             /* shu oqim hisoblaydigan qatorlar: [bosh, oxir) */
};

/* qatorlar oralig'ini hisoblaydi: C[i][*] = A[i][*] x B. Har oqim BOSHQA qatorlarga yozadi - umumiy xotirada to'qnashuv yo'q */
static void qatorlar_hisobla(long c[N][N], int bosh, int oxir)
{
    for (int i = bosh; i < oxir; i++)
        for (int k = 0; k < N; k++) {           /* i-k-j tartibi: b[k][j] ni qator bo'ylab o'qiymiz (keshga mos, 21-bob) */
            long aik = a[i][k];
            for (int j = 0; j < N; j++)
                c[i][j] += aik * b[k][j];
        }
}

static void *oqim_ishi(void *arg)
{
    const struct ish *ish = arg;
    qatorlar_hisobla(c_par, ish->bosh, ish->oxir);
    return NULL;
}

/* T oqim bilan hisoblaydi: N qatorni T bo'lakka bo'ladi */
static void parallel(int T)
{
    memset(c_par, 0, sizeof(c_par));
    pthread_t oqim[16];
    struct ish ish[16];
    int bolak = (N + T - 1) / T;                /* yuqoriga yaxlitlash: oxirgi oqimga kamroq tushishi mumkin */
    int ishga_tushdi = 0;
    for (int t = 0; t < T; t++) {
        ish[t].bosh = t * bolak;
        ish[t].oxir = (t + 1) * bolak < N ? (t + 1) * bolak : N;
        if (ish[t].bosh >= ish[t].oxir)
            break;
        pthread_create(&oqim[t], NULL, oqim_ishi, &ish[t]);
        ishga_tushdi++;
    }
    for (int t = 0; t < ishga_tushdi; t++)
        pthread_join(oqim[t], NULL);            /* hamma oqim tugashini kutamiz */
}

static uint32_t nazorat_yigindisi(long c[N][N])
{
    uint32_t s = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            s = s * 31u + (uint32_t)c[i][j];    /* tartibga bog'liq: bitta qiymat xato bo'lsa ham o'zgaradi */
    return s;
}

int main(void)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            a[i][j] = (i * 7 + j * 3) % 10;     /* deterministik "tasodifiy" ma'lumot */
            b[i][j] = (i * 5 + j * 11) % 10;
        }

    qatorlar_hisobla(c_ketma, 0, N);
    uint32_t etalon = nazorat_yigindisi(c_ketma);
    printf("ketma-ket: nazorat yig'indisi = %u, c[0][0] = %ld, c[%d][%d] = %ld\n", etalon, c_ketma[0][0], N - 1, N - 1,
           c_ketma[N - 1][N - 1]);

    int hamma_mos = 1;
    for (int T = 1; T <= 8; T *= 2) {
        parallel(T);
        uint32_t y = nazorat_yigindisi(c_par);
        int mos = memcmp(c_ketma, c_par, sizeof(c_ketma)) == 0;
        printf("%d oqim:     nazorat yig'indisi = %u (%s)\n", T, y, mos ? "ketma-ket bilan MOS" : "FARQ QILADI!");
        hamma_mos &= mos;
    }
    return hamma_mos ? 0 : 1;
}
