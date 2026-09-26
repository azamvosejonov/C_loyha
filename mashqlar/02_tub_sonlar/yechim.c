/* =============================================================================
 *  02 - Tub sonlar                                     [1-modul: turlar va sikllar]
 * =============================================================================
 *
 *  VAZIFA:
 *    tubmi(n)       - n tub sonmi? (0 va 1 - tub EMAS, 2 - tub)
 *    tublar_soni(n) - 2 dan n gacha (n ham kiradi) nechta tub son bor.
 *
 *  PYTHON'DA:
 *    def tubmi(n): return n >= 2 and all(n % d for d in range(2, isqrt(n) + 1))
 *
 *  C'DA NIMA BOSHQA:
 *    * bool - <stdbool.h> dan: true/false (aslida 1/0).
 *    * unsigned - ishorasiz: manfiy bo'lmaydi, lekin 0 dan pastga tushsa
 *      eng katta songa AYLANADI (0u - 1 = 4294967295). Ehtiyot bo'ling.
 *
 *  MASLAHAT:
 *    * Faqat sqrt(n) gacha tekshirish yetarli. `d * d <= n` sharti bilan -
 *      lekin n = 4294967291 da d * d unsigned'ga sig'maydi va 0 ga "aylanib"
 *      qolishi mumkin! `d <= n / d` - toshmaydigan variant.
 *    * tublar_soni(100000) tez bo'lishi uchun: Eratosfen g'alviri - bayroqlar
 *      massivi (malloc bilan yoki `static bool elak[100001]`).
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 02
 * ============================================================================= */
#include "mashq.h"

bool tubmi(unsigned n)
{
    /* TODO */
    (void)n;
    return false;
}

int tublar_soni(unsigned n)
{
    /* TODO */
    (void)n;
    return 0;
}
