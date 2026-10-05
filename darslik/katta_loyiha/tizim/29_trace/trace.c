/* trace.c - LD_PRELOAD kutubxonasi: dasturning malloc/free chaqiruvlarini KOD O'ZGARTIRMASDAN kuzatadi va oxirida sizib chiqishni ko'rsatadi */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAKS 4096

struct yozuv {
    void *p;                                    /* berilgan ko'rsatkich */
    size_t hajm;
    void *chaqiruvchi;                          /* malloc dan qaytish manzili: kim chaqirgan (shu funksiya ichida) */
    int tirik;
};

static struct yozuv jadval[MAKS];
static unsigned long malloc_soni, free_soni, jami_bayt, hozir_bayt, eng_katta_bayt;
static pthread_mutex_t qulf = PTHREAD_MUTEX_INITIALIZER;

static void *(*haqiqiy_malloc)(size_t);
static void (*haqiqiy_free)(void *);
static void *(*haqiqiy_realloc)(void *, size_t);
static void *(*haqiqiy_calloc)(size_t, size_t);
static __thread int ichkarida;                  /* qayta kirishdan himoya: printf/dlsym o'zi malloc chaqirsa, uni KUZATMAYMIZ */
static char boshlangich[4096];                  /* dlsym o'zi calloc chaqirganda beriladigan vaqtinchalik xotira */
static size_t boshlangich_ishlatildi;
static int tayyor;

static void yukla(void)
{
    haqiqiy_malloc = dlsym(RTLD_NEXT, "malloc");        /* RTLD_NEXT: bizdan KEYINGI (libc dagi) malloc */
    haqiqiy_free = dlsym(RTLD_NEXT, "free");
    haqiqiy_realloc = dlsym(RTLD_NEXT, "realloc");
    haqiqiy_calloc = dlsym(RTLD_NEXT, "calloc");
    tayyor = 1;
}

static void yoz(void *p, size_t hajm, void *chaqiruvchi)
{
    pthread_mutex_lock(&qulf);
    for (int i = 0; i < MAKS; i++)
        if (!jadval[i].tirik) {
            jadval[i] = (struct yozuv){ p, hajm, chaqiruvchi, 1 };
            break;
        }
    malloc_soni++;
    jami_bayt += hajm;
    hozir_bayt += hajm;
    if (hozir_bayt > eng_katta_bayt)
        eng_katta_bayt = hozir_bayt;
    pthread_mutex_unlock(&qulf);
}

static void ochir(void *p)
{
    pthread_mutex_lock(&qulf);
    for (int i = 0; i < MAKS; i++)
        if (jadval[i].tirik && jadval[i].p == p) {
            jadval[i].tirik = 0;
            hozir_bayt -= jadval[i].hajm;
            break;
        }
    free_soni++;
    pthread_mutex_unlock(&qulf);
}

void *malloc(size_t n)
{
    if (!tayyor)
        yukla();
    void *p = haqiqiy_malloc(n);
    if (p && !ichkarida) {
        ichkarida = 1;
        yoz(p, n, __builtin_return_address(0));
        ichkarida = 0;
    }
    return p;
}

void *calloc(size_t a, size_t b)
{
    if (!tayyor) {                              /* dlsym ichidan chaqirilgan bo'lishi mumkin: vaqtinchalik xotira beramiz */
        size_t kerak = a * b;
        if (boshlangich_ishlatildi + kerak > sizeof(boshlangich))
            return NULL;
        void *p = boshlangich + boshlangich_ishlatildi;
        boshlangich_ishlatildi += (kerak + 15) & ~(size_t)15;
        return p;                               /* statik massiv allaqachon nollangan */
    }
    void *p = haqiqiy_calloc(a, b);
    if (p && !ichkarida) {
        ichkarida = 1;
        yoz(p, a * b, __builtin_return_address(0));
        ichkarida = 0;
    }
    return p;
}

void *realloc(void *eski, size_t n)
{
    if (!tayyor)
        yukla();
    if ((char *)eski >= boshlangich && (char *)eski < boshlangich + sizeof(boshlangich))
        eski = NULL;                            /* vaqtinchalik xotirani realloc qilib bo'lmaydi */
    void *p = haqiqiy_realloc(eski, n);
    if (p && !ichkarida) {
        ichkarida = 1;
        if (eski)
            ochir(eski);
        yoz(p, n, __builtin_return_address(0));
        ichkarida = 0;
    }
    return p;
}

void free(void *p)
{
    if (!tayyor)
        yukla();
    if (!p || ((char *)p >= boshlangich && (char *)p < boshlangich + sizeof(boshlangich)))
        return;
    if (!ichkarida) {
        ichkarida = 1;
        ochir(p);
        ichkarida = 0;
    }
    haqiqiy_free(p);
}

static int solishtir(const void *a, const void *b)
{
    const struct yozuv *x = a, *y = b;
    if (x->chaqiruvchi != y->chaqiruvchi)
        return x->chaqiruvchi < y->chaqiruvchi ? -1 : 1;
    return x->hajm < y->hajm ? -1 : x->hajm > y->hajm;
}

/* dastur tugaganda avtomatik chaqiriladi (destructor) */
__attribute__((destructor)) static void hisobot(void)
{
    ichkarida = 1;                              /* hisobot chiqarishda printf ning malloc'i kuzatilmasin */
    fprintf(stderr, "\n=== [trace] malloc hisoboti ===\n");
    fprintf(stderr, "ajratishlar: %lu, bo'shatishlar: %lu, jami: %lu bayt, eng ko'pi bir vaqtda: %lu bayt\n", malloc_soni,
            free_soni, jami_bayt, eng_katta_bayt);

    static struct yozuv sizganlar[MAKS];
    int k = 0;
    for (int i = 0; i < MAKS; i++)
        if (jadval[i].tirik)
            sizganlar[k++] = jadval[i];
    qsort(sizganlar, (size_t)k, sizeof(sizganlar[0]), solishtir);

    unsigned long jami = 0, jami_baytlar = 0;
    for (int i = 0; i < k; i++) {
        Dl_info info;
        int bor = dladdr(sizganlar[i].chaqiruvchi, &info);
        if (bor && info.dli_fname && strstr(info.dli_fname, "libc.so"))
            continue;                           /* libc ning o'z ichki xotirasi (masalan stdout buferi): dastur xatosi emas */
        const char *kim = bor && info.dli_sname ? info.dli_sname : "?";
        fprintf(stderr, "  SIZIB CHIQDI: %5zu bayt, ajratgan funksiya: %s()\n", sizganlar[i].hajm, kim);
        jami++, jami_baytlar += sizganlar[i].hajm;
    }
    if (jami == 0)
        fprintf(stderr, "  sizib chiqish yo'q.\n");
    else
        fprintf(stderr, "  JAMI: %lu ta blok, %lu bayt sizib chiqdi\n", jami, jami_baytlar);
}
