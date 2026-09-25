/* =============================================================================
 *  drivers/rtc.h - real vaqt soati (CMOS RTC)
 * ============================================================================= */
#pragma once

#include <stdint.h>

void rtc_init(void);
/* Hozirgi Unix vaqti (1970-01-01 dan beri soniyalar). */
uint64_t time_now(void);
