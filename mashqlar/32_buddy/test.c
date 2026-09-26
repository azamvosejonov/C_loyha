#include "test.h"
#include "mashq.h"

static unsigned char band[MAKS_SAHIFA];     /* testning o'z hisobi: qaysi sahifa berilgan */

static int belgila(long s, int k, unsigned char v)
{
    if (s < 0 || s + (1L << k) > MAKS_SAHIFA)
        return 0;
    for (long i = s; i < s + (1L << k); i++) {
        if (band[i] == v)
            return 0;               /* ikki marta berildi yoki berilmaganni qaytardi */
        band[i] = v;
    }
    return 1;
}

static unsigned long rng = 7;
static unsigned tasodif(unsigned n)
{
    rng = rng * 6364136223846793005UL + 1442695040888963407UL;
    return (unsigned)(rng >> 33) % n;
}

int main(void)
{
    TEST_BOSHLA();
    BOLIM("buddy_init(1024)");
    buddy_init(1024);
    CHECK_INT(buddy_bosh(), 1024);

    BOLIM("oddiy ajratish: tekislangan va bir-biriga tegmaydi");
    long a = buddy_ajrat(0), b = buddy_ajrat(0), c = buddy_ajrat(3), d = buddy_ajrat(9);
    CHECK(a >= 0 && b >= 0 && c >= 0 && d >= 0);
    if (a < 0 || b < 0 || c < 0 || d < 0)
        TEST_TUGADI();
    CHECK_INT(c % 8, 0);
    CHECK_INT(d % 512, 0);
    CHECK(belgila(a, 0, 1) && belgila(b, 0, 1) && belgila(c, 3, 1) && belgila(d, 9, 1));
    CHECK_INT(buddy_bosh(), 1024 - 1 - 1 - 8 - 512);
    CHECK_INT(buddy_ajrat(10), -1);                 /* 1024 li blok endi yo'q */

    BOLIM("ozod qilish va birlashtirish");
    buddy_ozod(a, 0); belgila(a, 0, 0);
    buddy_ozod(b, 0); belgila(b, 0, 0);
    buddy_ozod(c, 3); belgila(c, 3, 0);
    buddy_ozod(d, 9); belgila(d, 9, 0);
    CHECK_INT(buddy_bosh(), 1024);
    long e = buddy_ajrat(10);                       /* hammasi birlashgan bo'lsa - bor */
    CHECK_INT(e, 0);
    buddy_ozod(e, 10);

    BOLIM("1024 ga karrali bo'lmagan hajm: 1000 sahifa");
    buddy_init(1000);
    CHECK_INT(buddy_bosh(), 1000);
    CHECK_INT(buddy_ajrat(10), -1);
    long f = buddy_ajrat(9);
    CHECK_INT(f, 0);                                /* 0..511 - yagona 512 li blok */
    CHECK_INT(buddy_ajrat(9), -1);                  /* 512..999 da 512 li blok sig'maydi */
    long g = buddy_ajrat(8);
    CHECK_INT(g, 512);
    int ok = 1;
    for (int i = 0; i < 232; i++) {                 /* qolgan 232 sahifa bittalab */
        long p = buddy_ajrat(0);
        ok &= p >= 768 && p < 1000;
    }
    CHECK(ok);
    CHECK_INT(buddy_ajrat(0), -1);
    CHECK_INT(buddy_bosh(), 0);

    BOLIM("tasodifiy stress: 20000 amal");
    buddy_init(4096);
    memset(band, 0, sizeof(band));
    long bloklar[512];
    int tartiblar[512], n = 0;
    ok = 1;
    size_t band_soni = 0;
    for (int r = 0; r < 20000; r++) {
        if (n < 512 && (n == 0 || tasodif(3))) {
            int k = (int)tasodif(6);
            long p = buddy_ajrat(k);
            if (p >= 0) {
                ok &= p % (1L << k) == 0 && belgila(p, k, 1);
                bloklar[n] = p;
                tartiblar[n++] = k;
                band_soni += 1u << k;
            }
        } else {
            int i = (int)tasodif((unsigned)n);
            buddy_ozod(bloklar[i], tartiblar[i]);
            belgila(bloklar[i], tartiblar[i], 0);
            band_soni -= 1u << tartiblar[i];
            bloklar[i] = bloklar[--n];
            tartiblar[i] = tartiblar[n];
        }
        if (buddy_bosh() != 4096 - band_soni)
            ok = 0;
    }
    CHECK(ok);
    while (n > 0) {
        n--;
        buddy_ozod(bloklar[n], tartiblar[n]);
    }
    CHECK_INT(buddy_bosh(), 4096);
    ok = 1;
    for (int i = 0; i < 4; i++)
        ok &= buddy_ajrat(10) >= 0;                 /* to'liq birlashgan: 4 ta 1024 li blok */
    CHECK(ok);
    TEST_TUGADI();
}
