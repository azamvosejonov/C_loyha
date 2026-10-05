/* main.c - kirish nuqtasi. Ma'lumot hozircha kod ichida (12-bobda fayldan o'qiladi) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hisobot.h"
#include "ombor.h"

struct yozuv {
    int id;
    const char *ism;
    enum toifa toifa;
    int64_t tarif;
    int oddiy_daq, qosh_daq;
};

static const struct yozuv boshlangich[] = {
    { 1042, "Aziza", T_MUTAXASSIS, 2500050, 1440, 90 },
    { 2087, "Bobur", T_BOSHLOVCHI, 3150000, 900, 0 },
    { 3150, "Dilnoza", T_YETAKCHI, 1875050, 900, 120 },
    { 9999, "Sardor", T_RAHBAR, 1250000075LL, 960, 180 },
};

int main(int argc, char **argv)
{
    struct ombor *o = ombor_yarat();
    if (!o)
        return 2;
    for (size_t i = 0; i < sizeof(boshlangich) / sizeof(boshlangich[0]); i++) {
        struct xodim x = { .id = boshlangich[i].id, .toifa = boshlangich[i].toifa, .tarif = boshlangich[i].tarif,
                           .oddiy_daq = boshlangich[i].oddiy_daq, .qosh_daq = boshlangich[i].qosh_daq };
        snprintf(x.ism, sizeof(x.ism), "%s", boshlangich[i].ism);
        ombor_qosh(o, &x);
    }

    int kod = 0;
    if (argc >= 3 && strcmp(argv[1], "varaqa") == 0) {
        const struct xodim *x = ombor_top(o, atoi(argv[2]));
        if (x)
            xodim_varaqa(x);
        else {
            fprintf(stderr, "xodim topilmadi: %s\n", argv[2]);
            kod = 1;
        }
    } else {
        hisobot_royxat(o);
        hisobot_jami(o);
    }
    ombor_yoq_qil(o);
    return kod;
}
