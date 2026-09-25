/* =============================================================================
 *  drivers/pit.h - Programmable Interval Timer (tizim soati)
 * ============================================================================= */
#pragma once

#include <stdint.h>

/* Sekundiga necha marta taymer uzilishi (tik) keladi. 100 Hz = har 10 ms. */
#define TIMER_HZ 100

void pit_init(void);
/* Tizim yoqilgandan beri o'tgan tiklar soni. */
uint64_t timer_ticks(void);
/* Har bir tikda chaqiriladigan funksiya (scheduler uchun, 6-bosqich). */
void timer_set_tick_callback(void (*callback)(void));
