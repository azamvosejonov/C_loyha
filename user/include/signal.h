/* signal.h - jarayonlarga signal yuborish.
 * HOZIRCHA: yadroda signallar yo'q, kill() jarayonni darhol tugatadi (SIGKILL
 * kabi). Signallar keyingi bosqichda qo'shiladi. */
#pragma once

#define SIGINT  2
#define SIGKILL 9
#define SIGTERM 15

int kill(int pid, int sig);
