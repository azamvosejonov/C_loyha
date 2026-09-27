/* =============================================================================
 *  43 - Sahifa almashtirish algoritmlari                 [7-modul: nazariya amalda]
 * =============================================================================
 *
 *  VAZIFA (darslik 24-bob):
 *    Sahifa murojaatlari ketma-ketligi (murojaat[0..n-1], sahifa raqamlari >= 0) va
 *    freymlar soni berilgan (1 <= freymlar <= 64). Boshida hamma freym bo'sh. Har bir
 *    funksiya PAGE FAULT'lar (sahifa xotirada yo'q bo'lgan murojaatlar) sonini qaytaradi.
 *
 *    fifo_xatolar  - eng BIRINCHI yuklangan sahifani chiqarish
 *    lru_xatolar   - eng UZOQ VAQT murojaat qilinmaganini chiqarish
 *    opt_xatolar   - KELAJAKDA eng uzoq vaqt kerak bo'lmaydiganini (umuman kerak
 *                    bo'lmasa - o'shani) chiqarish (Belady'ning optimal algoritmi)
 *    clock_xatolar - "ikkinchi imkoniyat" algoritmi, aniq qoidalari:
 *        * freymlar 0..F-1, har birida "murojaat biti"; soat mili boshida 0-freymda;
 *        * murojaat xotirada bor sahifaga (hit): uning bitini 1 qilish (mil qimirlamaydi);
 *        * fault va bo'sh freym bor: sahifa birinchi bo'sh freymga (kichik indeksli),
 *          biti = 1 (mil qimirlamaydi);
 *        * fault va bo'sh freym yo'q: mil ostidagi freymning biti 1 bo'lsa - 0 qilib,
 *          mil = (mil + 1) % F; bit 0 bo'lgan freym topilguncha takrorlash. Topilganga
 *          yangi sahifani qo'yish, biti = 1, mil = (mil + 1) % F.
 *
 *  NEGA:
 *    RAM to'lganda yadro qaysi sahifani diskka chiqarishini hal qiladi - noto'g'ri
 *    tanlov tizimni "thrashing"ga olib keladi. OPT - erishib bo'lmaydigan ideal (kelajakni
 *    biladi), LRU - yaxshi, lekin qimmat, Clock - apparatdagi "accessed" bit bilan arzon
 *    yaqinlashuv (Linux'dagi algoritmlar shu oiladan). Test FIFO'dagi Belady
 *    anomaliyasini ham tekshiradi: freymlar ko'paysa, xatolar KO'PAYISHI mumkin!
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 43
 * ============================================================================= */
#include "mashq.h"

int fifo_xatolar(const int *murojaat, size_t n, int freymlar)
{
    /* TODO */
    (void)murojaat; (void)n; (void)freymlar;
    return -1;
}

int lru_xatolar(const int *murojaat, size_t n, int freymlar)
{
    /* TODO */
    (void)murojaat; (void)n; (void)freymlar;
    return -1;
}

int opt_xatolar(const int *murojaat, size_t n, int freymlar)
{
    /* TODO */
    (void)murojaat; (void)n; (void)freymlar;
    return -1;
}

int clock_xatolar(const int *murojaat, size_t n, int freymlar)
{
    /* TODO */
    (void)murojaat; (void)n; (void)freymlar;
    return -1;
}
