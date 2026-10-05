#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "ikkilik.h"

/* ---- baytlar <-> sonlar: SILJITISH va MASKA bilan. struct ni to'g'ridan-to'g'ri yozmaymiz: u protsessor tartibiga va to'ldirishga (padding) bog'liq ---- */
static void yoz16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFF);                 /* past bayt birinchi */
    p[1] = (uint8_t)(v >> 8);
}

static void yoz32(uint8_t *p, uint32_t v)
{
    for (int i = 0; i < 4; i++)
        p[i] = (uint8_t)(v >> (8 * i));
}

static void yoz64(uint8_t *p, uint64_t v)
{
    for (int i = 0; i < 8; i++)
        p[i] = (uint8_t)(v >> (8 * i));
}

static uint16_t oqi16(const uint8_t *p)
{
    return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t oqi32(const uint8_t *p)
{
    uint32_t v = 0;
    for (int i = 3; i >= 0; i--)
        v = (v << 8) | p[i];                    /* yuqori bayt oxirida: teskari tartibda yig'amiz */
    return v;
}

static uint64_t oqi64(const uint8_t *p)
{
    uint64_t v = 0;
    for (int i = 7; i >= 0; i--)
        v = (v << 8) | p[i];
    return v;
}

uint16_t nazorat16(const uint8_t *b, size_t uzunlik)
{
    uint32_t s1 = 0, s2 = 0;
    for (size_t i = 0; i < uzunlik; i++) {
        s1 = (s1 + b[i]) % 255;
        s2 = (s2 + s1) % 255;
    }
    return (uint16_t)((s2 << 8) | s1);
}

void yozuv_yoz(const struct xodim *x, uint8_t c[IKKILIK_YOZUV])
{
    memset(c, 0, IKKILIK_YOZUV);                /* zaxira va ism oxiri nollar bilan to'lgan bo'lsin: fayl har safar bir xil */
    yoz16(c + 0, (uint16_t)x->id);
    c[2] = (uint8_t)x->toifa;
    c[3] = x->bayroq;
    yoz64(c + 4, (uint64_t)x->tarif);
    yoz32(c + 12, (uint32_t)x->oddiy_daq);
    yoz32(c + 16, (uint32_t)x->qosh_daq);
    memcpy(c + 20, x->ism, ISM_UZ);
    yoz16(c + 44, nazorat16(c, 44));
}

int yozuv_oqi(const uint8_t c[IKKILIK_YOZUV], struct xodim *x, const char **sabab)
{
    if (oqi16(c + 44) != nazorat16(c, 44)) {
        *sabab = "nazorat yig'indisi mos emas (yozuv buzilgan)";
        return -1;
    }
    memset(x, 0, sizeof(*x));
    x->id = oqi16(c + 0);
    x->toifa = (enum toifa)c[2];
    x->bayroq = c[3];
    x->tarif = (int64_t)oqi64(c + 4);
    x->oddiy_daq = (int)oqi32(c + 12);
    x->qosh_daq = (int)oqi32(c + 16);
    memcpy(x->ism, c + 20, ISM_UZ);
    x->ism[ISM_UZ - 1] = '\0';                  /* faylga ishonmaymiz: '\0' kafolatlanmagan */
    if (x->id < 1000 || x->id > 9999 || x->toifa >= T_SONI || x->tarif <= 0 || x->tarif > TARIF_MAKS ||
        x->oddiy_daq < 0 || x->qosh_daq < 0 || x->oddiy_daq + x->qosh_daq > OY_MAKS_DAQ || (x->bayroq & ~F_HAMMASI)) {
        *sabab = "maydon qiymati yaroqsiz";
        return -1;
    }
    return 0;
}

int ikkilik_saqla(const struct ombor *o, const char *yol)
{
    size_t n = ombor_soni(o);
    if (n > 0xFFFF) {
        errno = EFBIG;
        return -1;
    }
    size_t hajm = IKKILIK_SARLAVHA + n * IKKILIK_YOZUV;
    uint8_t *b = calloc(1, hajm);
    if (!b)
        return -1;
    memcpy(b, IKKILIK_SEHR, 4);
    yoz16(b + 4, 1);                            /* versiya */
    yoz16(b + 6, (uint16_t)n);
    for (size_t i = 0; i < n; i++)
        yozuv_yoz(ombor_ol(o, i), b + IKKILIK_SARLAVHA + i * IKKILIK_YOZUV);

    FILE *f = fopen(yol, "wbx");                /* "x" - fayl bor bo'lsa ochmaydi (O_EXCL) */
    int xato = 0;
    if (!f) {
        xato = 1;
    } else {
        if (fwrite(b, 1, hajm, f) != hajm)
            xato = 1;
        if (fclose(f) != 0)
            xato = 1;
    }
    int e = errno;
    free(b);
    errno = e;
    return xato ? -1 : 0;
}

int ikkilik_yukla(struct ombor *o, const char *yol, struct yuklash *h)
{
    FILE *f = fopen(yol, "rb");
    if (!f) {
        fprintf(stderr, "%s: ochilmadi: %s\n", yol, strerror(errno));
        return -1;
    }
    uint8_t sar[IKKILIK_SARLAVHA];
    if (fread(sar, 1, sizeof(sar), f) != sizeof(sar) || memcmp(sar, IKKILIK_SEHR, 4) != 0) {
        fprintf(stderr, "%s: bu KDR1 fayli emas (sehrli baytlar noto'g'ri)\n", yol);
        fclose(f);
        return -1;
    }
    if (oqi16(sar + 4) != 1) {
        fprintf(stderr, "%s: versiya %u qo'llab-quvvatlanmaydi\n", yol, oqi16(sar + 4));
        fclose(f);
        return -1;
    }
    unsigned soni = oqi16(sar + 6);
    uint8_t c[IKKILIK_YOZUV];
    for (unsigned i = 0; i < soni; i++) {
        if (fread(c, 1, sizeof(c), f) != sizeof(c)) {
            fprintf(stderr, "%s: fayl qisqa: %u ta yozuv e'lon qilingan, %u tasi o'qildi\n", yol, soni, i);
            h->rad += (int)(soni - i);
            break;
        }
        struct xodim x;
        const char *sabab;
        if (yozuv_oqi(c, &x, &sabab) != 0) {
            fprintf(stderr, "%s: %u-yozuv: %s\n", yol, i + 1, sabab);
            h->rad++;
        } else if (ombor_qosh(o, &x) != 0) {
            fprintf(stderr, "%s: %u-yozuv: ID takrori yoki xotira yetmadi\n", yol, i + 1);
            h->rad++;
        } else {
            h->qabul++;
        }
    }
    fclose(f);
    return 0;
}

int hex_chiqar(const char *yol, size_t n)
{
    FILE *f = fopen(yol, "rb");
    if (!f)
        return -1;
    uint8_t b[16];
    for (size_t siljish = 0; siljish < n;) {
        size_t k = fread(b, 1, sizeof(b), f);
        if (k == 0)
            break;
        printf("%04zx  ", siljish);
        for (size_t i = 0; i < 16; i++) {
            if (i < k)
                printf("%02x ", b[i]);          /* %02x: 2 xonali hex, bosh nol bilan */
            else
                printf("   ");
            if (i == 7)
                putchar(' ');
        }
        printf(" |");
        for (size_t i = 0; i < k; i++)
            putchar(b[i] >= 32 && b[i] < 127 ? b[i] : '.');
        printf("|\n");
        siljish += k;
    }
    fclose(f);
    return 0;
}
