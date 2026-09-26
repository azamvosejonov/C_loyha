/* =============================================================================
 *  20 - Funksiya ko'rsatkichlari jadvali                [4-modul: yadro uslubidagi C]
 * =============================================================================
 *
 *  VAZIFA:
 *    1) Beshta amal funksiyasini yozing: + - * / %  (turi: amal_fn, mashq.h da).
 *       Har biri: natija int'ga sig'sa va hisoblash mumkin bo'lsa - *natija ga
 *       yozib 0; aks holda -1 (toshish, nolga bo'lish, INT_MIN / -1 ...).
 *    2) `amallar[]` jadvalini to'ldiring: { "+", qoshish }, ... , { NULL, NULL }.
 *    3) hisobla(nom, a, b, &r) - jadvaldan nomni topib, funksiyasini chaqiradi.
 *       Nom topilmasa: -2. Amal xato qaytarsa: -1. Muvaffaqiyat: 0.
 *
 *  PYTHON'DA:
 *    AMALLAR = {'+': operator.add, ...};  AMALLAR[nom](a, b)
 *
 *  NEGA BU YADRONING ASOSI:
 *    Linux (va MyOS) VFS'i aynan shunday ishlaydi: `struct file_ops` - read,
 *    write, open funksiyalari ko'rsatkichlari jadvali. pipe, tty, ext2, tmpfs -
 *    har biri o'z jadvalini beradi va VFS `f->ops->read(...)` ni chaqiradi,
 *    qaysi fayl tizimi ekanini BILMASDAN. Bu C'dagi "polimorfizm".
 *    MyOS: kernel/fs/pipe.c dagi `pipe_fops`, kernel/drivers/ahci.c dagi pci_driver.
 *
 *  MASLAHAT:
 *    * 03-mashqdagi toshish tekshiruvini qayta ishlating.
 *    * INT_MIN / -1 va INT_MIN % -1 - ikkalasi ham UB (natija sig'maydi).
 *    * Satrlarni `==` bilan solishtirmang (manzillar solishtiriladi!) - strcmp.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 20
 * ============================================================================= */
#include <limits.h>
#include <string.h>

#include "mashq.h"

/* TODO: amal funksiyalarini shu yerga yozing */

const struct amal amallar[] = {
    /* TODO: { "+", qoshish }, ... */
    { NULL, NULL },
};

int hisobla(const char *nom, int a, int b, int *natija)
{
    /* TODO */
    (void)nom; (void)a; (void)b; (void)natija;
    return -2;
}
