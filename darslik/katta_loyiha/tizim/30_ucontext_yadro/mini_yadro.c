/* mini_yadro.c - foydalanuvchi maydonida "yadro": jarayonlar jadvali, HAQIQIY kontekst almashish (ucontext), syscall jadvali, taymer tiklari, uxlash/kutish */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

#define MAKS 8
#define STEK_HAJM (64 * 1024)

enum holat { BOSH, TAYYOR, UXLAYDI, KUTADI, TUGADI };

struct jarayon {
    int pid;
    char nom[8];
    enum holat holat;
    ucontext_t ctx;                             /* saqlangan registrlar va stek ko'rsatkichi: "to'xtatilgan jarayon surati" */
    char *stek;                                 /* har jarayonning O'Z steki */
    void (*kirish)(void);
    long uyg_tik;                               /* UXLAYDI: qaysi tikda uyg'onadi */
    int kutilgan_pid;                           /* KUTADI: qaysi jarayon tugashini kutyapti */
    int chiqish_kodi;
    long a, b, natija;                          /* syscall argumentlari va natijasi (user -> yadro -> user) */
    int nr;                                     /* qaysi syscall so'ralgan */
};

static struct jarayon jadval[MAKS];
static int joriy = -1;                          /* hozir CPU da turgan jarayon */
static ucontext_t yadro_ctx;                    /* yadroning o'z konteksti */
static long tik;                                /* taymer tiklari soni */

/* ---------------- syscall jadvali (30-bob): raqam -> ishlovchi ---------------- */
enum { SYS_YIELD, SYS_SLEEP, SYS_EXIT, SYS_SPAWN, SYS_JOIN, SYS_WRITE, SYS_SONI };

static long s_yield(struct jarayon *j) { (void)j; return 0; }

static long s_sleep(struct jarayon *j)
{
    j->uyg_tik = tik + j->a;
    j->holat = UXLAYDI;                         /* tayyorlar ro'yxatidan chiqadi: uxlash navbatiga */
    return 0;
}

static long s_exit(struct jarayon *j)
{
    j->chiqish_kodi = (int)j->a;
    j->holat = TUGADI;
    return 0;
}

static long s_spawn(struct jarayon *j)
{
    for (int i = 1; i < MAKS; i++)
        if (jadval[i].holat == BOSH) {
            struct jarayon *y = &jadval[i];
            y->pid = i;
            snprintf(y->nom, sizeof(y->nom), "%s", (const char *)j->b);
            y->kirish = (void (*)(void))j->a;
            y->stek = malloc(STEK_HAJM);
            getcontext(&y->ctx);                /* hozirgi kontekstni nusxalab, so'ng o'zgartiramiz */
            y->ctx.uc_stack.ss_sp = y->stek;
            y->ctx.uc_stack.ss_size = STEK_HAJM;
            y->ctx.uc_link = &yadro_ctx;        /* kirish funksiyasi qaytsa, yadroga qaytamiz */
            makecontext(&y->ctx, y->kirish, 0);
            y->holat = TAYYOR;
            return i;
        }
    return -1;
}

static long s_join(struct jarayon *j)
{
    int pid = (int)j->a;
    if (pid <= 0 || pid >= MAKS || jadval[pid].holat == BOSH)
        return -1;
    if (jadval[pid].holat != TUGADI) {
        j->kutilgan_pid = pid;
        j->holat = KUTADI;
    }
    return 0;                                   /* tugagan bo'lsa darrov chiqish kodi keyin o'qiladi */
}

static long s_write(struct jarayon *j)
{
    printf("[tik %2ld] %-5s: %s\n", tik, j->nom, (const char *)j->a);
    return 0;
}

static long (*const jadval_syscall[SYS_SONI])(struct jarayon *) = {
    [SYS_YIELD] = s_yield, [SYS_SLEEP] = s_sleep, [SYS_EXIT] = s_exit,
    [SYS_SPAWN] = s_spawn, [SYS_JOIN] = s_join,   [SYS_WRITE] = s_write,
};

/* ---------------- "trap": jarayondan yadroga o'tish (haqiqiy mashinada: syscall buyrug'i) ---------------- */
static long trap(int nr, long a, long b)
{
    struct jarayon *j = &jadval[joriy];
    j->nr = nr;
    j->a = a;
    j->b = b;
    swapcontext(&j->ctx, &yadro_ctx);           /* ENG MUHIM QATOR: registrlarni saqlab, yadro kontekstiga o'tamiz */
    return j->natija;                           /* yadro bizni qayta ishga tushirganda shu yerdan davom etamiz */
}

