#include <stdio.h>
#include <string.h>

#include "ombor.h"
#include "test.h"

static void xodim_yoz(struct xodim *x, int id, const char *ism, int toifa, long long tarif)
{
    memset(x, 0, sizeof(*x));
    x->id = id;
    snprintf(x->ism, sizeof(x->ism), "%s", ism);
    x->toifa = toifa;
    x->tarif = tarif;
}

static void tolat(struct ombor *o)
{
    struct xodim x;
    ombor_boshla(o);
    xodim_yoz(&x, 1111, "Ali", 1, 500);
    ombor_qosh(o, &x);
    xodim_yoz(&x, 2222, "Vali", 2, 900);
    ombor_qosh(o, &x);
    xodim_yoz(&x, 3333, "Sami", 3, 700);
    ombor_qosh(o, &x);
}

int main(void)
{
    struct ombor o;
    tolat(&o);

    BOSHLA("T10 ombor_top");
    struct xodim *p = ombor_top(&o, 2222);
    TEKSHIR_SHART(p != NULL);
    if (p) {
        TEKSHIR(p->id, 2222);
        TEKSHIR_SHART(p == &o.a[1]);                    /* NUSXA emas, omborning o'zidagi xodim */
        p->oddiy_daq = 77;
        TEKSHIR(o.a[1].oddiy_daq, 77);                  /* ko'rsatkich orqali o'zgartirish omborga ta'sir qiladi */
    }
    TEKSHIR_SHART(ombor_top(&o, 9999) == NULL);
    TEKSHIR_SHART(ombor_top(&o, 1111) == &o.a[0]);
    TUGAT();

    BOSHLA("T11 ombor_ism_bilan_top");
    tolat(&o);
    struct xodim *q = ombor_ism_bilan_top(&o, "Sami");
    TEKSHIR_SHART(q == &o.a[2]);
    TEKSHIR_SHART(ombor_ism_bilan_top(&o, "sami") == NULL);
    TEKSHIR_SHART(ombor_ism_bilan_top(&o, "Nobor") == NULL);
    TEKSHIR_SHART(ombor_ism_bilan_top(&o, "Ali") == &o.a[0]);
    TUGAT();

    BOSHLA("T12 ombor_saralash_tarif");
    tolat(&o);
    ombor_saralash_tarif(&o);
    TEKSHIR(o.soni, 3);
    TEKSHIR(o.a[0].tarif, 900);
    TEKSHIR(o.a[1].tarif, 700);
    TEKSHIR(o.a[2].tarif, 500);
    TEKSHIR(o.a[0].id, 2222);                           /* butun struct birga ko'chishi kerak, faqat tarif emas */
    TEKSHIR_SHART(strcmp(o.a[2].ism, "Ali") == 0);
    ombor_boshla(&o);
    ombor_saralash_tarif(&o);                           /* bo'sh omborda ham xato bermasin */
    TEKSHIR(o.soni, 0);
    TUGAT();

    BOSHLA("T13 ombor_yukla");
    FILE *f = fopen("/tmp/kadrlar_test_xodimlar.txt", "w");
    if (f) {
        fputs("# izoh\n"
              "\n"
              "1111 Ali 1 500\n"
              "2222 Vali 2 900\n"
              "12 Qisqa 1 100\n"            /* ID noto'g'ri */
              "1111 Takror 1 100\n"         /* ID takrori */
              "3333 Toifa 9 100\n"          /* toifa noto'g'ri */
              "4444 Tarif 2 -5\n"           /* tarif noto'g'ri */
              "5555 Yarim 2\n"              /* maydon yetishmaydi */
              "6666 Sami 3 700\n", f);
        fclose(f);
    }
    ombor_boshla(&o);
    int rad = 0;
    int n = ombor_yukla(&o, "/tmp/kadrlar_test_xodimlar.txt", &rad);
    TEKSHIR(n, 3);
    TEKSHIR(rad, 5);
    TEKSHIR(o.soni, 3);
    TEKSHIR(o.a[1].id, 2222);
    TEKSHIR(o.a[2].tarif, 700);
    TEKSHIR_SHART(strcmp(o.a[2].ism, "Sami") == 0);
    rad = 0;
    TEKSHIR(ombor_yukla(&o, "/tmp/mavjud_emas_fayl.txt", &rad), -1);
    TUGAT();
    YAKUN();
}
