/* =============================================================================
 *  arch/tsc.h - vaqtni o'lchash va aniq kutish (udelay)
 * ============================================================================= */
#pragma once

#include <stdint.h>

extern uint64_t tsc_khz;                /* TSC chastotasi (kHz) */

void tsc_calibrate(void);
/* PIT 2-kanali yordamida aniq ms kutish (kalibrlash uchun). */
void pit_wait_ms(uint32_t ms);
void udelay(uint64_t us);
void mdelay(uint64_t ms);