static void sys_yield(void) { trap(SYS_YIELD, 0, 0); }
static void sys_sleep(long tiklar) { trap(SYS_SLEEP, tiklar, 0); }
static void sys_exit(int kod) { trap(SYS_EXIT, kod, 0); }
static int sys_spawn(const char *nom, void (*f)(void)) { return (int)trap(SYS_SPAWN, (long)f, (long)nom); }
static int sys_join(int pid) { trap(SYS_JOIN, pid, 0); return jadval[pid].chiqish_kodi; }
static void sys_write(const char *s) { trap(SYS_WRITE, (long)s, 0); }

/* ---------------- yadro: rejalashtiruvchi sikl ---------------- */
static int keyingi_tayyor(void)                 /* aylanma navbat (round-robin): joriydan KEYINGI tayyor jarayon */
{
    for (int k = 1; k <= MAKS; k++) {
        int i = (joriy + k) % MAKS;
        if (jadval[i].holat == TAYYOR)
            return i;
    }
    return -1;
}

static void uyg_otish(void)
{
    for (int i = 0; i < MAKS; i++) {
        if (jadval[i].holat == UXLAYDI && jadval[i].uyg_tik <= tik)
            jadval[i].holat = TAYYOR;
        if (jadval[i].holat == KUTADI && jadval[jadval[i].kutilgan_pid].holat == TUGADI)
            jadval[i].holat = TAYYOR;
    }
}

static int tirik_bormi(void)
{
    for (int i = 0; i < MAKS; i++)
        if (jadval[i].holat != BOSH && jadval[i].holat != TUGADI)
            return 1;
    return 0;
}

static void yadro_kirish(void)
{
    while (tirik_bormi()) {
        uyg_otish();
        int k = keyingi_tayyor();
        if (k < 0) {
            tik++;                              /* hamma uxlayapti: vaqt o'tadi (CPU "bo'sh turadi", idle) */
            continue;
        }
        joriy = k;
        tik++;                                  /* har rejalashtirish qarori bitta tik (taymer uzilishi) */
        swapcontext(&yadro_ctx, &jadval[k].ctx);        /* jarayonga o'tamiz; u trap() qilganda shu yerga qaytamiz */

        struct jarayon *j = &jadval[k];
        if (j->nr >= 0 && j->nr < SYS_SONI)
            j->natija = jadval_syscall[j->nr](j);       /* so'ralgan syscall ni jadval orqali bajaramiz */
        if (j->holat == TUGADI) {                       /* sys_exit: stekni qaytaramiz, jarayon boshqa ishlamaydi */
            free(j->stek);
            j->stek = NULL;
        }
    }
}

/* ---------------- "dasturlar" (user rejimi: faqat sys_* chaqiradi) ---------------- */
static void dastur_a(void)
{
    for (int i = 1; i <= 3; i++) {
        char s[40];
        snprintf(s, sizeof(s), "qadam %d, 4 tik uxlayman", i);
        sys_write(s);
        sys_sleep(4);
    }
    sys_exit(10);
}

static void dastur_b(void)
{
    for (int i = 1; i <= 4; i++) {
        char s[40];
        snprintf(s, sizeof(s), "hisoblayapman %d/4 (yield)", i);
        sys_write(s);
        sys_yield();                            /* ixtiyoriy ravishda CPU ni boshqalarga beramiz */
    }
    sys_exit(20);
}

static void dastur_c(void)
{
    sys_sleep(9);
    sys_write("uyg'ondim, ish qildim");
    sys_exit(30);
}

static void init_dastur(void)
{
    sys_write("init ishga tushdi, bolalar yaratilyapti");
    int a = sys_spawn("A", dastur_a);
    int b = sys_spawn("B", dastur_b);
    int c = sys_spawn("C", dastur_c);
    int ka = sys_join(a), kb = sys_join(b), kc = sys_join(c);
    char s[64];
    snprintf(s, sizeof(s), "hammasi tugadi: chiqish kodlari A=%d B=%d C=%d", ka, kb, kc);
    sys_write(s);
    sys_exit(0);
}

int main(void)
{
    /* 0-jarayon "bo'sh" (BOSH) qoladi; init - 1-pid */
    struct jarayon *y = &jadval[1];
    y->pid = 1;
    snprintf(y->nom, sizeof(y->nom), "init");
    y->stek = malloc(STEK_HAJM);
    getcontext(&y->ctx);
    y->ctx.uc_stack.ss_sp = y->stek;
    y->ctx.uc_stack.ss_size = STEK_HAJM;
    y->ctx.uc_link = &yadro_ctx;
    makecontext(&y->ctx, init_dastur, 0);
    y->holat = TAYYOR;

    yadro_kirish();
    printf("yadro to'xtadi: jami %ld tik\n", tik);
    return 0;
}
