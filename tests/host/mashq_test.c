/*
 * tests/host/mashq_test.c - MyOS mashqlari (M1-M4) uchun tezkor host testi (QEMU'siz, ~1 soniya).
 *
 * Bizning libc/yadro fayllari kompyuterda kompilyatsiya qilinib, belgilariga "lab_" prefiksi qo'shiladi
 * (glibc bilan to'qnashmasligi uchun). Kutilgan natijalar uchun "hakam" - glibc'ning o'z snprintf'i:
 * standart formatlar uchun u to'g'ri javobni biladi.
 *
 *   M1  user/libc/printf.c   emit_number      sonni matnga: asos, ishora, kenglik, '0', '-', aniqlik
 *   M2  kernel/lib/kprintf.c format_core      bayroqlar, kenglik, aniqlik ("%-08.3s")
 *   M3  user/libc/malloc.c   malloc           first fit, bo'lish (split), qayta ishlatish
 *   M4  user/libc/malloc.c   insert_free      tartiblangan ro'yxat, qo'shnilar bilan birlashtirish
 *
 * Ishlatish: tools/myos_mashq.sh  (bu fayl to'g'ridan-to'g'ri emas)
 */
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int lab_snprintf(char *buf, size_t size, const char *fmt, ...);
int lab_ksnprintf(char *buf, size_t size, const char *fmt, ...);
void *lab_malloc(size_t size);
void lab_free(void *p);
void *lab_realloc(void *p, size_t size);

struct malloc_stats {                   /* user/include/myos.h dagi bilan bir xil tartib */
    size_t heap_bytes, used_bytes, free_bytes, free_blocks, used_blocks;
};
void lab_malloc_get_stats(struct malloc_stats *out);

extern jmp_buf abort_nuqta;             /* mashq_shim.c: lab_abort() shu yerga qaytadi */
extern int abort_kutilmoqda;

static const char *g_nom;
static int g_jami, g_xato;

static void boshla(const char *nom)
{
    g_nom = nom;
    g_jami = g_xato = 0;
}

static void satr_teng(const char *olindi, const char *kutilgan, const char *izoh)
{
    g_jami++;
    if (strcmp(olindi, kutilgan) != 0) {
        if (++g_xato <= 8)
            printf("        %-34s -> olindi \"%s\", kutilgan \"%s\"\n", izoh, olindi, kutilgan);
    }
}

static void rost(int shart, const char *izoh)
{
    g_jami++;
    if (!shart && ++g_xato <= 8)
        printf("        %s\n", izoh);
}

static int tugat(void)
{
    printf("  [%s] %s: %d/%d\n", g_xato ? "XATO" : " OK ", g_nom, g_jami - g_xato, g_jami);
    return g_xato != 0;
}

