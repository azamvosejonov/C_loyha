/* guard.h - "qo'riqchi sahifali" ajratuvchi: xotira xatolarini ZUDLIK bilan (SIGSEGV) ushlaydi */
#ifndef GUARD_H
#define GUARD_H

#include <stddef.h>

enum guard_rejim {
    GUARD_OXIRI,                                /* foydalanuvchi xotirasi sahifa OXIRIGA taqalgan: 1 baytlik overflow ham qulatadi */
    GUARD_BOSHI                                 /* sahifa BOSHIGA taqalgan: underflow (p[-1]) ni ushlaydi */
};

void *gmalloc(size_t n, enum guard_rejim rejim);
int gfree(void *p);                             /* 0 - OK, -1 - noto'g'ri yoki ikki marta free */

#endif
