#pragma once

#include <stdint.h>

enum float_tur { F_NOL, F_DENORMAL, F_NORMAL, F_CHEKSIZ, F_NAN };

struct float_qism {
    int ishora;             /* 0 yoki 1 */
    int e;                  /* eksponenta maydoni, xomligicha (0..255) */
    uint32_t m;             /* mantissa maydoni (23 bit) */
    int daraja;             /* haqiqiy daraja: normal - e-127, denormal - -126, qolganlari - 0 */
    enum float_tur tur;
};

void float_ajrat(uint32_t bitlar, struct float_qism *q);
uint32_t butundan_float(int32_t x);
