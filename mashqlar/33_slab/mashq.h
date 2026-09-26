#pragma once

#include <stddef.h>

#define SLAB_HAJMI 4096

struct kesh;                                    /* ichki tuzilishi - yechim.c da */

struct kesh *kesh_yarat(size_t obyekt_hajmi);   /* 1..1024 bayt */
void *kesh_ol(struct kesh *k);
void kesh_ber(struct kesh *k, void *obyekt);
void kesh_yoq(struct kesh *k);
size_t kesh_slablar(const struct kesh *k);
