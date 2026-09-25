/* =============================================================================
 *  proc/process.h - jarayonlar va scheduler
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_PROCS       64
#define MAX_FDS         16              /* har bir jarayonda ochiq fayllar soni */
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
};

struct file;                            /* fs/file.h da */

struct process {
    int pid;
    enum proc_state state;
    char name[PROC_NAME_LEN];

    /* --- Kontekst --- */
    uint64_t kernel_rsp;                /* context_switch saqlagan RSP */
    uint64_t kstack_base;               /* yadro steki boshi (past manzil) */
    uint64_t pml4;                      /* manzil maydoni (CR3 qiymati) */
    bool is_user;                       /* ring 3 dasturmi yoki yadro oqimi (kernel thread) */

    /* --- Oila --- */
    struct process *parent;
    int exit_code;
    bool killed;                        /* kill() qilingan: user rejimiga qaytishda chiqadi */

    /* --- Kutish --- */
    const void *wait_channel;           /* nimani kutyapti (ixtiyoriy manzil - "kanal") */
    uint64_t wake_tick;                 /* sleep: qaysi tikda uyg'onish kerak */

    /* --- User xotirasi --- */
    uint64_t heap_start;                /* ELF segmentlaridan keyingi birinchi sahifa */
    uint64_t brk;                       /* joriy heap chegarasi (sbrk) */

    /* --- Fayllar --- */
    struct file *files[MAX_FDS];

    /* --- Statistika --- */
    uint64_t cpu_ticks;                 /* shu jarayon ishlagan tiklar soni */
    int quantum_left;
};

/* ps buyrug'i uchun jarayon haqida qisqa ma'lumot (user rejimiga nusxalanadi). */
struct proc_info {
    int32_t pid;
    int32_t ppid;
    int32_t state;
    int32_t is_user;
    uint64_t cpu_ticks;
    uint64_t mem_pages;
    char name[PROC_NAME_LEN];
};

/* Joriy (hozir ishlayotgan) jarayon. */
extern struct process *current;

/* Jarayonlar tizimini ishga tushirish: kmain() ning o'zi 0-jarayon ("idle")
 * bo'ladi. Taymerga scheduler'ni ulaydi. */
void proc_init(void);

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
/* Keyingi jarayonga o'tish. FAQAT uzilishlar o'chiq holda chaqiriladi. */
void schedule(void);
/* Ixtiyoriy ravishda CPU'ni boshqalarga berish. */
void proc_yield(void);
/* Idle tsikli - kmain oxirida chaqiriladi, qaytmaydi. */
__attribute__((noreturn)) void proc_idle_loop(void);

/* ---- Kutish va uyg'otish ----
 * sleep_on: uzilishlar O'CHIQ holda chaqirilishi SHART (shartni tekshirish va
 * uxlash orasida uyg'otish yo'qolmasligi uchun). Qaytganda ham o'chiq bo'ladi. */
void proc_sleep_on(const void *channel);
void proc_wakeup(const void *channel);
void proc_sleep_ms(uint64_t ms);

/* ---- Tugash ---- */
__attribute__((noreturn)) void proc_exit(int code);
/* pid li bolani (yoki -1: istalganini) kutish. Qaytaradi: pid yoki -1. */
int proc_wait(int pid, int *exit_code);
int proc_kill(int pid);

/* ps uchun. Qaytaradi: nechta yozuv to'ldirildi. */
int proc_list(struct proc_info *out, int max);
