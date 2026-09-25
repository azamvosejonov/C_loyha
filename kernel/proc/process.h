/* =============================================================================
 *  proc/process.h - jarayonlar va scheduler
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/percpu.h"
#include "lib/spinlock.h"
#include "myos/abi.h"

#define MAX_PROCS       64
#define MAX_FDS         16              /* har bir jarayonda ochiq fayllar soni */
#define MAX_ARGS        16              /* spawn: argv elementlari soni chegarasi */
#define PROC_NAME_LEN   32
#define KSTACK_PAGES    4               /* har bir jarayonning yadro steki: 16 KB */
#define SCHED_QUANTUM   5               /* vaqt kvanti: 5 tik = 50 ms */

/* Jarayonning hayot tsikli:
 *
 *   UNUSED ──alloc──► EMBRYO ──make_ready──► READY ◄─── wakeup ─── BLOCKED
 *                                             │  ▲                    ▲
 *                                   schedule  │  │ kvant tugadi/yield │ sleep_on
 *                                             ▼  │                    │
 *                                           RUNNING ──────────────────┘
 *                                             │
 *                                             │ exit
 *                                             ▼
 *                                           ZOMBIE ──ota-ona wait()──► UNUSED
 */
enum proc_state {
    PROC_UNUSED = 0,
    PROC_EMBRYO,                        /* yaratilmoqda: hali scheduler ko'rmaydi */
    PROC_READY,                         /* ishlashga tayyor, navbat kutyapti */
    PROC_RUNNING,                       /* hozir CPU'da (bitta protsessorda faqat bittasi) */
    PROC_BLOCKED,                       /* nimanidir kutyapti (klaviatura, taymer, bola) */
    PROC_ZOMBIE,                        /* tugagan, lekin ota-ona natijani hali olmagan */
    PROC_DEAD,                          /* resurslari tozalanmoqda (keyin UNUSED) */
};

struct file;                            /* fs/file.h da */
struct mm;                              /* mm/mm.h da */
struct interrupt_frame;

struct process {
    int pid;
    enum proc_state state;
    char name[PROC_NAME_LEN];

    /* --- Kontekst --- */
    uint64_t kernel_rsp;                /* context_switch saqlagan RSP */
    uint64_t kstack_base;               /* yadro steki boshi (past manzil) */
    uint64_t pml4;                      /* manzil maydoni (CR3 qiymati) */
    bool is_user;                       /* ring 3 dasturmi yoki yadro oqimi (kernel thread) */
    bool is_idle;                       /* CPU'ning idle "jarayoni" (scheduler tsikli) */
    int last_cpu;                       /* oxirgi marta qaysi CPU'da ishlagan */

    /* --- Oila --- */
    struct process *parent;
    int exit_code;
    bool killed;                        /* kill() qilingan: user rejimiga qaytishda chiqadi */

    /* --- Kutish --- */
    const void *wait_channel;           /* nimani kutyapti (ixtiyoriy manzil - "kanal") */
    uint64_t wake_tick;                 /* sleep: qaysi tikda uyg'onish kerak */

    /* --- User xotirasi --- */
    struct mm *mm;                      /* manzil maydoni (yadro oqimlarida NULL) */

    /* --- Fayllar --- */
    struct file *files[MAX_FDS];

    /* --- Statistika --- */
    uint64_t cpu_ticks;                 /* shu jarayon ishlagan tiklar soni */
    int quantum_left;
};

/* Jarayonlar jadvali qulfi. Holat (state), kanal, ota-ona munosabatlari
 * faqat shu qulf ostida o'zgaradi. */
extern spinlock_t proc_lock;

/* Jarayonlar tizimini ishga tushirish (BSP da). */
void proc_init(void);
/* Har bir CPU uchun idle jarayon strukturasini tayyorlash. */
void proc_init_cpu(struct cpu *c);

/* Yadro oqimi (kernel thread) yaratish: fn(arg) alohida stekda ishlaydi.
 * Qaytaradi: pid yoki -1. */
int proc_create_kernel_thread(const char *name, int (*fn)(void *), void *arg);

/* Bo'sh jarayon strukturasi va yadro stekini ajratish (7-bosqich: user jarayonlar). */
struct process *proc_alloc(const char *name);
/* proc_alloc() dan keyin xato bo'lsa - tozalash. */
void proc_free(struct process *p);
/* Tayyorlangan jarayonni navbatga qo'yish. */
void proc_make_ready(struct process *p);

/* ---- Scheduler ---- */
/* Ixtiyoriy ravishda CPU'ni boshqalarga berish. */
void proc_yield(void);
/* Har bir CPU'ning scheduler tsikli - qaytmaydi. */
__attribute__((noreturn)) void scheduler_loop(void);
/* Taymer uzilishidan (har bir CPU'da) chaqiriladi. */
void sched_tick(void);

/* ---- Kutish va uyg'otish ----
 * proc_sleep: `lock` ushlangan holda chaqiriladi. Qulf ATOMAR ravishda qo'yib
 * yuboriladi va jarayon uxlaydi; uyg'onganda qulf qayta olinadi. Shu tufayli
 * "shartni tekshirdim -> uxlamoqchiman" orasida uyg'otish yo'qolmaydi. */
void proc_sleep(const void *channel, spinlock_t *lock);
void proc_wakeup(const void *channel);
void proc_sleep_ms(uint64_t ms);

/* ---- Tugash ---- */
__attribute__((noreturn)) void proc_exit(int code);
/* pid li bolani (yoki -1: istalganini) kutish. Qaytaradi: pid yoki -1.
 * nohang=true: bola hali tugamagan bo'lsa kutmasdan 0 qaytaradi. */
int proc_wait(int pid, int *exit_code, bool nohang);
int proc_kill(int pid);

/* ps uchun. Qaytaradi: nechta yozuv to'ldirildi. */
int proc_list(struct myos_proc_info *out, int max);

/* ELF dasturni yuklab, yangi USER jarayon yaratish.
 * argv - yadro xotirasidagi satrlar. Qaytaradi: pid yoki manfiy xato. */
int proc_spawn(const char *path, int argc, char *const argv[]);
/* Joriy jarayonni boshqa dastur bilan ALMASHTIRISH. Muvaffaqiyatda frame
 * yangi dasturning boshiga ko'rsatadi (0); xatoda eski dastur davom etadi (<0). */
int proc_exec(struct interrupt_frame *f, const char *path, int argc, char *const argv[]);
/* Joriy jarayon nusxasi (copy-on-write). Otaga bola pid, bolaga 0 qaytadi. */
int proc_fork(struct interrupt_frame *f);
