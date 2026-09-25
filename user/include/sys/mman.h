/* sys/mman.h - xotira xaritalash */
#pragma once
#include <stddef.h>
#include "myos/abi.h"

void *mmap(void *addr, size_t len, int prot, int flags, int fd, long off);
int munmap(void *addr, size_t len);
