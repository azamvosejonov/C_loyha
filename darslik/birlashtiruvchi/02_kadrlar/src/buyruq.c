#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buyruq.h"
#include "hisobot.h"
#include "matn.h"

int buyruq_bajar(struct ombor *o, int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "foydalanish: kadrlar royxat | varaqa ID | top | jami | izla ISM\n");
        return 1;
    }
    const char *b = argv[1];

    if (strcmp(b, "royxat") == 0) {
        hisobot_royxat(o);
    } else if (strcmp(b, "top") == 0) {
        ombor_saralash_tarif(o);
        printf("Tarif bo'yicha reyting:\n");
        for (int i = 0; i < o->soni; i++)
            printf("%2d. %-10s %12lld.%02lld so'm/soat\n", i + 1, o->a[i].ism, (long long)(o->a[i].tarif / 100), (long long)(o->a[i].tarif % 100));
    } else if (strcmp(b, "jami") == 0) {
        hisobot_jami_chiqar(o);
    } else if (strcmp(b, "varaqa") == 0 && argc >= 3) {
        struct xodim *x = ombor_top(o, atoi(argv[2]));
        if (!x) {
            fprintf(stderr, "xodim topilmadi: %s\n", argv[2]);
            return 1;
        }
        xodim_varaqa(x);
    } else if (strcmp(b, "izla") == 0 && argc >= 3) {
        struct xodim *x = ombor_ism_bilan_top(o, argv[2]);
        if (!x) {
            fprintf(stderr, "xodim topilmadi: %s\n", argv[2]);
            return 1;
        }
        printf("%s: ID %d-%d, toifa %d (ism %d belgi)\n", x->ism, x->id, id_nazorat(x->id), x->toifa, nom_uzunligi(x->ism));
    } else {
        fprintf(stderr, "noma'lum buyruq: %s\n", b);
        return 1;
    }
    return 0;
}
