/* =============================================================================
 *  user/bin/crash.c - XIMOYA MEXANIZMLARINI SINASH
 * =============================================================================
 *
 *  Har bir variant user dastur QILA OLMAYDIGAN narsani qilishga urinadi.
 *  Yadro CPU exception'ini ushlab, FAQAT shu jarayonni o'ldiradi - shell va
 *  qolgan tizim ishlashda davom etadi. Operatsion tizim himoyasining butun
 *  mazmuni shu.
 *
 *    crash null    - NULL ga yozish            -> #PF (sahifa yo'q)
 *    crash kernel  - yadro kodini o'qish        -> #PF (U=0 sahifa, ruxsat yo'q)
 *    crash cli     - uzilishlarni o'chirish    -> #GP (imtiyozli instruksiya)
 *    crash div0    - nolga bo'lish             -> #DE
 *    crash stack   - cheksiz rekursiya         -> #PF (stek ostidagi himoya sahifasi)
 * ============================================================================= */
#include <stdio.h>
#include <string.h>

__attribute__((noinline)) static int deep(volatile int n)
{
    volatile char pad[1024];
    pad[0] = (char)n;
    if (n > 1000000)
        return pad[0];
    return deep(n + 1) + pad[0];
}

int main(int argc, char **argv)
{
    const char *what = argc > 1 ? argv[1] : "";
    /* Dastur qulashidan OLDIN xabar chiqsin: buferlashni o'chiramiz (exit() chaqirilmaydi!). */
    setvbuf(stdout, NULL, _IONBF, 0);

    if (strcmp(what, "null") == 0) {
        printf("NULL ga yozyapman...\n");
        *(volatile int *)0 = 1;
    } else if (strcmp(what, "kernel") == 0) {
        printf("Yadro kodini (0xFFFFFFFF80110000) o'qiyapman...\n");
        printf("%x\n", *(volatile unsigned *)0xFFFFFFFF80110000UL);
    } else if (strcmp(what, "cli") == 0) {
        printf("'cli' instruksiyasini bajaryapman (faqat ring 0 uchun)...\n");
        __asm__ volatile("cli");
    } else if (strcmp(what, "div0") == 0) {
        printf("Nolga bo'lyapman...\n");
        volatile int z = 0;
        printf("%d\n", 10 / z);
    } else if (strcmp(what, "stack") == 0) {
        printf("Cheksiz rekursiya...\n");
        deep(0);
    } else {
        printf("ishlatish: crash null|kernel|cli|div0|stack\n");
        return 1;
    }
    printf("Bu qator chiqmasligi kerak edi!\n");
    return 0;
}
