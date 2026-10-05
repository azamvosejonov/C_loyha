/* parser.h - "nom:narx:soni" qatorlarini o'qiydigan kutubxona interfeysi (ikki ichki variant bor: zaif va xavfsiz) */
#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

struct yozuv {
    char nom[16];                               /* eng ko'pi bilan 15 belgi + '\0' */
    int narx;                                   /* so'mda */
    int soni;
};

/* "non:4000:120" ni y ga o'qiydi. 0 - muvaffaqiyat, -1 - format noto'g'ri */
int yozuv_oqi(const char *satr, struct yozuv *y);

/* hamma yozuvlar narx * soni yig'indisi. 0 - muvaffaqiyat, -1 - toshib ketdi */
int yozuvlar_jami(const struct yozuv *y, size_t n, long *jami);

#endif
