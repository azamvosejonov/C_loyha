/* bayroq.h - xodim bayroqlari: BITTA baytda 8 ta "ha/yo'q" belgisi (16-bob: bit maskalar).
   X-makro (10-bob) enum bitlarini va fayldagi harflarni BIR joydan hosil qiladi. */
#ifndef BAYROQ_H
#define BAYROQ_H

#include <stddef.h>
#include <stdint.h>

/* X(NOM, harf, bit_raqami): fayl matnida harf yoziladi; xotirada shu bit yoqiladi */
#define BAYROQLAR(X)                            \
    X(TOLIQ, 'T', 0)        /* to'liq stavka */ \
    X(MASOFAVIY, 'M', 1)    /* masofadan ishlaydi */ \
    X(RAHBAR, 'R', 2)       /* rahbarlik huquqi */ \
    X(SINOV, 'S', 3)        /* sinov muddatida */ \
    X(KASAL, 'K', 4)        /* kasallik ta'tilida */

enum {
#define X(nom, harf, bit) F_##nom = 1u << (bit),         /* 1u << 3 = 0b00001000 */
    BAYROQLAR(X)
#undef X
};

/* barcha ma'lum bitlar yig'indisi: bundan tashqari bit yoqilgan bo'lsa, ma'lumot yaroqsiz */
enum {
    F_HAMMASI = 0
#define X(nom, harf, bit) | F_##nom
    BAYROQLAR(X)
#undef X
};

#define BAYROQ_YOQ(b, m) ((b) |= (uint8_t)(m))          /* bitni yoqish: OR */
#define BAYROQ_OCH(b, m) ((b) &= (uint8_t)~(m))         /* bitni o'chirish: AND + inversiya */
#define BAYROQ_BOR(b, m) (((b) & (m)) != 0)             /* bit yoqilganmi: AND */

#define BAYROQ_MAKS_UZ 8

/* bayroqlarni harflar qatori ko'rinishiga: yoqilgan bit - harf, o'chiq - '-'. Masalan T va R yoqiq: "T-R--" (5 belgi + '\0') */
void bayroq_matn(uint8_t b, char *chiqish);

/* "TMR" kabi harflarni bayroq baytiga aylantiradi. Noma'lum harf bo'lsa -1, aks holda 0 va *b ga yozadi */
int bayroq_oqi(const char *harflar, uint8_t *b);

#endif
