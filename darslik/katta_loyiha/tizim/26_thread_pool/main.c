/* main.c - pulni sinash: 200000 gacha tub sonlarni 20 ta bo'lakka bo'lib, 4 ishchi parallel sanaydi */
#include <stdio.h>

#include "pool.h"

#define BOLAKLAR 20
#define BOLAK_UZ 10000                          /* har topshiriq 10000 ta sonni tekshiradi */

struct bolak {
    int id;
    int bosh, oxir;                             /* [bosh, oxir) */
    int tublar;                                 /* natija: shu oraliqda nechta tub son */
};

static int tub_mi(int n)
{
    if (n < 2)
        return 0;
    for (int d = 2; d * d <= n; d++)
        if (n % d == 0)
            return 0;
    return 1;
}

/* topshiriq: o'z bo'lagiga yozadi (har topshiriq BOSHQA struct ga yozadi: poyga yo'q, qulf kerak emas) */
static void sana(void *arg)
{
    struct bolak *b = arg;
    int s = 0;
    for (int n = b->bosh; n < b->oxir; n++)
        s += tub_mi(n);
    b->tublar = s;
}

int main(void)
{
    static struct bolak bolaklar[BOLAKLAR];
    struct pool *pul = pool_yarat(4, 3);        /* 4 ishchi, navbat sig'imi atigi 3: yuboruvchi ba'zan KUTADI */
    if (!pul)
        return 1;

    for (int i = 0; i < BOLAKLAR; i++) {
        bolaklar[i] = (struct bolak){ i, i * BOLAK_UZ, (i + 1) * BOLAK_UZ, 0 };
        pool_yubor(pul, sana, &bolaklar[i]);
    }
    pool_tugat(pul);                            /* hamma ish tugagach qaytadi: natijalar tayyor */

    int jami = 0;
    for (int i = 0; i < BOLAKLAR; i++) {
        printf("  bo'lak %2d [%6d, %6d): %4d ta tub son\n", i, bolaklar[i].bosh, bolaklar[i].oxir, bolaklar[i].tublar);
        jami += bolaklar[i].tublar;
    }
    printf("jami: %d ta tub son (0..199999). To'g'ri javob: 17984\n", jami);
    return jami == 17984 ? 0 : 1;
}
