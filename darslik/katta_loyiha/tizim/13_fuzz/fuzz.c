/* fuzz.c - fuzzer: tasodifiy buzilgan kirishlarni parserga yuboradi; xato bo'lsa sanitizer to'xtatadi */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static unsigned long holat;                     /* tasodif generatori holati (bir xil urug' = bir xil ketma-ketlik) */

static unsigned tasodif(void)
{
    holat = holat * 6364136223846793005UL + 1442695040888963407UL;
    return (unsigned)(holat >> 33);
}

/* haqiqiy kirishni olib, tasodifiy "buzamiz": uzaytirish, belgi almashtirish, ikki nuqta qo'shish */
static void buz(char *bufer, size_t sigim)
{
    static const char *urug[] = { "non:4000:120", "sut:12000:45", "guruch:18000:8", "shakar:15000:60" };
    snprintf(bufer, sigim, "%s", urug[tasodif() % 4]);
    size_t uz = strlen(bufer);

    switch (tasodif() % 5) {
    case 0:                                     /* nomni uzun qilamiz */
        for (unsigned k = tasodif() % 100; k && uz + 1 < sigim; k--)
            memmove(bufer + 1, bufer, ++uz), bufer[0] = 'a' + (char)(tasodif() % 26);
        break;
    case 1:                                     /* ko'p raqam qo'shamiz */
        for (unsigned k = tasodif() % 30; k && uz + 1 < sigim; k--)
            bufer[uz++] = '0' + (char)(tasodif() % 10);
        bufer[uz] = '\0';
        break;
    case 2:                                     /* tasodifiy belgi almashtiramiz */
        if (uz)
            bufer[tasodif() % uz] = (char)(33 + tasodif() % 90);
        break;
    case 3:                                     /* ':' belgilarini ko'paytiramiz yoki yo'qotamiz */
        for (size_t i = 0; i < uz; i++)
            if (bufer[i] == ':' && tasodif() % 2)
                bufer[i] = 'x';
        break;
    default:                                    /* hech narsa: to'g'ri kirish */
        break;
    }
}

int main(int argc, char **argv)
{
    unsigned long urug = argc > 1 ? strtoul(argv[1], NULL, 10) : 1;
    long n = argc > 2 ? atol(argv[2]) : 1000;
    holat = urug;

    /* 1) aniq sinov: 12 ta juda katta yozuvning yig'indisi long ga ham sig'maydi */
    struct yozuv katta[12];
    for (int i = 0; i < 12; i++) {
        strcpy(katta[i].nom, "x");
        katta[i].narx = 999999999;
        katta[i].soni = 999999999;
    }
    long yig;
    int r = yozuvlar_jami(katta, 12, &yig);
    printf("toshish sinovi: yozuvlar_jami() = %d (%s)\n", r, r ? "toshish aniqlandi" : "toshish ANIQLANMADI!");

    /* 2) fuzz: n ta tasodifiy buzilgan kirish */
    long togri = 0, rad = 0, toshdi = 0;
    for (long i = 0; i < n; i++) {
        char kirish[256];
        buz(kirish, sizeof(kirish));
        FILE *f = fopen("oxirgi_kirish.txt", "w");  /* qulash bo'lsa, qaysi kirish sababchi ekanini bilamiz */
        if (f) {
            fputs(kirish, f);
            fclose(f);
        }

        struct yozuv y[2];
        long jami;
        if (yozuv_oqi(kirish, &y[0]) == 0) {
            togri++;
            y[1] = y[0];                        /* ikkita yozuv: yig'indini tekshirish uchun */
            if (yozuvlar_jami(y, 2, &jami) != 0)
                toshdi++;
        } else {
            rad++;
        }
    }
    printf("urug %lu, %ld ta kirish: %ld ta to'g'ri o'qildi, %ld ta rad etildi, %ld ta yig'indi toshdi (aniqlandi)\n",
           urug, n, togri, rad, toshdi);
    return 0;
}
