/* =============================================================================
 *  sys/wait.h - bola jarayonlarni kutish va holat so'zini ochish
 * =============================================================================
 *  waitpid() qaytargan status - bitta int ichida uch xil ma'lumot:
 *     tugadi (exit):        [ kod (8 bit) ][ 0x00 ]
 *     signal o'ldirdi:      [    0        ][ signal ]
 *     to'xtatildi (Ctrl-Z): [ signal      ][ 0x7F   ]
 * ============================================================================= */
#pragma once
#include "myos/abi.h"

#define WNOHANG   WAIT_NOHANG
#define WUNTRACED WAIT_UNTRACED

#define WEXITSTATUS(s) (((s) >> 8) & 0xFF)
#define WTERMSIG(s)    ((s) & 0x7F)
#define WSTOPSIG(s)    WEXITSTATUS(s)
#define WIFEXITED(s)   (WTERMSIG(s) == 0)
#define WIFSTOPPED(s)  (((s) & 0xFF) == 0x7F)
#define WIFSIGNALED(s) (WTERMSIG(s) != 0 && WTERMSIG(s) != 0x7F)

int waitpid(int pid, int *status, int flags);
int wait(int *status);
