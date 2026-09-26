#pragma once

#include <stdbool.h>

typedef struct {
    int band;                   /* 0 - bo'sh, 1 - band */
} spinlock_t;

typedef struct {
    unsigned keyingi;           /* keyingi beriladigan chipta raqami */
    unsigned xizmatda;          /* hozir xizmat ko'rsatilayotgan chipta */
} ticketlock_t;

#define SPINLOCK_INIT   { 0 }
#define TICKETLOCK_INIT { 0, 0 }

void spin_lock(spinlock_t *l);
void spin_unlock(spinlock_t *l);
bool spin_trylock(spinlock_t *l);

void ticket_lock(ticketlock_t *l);
void ticket_unlock(ticketlock_t *l);
