#pragma once

#include <stddef.h>

#define ARENA_HAJMI 65536

void mm_init(void);
void *mm_alloc(size_t n);
void mm_free(void *p);
size_t mm_bosh_joy(void);
