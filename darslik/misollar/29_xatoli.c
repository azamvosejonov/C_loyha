/* =============================================================================
 *  29_xatoli.c - ATAYLAB XATOLI dastur: debug vositalarini sinash uchun   (29-bob)
 * =============================================================================
 *  Bu dasturda 3 ta xato bor. Ularni vositalar yordamida toping (kodni o'qimasdan!):
 *
 *  1) gdb bilan:
 *       gcc -g -O0 29_xatoli.c -o xatoli && gdb ./xatoli
 *       (gdb) run            - qayerda qulaydi?
 *       (gdb) bt             - chaqiruvlar zanjiri
 *       (gdb) print i        - o'sha paytdagi qiymatlar
 *  2) AddressSanitizer bilan:
 *       gcc -g -fsanitize=address 29_xatoli.c -o xatoli && ./xatoli
 *  3) Valgrind bilan (qayta kompilyatsiyasiz):
 *       gcc -g 29_xatoli.c -o xatoli && valgrind --leak-check=full ./xatoli
 *  4) massiv[4] aslida QAYERGA yozadi? (gdb bilan, sanitizer'siz yig'ilgan dasturda)
 *       (gdb) break toldir
 *       (gdb) run
 *       (gdb) print &massiv[4]
 *       (gdb) info symbol &massiv[4]      - o'sha manzilda qaysi o'zgaruvchi turibdi?
 *       (gdb) watch -l massiv[4]          - kim yozganda to'xtash
 *       (gdb) continue
 *     Natija kompilyator joylashuviga bog'liq: bir yig'ishda "hisob" buziladi, boshqasida
 *     hech narsa ko'rinmaydi. Aniqlanmagan xatti-harakat shunday JIM bo'ladi (13-bob).
 *
 *  Javoblar (avval o'zingiz toping!): pastda, fayl oxirida.
 * ============================================================================= */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int hisob = 100;
static int massiv[4];

static void toldir(void)
{
    for (int i = 0; i <= 4; i++)                /* ... */
        massiv[i] = i * 10;
}

static char *nusxa(const char *s)
{
    char *p = malloc(strlen(s));                /* ... */
    strcpy(p, s);
    return p;
}

int main(void)
{
    toldir();
    printf("hisob = %d (100 bo'lishi kerak; boshqa bo'lsa - massiv[4] uni buzgan)\n", hisob);
    char *s = nusxa("salom");
    printf("nusxa: %s\n", s);
    char *t = nusxa("dunyo");                   /* ... */
    printf("nusxa: %s\n", t);
    free(s);
    return 0;
}

/* JAVOBLAR:
 * 1) toldir: i <= 4 - massiv[4] chegaradan tashqari (global-buffer-overflow). Qo'shni
 *    global o'zgaruvchi buzilishi mumkin - qaysi biri ekanini "info symbol" aytadi.
 * 2) nusxa: malloc(strlen(s)) - '\0' uchun +1 yo'q (heap-buffer-overflow).
 * 3) t hech qachon free qilinmaydi (memory leak). */
