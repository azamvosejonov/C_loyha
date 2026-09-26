/* =============================================================================
 *  04 - Bitlar bilan ishlash                           [1-modul: turlar va sikllar]
 * =============================================================================
 *
 *  VAZIFA:
 *    bitlar_soni(x)             - x da nechta 1-bit bor (0b1011 -> 3)
 *    ikkining_darajasimi(x)     - x = 1, 2, 4, 8, ... mi? (0 - YO'Q)
 *    yuqoriga_tekislash(x, t)   - x dan katta yoki teng, t ga karrali eng kichik son.
 *                                 t - doim ikkining darajasi. (5, 4) -> 8, (8, 4) -> 8
 *    bitni_yoq(x, n)            - n-bitni 1 qilish (n = 0..31)
 *    bitni_ochir(x, n)          - n-bitni 0 qilish
 *    bit_bormi(x, n)            - n-bit 1 mi?
 *
 *  PYTHON'DA:
 *    bin(x).count('1'),  x & (x - 1) == 0,  x | (1 << n) ...
 *
 *  NEGA BU YADRO UCHUN ENG MUHIM MAVZULARDAN BIRI:
 *    Sahifa jadvali yozuvlari (PTE), qurilma registrlari, bayroqlar - hammasi
 *    bitlar. `addr & ~0xFFF` - sahifa boshi, `(addr + 4095) & ~4095` - sahifaga
 *    tekislash. MyOS'da: kernel/mm/ papkasi, include/myos/abi.h, drivers/ papkasi.
 *
 *  MASLAHAT:
 *    * `x & (x - 1)` - eng pastki 1-bitni o'chiradi. Nimaga foydali ekanini o'ylang.
 *    * Tekislash: `(x + t - 1) & ~(t - 1)`. Nega ishlaydi? Qog'ozda t = 8 bilan sinang.
 *    * TUZOQ: `1 << 31` - int (ishorali) uchun UB! Ishorasiz yozing: `1u << n`.
 *      Test n = 31 ni tekshiradi va sanitizer buni ushlaydi.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 04
 * ============================================================================= */
#include "mashq.h"

int bitlar_soni(uint32_t x)
{
    /* TODO */
    (void)x;
    return 0;
}

bool ikkining_darajasimi(uint64_t x)
{
    /* TODO */
    (void)x;
    return false;
}

uint64_t yuqoriga_tekislash(uint64_t x, uint64_t tekislik)
{
    /* TODO */
    (void)x; (void)tekislik;
    return 0;
}

uint32_t bitni_yoq(uint32_t x, int n)
{
    /* TODO */
    (void)n;
    return x;
}

uint32_t bitni_ochir(uint32_t x, int n)
{
    /* TODO */
    (void)n;
    return x;
}

bool bit_bormi(uint32_t x, int n)
{
    /* TODO */
    (void)x; (void)n;
    return false;
}
