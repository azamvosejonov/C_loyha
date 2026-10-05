/* parallel.h - ko'p xodimning maoshini bir necha OQIM (thread) bilan hisoblash (15-bob).
   Hamma variant bir xil javob berishi shart: ketma-ket variant - ETALON. */
#ifndef PARALLEL_H
#define PARALLEL_H

#include <stddef.h>

#include "xodim.h"

#define MAKS_OQIM 16

/* etalon: bitta oqimda. Hamma xodimlar bo'yicha YIG'INDI *jami ga yoziladi (avval nollanadi) */
void jami_ketma_ket(const struct xodim *a, size_t n, struct natija *jami);

/* TO'G'RI va TEZ: ish oqimlarga bo'linadi, HAR OQIM O'Z yig'indisiga yozadi (hech narsa bo'lishilmaydi), oxirida asosiy oqim qo'shadi.
   0 - OK, -1 - oqim yaratib bo'lmadi */
int jami_parallel(const struct xodim *a, size_t n, int oqimlar, struct natija *jami);

/* TO'G'RI, lekin SEKINROQ: hamma oqim bitta umumiy yig'indiga yozadi, har yozish mutex bilan himoyalangan (oqimlar navbat kutadi) */
int jami_qulf(const struct xodim *a, size_t n, int oqimlar, struct natija *jami);

/* XATOLI (ataylab): umumiy yig'indiga qulfsiz yozadi - ma'lumot poygasi. ThreadSanitizer bilan sinash uchun, ishlatmang! */
int jami_poyga(const struct xodim *a, size_t n, int oqimlar, struct natija *jami);

/* sinov uchun n ta deterministik "tasodifiy" xodim yaratadi (heap; chaqiruvchi free qiladi) */
struct xodim *sinov_xodimlari(size_t n);

#endif
