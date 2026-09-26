/* =============================================================================
 *  16 - Bog'langan ro'yxat                                     [3-modul: xotira]
 * =============================================================================
 *
 *  VAZIFA (struct tugun - mashq.h da):
 *    boshiga_qosh(bosh, x)  - yangi tugunni boshiga qo'shib, YANGI boshni qaytarish.
 *                             malloc muvaffaqiyatsiz bo'lsa - eski boshni qaytaring.
 *    uzunlik(bosh)          - tugunlar soni (NULL -> 0)
 *    teskari_royxat(bosh)   - ro'yxatni JOYIDA teskari aylantirish (yangi malloc'siz),
 *                             yangi boshni qaytarish
 *    ochir(bosh, x)         - qiymati x bo'lgan HAMMA tugunlarni olib tashlab,
 *                             free qilish; yangi boshni qaytarish
 *    royxat_ozod(bosh)      - hamma tugunlarni free qilish
 *
 *  PYTHON'DA:
 *    Kerak emas - list bor. Lekin yadroda bog'langan ro'yxatlar HAMMA JOYDA:
 *    jarayonlar, ochiq fayllar, bo'sh sahifalar, kutish navbatlari...
 *
 *  C'DA NIMA BOSHQA:
 *    * `struct tugun *keyingi` - struktura o'ziga o'xshash strukturaga ko'rsatadi.
 *    * `p->qiymat` - bu (*p).qiymat ning qisqartmasi.
 *    * TUZOQ (royxat_ozod): `free(p); p = p->keyingi;` - free qilingan
 *      xotiradan o'qish (use-after-free)! Avval keyingisini saqlang.
 *
 *  MASLAHAT:
 *    * teskari: uchta ko'rsatkich - oldingi, joriy, keyingi. Qog'ozda 3 ta
 *      tugun chizib, strelkalarni qanday almashtirishni ko'ring.
 *    * ochir: "ko'rsatkichga ko'rsatkich" usuli (struct tugun **pp) boshni
 *      alohida holat qilmasdan o'chirish imkonini beradi. Linus Torvalds buni
 *      "yaxshi did" (good taste) misoli sifatida keltirgan.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 16
 * ============================================================================= */
#include <stdlib.h>

#include "mashq.h"

struct tugun *boshiga_qosh(struct tugun *bosh, int x)
{
    /* TODO */
    (void)x;
    return bosh;
}

size_t uzunlik(const struct tugun *bosh)
{
    /* TODO */
    (void)bosh;
    return 0;
}

struct tugun *teskari_royxat(struct tugun *bosh)
{
    /* TODO */
    return bosh;
}

struct tugun *ochir(struct tugun *bosh, int x)
{
    /* TODO */
    (void)x;
    return bosh;
}

void royxat_ozod(struct tugun *bosh)
{
    /* TODO */
    (void)bosh;
}
