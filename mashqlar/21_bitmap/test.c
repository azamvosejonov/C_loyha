#include "test.h"
#include "mashq.h"

int main(void)
{
    TEST_BOSHLA();
    size_t n = 200;
    uint64_t *bm = calloc(BITMAP_SOZLAR(n), sizeof(uint64_t));   /* aniq 4 so'z */

    BOLIM("yoqish / tekshirish / o'chirish");
    bm_yoq(bm, 0);
    bm_yoq(bm, 63);
    bm_yoq(bm, 64);
    bm_yoq(bm, 199);
    CHECK(bm_bormi(bm, 0) && bm_bormi(bm, 63) && bm_bormi(bm, 64) && bm_bormi(bm, 199));
    CHECK(!bm_bormi(bm, 1) && !bm_bormi(bm, 62) && !bm_bormi(bm, 65));
    CHECK_HEX(bm[0], 0x8000000000000001ull);
    CHECK_HEX(bm[1], 1);
    bm_ochir(bm, 63);
    CHECK(!bm_bormi(bm, 63));
    CHECK_HEX(bm[0], 1);

    BOLIM("bm_birinchi_nol");
    memset(bm, 0, BITMAP_SOZLAR(n) * 8);
    CHECK_INT(bm_birinchi_nol(bm, n), 0);
    for (size_t i = 0; i < 130; i++)
        bm_yoq(bm, i);
    CHECK_INT(bm_birinchi_nol(bm, n), 130);
    for (size_t i = 130; i < n; i++)
        bm_yoq(bm, i);
    CHECK_INT(bm_birinchi_nol(bm, n), -1);      /* hammasi band */
    bm_ochir(bm, 199);
    CHECK_INT(bm_birinchi_nol(bm, n), 199);
    CHECK_INT(bm_birinchi_nol(bm, 199), -1);    /* n = 199: 199-bit hisobga olinmaydi */

    BOLIM("bm_ketma_ket_nollar");
    memset(bm, 0xff, BITMAP_SOZLAR(n) * 8);     /* hammasi band */
    for (size_t i = 10; i < 13; i++)
        bm_ochir(bm, i);                        /* 3 ta bo'sh: 10..12 */
    for (size_t i = 60; i < 70; i++)
        bm_ochir(bm, i);                        /* 10 ta bo'sh: 60..69 (so'z chegarasidan o'tadi) */
    for (size_t i = 190; i < 200; i++)
        bm_ochir(bm, i);                        /* oxirida 10 ta */
    CHECK_INT(bm_ketma_ket_nollar(bm, n, 1), 10);
    CHECK_INT(bm_ketma_ket_nollar(bm, n, 3), 10);
    CHECK_INT(bm_ketma_ket_nollar(bm, n, 4), 60);
    CHECK_INT(bm_ketma_ket_nollar(bm, n, 10), 60);
    CHECK_INT(bm_ketma_ket_nollar(bm, n, 11), -1);
    CHECK_INT(bm_ketma_ket_nollar(bm, 195, 10), 60);
    CHECK_INT(bm_ketma_ket_nollar(bm, 199, 10), 60);
    bm_yoq(bm, 65);
    CHECK_INT(bm_ketma_ket_nollar(bm, n, 10), 190);
    CHECK_INT(bm_ketma_ket_nollar(bm, 199, 10), -1);   /* 190..198 - faqat 9 ta */
    free(bm);
    TEST_TUGADI();
}
