/* =============================================================================
 *  09_struct.c - struct, tekislash, packed, union, enum      (darslik 9-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 09_struct.c -o struct && ./struct
 *
 *  Kutilgan natija:
 *      struct a: sizeof = 12, c @ 0, i @ 4, d @ 8
 *      struct b (tartiblangan): sizeof = 8
 *      packed MBR yozuvi: sizeof = 16, lba_boshi @ 8
 *      talaba: Ali, 20 yosh, 87.5 ball -> nusxada ball 90.0, aslida 87.5
 *      union: u32 = 0x11223344, bayt[0] = 0x44 (kichik bayt oldin)
 *      holat: ISHLAYAPTI (1)
 *
 *  Sinab ko'ring:
 *      1) struct a ning boshiga `double z;` qo'shing. sizeof va offset'larni AVVAL qog'ozda
 *         hisoblang, keyin tekshiring.
 *      2) mbr_yozuv dan `__attribute__((packed))` ni o'chiring - hech narsa o'zgarmaydi. Nega?
 *         (Maydonlar allaqachon tabiiy tekislangan.) Endi `tur` ni uint16_t qiling -
 *         _Static_assert xatosini o'qing.
 *      3) `q.bayt[3] = 0xff;` qo'shib, q.u32 ni chop eting. Natijani oldindan ayting.
 *      4) `enum holat { TAYYOR = 5, ... }` qiling. UXLAYAPTI endi nechaga teng?
 * ============================================================================= */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct a { char c; int i; char d; };
struct b { int i; char c; char d; };

struct __attribute__((packed)) mbr_yozuv {
    uint8_t holat;
    uint8_t chs_boshi[3];
    uint8_t tur;
    uint8_t chs_oxiri[3];
    uint32_t lba_boshi;
    uint32_t sektorlar;
};
_Static_assert(sizeof(struct mbr_yozuv) == 16, "MBR yozuvi 16 bayt bo'lishi kerak");

struct talaba {
    char ism[32];
    int yosh;
    double ball;
};                                  /* ; SHART */

union qiymat {
    uint32_t u32;
    uint8_t bayt[4];
};

enum holat { TAYYOR, ISHLAYAPTI, UXLAYAPTI };

int main(void)
{
    printf("struct a: sizeof = %zu, c @ %zu, i @ %zu, d @ %zu\n", sizeof(struct a),
           offsetof(struct a, c), offsetof(struct a, i), offsetof(struct a, d));
    printf("struct b (tartiblangan): sizeof = %zu\n", sizeof(struct b));
    printf("packed MBR yozuvi: sizeof = %zu, lba_boshi @ %zu\n", sizeof(struct mbr_yozuv),
           offsetof(struct mbr_yozuv, lba_boshi));

    struct talaba t = { "Ali", 20, 87.5 };
    struct talaba nusxa = t;        /* struct = bilan TO'LIQ nusxalanadi */
    nusxa.ball = 90.0;
    struct talaba *p = &t;
    printf("talaba: %s, %d yosh, %.1f ball -> nusxada ball %.1f, aslida %.1f\n",
           p->ism, p->yosh, p->ball, nusxa.ball, t.ball);

    union qiymat q;
    q.u32 = 0x11223344;
    printf("union: u32 = 0x%x, bayt[0] = 0x%x (kichik bayt oldin)\n", q.u32, q.bayt[0]);

    enum holat h = ISHLAYAPTI;
    printf("holat: %s (%d)\n", h == ISHLAYAPTI ? "ISHLAYAPTI" : "boshqa", h);
    return 0;
}
