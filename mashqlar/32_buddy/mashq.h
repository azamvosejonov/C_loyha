#pragma once

#include <stddef.h>

#define MAKS_TARTIB 10          /* eng katta blok: 2^10 = 1024 sahifa */
#define MAKS_SAHIFA 4096        /* boshqariladigan sahifalar soni yuqori chegarasi */

void buddy_init(size_t sahifalar);
long buddy_ajrat(int tartib);
void buddy_ozod(long sahifa, int tartib);
size_t buddy_bosh(void);
