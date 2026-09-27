/* =============================================================================
 *  25_malloc_ichi.c - malloc ichiga qarash: sarlavhalar, tekislash, heap o'sishi (25-bob)
 * =============================================================================
 *  Ishga tushirish (glibc, Linux):
 *      gcc -Wall -Wextra -g 25_malloc_ichi.c -o malloc_ichi && ./malloc_ichi
 *
 *  Kutilgan natija (glibc 2.3x, x86-64; qiymatlar boshqa versiyada farq qilishi mumkin):
 *      malloc(1)   -> manzil % 16 = 0, haqiqiy foydali hajm 24
 *      malloc(24)  -> manzil % 16 = 0, haqiqiy foydali hajm 24
 *      malloc(25)  -> manzil % 16 = 0, haqiqiy foydali hajm 40
 *      malloc(100) -> manzil % 16 = 0, haqiqiy foydali hajm 104
 *      ketma-ket ikki malloc(24) orasidagi masofa: 32 bayt (24 + sarlavha 8, tekislangan)
 *      free + malloc(24): o'sha manzil qaytdi (bo'sh ro'yxatdan)
 *      katta blok (1 MB): heap chegarasi (sbrk) o'zgarmadi - mmap bilan olindi
 *
 *  Sinab ko'ring:
 *      1) korsat(0) ni qo'shing. malloc(0) NULL qaytaradimi?
 *      2) a va b ni malloc(40) bilan ajrating. Masofani oldindan ayting: 40 + 8, 16 ga tekislangan.
 *      3) Katta malloc'dan oldin `mallopt(M_MMAP_THRESHOLD, 4 << 20);` qo'shing. Endi 1 MB
 *         heap'dan olinadi - "o'sdi" chiqadi.
 *      4) free(a) ni ikki marta chaqiring. glibc nima deydi? (double free - dastur to'xtatiladi.)
 * ============================================================================= */
#include <malloc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void korsat(size_t n)
{
    void *p = malloc(n);
    printf("malloc(%zu)%*s-> manzil %% 16 = %lu, haqiqiy foydali hajm %zu\n", n,
           n < 10 ? 3 : n < 100 ? 2 : 1, "", (unsigned long)((uintptr_t)p % 16), malloc_usable_size(p));
    free(p);
}

int main(void)
{
    /* Birinchi navbatda (printf o'z buferini ajratishidan OLDIN) - ketma-ket ikki blok */
    char *a = malloc(24), *b = malloc(24);
    ptrdiff_t masofa = b - a;
    uintptr_t eski_b = (uintptr_t)b;            /* free'dan keyin b ni ishlatmaslik uchun - raqam sifatida */
    free(b);
    char *c = malloc(24);

    korsat(1);
    korsat(24);
    korsat(25);
    korsat(100);

    printf("ketma-ket ikki malloc(24) orasidagi masofa: %td bayt (24 + sarlavha 8, tekislangan)\n", masofa);
    printf("free + malloc(24): %s\n", (uintptr_t)c == eski_b ? "o'sha manzil qaytdi (bo'sh ro'yxatdan)" : "boshqa manzil");

    void *chegara = sbrk(0);                    /* heap'ning hozirgi oxiri */
    void *katta = malloc(1 << 20);
    printf("katta blok (1 MB): heap chegarasi (sbrk) %s\n",
           sbrk(0) == chegara ? "o'zgarmadi - mmap bilan olindi" : "o'sdi");
    free(katta);
    free(a);
    free(c);
    return 0;
}
