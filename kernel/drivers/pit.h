/* =============================================================================
 *  drivers/pit.h - tizim taymeri
 * ============================================================================= */
#pragma once

#include <stdint.h>

/* Sekundiga necha marta taymer uzilishi (tik) keladi. 100 Hz = har 10 ms. */
#define TIMER_HZ 100

/* LAPIC taymeri (APIC bo'lsa) yoki PIT ni ishga tushirish. */
void timer_init(void);
/* Tizim yoqilgandan beri o'tgan tiklar soni. */
uint64_t timer_ticks(void);
