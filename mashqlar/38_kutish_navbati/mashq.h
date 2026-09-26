#pragma once

#include <stddef.h>

struct navbat;                          /* ichki tuzilishi - yechim.c da */

struct navbat *navbat_yarat(size_t sigim);
void navbat_qoy(struct navbat *q, int x);
int navbat_ol(struct navbat *q);
void navbat_yoq(struct navbat *q);
