#include <stdio.h>

#include "config.h"
#include "davomat.h"
#include "vaqt.h"

int davomat_yukla(struct ombor *o, const char *fayl, int *rad)
{
    FILE *f = fopen(fayl, "r");
    if (!f)
        return -1;

    int qabul = 0;
    char qator[128];
    while (fgets(qator, sizeof(qator), f)) {
        if (qator[0] == '#' || qator[0] == '\n')
            continue;
        int id, kun, kirish, chiqish;
        if (sscanf(qator, "%d %d %d %d", &id, &kun, &kirish, &chiqish) != 4) {
            (*rad)++;
            continue;
        }
        struct xodim *x = ombor_top(o, id);         /* ko'rsatkich: omborning o'zidagi xodim */
        int sof = ish_daqiqalari(kirish, chiqish, TANAFFUS_DAQ);
        if (x == NULL || sof < 0) {
            (*rad)++;
            continue;
        }
        x->oddiy_daq += sof < NORMA_KUN_DAQ ? sof : NORMA_KUN_DAQ;
        x->qosh_daq += sof > NORMA_KUN_DAQ ? sof - NORMA_KUN_DAQ : 0;
        qabul++;
    }
    fclose(f);
    return qabul;
}
