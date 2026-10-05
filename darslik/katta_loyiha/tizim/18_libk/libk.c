/* libk.c - hech qanday libc funksiyasini chaqirmaydi: yadro ichida ham ishlay oladi */
#include "libk.h"

void *kmemset(void *dst, int bayt, size_t n)
{
    uint8_t *d = dst;
    while (n--)
        *d++ = (uint8_t)bayt;
    return dst;
}

void *kmemcpy(void *dst, const void *src, size_t n)
{
    uint8_t *d = dst;
    const uint8_t *s = src;
    while (n--)
        *d++ = *s++;
    return dst;
}

void *kmemmove(void *dst, const void *src, size_t n)
{
    uint8_t *d = dst;
    const uint8_t *s = src;
    if (d < s) {
        while (n--)                             /* oldinga ko'chirish: dst manba oldida */
            *d++ = *s++;
    } else if (d > s) {
        d += n;
        s += n;
        while (n--)                             /* ORQAGA ko'chirish: dst manba orqasida bo'lsa, oxiridan boshlaymiz */
            *--d = *--s;
    }
    return dst;
}

int kmemcmp(const void *a, const void *b, size_t n)
{
    const uint8_t *x = a, *y = b;
    for (; n; n--, x++, y++)
        if (*x != *y)
            return *x < *y ? -1 : 1;
    return 0;
}

size_t kstrlen(const char *s)
{
    size_t n = 0;
    while (s[n])
        n++;
    return n;
}

int kstrcmp(const char *a, const char *b)
{
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return (uint8_t)*a < (uint8_t)*b ? -1 : (uint8_t)*a > (uint8_t)*b;
}

size_t kstrlcpy(char *dst, const char *src, size_t hajm)
{
    size_t uz = kstrlen(src);
    if (hajm) {
        size_t k = uz < hajm - 1 ? uz : hajm - 1;
        kmemcpy(dst, src, k);
        dst[k] = '\0';                          /* '\0' DOIM qo'yiladi */
    }
    return uz;                                  /* qirqilganini chaqiruvchi (uz >= hajm) bilib oladi */
}

char *kutoa(uint64_t son, char *bufer, int asos)
{
    static const char raqamlar[] = "0123456789abcdef";
    char teskari[65];
    int n = 0;
    if (asos < 2 || asos > 16)
        asos = 10;
    do {
        teskari[n++] = raqamlar[son % (uint64_t)asos];      /* eng past raqam birinchi chiqadi */
        son /= (uint64_t)asos;
    } while (son);
    int i = 0;
    while (n)
        bufer[i++] = teskari[--n];              /* teskari tartibda yozamiz */
    bufer[i] = '\0';
    return bufer;
}

void kbit_yoq(uint8_t *b, size_t i)
{
    b[i / 8] |= (uint8_t)(1u << (i % 8));
}

void kbit_och(uint8_t *b, size_t i)
{
    b[i / 8] &= (uint8_t)~(1u << (i % 8));
}

int kbit_bor(const uint8_t *b, size_t i)
{
    return (b[i / 8] >> (i % 8)) & 1;
}

long kbit_bosh_top(const uint8_t *b, size_t n)
{
    for (size_t i = 0; i < n; i++)
        if (!kbit_bor(b, i))
            return (long)i;
    return -1;
}

void kring_boshla(struct kring *r, uint8_t *xotira, size_t sigim)
{
    r->ma_lumot = xotira;
    r->sigim = sigim;
    r->bosh = r->oxir = 0;
}

size_t kring_band(const struct kring *r)
{
    return r->oxir - r->bosh;                   /* ayirma toshsa ham (ishorasiz) to'g'ri qoladi */
}

int kring_yoz(struct kring *r, uint8_t bayt)
{
    if (kring_band(r) == r->sigim)
        return -1;
    r->ma_lumot[r->oxir & (r->sigim - 1)] = bayt;   /* & (sigim-1): 2 ning darajasida % o'rniga */
    r->oxir++;
    return 0;
}

int kring_oqi(struct kring *r, uint8_t *bayt)
{
    if (kring_band(r) == 0)
        return -1;
    *bayt = r->ma_lumot[r->bosh & (r->sigim - 1)];
    r->bosh++;
    return 0;
}
