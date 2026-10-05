/* sched_lab.c - rejalashtiruvchi laboratoriyasi: FIFO, SJF, Round Robin, ustuvorlik (aging bilan) va ularning ko'rsatkichlari */
#include <stdio.h>
#include <string.h>

#define MAKS 16
#define MAKS_VAQT 512

struct is {
    char nom[8];
    int kelish, davom, ustuvor;                 /* kirish ma'lumoti: kelish vaqti, kerakli CPU vaqti, ustuvorlik (kichik = muhim) */
    int qoldi;                                  /* hali bajarilishi kerak bo'lgan vaqt */
    int birinchi, tugadi;                       /* birinchi marta ishga tushgan va tugagan vaqt (-1: hali emas) */
    int kutdi;                                  /* aging uchun: ketma-ket kutgan tiklar */
    int hozirgi_ustuvor;
};

struct sim {
    struct is is[MAKS];
    int n;
    int t;                                      /* hozirgi vaqt (tik) */
    int joriy;                                  /* hozir CPU da turgan ish (-1 - hech kim) */
    int kvant;                                  /* RR: kvant uzunligi */
    int kvant_ishlatdi;                         /* RR: joriy ish shu kvantda necha tik ishladi */
    int aging;                                  /* 1 - kutganlar ustuvorligi oshiriladi */
    int navbat[MAKS * 64];                      /* RR: tayyor ishlar navbati */
    int bosh, oxir;
};

/* siyosat: "keyingi tikda KIM ishlasin?" degan savolga javob beradi (funksiya ko'rsatkichi, 7-bob) */
typedef int (*siyosat)(struct sim *s);

static int tayyor(const struct sim *s, int i)
{
    return s->is[i].kelish <= s->t && s->is[i].qoldi > 0;
}

static int fifo(struct sim *s)
{
    if (s->joriy >= 0 && s->is[s->joriy].qoldi > 0)
        return s->joriy;                        /* nopreemptiv: boshlangan ish tugamaguncha turadi */
    int eng = -1;
    for (int i = 0; i < s->n; i++)
        if (tayyor(s, i) && (eng < 0 || s->is[i].kelish < s->is[eng].kelish))
            eng = i;                            /* eng erta kelgan */
    return eng;
}

static int sjf(struct sim *s)
{
    if (s->joriy >= 0 && s->is[s->joriy].qoldi > 0)
        return s->joriy;
    int eng = -1;
    for (int i = 0; i < s->n; i++)
        if (tayyor(s, i) && (eng < 0 || s->is[i].davom < s->is[eng].davom))
            eng = i;                            /* eng qisqa ish (davomiyligi bo'yicha) */
    return eng;
}

static int rr(struct sim *s)
{
    if (s->joriy >= 0 && s->is[s->joriy].qoldi > 0 && s->kvant_ishlatdi < s->kvant)
        return s->joriy;                        /* kvant hali tugamagan */
    if (s->joriy >= 0 && s->is[s->joriy].qoldi > 0)
        s->navbat[s->oxir++] = s->joriy;        /* kvant tugadi, ish tugamagan: navbat OXIRIGA */
    s->kvant_ishlatdi = 0;
    return s->bosh < s->oxir ? s->navbat[s->bosh++] : -1;
}

static int ustuvorlik(struct sim *s)            /* preemptiv ustuvorlik: har tikda eng muhimini tanlaymiz */
{
    int eng = -1;
    for (int i = 0; i < s->n; i++) {
        if (!tayyor(s, i))
            continue;
        if (eng < 0 || s->is[i].hozirgi_ustuvor < s->is[eng].hozirgi_ustuvor)
            eng = i;
    }
    return eng;
}

