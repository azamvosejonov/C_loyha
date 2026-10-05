/* main.c - kirish nuqtasi: xodimlar.txt -> davomat.txt -> buyruq.
   Buyruqlar:  royxat [ism|maosh] | varaqa ID | jami | saqla FAYL.  Ma'lumot papkasi: KADRLAR_DATA muhit o'zgaruvchisi yoki "data" */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hisobot.h"
#include "ombor.h"
#include "yukla.h"

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
    } else {
        fprintf(stderr, "foydalanish: kadrlar royxat [ism|maosh] | varaqa ID | jami | saqla FAYL\n");
        kod = 1;
    }
    ombor_yoq_qil(o);
    return kod;
}
