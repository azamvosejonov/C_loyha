/* =============================================================================
 *  30 - O'z malloc'ingiz                            [5-modul: tizim chaqiruvlari]
 * =============================================================================
 *
 *  VAZIFA - ARENA_HAJMI (64 KB) baytli statik massiv ichida xotira boshqaruvchi:
 *    mm_init()      - arenani bitta katta bo'sh blok qilish (qayta chaqirilsa - boshidan)
 *    mm_alloc(n)    - kamida n baytli blok; manzil 16 ga KARRALI bo'lsin.
 *                     n == 0 yoki joy yo'q -> NULL
 *    mm_free(p)     - blokni bo'shatish; NULL -> hech narsa. Qo'shni bo'sh
 *                     bloklar bilan BIRLASHTIRING (aks holda xotira mayda
 *                     bo'laklarga bo'linib ketadi - fragmentatsiya).
 *    mm_bosh_joy()  - bo'sh bloklarning FOYDALI baytlari yig'indisi
 *                     (sarlavhalarsiz). Hammasi free qilingandan keyin qiymat
 *                     mm_init dan keyingi qiymat bilan bir xil bo'lishi kerak.
 *
 *  PYTHON'DA:
 *    Ko'rinmaydi - lekin Python obyekti yaratilganda ichkarida aynan shu ishlaydi.
 *
 *  QANDAY ISHLAYDI (eng oddiy variant):
 *    Arena bloklar ketma-ketligi: [sarlavha|foydali qism][sarlavha|foydali qism]...
 *    Sarlavhada: blok hajmi va "bo'shmi" bayrog'i. Keyingi blok = joriy manzil +
 *    sarlavha + hajm. alloc: birinchi mos bo'sh blokni topish (first fit), kerak
 *    bo'lsa ikkiga bo'lish. free: bayroqni o'chirish va qo'shnilarni birlashtirish
 *    (eng oddiysi - butun arenani aylanib, ketma-ket bo'sh bloklarni qo'shish).
 *
 *  BU QAYERGA OLIB BORADI:
 *    MyOS'dagi user/libc/malloc.c (malloc lab'i) - xuddi shu g'oya, lekin sbrk
 *    bilan o'sadi. Yadroda: kernel/mm/slab.c (kmalloc) va pmm.c (buddy).
 *
 *  MASLAHAT:
 *    * Arena: `static _Alignas(16) unsigned char arena[ARENA_HAJMI];`
 *    * Sarlavhani 16 bayt qiling va hajmlarni 16 ga yaxlitlang - tekislash o'z-o'zidan saqlanadi.
 *    * Sarlavhaga `struct blok *b = (struct blok *)(arena + siljish);` bilan murojaat.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 30
 * ============================================================================= */
#include <stddef.h>
#include <stdint.h>

#include "mashq.h"

void mm_init(void)
{
    /* TODO */
}

void *mm_alloc(size_t n)
{
    /* TODO */
    (void)n;
    return NULL;
}

void mm_free(void *p)
{
    /* TODO */
    (void)p;
}

size_t mm_bosh_joy(void)
{
    /* TODO */
    return 0;
}
