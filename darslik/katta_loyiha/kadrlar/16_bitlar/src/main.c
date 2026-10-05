/* main.c - kirish nuqtasi: xodimlar.txt -> davomat.txt -> buyruq.
   Buyruqlar:  royxat [ism|maosh] | varaqa ID | jami | saqla FAYL | eksport FAYL.csv | filtr DASTUR [ARG...] | sinov N OQIMLAR [poyga] | bin-saqla FAYL | bin-yukla FAYL | hex FAYL [N].  Ma'lumot papkasi: KADRLAR_DATA muhit o'zgaruvchisi yoki "data" */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include "eksport.h"
#include "hisobot.h"
#include "ikkilik.h"
#include "parallel.h"
#include "ombor.h"
#include "sistema.h"
#include "yukla.h"

/* n ta sinov xodimi uchun ketma-ket va parallel yig'indilarni solishtiradi */
static int parallel_sinov(long n, int oqimlar, int poyga_bilan)
{
    struct xodim *a = n > 0 ? sinov_xodimlari((size_t)n) : NULL;
    if (!a) {
        fprintf(stderr, "sinov: N musbat son bo'lishi kerak (yoki xotira yetmadi)\n");
        return 1;
    }
    struct natija etalon, r;
    jami_ketma_ket(a, (size_t)n, &etalon);
    printf("%ld xodim, %d oqim\n", n, oqimlar);
    printf("ketma-ket (etalon): qo'lga tegadi %lld tiyin\n", (long long)etalon.sof);

    int kod = 0;
    struct {
        const char *nom;
        int (*f)(const struct xodim *, size_t, int, struct natija *);
        int yoqilgan;
    } variantlar[] = {
        { "parallel (bo'lak)", jami_parallel, 1 },
        { "qulf bilan", jami_qulf, 1 },
        { "POYGA (xatoli)", jami_poyga, poyga_bilan },
    };
    for (size_t i = 0; i < sizeof(variantlar) / sizeof(variantlar[0]); i++) {
        if (!variantlar[i].yoqilgan)
            continue;
        if (variantlar[i].f(a, (size_t)n, oqimlar, &r) != 0) {
            printf("%-20s oqim yaratib bo'lmadi\n", variantlar[i].nom);
            kod = 1;
            continue;
        }
        int mos = r.sof == etalon.sof && r.brutto == etalon.brutto && r.soliq == etalon.soliq;
        printf("%-20s %s\n", variantlar[i].nom, mos ? "etalon bilan MOS" : "FARQ QILADI");
        if (!mos && i < 2)
            kod = 1;                            /* to'g'ri variantlar farq qilsa - xato; poyga farq qilsa - kutilgan hol */
    }
    free(a);
    return kod;
}

static int yukla_hammasi(struct ombor *o)
{
    const char *papka = getenv("KADRLAR_DATA");
    if (!papka)
        papka = "data";
    char yol[256];
    struct yuklash h = { 0, 0 };

    snprintf(yol, sizeof(yol), "%s/xodimlar.txt", papka);
    if (xodimlar_yukla(o, yol, &h) != 0)
        return -1;
    fprintf(stderr, "xodimlar: %d ta qabul, %d ta rad\n", h.qabul, h.rad);

    h = (struct yuklash){ 0, 0 };
    snprintf(yol, sizeof(yol), "%s/davomat.txt", papka);
    if (davomat_yukla(o, yol, &h) != 0)
        return -1;
    fprintf(stderr, "davomat: %d ta qabul, %d ta rad\n", h.qabul, h.rad);
    return 0;
}

