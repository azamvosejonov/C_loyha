/* =============================================================================
 *  13_ub.c - aniqlanmagan xatti-harakat (UB) jonli namoyishi    (darslik 13-bob)
 * =============================================================================
 *  Uch xil yig'ib, natijalarni solishtiring:
 *      gcc -O0 13_ub.c -o ub0 && ./ub0
 *      gcc -O2 13_ub.c -o ub2 && ./ub2
 *      gcc -O0 -fsanitize=undefined 13_ub.c -o ubs && ./ubs
 *
 *  Kutilgan natija (GCC 13, x86-64):
 *      -O0 va -O2 da:   toshadimi(INT_MAX) = 0   <- tekshiruvingiz ISHLAMADI!
 *                       (GCC bu shartni optimallashtirishsiz ham "doim yolg'on" deb soddalashtiradi)
 *      sanitizer bilan: "runtime error: signed integer overflow: 2147483647 + 1 ..." va keyin 1
 *      to'g'ri tekshiruv: 1 (hamma rejimda)
 *
 *  Nega: ishorali toshish UB -> kompilyator "x + 1 < x hech qachon rost emas" deb
 *  hisoblaydi. Assembly'ni ko'ring: gcc -O2 -S 13_ub.c -o - | grep -A3 "^toshadimi:"
 *
 *  Sinab ko'ring:
 *      1) `-fwrapv` bilan yig'ing (gcc -O2 -fwrapv ...). Endi toshadimi(INT_MAX) = 1. Nega?
 *         (Bu bayroq ishorali toshishni "aylanadi" deb aniqlaydi. Linux yadrosi ham shunday yig'iladi.)
 *      2) toshadimi() ni `unsigned` bilan yozing: UINT_MAX uchun to'g'ri ishlaydi. Nega?
 *      3) __builtin_add_overflow(x, 1, &natija) bilan uchinchi, to'g'ri variant yozing.
 * ============================================================================= */
#include <limits.h>
#include <stdio.h>

__attribute__((noinline)) int toshadimi(int x)
{
    return x + 1 < x;                           /* XATO: toshishni toshgandan KEYIN tekshirish */
}

__attribute__((noinline)) int toshadimi_togri(int x)
{
    return x == INT_MAX;                        /* TO'G'RI: hisoblashdan OLDIN tekshirish */
}

int main(void)
{
    printf("toshadimi(INT_MAX) = %d\n", toshadimi(INT_MAX));
    printf("to'g'ri tekshiruv: %d\n", toshadimi_togri(INT_MAX));
    return 0;
}
