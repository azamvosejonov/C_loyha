/* =============================================================================
 *  08 - Satrlar va ko'rsatkich arifmetikasi                [2-modul: ko'rsatkichlar]
 * =============================================================================
 *
 *  VAZIFA (string.h dagi funksiyalarni o'zingiz yozasiz, uni #include QILMANG):
 *    mening_strlen(s)     - satr uzunligi ('\0' gacha nechta bayt)
 *    mening_strchr(s, c)  - s da c belgisining BIRINCHI uchrashiga ko'rsatkich,
 *                           topilmasa NULL. c == '\0' bo'lsa - satr oxiridagi
 *                           '\0' ning manzilini qaytaring (standart shunday).
 *    mening_strcmp(a, b)  - a < b: manfiy, a == b: 0, a > b: musbat.
 *                           Baytlarni `unsigned char` sifatida solishtiring!
 *
 *  PYTHON'DA:
 *    len(s),  s.find(c),  (a > b) - (a < b)
 *
 *  C'DA NIMA BOSHQA - SATR NIMA:
 *    C'da "satr" turi YO'Q. Satr - bu '\0' (nol bayt) bilan tugaydigan char
 *    massivi: "salom" xotirada 6 bayt: s a l o m \0. Uzunlik hech qayerda
 *    saqlanmaydi - uni bilish uchun '\0' gacha SANASH kerak.
 *    Ko'rsatkich arifmetikasi: `p + 1` - keyingi element, `*p` - joriy element,
 *    `p++` - ko'rsatkichni surish, `q - p` - ikki ko'rsatkich orasidagi masofa.
 *
 *  MASLAHAT:
 *    * strlen: `const char *p = s; while (*p) p++; return p - s;`  - buni
 *      o'zingiz tushunib yozing, ko'chirmang.
 *    * strchr: qaytish turi `const char *` - topilgan joyning manzili.
 *    * strcmp: '\xff' (255) 'a' (97) dan KATTA bo'lishi kerak. char esa x86'da
 *      ishorali: (char)0xff = -1. Shuning uchun `(unsigned char)*a`.
 *    * MyOS'dagi user/libc/string.c ham aynan shu funksiyalarni yozadi.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 08
 * ============================================================================= */
#include "mashq.h"

size_t mening_strlen(const char *s)
{
    /* TODO */
    (void)s;
    return 0;
}

const char *mening_strchr(const char *s, int c)
{
    /* TODO */
    (void)s; (void)c;
    return NULL;
}

int mening_strcmp(const char *a, const char *b)
{
    /* TODO */
    (void)a; (void)b;
    return 0;
}
