/* =============================================================================
 *  48 - Deadlock'ni aniqlash: yo'naltirilgan grafda sikl  [7-modul: nazariya amalda]
 * =============================================================================
 *
 *  VAZIFA (darslik 26 va 28-boblar):
 *    sikl_top(n, qirralar, m, sikl, &uzunlik)
 *      n ta tugun (oqimlar yoki qulflar), m ta yo'naltirilgan qirra {u, v}: "u v ni kutadi".
 *      Sikl bor bo'lsa: uning tugunlarini sikl[] ga TARTIB BILAN yozing
 *      (sikl[0] -> sikl[1] -> ... -> sikl[uzunlik-1] -> sikl[0] - har bir qadam haqiqiy
 *      qirra), *uzunlik ga sikl uzunligini, return 1. Sikl yo'q bo'lsa return 0.
 *      sikl[] massivida n ta joy bor. O'z-o'ziga qirra {u, u} - uzunligi 1 li sikl.
 *
 *  QANDAY: DFS, uch rang (oq - ko'rilmagan, kulrang - hozir yo'lda, qora - tugagan).
 *    Kulrang tugunga olib boruvchi qirra topilsa - sikl: yo'ldagi (stekdagi) o'sha
 *    tugundan hozirgacha bo'lgan qism - javob.
 *
 *  MUHIM - REKURSIYASIZ YOZING:
 *    Test 1 000 000 tugunli zanjirni beradi. Rekursiv DFS 1 000 000 marta ichma-ich
 *    chaqiriladi va stek to'lib, dastur qulaydi (user stek 8 MB, yadroda esa 8-16 KB!).
 *    Stekni o'zingiz boshqaring: malloc qilingan massiv - (tugun, keyingi qirra indeksi)
 *    juftlari. Qo'shnilik ro'yxatini qurish: har bir tugunning chiquvchi qirralarini
 *    sanab, "prefiks yig'indi" bilan bitta massivga joylash (CSR formati) - tez va ixcham.
 *
 *  NEGA:
 *    Linux'dagi `lockdep` har bir qulf olinganda "qulf A ushlanganda B olindi" qirrasini
 *    qo'shadi va sikl paydo bo'lsa - deadlock EHTIMOLI haqida (hali sodir bo'lmasdan!)
 *    ogohlantiradi. Ma'lumotlar bazalari kutish grafida sikl topib, bitta tranzaksiyani
 *    bekor qiladi. `make` bog'liqliklardagi siklni ham shunday topadi.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 48
 * ============================================================================= */
#include <stdlib.h>

#include "mashq.h"

int sikl_top(int n, const int (*qirralar)[2], size_t m, int *sikl, int *uzunlik)
{
    /* TODO */
    (void)n; (void)qirralar; (void)m; (void)sikl; (void)uzunlik;
    return 0;
}
