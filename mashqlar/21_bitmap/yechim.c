/* =============================================================================
 *  21 - Bitmap (bitlar xaritasi)                       [4-modul: yadro uslubidagi C]
 * =============================================================================
 *
 *  VAZIFA - bitmap: uint64_t massivi, i-bit = (i / 64)-so'zning (i % 64)-biti.
 *    bm_yoq(bm, i) / bm_ochir(bm, i) / bm_bormi(bm, i)
 *    bm_birinchi_nol(bm, n)          - [0, n) oralig'idagi birinchi 0-bit indeksi, yo'q -> -1
 *    bm_ketma_ket_nollar(bm, n, k)   - [0, n) da ketma-ket k ta 0-bit boshlanadigan
 *                                      eng birinchi indeks, yo'q -> -1  (k >= 1)
 *
 *  PYTHON'DA:
 *    set() yoki [False] * n - har biri uchun ~8-28 bayt. Bitmap'da - 1 BIT.
 *
 *  NEGA BU YADRO UCHUN:
 *    Qaysi fizik sahifalar bo'sh? Qaysi inode'lar band? Qaysi PID bo'sh?
 *    ext2 disk bloklari va inode'lar aynan bitmap'da saqlanadi (MyOS:
 *    kernel/fs/ext2.c), boot paytidagi xotira ham. 4 GB RAM = 1 048 576
 *    sahifa = atigi 128 KB bitmap.
 *
 *  MASLAHAT:
 *    * 04-mashqni eslang: `1ull << (i % 64)` (1 << ... emas - 32 bit!).
 *    * bm_birinchi_nol TEZ bo'lishi uchun: so'z == ~0ull (hammasi band) bo'lsa,
 *      64 bitni bittada o'tkazib yuboring. GCC'da __builtin_ctzll(x) - eng
 *      pastki 1-bitning o'rni (0 bo'lmagan x uchun). ~so'z ning pastki 1-biti = ?
 *    * n 64 ga karrali bo'lmasligi mumkin: n dan keyingi bitlarni hisobga olmang.
 *    * Ketma-ket nollar: "joriy ketma-ketlik uzunligi" hisoblagichi bilan bitma-bit
 *      yurish ham yetarli (keyin tezlashtirishni o'ylab ko'ring).
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 21
 * ============================================================================= */
#include "mashq.h"

void bm_yoq(uint64_t *bm, size_t i)
{
    /* TODO */
    (void)bm; (void)i;
}

void bm_ochir(uint64_t *bm, size_t i)
{
    /* TODO */
    (void)bm; (void)i;
}

bool bm_bormi(const uint64_t *bm, size_t i)
{
    /* TODO */
    (void)bm; (void)i;
    return false;
}

long bm_birinchi_nol(const uint64_t *bm, size_t n)
{
    /* TODO */
    (void)bm; (void)n;
    return -1;
}

long bm_ketma_ket_nollar(const uint64_t *bm, size_t n, size_t k)
{
    /* TODO */
    (void)bm; (void)n; (void)k;
    return -1;
}
