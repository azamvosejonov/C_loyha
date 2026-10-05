/* main.c - dastur kirish nuqtasi: ma'lumotlarni yuklaydi va buyruqni bajaradi.
   Yuklash tartibi: xodimlar.txt -> davomat.txt -> buyruq. Fayllar yo'li: data/ papkasi (KADRLAR_DATA muhit o'zgaruvchisi bilan almashtiriladi) */
#include <stdio.h>
#include <stdlib.h>

#include "buyruq.h"
#include "davomat.h"
#include "ombor.h"

int main(int argc, char **argv)
{
    const char *papka = getenv("KADRLAR_DATA");
    if (!papka)
        papka = "data";

    char yol[256];
    static struct ombor ombor;                  /* static: katta struct stekka sig'masligi mumkin */
    ombor_boshla(&ombor);

    int rad = 0;
    snprintf(yol, sizeof(yol), "%s/xodimlar.txt", papka);
    int n = ombor_yukla(&ombor, yol, &rad);
    if (n < 0) {
        fprintf(stderr, "%s ochilmadi\n", yol);
        return 2;
    }
    fprintf(stderr, "xodimlar: %d ta yuklandi, %d ta rad etildi\n", n, rad);

    rad = 0;
    snprintf(yol, sizeof(yol), "%s/davomat.txt", papka);
    n = davomat_yukla(&ombor, yol, &rad);
    if (n < 0) {
        fprintf(stderr, "%s ochilmadi\n", yol);
        return 2;
    }
    fprintf(stderr, "davomat: %d ta yozuv qabul qilindi, %d ta rad etildi\n", n, rad);

    return buyruq_bajar(&ombor, argc, argv);
}
