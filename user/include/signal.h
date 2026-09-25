/* =============================================================================
 *  signal.h - signallar: yuborish, ushlash, bloklash
 * ============================================================================= */
#pragma once
#include <stdint.h>

#include "myos/abi.h"

typedef uint64_t sigset_t;
typedef void (*sighandler_t)(int);
typedef int sig_atomic_t;

/* Yadro ABI'sida SIG_DFL/SIG_IGN - sonlar (0, 1). C da ular funksiya ko'rsatkichi. */
#undef SIG_DFL
#undef SIG_IGN
#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)
#define SIG_ERR ((sighandler_t)-1)

/* Maydonlar va tartib - yadrodagi struct myos_sigaction bilan bir xil. */
struct sigaction {
    sighandler_t sa_handler;
    unsigned long sa_flags;
    void (*sa_restorer)(void);
    sigset_t sa_mask;
};

int kill(int pid, int sig);
int raise(int sig);
sighandler_t signal(int sig, sighandler_t handler);
int sigaction(int sig, const struct sigaction *act, struct sigaction *old);
int sigprocmask(int how, const sigset_t *set, sigset_t *old);

static inline int sigemptyset(sigset_t *s) { *s = 0; return 0; }
static inline int sigfillset(sigset_t *s) { *s = ~(sigset_t)0; return 0; }
static inline int sigaddset(sigset_t *s, int sig) { *s |= (sigset_t)1 << sig; return 0; }
static inline int sigdelset(sigset_t *s, int sig) { *s &= ~((sigset_t)1 << sig); return 0; }
static inline int sigismember(const sigset_t *s, int sig) { return (int)((*s >> sig) & 1); }

/* Signal nomi ("Segmentation fault" ...). string.h dagi strsignal bilan bir xil. */
char *strsignal(int sig);
