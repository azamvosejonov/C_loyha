/* =============================================================================
 *  22_boglash.c - bo'limlar (.text .data .bss .rodata) va belgilar   (22-bob)
 * =============================================================================
 *  Ishga tushirish va ichini ko'rish:
 *      gcc -Wall -Wextra -g -c 22_boglash.c -o boglash.o    (faqat obyekt fayl)
 *      nm boglash.o                  - belgilar: T/t (kod), D/d (.data), B/b (.bss), R/r (.rodata), U
 *      readelf -S boglash.o          - bo'limlar va ularning hajmi (.bss ~40 KB, lekin faylda joy yo'q!)
 *      objdump -dr boglash.o         - relokatsiyalar: printf chaqiruvi "R_X86_64_PLT32 printf"
 *      gcc boglash.o -o boglash && ./boglash
 *      readelf -l boglash            - segmentlar (PT_LOAD) - yuklovchi uchun
 *      ls -l boglash.o boglash       - fayllar hajmi (katta .bss hajmga kirmagan)
 *
 *  Kutilgan natija (./boglash, manzillar boshqacha):
 *      .text   (kod)          main       @ 0x55...
 *      .rodata (o'zgarmas)    salom      @ 0x55...
 *      .data   (qiymatli)     hisob      @ 0x55...  = 5
 *      .bss    (nol)          katta_bufer@ 0x55...  [0] = 0
 *      stek    (lokal)        lokal      @ 0x7ff...
 *      heap    (malloc)       p          @ 0x55...
 *
 *  Sinab ko'ring:
 *      1) `int hisob = 5;` ni `= 0` qiling. nm'da D o'rniga nima chiqadi? Nega?
 *      2) katta_bufer dan `static` ni o'chiring: b -> B (lokal -> global belgi).
 *      3) main'da yoq_ozgaruvchi ni ishlating. `gcc -c` o'tadi, bog'lash (link) esa
 *         "undefined reference" beradi - xato qaysi bosqichda ekanini ko'ring.
 *      4) katta_bufer ni 4000000 qiling - boglash.o hajmi deyarli o'zgarmaydi. Endi `= { 1 }`
 *         bilan boshlang - .data ga o'tadi va fayl 4 MB ga o'sadi (ls -l).
 * ============================================================================= */
#include <stdio.h>
#include <stdlib.h>

const char salom[] = "salom";           /* .rodata */
int hisob = 5;                          /* .data  (D) */
static char katta_bufer[40000];         /* .bss   (b - static: lokal belgi) */
extern int yoq_ozgaruvchi;              /* e'lon - ishlatilsa "undefined reference" bo'lardi */

static int yordamchi(int x) { return x * 2; }   /* t - lokal funksiya */

int main(void)
{
    int lokal = yordamchi(1);
    int *p = malloc(sizeof(int));
    printf(".text   (kod)          main       @ %p\n", (void *)main);
    printf(".rodata (o'zgarmas)    salom      @ %p\n", (void *)salom);
    printf(".data   (qiymatli)     hisob      @ %p  = %d\n", (void *)&hisob, hisob);
    printf(".bss    (nol)          katta_bufer@ %p  [0] = %d\n", (void *)katta_bufer, katta_bufer[0]);
    printf("stek    (lokal)        lokal      @ %p\n", (void *)&lokal);
    printf("heap    (malloc)       p          @ %p\n", (void *)p);
    free(p);
    return 0;
}
