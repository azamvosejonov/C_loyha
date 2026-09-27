#pragma once

#include <stddef.h>

/* qirra {u, v}: "u tugun v tugunni kutyapti" (u -> v). Tugunlar: 0..n-1. */
int sikl_top(int n, const int (*qirralar)[2], size_t m, int *sikl, int *uzunlik);
