/* kadrlar.c - Kadrlar tizimi, 10-bosqich: PREPROTSESSOR (makrolar). Hali bitta fayl, ma'lumot kod ichida.
   Maqsad: takrorlanadigan narsalarni (toifalar jadvali, pul formati, jurnal) makro bilan BIR JOYDA yozish. */
#include <stdint.h>
#include <stdio.h>

/* ---- 1) doimiylar: sehrli sonlar o'rniga NOM (o'zgartirish bir joyda) ---- */
#define MAKS_XODIM 32
#define TANAFFUS_DAQ 60
#define KASABA_FOIZ 1
#define SOLIQ_CHEGARA1 300000000LL              /* 3 000 000 so'm (tiyinda) */
#define SOLIQ_CHEGARA2 800000000LL
#define SOLIQ_FOIZ1 12
#define SOLIQ_FOIZ2 15
#define SOLIQ_FOIZ3 20

/* ---- 2) X-makro: toifalar jadvali BIR marta yoziladi, undan enum, matnlar va bonuslar HOSIL QILINADI ---- */
#define TOIFALAR(X)                             \
    X(BOSHLOVCHI, "Boshlovchi", 0)              \
    X(MUTAXASSIS, "Mutaxassis", 5)              \
    X(YETAKCHI, "Yetakchi", 10)                 \
    X(RAHBAR, "Rahbar", 20)

enum toifa {
#define X(nom, matn, bonus) T_##nom,            /* ## - ikki bo'lakni yopishtiradi: T_ + BOSHLOVCHI */
    TOIFALAR(X)
#undef X
    T_SONI                                      /* oxirgi: toifalar soni */
};

static const char *const toifa_matni[] = {
#define X(nom, matn, bonus) [T_##nom] = matn,
    TOIFALAR(X)
#undef X
};

static const int toifa_bonusi[] = {
#define X(nom, matn, bonus) [T_##nom] = bonus,
    TOIFALAR(X)
#undef X
};

/* ---- 3) kichik yordamchi makrolar ---- */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))       /* har argument QAVSDA: MIN(x + 1, y) to'g'ri ishlasin */

/* kompilyatsiya vaqtida tekshiruv: jadvallar toifalar soniga mos bo'lishi shart */
_Static_assert(ARRAY_SIZE(toifa_matni) == T_SONI, "toifa_matni uzunligi toifalar soniga teng emas");
_Static_assert(ARRAY_SIZE(toifa_bonusi) == T_SONI, "toifa_bonusi uzunligi toifalar soniga teng emas");

/* pul formati: tiyinni "so'm.tiyin" ko'rinishida chiqarish uchun ikki makro birga ishlaydi */
#define PUL_FMT "%lld.%02lld"                   /* oddiy */
#define PUL_USTUN "%11lld.%02lld"               /* jadval ustuni uchun: 14 belgi, o'ngga tekis */
#define PUL_ARG(t) (long long)((t) / 100), (long long)((t) % 100)

/* ---- 4) -DDEBUG bilan yoqiladigan jurnal. DEBUG bo'lmasa makro BO'SH: kodga umuman kirmaydi ---- */
#ifdef DEBUG
#define LOG(fmt, ...) fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG(fmt, ...) ((void)0)
#endif

/* shart bajarilmasa dasturni to'xtatadigan tekshiruv: #shart - ifodaning O'Z MATNI */
#define SHART(cond)                                                                    \
    do {                                                                               \
        if (!(cond)) {                                                                 \
            fprintf(stderr, "SHART buzildi: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
            return 1;                                                                  \
        }                                                                              \
    } while (0)                                 /* do{}while(0): makro bitta operator bo'lib ishlashi uchun */

struct xodim {
    int id;
    const char *ism;
    enum toifa toifa;
    int64_t tarif;                              /* tiyin / soat */
    int oddiy_daq, qosh_daq;
};

static const struct xodim xodimlar[] = {
    { 1042, "Aziza", T_MUTAXASSIS, 2500050, 1440, 90 },
    { 2087, "Bobur", T_BOSHLOVCHI, 3150000, 900, 0 },
    { 3150, "Dilnoza", T_YETAKCHI, 1875050, 900, 120 },
    { 9999, "Sardor", T_RAHBAR, 1250000075LL, 960, 180 },
};

/* summaning p foizi, tiyinga yaxlitlab */
static int64_t foiz(int64_t summa, int p)
{
    return (summa * p + 50) / 100;
}

/* progressiv soliq: 3 bo'lak, har biri o'z foizi bilan */
static int64_t soliq(int64_t brutto)
{
    int64_t q1 = MIN(brutto, SOLIQ_CHEGARA1);
    int64_t q2 = brutto > SOLIQ_CHEGARA1 ? MIN(brutto, SOLIQ_CHEGARA2) - SOLIQ_CHEGARA1 : 0;
    int64_t q3 = brutto > SOLIQ_CHEGARA2 ? brutto - SOLIQ_CHEGARA2 : 0;
    LOG("soliq bo'laklari: " PUL_FMT " + " PUL_FMT " + " PUL_FMT, PUL_ARG(q1), PUL_ARG(q2), PUL_ARG(q3));
    return foiz(q1, SOLIQ_FOIZ1) + foiz(q2, SOLIQ_FOIZ2) + foiz(q3, SOLIQ_FOIZ3);
}

int main(void)
{
    SHART(ARRAY_SIZE(xodimlar) <= MAKS_XODIM);

    printf("%-5s %-9s %-11s %14s %14s %14s\n", "ID", "Ism", "Toifa", "Brutto", "Soliq", "Qo'lga");
    int64_t jami_sof = 0;
    for (size_t i = 0; i < ARRAY_SIZE(xodimlar); i++) {
        const struct xodim *x = &xodimlar[i];
        SHART(x->toifa >= 0 && x->toifa < T_SONI);

        int64_t asosiy = (x->tarif * x->oddiy_daq + 30) / 60;
        int64_t ustama = (x->tarif * x->qosh_daq + 30) / 60 * 3 / 2;
        int64_t bonus = foiz(asosiy, toifa_bonusi[x->toifa]);
        int64_t brutto = asosiy + ustama + bonus;
        int64_t s = soliq(brutto);
        int64_t sof = brutto - s - foiz(brutto, KASABA_FOIZ);
        LOG("%s: asosiy=" PUL_FMT " ustama=" PUL_FMT " bonus=" PUL_FMT, x->ism, PUL_ARG(asosiy), PUL_ARG(ustama), PUL_ARG(bonus));

        printf("%-5d %-9s %-11s " PUL_USTUN " " PUL_USTUN " " PUL_USTUN "\n", x->id, x->ism, toifa_matni[x->toifa], PUL_ARG(brutto),
               PUL_ARG(s), PUL_ARG(sof));
        jami_sof += sof;
    }
    printf("Jami qo'lga tegadi: " PUL_FMT " so'm\n", PUL_ARG(jami_sof));
    return 0;
}