/* bitta simulyatsiya: har tikda siyosat tanlaydi, tanlangan ish bitta tik ishlaydi */
static void yurgiz(struct sim *s, siyosat tanla, char *gantt)
{
    int bajarildi = 0;
    for (s->t = 0; bajarildi < s->n && s->t < MAKS_VAQT; s->t++) {
        for (int i = 0; i < s->n; i++)          /* RR: shu tikda kelgan ishlar navbatga qo'shiladi (preempt qilinganlardan OLDIN) */
            if (s->is[i].kelish == s->t && tanla == rr)
                s->navbat[s->oxir++] = i;

        int k = tanla(s);
        s->joriy = k;
        if (k < 0) {
            gantt[s->t] = '.';                  /* CPU bo'sh */
            continue;
        }
        struct is *p = &s->is[k];
        if (p->birinchi < 0)
            p->birinchi = s->t;
        gantt[s->t] = p->nom[0];
        p->qoldi--;
        p->kutdi = 0;
        p->hozirgi_ustuvor = p->ustuvor;         /* ishlagan ish asl ustuvorligiga QAYTADI (aging faqat kutganlarga) */
        s->kvant_ishlatdi++;
        if (p->qoldi == 0) {
            p->tugadi = s->t + 1;
            bajarildi++;
        }

        for (int i = 0; i < s->n; i++)          /* aging: kutganlar har 2 tikda bir pog'ona "muhimroq" bo'ladi */
            if (i != k && tayyor(s, i) && s->aging && ++s->is[i].kutdi % 2 == 0 && s->is[i].hozirgi_ustuvor > 0)
                s->is[i].hozirgi_ustuvor--;
    }
    gantt[s->t] = '\0';
}

static void hisobot(const char *nom, const struct sim *s, const char *gantt)
{
    printf("== %s ==\n  %s\n", nom, gantt);
    printf("  ish  kelish davom  boshlandi tugadi  aylanish javob kutish\n");
    double a = 0, j = 0, k = 0;
    for (int i = 0; i < s->n; i++) {
        const struct is *p = &s->is[i];
        int aylanish = p->tugadi - p->kelish;   /* kelganidan tugaguncha */
        int javob = p->birinchi - p->kelish;    /* kelganidan birinchi ishlagunicha */
        int kutish = aylanish - p->davom;       /* tayyor turib ishlamagan vaqt */
        printf("  %-4s %5d %5d %9d %6d %8d %5d %5d\n", p->nom, p->kelish, p->davom, p->birinchi, p->tugadi, aylanish, javob, kutish);
        a += aylanish;
        j += javob;
        k += kutish;
    }
    printf("  o'rtacha: aylanish %.2f, javob %.2f, kutish %.2f\n", a / s->n, j / s->n, k / s->n);
}

static int oqi_ishlar(struct sim *s, FILE *f)
{
    memset(s, 0, sizeof(*s));
    while (s->n < MAKS && fscanf(f, "%7s %d %d %d", s->is[s->n].nom, &s->is[s->n].kelish, &s->is[s->n].davom, &s->is[s->n].ustuvor) == 4)
        s->n++;
    return s->n;
}

static void qayta_tayyorla(struct sim *s, const struct sim *asl, int kvant, int aging)
{
    *s = *asl;
    s->kvant = kvant;
    s->aging = aging;
    s->joriy = -1;
    for (int i = 0; i < s->n; i++) {
        s->is[i].qoldi = s->is[i].davom;
        s->is[i].birinchi = s->is[i].tugadi = -1;
        s->is[i].hozirgi_ustuvor = s->is[i].ustuvor;
        s->is[i].kutdi = 0;
    }
}

int main(int argc, char **argv)
{
    FILE *f = argc > 1 ? fopen(argv[1], "r") : stdin;
    struct sim asl;
    if (!f || oqi_ishlar(&asl, f) == 0) {
        fprintf(stderr, "ishlar o'qilmadi\n");
        return 1;
    }

    static struct sim s;
    char gantt[MAKS_VAQT + 1];

    qayta_tayyorla(&s, &asl, 0, 0);
    yurgiz(&s, fifo, gantt);
    hisobot("FIFO (kim oldin kelsa)", &s, gantt);

    qayta_tayyorla(&s, &asl, 0, 0);
    yurgiz(&s, sjf, gantt);
    hisobot("SJF (eng qisqa ish birinchi, nopreemptiv)", &s, gantt);

    qayta_tayyorla(&s, &asl, 3, 0);
    yurgiz(&s, rr, gantt);
    hisobot("Round Robin (kvant = 3)", &s, gantt);

    qayta_tayyorla(&s, &asl, 0, 0);
    yurgiz(&s, ustuvorlik, gantt);
    hisobot("Ustuvorlik (preemptiv, aging YO'Q)", &s, gantt);

    qayta_tayyorla(&s, &asl, 0, 1);
    yurgiz(&s, ustuvorlik, gantt);
    hisobot("Ustuvorlik (preemptiv, aging BOR)", &s, gantt);
    return 0;
}
