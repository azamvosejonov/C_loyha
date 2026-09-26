#include "test.h"
#include "mashq.h"

int main(void)
{
    TEST_BOSHLA();
    struct halqa h;
    memset(&h, 0x7f, sizeof(h));
    char out[32];

    BOLIM("oddiy yozish va o'qish");
    halqa_init(&h);
    CHECK_INT(h.soni, 0);
    if (h.soni != 0)
        TEST_TUGADI();
    CHECK_INT(halqa_yoz(&h, "abc", 3), 3);
    memset(out, 0, sizeof(out));
    CHECK_INT(halqa_oqi(&h, out, 2), 2);
    CHECK_STR(out, "ab");
    memset(out, 0, sizeof(out));
    CHECK_INT(halqa_oqi(&h, out, 10), 1);       /* faqat 1 ta qolgan */
    CHECK_STR(out, "c");
    CHECK_INT(halqa_oqi(&h, out, 10), 0);       /* bo'sh */

    BOLIM("to'lish");
    CHECK_INT(halqa_yoz(&h, "0123456789", 10), 8);   /* faqat 8 ta sig'adi */
    CHECK_INT(halqa_yoz(&h, "x", 1), 0);
    memset(out, 0, sizeof(out));
    CHECK_INT(halqa_oqi(&h, out, 32), 8);
    CHECK_STR(out, "01234567");

    BOLIM("o'ralish (wrap-around)");
    halqa_init(&h);
    halqa_yoz(&h, "abcdef", 6);
    halqa_oqi(&h, out, 5);                      /* bosh = 5 */
    CHECK_INT(halqa_yoz(&h, "GHIJKLM", 7), 7);  /* 5,6,7 va 0,1,2,3 ga */
    memset(out, 0, sizeof(out));
    CHECK_INT(halqa_oqi(&h, out, 32), 8);
    CHECK_STR(out, "fGHIJKLM");

    BOLIM("ko'p aylanish");
    halqa_init(&h);
    int ok = 1;
    unsigned char k = 0, o = 0;
    for (int i = 0; i < 1000; i++) {
        unsigned char in[3] = { k, (unsigned char)(k + 1), (unsigned char)(k + 2) };
        size_t w = halqa_yoz(&h, in, (size_t)(i % 3) + 1);
        k = (unsigned char)(k + w);
        unsigned char got[8];
        size_t r = halqa_oqi(&h, got, (size_t)(i % 4) + 1);
        for (size_t j = 0; j < r; j++)
            ok &= got[j] == o++;
    }
    CHECK(ok);
    TEST_TUGADI();
}
