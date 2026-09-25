/* fcntl.h - open() va uning bayroqlari */
#pragma once
#include "myos/abi.h"

int open(const char *path, int flags, ...);
int creat(const char *path, unsigned mode);
