/* ombor_chop.h - chiqarish funksiyalarining e'lonlari (2-bosqich: aniq turlar) */
#ifndef OMBOR_CHOP_H
#define OMBOR_CHOP_H

#include <stdint.h>

void chop_pul(long tiyin);                                  /* 400000 -> "4000.00" */
void chop_sarlavha(void);
void chop_qator(const char *nom, long narx, uint16_t soni);
void chop_jami(long jami, int qqs_foiz);

#endif
