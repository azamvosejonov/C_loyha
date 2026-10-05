#include <stdio.h>
#include <string.h>

#include "hisobot.h"
#include "pul.h"
#include "xodim.h"

static void pul_ustun(int64_t tiyin, int kenglik)
{
    printf("%*lld.%02lld", kenglik - 3, (long long)(tiyin / 100), (long long)(tiyin % 100));
}

void hisobot_royxat(const struct ombor *o)
{
    printf("%-5s %-10s %-10s %13s %13s %13s\n", "ID", "Ism", "Toifa", "Brutto", "Soliq", "Qo'lga");
    for (int i = 0; i < o->soni; i++) {
        const struct xodim *x = &o->a[i];
        struct natija n;
        xodim_hisobla(x, &n);
        printf("%-5d %-10s %-10s ", x->id, x->ism, toifa_nomi(x->toifa));
        pul_ustun(n.brutto, 13);
        putchar(' ');
        pul_ustun(n.soliq, 13);
        putchar(' ');
        pul_ustun(n.sof, 13);
        putchar('\n');
    }
    printf("Jami xodim: %d\n", o->soni);
}

void hisobot_jami(const struct ombor *o, struct natija *j)
{
    /* TODO T14: shu yerga yozing (nollash + sikl + xodim_hisobla) */
    (void)o;
    memset(j, 0, sizeof(*j));
}

void hisobot_jami_chiqar(const struct ombor *o)
{
    struct natija j;
    hisobot_jami(o, &j);
    printf("=== JAMI (%d xodim) ===\n", o->soni);
    pul_chiqar("Asosiy:", j.asosiy);
    pul_chiqar("Ustama:", j.ustama);
    pul_chiqar("Bonus:", j.bonus);
    pul_chiqar("Brutto:", j.brutto);
    pul_chiqar("Soliq:", j.soliq);
    pul_chiqar("Kasaba:", j.kasaba);
    pul_chiqar("Qo'lga tegadi:", j.sof);
}
