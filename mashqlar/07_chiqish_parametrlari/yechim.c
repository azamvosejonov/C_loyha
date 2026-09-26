/* =============================================================================
 *  07 - Chiqish parametrlari                              [2-modul: ko'rsatkichlar]
 * =============================================================================
 *
 *  VAZIFA:
 *    min_max(a, n, &mn, &mx)
 *        n == 0 bo'lsa: -1 qaytaring va mn/mx ga TEGMANG.
 *        aks holda: *min va *max ga eng kichik/katta qiymatni yozib, 0 qaytaring.
 *
 *    qidir(a, n, x, indekslar, max)
 *        a da x qayerlarda uchrasa, o'sha indekslarni `indekslar` massiviga
 *        yozing - lekin max tadan KO'P emas (massiv faqat max o'rinli!).
 *        Qaytish qiymati: x JAMI necha marta uchragani (max dan ko'p bo'lishi mumkin).
 *
 *  PYTHON'DA:
 *    return min(a), max(a)                          # bir nechta qiymat qaytarish
 *    idx = [i for i, v in enumerate(a) if v == x]  # ro'yxat avtomatik o'sadi
 *
 *  C'DA NIMA BOSHQA:
 *    * Funksiya faqat BITTA qiymat qaytaradi. Ko'p natija kerak bo'lsa -
 *      chaqiruvchi o'zgaruvchilarining manzilini beradi ("chiqish parametri").
 *      Qaytish qiymati esa odatda "muvaffaqiyat/xato" kodi. Yadroda hamma joyda
 *      shunday: `int vfs_stat(const char *path, struct stat *out)`.
 *    * Natija massivining joyini CHAQIRUVCHI ajratadi va uning hajmini aytadi.
 *      Siz hajmdan oshib yozsangiz - bufer to'lishi (buffer overflow), C'dagi
 *      eng mashhur xavfsizlik teshigi. snprintf, read(), getdents() hammasi
 *      shu qoidaga amal qiladi: "joy - max, lekin haqiqiy sonni qaytaraman".
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 07
 * ============================================================================= */
#include "mashq.h"

int min_max(const int *a, size_t n, int *min, int *max)
{
    /* TODO */
    (void)a; (void)n; (void)min; (void)max;
    return -1;
}

size_t qidir(const int *a, size_t n, int x, size_t *indekslar, size_t max)
{
    /* TODO */
    (void)a; (void)n; (void)x; (void)indekslar; (void)max;
    return 0;
}
