/* =============================================================================
 *  arch/percpu.h - HAR BIR CPU'NING SHAXSIY MA'LUMOTLARI
 * =============================================================================
 *
 *  Ko'p yadroli tizimda "joriy jarayon" degan tushuncha CPU'ga bog'liq: CPU0
 *  bir jarayonni, CPU1 boshqasini bajaryapti. Shuning uchun har bir CPU'ning
 *  o'z "struct cpu"si bor: joriy jarayon, o'z GDT/TSS, o'z steklari...
 *
 *  QANDAY TOPILADI: GS segment registri. 64-bitli rejimda GS ning "bazasi"
 *  MSR orqali istalgan manzilga o'rnatiladi. Biz har bir CPU'da GS.base =
 *  o'sha CPU'ning struct cpu manzili qilamiz. Shunda:
 *      mov rax, gs:[0]   ->  joriy CPU'ning struct cpu ko'rsatkichi
 *  Bitta instruksiya - qaysi CPU'da bajarilsa, o'sha CPU'niki.
 *
 *  SWAPGS: user dastur ham GS dan foydalanishi mumkin (TLS). Yadroga kirishda
 *  `swapgs` GS.base va KERNEL_GS_BASE MSR larini almashtiradi: user bazasi
 *  saqlanadi, yadroniki o'rnatiladi. Chiqishda yana swapgs.
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arch/gdt.h"

#define MAX_CPUS 64

struct process;

struct cpu {
    /* ---- Assembly ham ishlatadigan maydonlar: siljishlari o'zgarmasin! ---- */
    struct cpu *self;                   /* gs:0  - this_cpu() */
    uint64_t kernel_rsp;                /* gs:8  - syscall kirishida yadro steki */
    uint64_t user_rsp;                  /* gs:16 - syscall kirishida user RSP vaqtincha */
    struct process *cur_proc;           /* gs:24 - joriy jarayon (`current` makrosi) */

    /* ---- Oddiy maydonlar ---- */
    int id;                             /* mantiqiy raqam: 0, 1, 2 ... */
    uint32_t apic_id;                   /* Local APIC ID (apparat raqami) */
    volatile bool online;
    struct process *idle;               /* shu CPU'ning idle jarayoni (ps uchun) */
    uint64_t scheduler_rsp;             /* scheduler tsiklining saqlangan konteksti */

    int ncli;                           /* push_off chuqurligi */
    bool intena;                        /* birinchi push_off oldidan IF yoqilganmidi */

    uint64_t ticks;                     /* shu CPU taymer tiklari */
    uint64_t idle_ticks;
    uint64_t lapic_ticks_per_ms;

    struct tss tss __attribute__((aligned(16)));
    uint64_t gdt[GDT_ENTRIES];
    uint8_t *ist_stack;                 /* double fault / NMI steki */
};

#define CPU_OFF_SELF       0
#define CPU_OFF_KERNEL_RSP 8
#define CPU_OFF_USER_RSP   16
#define CPU_OFF_CURRENT    24
_Static_assert(offsetof(struct cpu, kernel_rsp) == CPU_OFF_KERNEL_RSP, "percpu siljishi");
_Static_assert(offsetof(struct cpu, user_rsp) == CPU_OFF_USER_RSP, "percpu siljishi");
_Static_assert(offsetof(struct cpu, cur_proc) == CPU_OFF_CURRENT, "percpu siljishi");

extern struct cpu *cpus[MAX_CPUS];
extern int ncpus;                       /* ishga tushgan CPU'lar */

static inline struct cpu *this_cpu(void)
{
    struct cpu *c;
    __asm__ volatile("mov %%gs:0, %0" : "=r"(c));
    return c;
}

/* Joriy jarayon: BITTA instruksiya bilan o'qiladi, shuning uchun o'rtada boshqa
 * CPU'ga ko'chib qolish (migration) xavfi yo'q. */
static inline struct process *get_current(void)
{
    struct process *p;
    __asm__ volatile("mov %%gs:24, %0" : "=r"(p));
    return p;
}
#define current (get_current())

static inline int cpu_id(void)
{
    return this_cpu()->id;
}

/* BSP (birinchi CPU) uchun struct cpu ni tayyorlab, GS ga ulash. Eng boshida. */
void percpu_init_bsp(void);
/* AP uchun: struct cpu ajratish (smp.c). */
struct cpu *percpu_alloc(int id, uint32_t apic_id);
/* GS.base ni o'rnatish. */
void percpu_install(struct cpu *c);

/* push_off/pop_off - ichma-ich cli/sti (xv6 dan). Birinchi push_off IF holatini
 * eslab qoladi, oxirgi pop_off uni tiklaydi. */
void push_off(void);
void pop_off(void);
