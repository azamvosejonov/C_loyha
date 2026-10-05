/* ip_paket.c - IPv4 paket dekoderi: baytlardan maydonlarni ajratish, bayt tartibi, bit maskalar, nazorat yig'indisi */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* tarmoqda ko'p baytli sonlar BIG-endian: katta bayt birinchi. Xotiradan (little-endian x86) to'g'ridan-to'g'ri o'qib bo'lmaydi */
static uint16_t o16(const uint8_t *p)
{
    return (uint16_t)((p[0] << 8) | p[1]);      /* katta bayt chapga 8 bitga suriladi */
}

static uint32_t o32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

/* Internet nazorat yig'indisi (RFC 1071): 16 bitli so'zlar yig'indisi, ortiqcha bitlar qo'shib qo'yiladi, so'ng inversiya.
   To'g'ri sarlavhada (nazorat maydoni bilan birga) natija 0 bo'lishi kerak. */
static uint16_t nazorat(const uint8_t *p, size_t uzunlik)
{
    uint32_t s = 0;
    for (size_t i = 0; i + 1 < uzunlik; i += 2)
        s += o16(p + i);
    if (uzunlik & 1)
        s += (uint32_t)p[uzunlik - 1] << 8;     /* toq bayt: o'ng tomoni nol bilan to'ldiriladi */
    while (s >> 16)
        s = (s & 0xFFFF) + (s >> 16);           /* 16 bitdan oshgan qismini pastga qo'shamiz */
    return (uint16_t)~s;
}

static const char *protokol_nomi(uint8_t p)
{
    switch (p) {
    case 1: return "ICMP";
    case 6: return "TCP";
    case 17: return "UDP";
    default: return "?";
    }
}

static void manzil(const char *nom, uint32_t a)
{
    printf("  %-14s %u.%u.%u.%u\n", nom, a >> 24, (a >> 16) & 0xFF, (a >> 8) & 0xFF, a & 0xFF);
}

/* hex matnni baytlarga aylantiradi. Baytlar sonini yoki -1 ni qaytaradi */
static int hex_oqi(const char *s, uint8_t *chiq, int sigim)
{
    int n = 0;
    while (s[0] && s[1] && s[0] != '\n') {
        unsigned v;
        if (n == sigim || sscanf(s, "%2x", &v) != 1)
            return -1;
        chiq[n++] = (uint8_t)v;
        s += 2;
    }
    return n;
}

static void tcp_chop(const uint8_t *t)
{
    uint8_t bayroq = t[13];                     /* 6 ta bayroq: pastki 6 bit */
    printf("  TCP: %u -> %u, ketma-ketlik raqami %u, bayroqlar:", o16(t), o16(t + 2), o32(t + 4));
    static const char *nomlar[6] = { "FIN", "SYN", "RST", "PSH", "ACK", "URG" };
    int bor = 0;
    for (int bit = 0; bit < 6; bit++)
        if (bayroq & (1u << bit)) {             /* har bit - bitta bayroq (3-bob maskalari) */
            printf(" %s", nomlar[bit]);
            bor = 1;
        }
    printf("%s, oyna %u\n", bor ? "" : " yo'q", o16(t + 14));
}

static void udp_chop(const uint8_t *u, int mavjud)
{
    unsigned uz = o16(u + 4);
    printf("  UDP: %u -> %u, uzunlik %u, ma'lumot: \"", o16(u), o16(u + 2), uz);
    for (int i = 8; i < (int)uz && i < mavjud; i++)
        putchar(u[i] >= 32 && u[i] < 127 ? u[i] : '.');
    printf("\"\n");
}

static void paket_tahlil(int raqam, const uint8_t *p, int n)
{
    printf("Paket %d (%d bayt):\n", raqam, n);
    if (n < 20 || (p[0] >> 4) != 4) {
        printf("  XATO: IPv4 sarlavhasi emas\n");
        return;
    }
    int ihl = (p[0] & 0x0F) * 4;                /* sarlavha uzunligi 4 baytli so'zlarda */
    unsigned jami = o16(p + 2);
    unsigned bayroq_siljish = o16(p + 6);       /* yuqori 3 bit - bayroqlar, pastki 13 bit - siljish */

    printf("  versiya %u, sarlavha %d bayt, umumiy uzunlik %u\n", p[0] >> 4, ihl, jami);
    int df = (bayroq_siljish & 0x4000) != 0, mf = (bayroq_siljish & 0x2000) != 0;
    printf("  id 0x%04X, bayroqlar:%s%s%s, bo'lak siljishi %u\n", o16(p + 4), df ? " DF" : "", mf ? " MF" : "",
           (df || mf) ? "" : " yo'q", bayroq_siljish & 0x1FFF);
    printf("  TTL %u, protokol %u (%s)\n", p[8], p[9], protokol_nomi(p[9]));
    manzil("jo'natuvchi", o32(p + 12));
    manzil("qabul qiluvchi", o32(p + 16));

    uint16_t y = nazorat(p, (size_t)ihl);
    printf("  nazorat yig'indisi: 0x%04X -> %s\n", o16(p + 10), y == 0 ? "TO'G'RI" : "NOTO'G'RI (paket buzilgan)");
    if (y != 0)
        return;                                 /* buzilgan paketning ichiga ishonib bo'lmaydi */

    if (p[9] == 6 && n >= ihl + 20)
        tcp_chop(p + ihl);
    else if (p[9] == 17 && n >= ihl + 8)
        udp_chop(p + ihl, n - ihl);
}

int main(void)
{
    char qator[512];
    int raqam = 0;
    while (fgets(qator, sizeof(qator), stdin)) {
        uint8_t paket[256];
        int n = hex_oqi(qator, paket, sizeof(paket));
        if (n < 0) {
            printf("Paket %d: XATO: hex matn noto'g'ri\n", ++raqam);
            continue;
        }
        paket_tahlil(++raqam, paket, n);
    }
    return 0;
}