int main(int argc, char **argv)
{
    struct ombor *o = ombor_yarat();
    if (!o)
        return 2;
    int kod = 0;
    if (yukla_hammasi(o) != 0) {
        ombor_yoq_qil(o);
        return 2;
    }

    const char *b = argc > 1 ? argv[1] : "royxat";
    if (strcmp(b, "royxat") == 0) {
        enum tartib t = TARTIB_YUKLANGAN;
        if (argc > 2)
            t = strcmp(argv[2], "ism") == 0 ? TARTIB_ISM : TARTIB_MAOSH;
        hisobot_royxat(o, t, stdout);
        hisobot_jami(o, stdout);
    } else if (strcmp(b, "varaqa") == 0 && argc > 2) {
        const struct xodim *x = ombor_top(o, atoi(argv[2]));
        if (x)
            xodim_varaqa(x);
        else {
            fprintf(stderr, "xodim topilmadi: %s\n", argv[2]);
            kod = 1;
        }
    } else if (strcmp(b, "jami") == 0) {
        hisobot_jami(o, stdout);
    } else if (strcmp(b, "saqla") == 0 && argc > 2) {
        kod = hisobot_saqla(o, argv[2]) == 0 ? 0 : 1;
        if (kod == 0)
            printf("hisobot %s fayliga saqlandi\n", argv[2]);
    } else if (strcmp(b, "eksport") == 0 || strcmp(b, "filtr") == 0) {
        size_t uz;
        char *csv = eksport_csv(o, &uz);
        if (!csv) {
            fprintf(stderr, "xotira yetmadi\n");
            kod = 2;
        } else if (strcmp(b, "eksport") == 0 && argc > 2) {
            if (fayl_yarat_yoz(argv[2], csv, uz) == 0) {
                printf("%zu bayt %s fayliga yozildi\n", uz, argv[2]);
            } else {
                fprintf(stderr, "%s: %s\n", argv[2], strerror(errno));
                kod = 1;
            }
        } else if (strcmp(b, "filtr") == 0 && argc > 2) {
            kod = dastur_ishlat(&argv[2], csv, uz);         /* argv[2]... = DASTUR va uning argumentlari (oxiri NULL: C standarti kafolat beradi) */
            if (kod < 0) {
                fprintf(stderr, "dastur ishga tushmadi: %s\n", strerror(errno));
                kod = 2;
            }
        } else {
            fprintf(stderr, "foydalanish: kadrlar eksport FAYL | filtr DASTUR [ARG...]\n");
            kod = 1;
        }
        free(csv);
    } else if (strcmp(b, "sinov") == 0 && argc > 3) {
        kod = parallel_sinov(atol(argv[2]), atoi(argv[3]), argc > 4);
    } else if (strcmp(b, "bin-saqla") == 0 && argc > 2) {
        if (ikkilik_saqla(o, argv[2]) == 0) {
            printf("%zu xodim %s fayliga (binar) saqlandi\n", ombor_soni(o), argv[2]);
        } else {
            fprintf(stderr, "%s: %s\n", argv[2], strerror(errno));
            kod = 1;
        }
    } else if (strcmp(b, "bin-yukla") == 0 && argc > 2) {
        struct ombor *y = ombor_yarat();
        struct yuklash h = { 0, 0 };
        if (!y || ikkilik_yukla(y, argv[2], &h) != 0) {
            kod = 1;
        } else {
            printf("binar fayldan: %d ta o'qildi, %d ta rad etildi\n", h.qabul, h.rad);
            hisobot_royxat(y, TARTIB_YUKLANGAN, stdout);
        }
        ombor_yoq_qil(y);
    } else if (strcmp(b, "hex") == 0 && argc > 2) {
        if (hex_chiqar(argv[2], argc > 3 ? (size_t)atol(argv[3]) : 64) != 0) {
            fprintf(stderr, "%s: %s\n", argv[2], strerror(errno));
            kod = 1;
        }
    } else {
        fprintf(stderr, "foydalanish: kadrlar royxat [ism|maosh] | varaqa ID | jami | saqla FAYL | eksport FAYL | filtr DASTUR... | sinov N OQIMLAR [poyga] | bin-saqla F | bin-yukla F | hex F [N]\n");
        kod = 1;
    }
    ombor_yoq_qil(o);
    return kod;
}
