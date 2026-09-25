/* =============================================================================
 *  proc/signal.h - Unix signallari
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stdint.h>

struct process;
struct interrupt_frame;

/* Jarayonga signal yuborish (proc_lock USHLANGAN holda). */
void signal_send_locked(struct process *p, int sig);
/* Guruhdagi hamma jarayonga (proc_lock ushlangan). Qaytaradi: nechtasiga. */
int signal_send_pgrp_locked(int pgid, int sig);
/* Guruhga yuborish - uzilish kontekstidan ham (tty: Ctrl-C). */
void signal_send_pgrp(int pgid, int sig);
/* kill() syscall semantikasi: pid > 0, 0 (o'z guruhim), -1 (hamma), < -1 (guruh). */
int signal_kill(int pid, int sig);

/* Jarayonni uyg'otishga arziydigan signal bormi? (bloklangan syscall'lar
 * shunda -EINTR qaytaradi) */
bool signal_interrupted(const struct process *p);

/* User rejimiga qaytishdan oldin: kutayotgan signallarni yetkazish.
 * Handler bo'lsa - freymni o'zgartiradi (handler'ga "sakraydi"). */
void signal_deliver(struct interrupt_frame *f);

/* -EINTR bilan tugagan syscall avtomatik qayta boshlanishi kerakmi? */
bool signal_should_restart(uint64_t nr);

/* CPU exception'i (masalan, #PF -> SIGSEGV). true - standart amal: jarayon tugashi kerak. */
bool signal_force(int sig);

/* exec: o'rnatilgan handler'lar SIG_DFL ga qaytadi (yangi dasturda u kod yo'q),
 * SIG_IGN esa saqlanadi (POSIX: `nohup` shunday ishlaydi). */
void signal_exec_reset(struct process *p);

/* Syscall'lar */
int64_t sys_sigaction(uint64_t sig, uint64_t uact, uint64_t uold);
int64_t sys_sigprocmask(uint64_t how, uint64_t uset, uint64_t uold);
int64_t sys_sigreturn(struct interrupt_frame *f);
int64_t sys_setpgid(int64_t pid, int64_t pgid);
int64_t sys_getpgid(int64_t pid);
int64_t sys_setsid(void);
int64_t sys_alarm(uint64_t seconds);
int64_t sys_pause(void);
/* Taymer (CPU0): muddati o'tgan alarm'lar uchun SIGALRM (proc_lock ushlangan). */
void signal_check_alarms_locked(uint64_t now);