/* bir xil format va argumentlar bilan: bizniki (lab_) va glibc (hakam) */
#define SOLISHTIR(funk, fmt, ...)                                                   \
    do {                                                                            \
        char a_[160], b_[160];                                                      \
        funk(a_, sizeof(a_), fmt, __VA_ARGS__);                                     \
        snprintf(b_, sizeof(b_), fmt, __VA_ARGS__);                                 \
        satr_teng(a_, b_, #funk "(\"" fmt "\")");                                   \
    } while (0)

static int m1(void)
{
    boshla("M1 printf: emit_number (user/libc/printf.c)");
    SOLISHTIR(lab_snprintf, "%d", 0);
    SOLISHTIR(lab_snprintf, "%d", 42);
    SOLISHTIR(lab_snprintf, "%d", -42);
    SOLISHTIR(lab_snprintf, "%d", INT32_MIN);
    SOLISHTIR(lab_snprintf, "%u", 4294967295u);
    SOLISHTIR(lab_snprintf, "%x|%X", 0xbeefu, 0xbeefu);
    SOLISHTIR(lab_snprintf, "%o", 8u);
    SOLISHTIR(lab_snprintf, "%lu", (unsigned long)UINT64_MAX);
    SOLISHTIR(lab_snprintf, "%ld", (long)INT64_MIN);
    SOLISHTIR(lab_snprintf, "[%5d]", 42);
    SOLISHTIR(lab_snprintf, "[%-5d]", 42);
    SOLISHTIR(lab_snprintf, "[%05d]", 42);
    SOLISHTIR(lab_snprintf, "[%05d]", -42);
    SOLISHTIR(lab_snprintf, "[%-05d]", -42);
    SOLISHTIR(lab_snprintf, "[%+d|%+d]", 5, -5);
    SOLISHTIR(lab_snprintf, "[% d|% d]", 5, -5);
    SOLISHTIR(lab_snprintf, "[%.3d]", 7);
    SOLISHTIR(lab_snprintf, "[%.3d]", -7);
    SOLISHTIR(lab_snprintf, "[%8.3d]", 7);
    SOLISHTIR(lab_snprintf, "[%-8.3d]", 7);
    SOLISHTIR(lab_snprintf, "[%08.3d]", 7);
    SOLISHTIR(lab_snprintf, "[%.0d]", 0);
    SOLISHTIR(lab_snprintf, "[%5.0d]", 0);
    SOLISHTIR(lab_snprintf, "[%*d]", 6, 42);
    SOLISHTIR(lab_snprintf, "[%*d]", -6, 42);
    SOLISHTIR(lab_snprintf, "[%08x]", 0xbeefu);
    SOLISHTIR(lab_snprintf, "[%3d]", 12345);
    char p[64];
    lab_snprintf(p, sizeof(p), "%p", (void *)0x1234abcd);
    satr_teng(p, "0x1234abcd", "lab_snprintf(\"%p\")");
    return tugat();
}

static int m2(void)
{
    boshla("M2 kprintf: bayroqlar/kenglik/aniqlik (kernel/lib/kprintf.c)");
    SOLISHTIR(lab_ksnprintf, "[%d]", -42);
    SOLISHTIR(lab_ksnprintf, "[%5d]", 42);
    SOLISHTIR(lab_ksnprintf, "[%-5d]", 42);
    SOLISHTIR(lab_ksnprintf, "[%05d]", -42);
    SOLISHTIR(lab_ksnprintf, "[%-05d]", 42);
    SOLISHTIR(lab_ksnprintf, "[%08x]", 0xbeefu);
    SOLISHTIR(lab_ksnprintf, "[%12lx]", 0xffffffff80001234ul);
    SOLISHTIR(lab_ksnprintf, "[%-8s]", "abc");
    SOLISHTIR(lab_ksnprintf, "[%8s]", "abc");
    SOLISHTIR(lab_ksnprintf, "[%.4s]", "FACPXYZ");
    SOLISHTIR(lab_ksnprintf, "[%10.3s]", "salom");
    SOLISHTIR(lab_ksnprintf, "[%-10.3s]", "salom");
    SOLISHTIR(lab_ksnprintf, "[%zu|%lu]", (size_t)123456789012ul, 42ul);
    SOLISHTIR(lab_ksnprintf, "[%3d%%]", 7);
    SOLISHTIR(lab_ksnprintf, "[%15d]", 1234567890);
    return tugat();
}

static int tekis16(void *p)
{
    return ((uintptr_t)p & 15) == 0;
}

static int m3(void)
{
    boshla("M3 malloc: first fit, split, qayta ishlatish (user/libc/malloc.c)");
    void *a = lab_malloc(1);
    rost(a && tekis16(a), "malloc(1): NULL emas va 16 ga tekis");
    struct malloc_stats s;
    lab_malloc_get_stats(&s);
    rost(s.heap_bytes >= 64 * 1024, "birinchi malloc: grow() yadrodan kamida 64 KB oldi");
    rost(s.free_blocks == 1 && s.free_bytes == s.heap_bytes - 16 - 2 * 16,
         "split: 64 KB dan 16 bayt berildi, QOLGANI bitta bo'sh blok (heap - 16 - 2 sarlavha)");
    rost(lab_malloc(0) == NULL, "malloc(0) == NULL");
    rost(lab_malloc((size_t)1 << 40) == NULL, "malloc(1 TB) == NULL (juda katta)");

    /* 64 ta har xil hajmli blok: tekis, bir-birini bosmaydi */
    static const size_t hajmlar[] = { 1, 7, 16, 17, 100, 255, 1000, 4096, 5000, 33 };
    unsigned char *p[64];
    int ok = 1;
    for (int i = 0; i < 64; i++) {
        size_t n = hajmlar[i % 10];
        p[i] = lab_malloc(n);
        if (!p[i] || !tekis16(p[i]))
            ok = 0;
        else
            memset(p[i], i, n);
    }
    rost(ok, "64 ta blok: hammasi ajratildi va 16 ga tekis");
    for (int i = 0; i < 64 && ok; i++)
        for (size_t k = 0; k < hajmlar[i % 10]; k++)
            if (p[i][k] != (unsigned char)i)
                ok = 0;
    rost(ok, "bloklar bir-birini bosib ketmagan (har biri o'z naqshini saqlagan)");

    /* qayta ishlatish: bo'shatilgan blok keyingi malloc'da qaytadi */
    void *x = lab_malloc(200);
    lab_free(x);
    void *y = lab_malloc(200);
    rost(y == x, "free(x); malloc(xuddi shu hajm) -> x qaytadi (free list ishlatildi)");

    /* hisobot: used_bytes 16 ga yaxlitlangan */
    struct malloc_stats s0, s1;
    lab_malloc_get_stats(&s0);
    void *z = lab_malloc(20);
    lab_malloc_get_stats(&s1);
    rost(s1.used_bytes - s0.used_bytes == 32, "malloc(20): used_bytes 32 ga oshdi (16 ga yaxlitlash)");
    rost(s1.used_blocks - s0.used_blocks == 1, "malloc: used_blocks 1 ga oshdi");

    /* split: katta bo'sh blokdan kichik so'rov - qolgani bo'sh ro'yxatda qoladi */
    rost(s1.free_bytes > 0 && s1.free_blocks >= 1, "bo'linishdan keyin qolgan qism bo'sh ro'yxatda");

    /* band blok to'g'ri belgilangan: free() ham, realloc() ham uni taniydi */
    abort_kutilmoqda = 1;
    int abort_boldi = setjmp(abort_nuqta);
    if (!abort_boldi)
        lab_free(z);
    rost(!abort_boldi, "malloc bergan blokni free() qabul qildi (next = USED_MAGIC)");
    char *r = lab_malloc(8);
    if (r) {
        strcpy(r, "salom");
        r = lab_realloc(r, 5000);
    }
    rost(r && strcmp(r, "salom") == 0, "realloc mazmunni saqlaydi");
    abort_kutilmoqda = 0;
    return tugat();
}

static int m4(void)
{
    boshla("M4 insert_free: tartib va birlashtirish (user/libc/malloc.c)");
    struct malloc_stats s;
    void *sinov = lab_malloc(32);       /* malloc bo'sh ro'yxatdan foydalanadimi (M3 yozilganmi)? */
    lab_free(sinov);
    if (lab_malloc(32) != sinov)
        printf("        (eslatma: M4 testlari ishlaydigan malloc ga tayanadi - avval M3 ni yozing)\n");
    /* to'rt qo'shni blok; d - "to'siq": c ni heap'ning qolgan qismi bilan birlashib ketishdan saqlaydi */
    char *a = lab_malloc(64), *b = lab_malloc(64), *c = lab_malloc(64), *d = lab_malloc(64);
    rost(a && b == a + 80 && c == b + 80 && d == c + 80, "a, b, c, d ketma-ket (har biri 64 + 16 sarlavha) - M3 kerak");
    lab_malloc_get_stats(&s);
    size_t asos = s.free_blocks;        /* heap'ning qolgan qismi (odatda 1) */

    lab_free(a);
    lab_free(c);
    lab_malloc_get_stats(&s);
    rost(s.free_blocks == asos + 2, "free(a), free(c): ikkita alohida bo'sh blok (qo'shni emas)");
    void *q = lab_malloc(64);
    rost(q == a, "manzil bo'yicha tartib: free(a), free(c) dan keyin malloc(64) -> a (eng past manzil)");
    lab_free(q);

    lab_free(b);                        /* a + b + c bitta blokka birlashishi kerak */
    lab_malloc_get_stats(&s);
    rost(s.free_blocks == asos + 1, "free(b): chap (a) va o'ng (c) qo'shnilar bilan birlashdi -> 1 blok");
    void *birlashgan = lab_malloc(64 * 3 + 16 * 2);
    rost(birlashgan == a, "birlashgan blok (3*64 + 2*16 bayt) a manzilidan beriladi");
    lab_free(birlashgan);
    lab_free(d);
    lab_malloc_get_stats(&s);
    rost(s.free_blocks == asos, "hammasi bo'shatildi: d ham heap qoldig'i bilan birlashdi");

    /* katta blok: tepada bo'shatilsa, xotira yadroga (sbrk) qaytariladi */
    lab_malloc_get_stats(&s);
    size_t oldin = s.heap_bytes;
    void *katta = lab_malloc(4 * 1024 * 1024);
    lab_free(katta);
    lab_malloc_get_stats(&s);
    rost(katta && s.heap_bytes < oldin + 4 * 1024 * 1024, "4 MB free() dan keyin heap kichraydi (trim_top: tartiblangan ro'yxat kerak)");
    return tugat();
}

int main(int argc, char **argv)
{
    const char *g = argc > 1 ? argv[1] : "";
    if (!strcmp(g, "m1"))
        return m1();
    if (!strcmp(g, "m2"))
        return m2();
    if (!strcmp(g, "m3"))
        return m3();
    if (!strcmp(g, "m4"))
        return m4();
    fprintf(stderr, "ishlatish: mashq_test m1|m2|m3|m4\n");
    return 2;
}
