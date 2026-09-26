/* =============================================================================
 *  15 - Satrni bo'laklarga ajratish (split)                    [3-modul: xotira]
 * =============================================================================
 *
 *  VAZIFA:
 *    ajrat(s, ',', &n) - Python'dagi s.split(',') kabi:
 *        "a,b,c"  -> ["a", "b", "c"]        n = 3
 *        "a,,b"   -> ["a", "", "b"]         n = 3  (bo'sh qismlar SAQLANADI)
 *        ""       -> [""]                   n = 1
 *        ",a,"    -> ["", "a", ""]          n = 3
 *      Qaytish: malloc qilingan `char *` massivi, oxirida qo'shimcha NULL
 *      (argv kabi!). Har bir qism ham alohida malloc qilingan satr.
 *      Xotira yetmasa - NULL (va o'sha paytgacha ajratilganlarni free qiling).
 *    ajrat_ozod(qismlar) - hamma qismlarni va massivning o'zini free qilish.
 *
 *  PYTHON'DA:
 *    s.split(',')
 *
 *  C'DA NIMA BOSHQA:
 *    * `char **` - "ko'rsatkichlar massivi". Xotirada: massiv (8 baytli
 *      manzillar) + har bir manzil alohida satrga ko'rsatadi. Ikki daraja malloc,
 *      shuning uchun ikki daraja free: avval har bir satr, keyin massiv.
 *    * Oxirida NULL - main(int argc, char **argv) dagi argv ham aynan shunday.
 *      Shell (MyOS: user/bin/sh.c) buyruq qatorini aynan shunday bo'laklaydi.
 *
 *  MASLAHAT:
 *    Avval ajratgichlarni sanang (qismlar = ajratgichlar + 1), massivni malloc
 *    qiling, keyin har bir qismni topib, uzunligi + 1 bayt ajrating.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 15
 * ============================================================================= */
#include <stdlib.h>
#include <string.h>

#include "mashq.h"

char **ajrat(const char *s, char ajratgich, size_t *soni)
{
    /* TODO */
    (void)s; (void)ajratgich; (void)soni;
    return NULL;
}

void ajrat_ozod(char **qismlar)
{
    /* TODO */
    (void)qismlar;
}
