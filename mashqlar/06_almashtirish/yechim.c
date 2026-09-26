/* =============================================================================
 *  06 - Ko'rsatkich orqali almashtirish                   [2-modul: ko'rsatkichlar]
 * =============================================================================
 *
 *  VAZIFA:
 *    almashtir(&x, &y)               - x va y qiymatlarini almashtirish
 *    uchtasini_saralash(&a, &b, &c)  - shunday almashtiringki, a <= b <= c bo'lsin
 *
 *  PYTHON'DA:
 *    x, y = y, x   - lekin buni FUNKSIYA ichida qilib, chaqiruvchining
 *    o'zgaruvchilarini o'zgartirib bo'lmaydi!
 *
 *  C'DA NIMA BOSHQA - KO'RSATKICH (POINTER) NIMA:
 *    Har bir o'zgaruvchi xotirada biror MANZILDA turadi. `&x` - x ning manzili.
 *    `int *p = &x;` - p ichida manzil saqlanadi. `*p` - "p ko'rsatgan joy":
 *    `*p = 5;` x ning o'zini o'zgartiradi.
 *    C'da argumentlar doim NUSXA sifatida uzatiladi. `void f(int a) { a = 5; }`
 *    chaqiruvchining o'zgaruvchisiga ta'sir qilmaydi. Manzilni uzatsangiz esa
 *    funksiya asl o'zgaruvchini o'zgartira oladi. Yadrodagi deyarli HAR BIR
 *    funksiya shu usulda ishlaydi (struct process *p, struct inode *ino ...).
 *
 *  MASLAHAT:
 *    * almashtir: vaqtinchalik o'zgaruvchi kerak: `int t = *a; ...`
 *    * uchtasini_saralash: almashtir() ni ishlating (3 ta taqqoslash yetarli).
 *      a, b, c bu yerda allaqachon ko'rsatkichlar - almashtir(a, b) deb yozasiz,
 *      almashtir(&a, &b) emas! Nega?
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 06
 * ============================================================================= */
#include "mashq.h"

void almashtir(int *a, int *b)
{
    /* TODO */
    (void)a; (void)b;
}

void uchtasini_saralash(int *a, int *b, int *c)
{
    /* TODO */
    (void)a; (void)b; (void)c;
}
