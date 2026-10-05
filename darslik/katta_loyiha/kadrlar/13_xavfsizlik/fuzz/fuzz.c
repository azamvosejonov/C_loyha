/* fuzz.c - tasodifiy buzilgan qatorlarni parserga yuboradi. Parser qabul qilsa - natija INVARIANTLARGA mos bo'lishi shart;
   xotira xatosi bo'lsa sanitizer (ASan/UBSan) to'xtatadi. Bir xil urug' = bir xil ketma-ketlik (xatoni qayta ishlab chiqarish mumkin) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "yukla.h"

#ifdef ZAIF
int xodim_tahlil_zaif(const char *qator, struct xodim *x, const char **sabab);
#define XODIM_TAHLIL xodim_tahlil_zaif
#else
#define XODIM_TAHLIL xodim_tahlil
#endif

static unsigned long holat;

static unsigned tasodif(void)
{
    holat = holat * 6364136223846793005UL + 1442695040888963407UL;
    return (unsigned)(holat >> 33);
}

/* haqiqiy qatordan boshlab, uni tasodifiy BUZADI */
static void buz(char *b, size_t hajm, const char *const urug[], int n)
{
    snprintf(b, hajm, "%s", urug[tasodif() % (unsigned)n]);
    size_t uz = strlen(b);
    switch (tasodif() % 7) {
    case 0:                                     /* boshiga ko'p harf qo'shamiz (uzun ism / uzun qator) */
        for (unsigned k = tasodif() % 300; k && uz + 1 < hajm; k--, uz++)
            memmove(b + 1, b, uz + 1), b[0] = (char)('a' + tasodif() % 26);
        break;
    case 1:                                     /* oxiriga raqamlar qo'shamiz (toshib ketadigan sonlar) */
        for (unsigned k = tasodif() % 40; k && uz + 1 < hajm; k--)
            b[uz++] = (char)('0' + tasodif() % 10);
        b[uz] = '\0';
        break;
    case 2:                                     /* tasodifiy bitta belgini almashtiramiz */
        if (uz)
            b[tasodif() % uz] = (char)(32 + tasodif() % 95);
        break;
    case 3:                                     /* bo'sh joylarni yo'qotamiz yoki ko'paytiramiz */
        for (size_t i = 0; i < uz; i++)
            if (b[i] == ' ' && tasodif() % 2)
                b[i] = tasodif() % 2 ? '\t' : 'x';
        break;
    case 4:                                     /* manfiy ishora */
        if (uz + 2 < hajm) {
            size_t i = tasodif() % (uz + 1);
            memmove(b + i + 1, b + i, uz - i + 1);
            b[i] = '-';
        }
        break;
    case 5:                                     /* juda katta son */
        snprintf(b, hajm, "%s 99999999999999999999999 %u", urug[0], tasodif());
        break;
    default:                                    /* o'zgartirmaymiz: to'g'ri qator */
        break;
    }
}

int main(int argc, char **argv)
{
    unsigned long urug_son = argc > 1 ? strtoul(argv[1], NULL, 10) : 1;
    long n = argc > 2 ? atol(argv[2]) : 1000;
    holat = urug_son;

    static const char *const xodim_urug[] = { "1042 Aziza 2 2500050", "2087 Bobur 1 3150000", "9999 Sardor 4 1250000075" };
    long qabul = 0, rad = 0;
#ifndef ZAIF
    static const char *const davomat_urug[] = { "1042 1 0900 1800", "3150 2 1000 1800", "9999 31 0000 2359" };
    long d_qabul = 0, d_rad = 0;
#endif
    for (long i = 0; i < n; i++) {
        char qator[QATOR_UZ * 2];
        struct xodim x;
        const char *sabab;

        buz(qator, sizeof(qator), xodim_urug, 3);
        FILE *f = fopen("oxirgi_kirish.txt", "w");      /* qulasa, qaysi qator sababchi ekanini bilamiz */
        if (f) {
            fputs(qator, f);
            fclose(f);
        }
        if (XODIM_TAHLIL(qator, &x, &sabab) == 0) {
            qabul++;
            int yaroqli = x.id >= 1000 && x.id <= 9999 && x.toifa >= 0 && x.toifa < T_SONI && x.tarif > 0 &&
                          x.tarif <= TARIF_MAKS && strlen(x.ism) < ISM_UZ;
            if (!yaroqli) {
                printf("INVARIANT BUZILDI: qabul qilingan xodim noto'g'ri: id=%d toifa=%d tarif=%lld  (qator: %.60s)\n", x.id,
                       (int)x.toifa, (long long)x.tarif, qator);
                return 1;
            }
        } else {
            rad++;
        }

#ifndef ZAIF
        buz(qator, sizeof(qator), davomat_urug, 3);
        int id, kirish, chiqish;
        if (davomat_tahlil(qator, &id, &kirish, &chiqish, &sabab) == 0) {
            d_qabul++;
            if (id < 1000 || id > 9999 || kirish < 0 || kirish > 2359 || chiqish < 0 || chiqish > 2359) {
                printf("INVARIANT BUZILDI: davomat: id=%d kirish=%d chiqish=%d\n", id, kirish, chiqish);
                return 1;
            }
        } else {
            d_rad++;
        }
#endif
    }
    printf("urug %lu, %ld ta kirish: xodim qabul %ld / rad %ld", urug_son, n, qabul, rad);
#ifndef ZAIF
    printf("; davomat qabul %ld / rad %ld", d_qabul, d_rad);
#endif
    printf(". Invariant buzilmadi, sanitizer jim.\n");
    return 0;
}
