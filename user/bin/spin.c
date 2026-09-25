/* =============================================================================
 *  user/bin/spin.c - CPU'ni band qiluvchi dastur (preemption namoyishi)
 *
 *  Bu dastur hech qachon o'zi CPU'ni bo'shatmaydi (yield/sleep yo'q). Shunga
 *  qaramay ikkita spin bir vaqtda ishlasa, ularning chiqishi ARALASHADI:
 *      spin a 10 &
 *      spin b 10
 *  Sababi - taymer har 50 ms da majburan scheduler'ni chaqiradi.
 * ============================================================================= */
#include <stdio.h>
#include <stdlib.h>
#include "myos.h"

int main(int argc, char **argv)
{
    const char *name = argc > 1 ? argv[1] : "spin";
    int rounds = argc > 2 ? atoi(argv[2]) : 10;
    volatile uint64_t x = 0;
    for (int r = 1; r <= rounds; r++) {
        uint64_t start = uptime_ms();
        while (uptime_ms() - start < 100)   /* ~100 ms band ishlash */
            x++;
        printf("[%s] %d/%d\n", name, r, rounds);
    }
    return 0;
}
