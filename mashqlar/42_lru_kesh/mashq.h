#pragma once

#include <stddef.h>

struct lru;                                 /* ichki tuzilishi - yechim.c da */

struct lru *lru_yarat(size_t sigim);
int lru_ol(struct lru *c, int kalit, int *qiymat);
void lru_qoy(struct lru *c, int kalit, int qiymat);
size_t lru_soni(const struct lru *c);
void lru_yoq(struct lru *c);
