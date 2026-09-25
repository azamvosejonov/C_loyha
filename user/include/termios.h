/* termios.h - terminal sozlamalari (kanonik/xom rejim, echo) */
#pragma once
#include "myos/abi.h"

#define termios myos_termios            /* struct termios == struct myos_termios */
#define winsize myos_winsize
#define TCSANOW 0

int tcgetattr(int fd, struct myos_termios *t);
int tcsetattr(int fd, int action, const struct myos_termios *t);
int isatty(int fd);
int ioctl(int fd, unsigned long cmd, ...);
