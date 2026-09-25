/* sys/wait.h - bola jarayonlarni kutish */
#pragma once
#include "myos/abi.h"

#define WNOHANG WAIT_NOHANG
/* MyOS: status - bevosita chiqish kodi (128+ - xato/signal bilan o'ldirilgan). */
#define WEXITSTATUS(s) (s)

int waitpid(int pid, int *status, int flags);
int wait(int *status);
