/* sys/ioctl.h - qurilmaga maxsus buyruqlar */
#pragma once
#include "myos/abi.h"

int ioctl(int fd, unsigned long cmd, ...);
