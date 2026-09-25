/* =============================================================================
 *  proc/process.c - jarayonlar jadvali, scheduler, uxlash/uyg'otish, exit/wait
 * =============================================================================
 *
 *  ASOSIY G'OYALAR:
 *
 *  1) JARAYONLAR JADVALI: MAX_PROCS ta "struct process" dan iborat statik massiv.
 *     Linux bog'langan ro'yxatlar va daraxtlar ishlatadi; massiv esa sodda va
 *     debug qilish oson. 0-yozuv - "idle" jarayoni (kmain ning o'zi).
 *
 *  2) ROUND-ROBIN SCHEDULER: tayyor (READY) jarayonlar navbat bilan, har biri
 *     SCHED_QUANTUM tik (50 ms) ishlaydi. Taymer uzilishi kvant tugaganini
 *     aniqlab, schedule() ni chaqiradi - bu PREEMPTION (majburiy almashtirish).
 *     Hech kim tayyor bo'lmasa - idle ishlaydi va CPU'ni `hlt` bilan uxlatadi.
 *
 *  3) BITTA QOIDA: schedule() FAQAT uzilishlar o'chiq (IF=0) holda chaqiriladi
 *     va IF=0 holda qaytadi. Jarayonlar jadvalini faqat IF=0 da o'zgartiramiz -
 *     bitta protsessorda bu "qulf" vazifasini bajaradi.
 *
 *  4) KANAL ORQALI UXLASH (Unix an'anasi): jarayon "kanal" (istalgan manzil)
 *     ustida uxlaydi, masalan klaviatura buferi manzilida. Hodisa bo'lganda
 *     proc_wakeup(o'sha manzil) shu kanalda uxlayotganlarni uyg'otadi.
 *
 *  5) ZOMBIE VA WAIT: tugagan jarayon darhol yo'qolmaydi - ota-onasi uning
 *     chiqish kodini wait() bilan olmaguncha ZOMBIE bo'lib turadi. Yadro steki
 *     ham shu paytgacha saqlanadi: jarayon O'Z stekida turib uni bo'shata olmaydi
 *     (oyog'i ostidagi yerni kesish!), shuning uchun buni boshqa jarayon qiladi.
 * ============================================================================= */
#include "proc/process.h"

#include "arch/cpu.h"
#include "arch/gdt.h"
#include "drivers/pit.h"
#include "fs/file.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/string.h"
#include "mm/pmm.h"
#include "mm/vmalloc.h"
#include "mm/vmm.h"

/* ABI (myos/abi.h) dagi holat raqamlari enum bilan mos bo'lishi SHART - ps
 * dasturi ularni ko'radi. Mos kelmasa, kompilyatsiya shu yerda to'xtaydi. */
_Static_assert(PROC_EMBRYO == MYOS_PROC_EMBRYO && PROC_READY == MYOS_PROC_READY &&
               PROC_RUNNING == MYOS_PROC_RUNNING && PROC_BLOCKED == MYOS_PROC_BLOCKED &&
               PROC_ZOMBIE == MYOS_PROC_ZOMBIE, "proc_state va ABI mos emas");

/* switch.asm */
extern void context_switch(uint64_t *old_rsp, uint64_t new_rsp);
extern void kthread_trampoline(void);

static struct process procs[MAX_PROCS];
struct process *current;
static struct process *idle;
static int next_pid = 1;

/* Taymer bo'yicha uxlayotganlar shu "kanal"da kutadi. */
static const char sleep_channel;

/* Yadro stekining eng yuqori manzili (stek pastga o'sadi). */
static inline uint64_t kstack_top(const struct process *p)
{
    return p->kstack_base + KSTACK_PAGES * PAGE_SIZE;
}

/* ---- Yaratish --------------------------------------------------------------- */

struct process *proc_alloc(const char *name)
{
    uint64_t flags = irq_save();
    struct process *p = NULL;
    for (int i = 1; i < MAX_PROCS; i++) {       /* 0 - idle, band */
        if (procs[i].state == PROC_UNUSED) {
            p = &procs[i];
            break;
        }
    }
    if (!p) {
        irq_restore(flags);
        return NULL;                    /* jadval to'la */
    }
    memset(p, 0, sizeof(*p));
    p->state = PROC_EMBRYO;             /* joyni band qildik, scheduler hali ko'rmaydi */
    p->pid = next_pid++;
    irq_restore(flags);

    strlcpy(p->name, name, sizeof(p->name));
    /* Yadro steki vmalloc'dan: atrofida HIMOYA SAHIFALARI bor. Stek to'lsa -
     * darhol page fault (keyin double fault), boshqa xotirani jim buzmaydi. */
    p->kstack_base = (uint64_t)vmalloc(KSTACK_PAGES * PAGE_SIZE);
    if (!p->kstack_base) {
        p->state = PROC_UNUSED;
        return NULL;
    }
    p->pml4 = vmm_kernel_pml4();        /* sukut bo'yicha - yadro manzil maydoni */
    p->parent = current;
    p->quantum_left = SCHED_QUANTUM;
    return p;
}

void proc_free(struct process *p)
{
    if (p->kstack_base)
        vfree((void *)p->kstack_base);
    if (p->is_user && p->pml4 && p->pml4 != vmm_kernel_pml4())
        vmm_destroy_address_space(p->pml4);
    p->state = PROC_UNUSED;
}

void proc_make_ready(struct process *p)
{
    uint64_t flags = irq_save();
    p->state = PROC_READY;
    irq_restore(flags);
}

int proc_create_kernel_thread(const char *name, int (*fn)(void *), void *arg)
{
    struct process *p = proc_alloc(name);
    if (!p)
        return -1;

    /* Yangi stekni shunday tayyorlaymiz: go'yo bu oqim context_switch() ichida
     * to'xtab turgan. context_switch 6 ta registrni pop qilib, `ret` qiladi ->
     * kthread_trampoline ga "qaytadi". R12 = arg, R13 = fn. */
    uint64_t *sp = (uint64_t *)kstack_top(p);
    *--sp = 0;                          /* soxta qaytish manzili (tekislash uchun) */
    *--sp = (uint64_t)kthread_trampoline;   /* context_switch dagi `ret` shu yerga sakraydi */
    *--sp = 0;                          /* RBX */
    *--sp = 0;                          /* RBP = 0: backtrace shu yerda to'xtaydi */
    *--sp = (uint64_t)arg;              /* R12 */
    *--sp = (uint64_t)fn;               /* R13 */
    *--sp = 0;                          /* R14 */
    *--sp = 0;                          /* R15 */
    p->kernel_rsp = (uint64_t)sp;

    int pid = p->pid;
    proc_make_ready(p);
    return pid;
}

/* ---- Scheduler -------------------------------------------------------------- */

/* Keyingi jarayonni tanlash: joriydan keyingisidan boshlab aylana bo'ylab
 * birinchi READY. Idle faqat hech kim bo'lmasa tanlanadi. */
static struct process *pick_next(void)
{
    int start = (int)(current - procs);
    for (int n = 1; n <= MAX_PROCS; n++) {
        struct process *p = &procs[(start + n) % MAX_PROCS];
        if (p != idle && p->state == PROC_READY)
            return p;
    }
    return idle;
}

void schedule(void)
{
    ASSERT(!cpu_interrupts_enabled());

    struct process *prev = current;
    struct process *next = pick_next();

    if (next == idle && prev != idle && prev->state == PROC_READY)
        next = prev;                    /* boshqa hech kim yo'q - o'zimiz davom etamiz */
    if (next == idle)
        idle->state = PROC_READY;       /* idle har doim ishlashga tayyor */

    next->state = PROC_RUNNING;
    next->quantum_left = SCHED_QUANTUM;
    if (next == prev)
        return;

    current = next;
    /* User rejimidan uzilish kelganda CPU shu stekka o'tadi (TSS.rsp0). */
    tss_set_kernel_stack(kstack_top(next));
    /* Manzil maydonini almashtiramiz. Yadro qismi hamma joyda bir xil, shuning
     * uchun keyingi instruksiya (va stek) "yo'qolmaydi". */
    vmm_switch(next->pml4);
    context_switch(&prev->kernel_rsp, next->kernel_rsp);
    /* Bu yerga ANCHA KEYIN, kimdir yana prev ga o'tganda qaytamiz. */
}

void proc_yield(void)
{
    uint64_t flags = irq_save();
    current->state = PROC_READY;
    schedule();
    irq_restore(flags);
}

/* Taymerning har bir tikida (uzilish kontekstida, IF=0) chaqiriladi. */
static void sched_tick(void)
{
    uint64_t now = timer_ticks();

    /* Vaqti kelgan uxlovchilarni uyg'otamiz. */
    for (int i = 0; i < MAX_PROCS; i++) {
        struct process *p = &procs[i];
        if (p->state == PROC_BLOCKED && p->wait_channel == &sleep_channel && p->wake_tick <= now) {
            p->wait_channel = NULL;
            p->state = PROC_READY;
        }
    }

    if (current == idle) {
        schedule();                     /* idle bo'lsak - tayyor bo'lgan har kimga joy beramiz */
        return;
    }
    current->cpu_ticks++;
    if (--current->quantum_left <= 0) { /* kvant tugadi - majburan almashtiramiz */
        current->state = PROC_READY;
        schedule();
    }
}

/* ZOMBIE bo'lgan va ota-onasi yo'q (yoki idle) jarayonlarni tozalash. */
static void reap_orphans(void)
{
    for (int i = 1; i < MAX_PROCS; i++) {
        struct process *p = &procs[i];
        if (p->state == PROC_ZOMBIE && (p->parent == NULL || p->parent == idle))
            proc_free(p);
    }
}

void proc_idle_loop(void)
{
    for (;;) {
        cpu_cli();
        reap_orphans();
        schedule();                     /* tayyor jarayon bo'lsa - unga o'tamiz */
        /* Hech kim tayyor emas: keyingi uzilishgacha uxlaymiz ("sti; hlt"
         * atomar - 02-uzilishlar.md ga qarang). */
        __asm__ volatile("sti; hlt" ::: "memory");
    }
}

void proc_init(void)
{
    /* kmain() ning o'zi 0-jarayonga aylanadi. Uning steki - boot.asm dagi stek,
     * shuning uchun kstack_base = 0 (uni hech qachon bo'shatmaymiz). */
    idle = &procs[0];
    memset(idle, 0, sizeof(*idle));
    idle->pid = 0;
    idle->state = PROC_RUNNING;
    strlcpy(idle->name, "idle", sizeof(idle->name));
    idle->pml4 = vmm_kernel_pml4();
    current = idle;

    timer_set_tick_callback(sched_tick);
    kprintf("[proc] Scheduler tayyor: round-robin, kvant = %d tik (%d ms)\n",
            SCHED_QUANTUM, SCHED_QUANTUM * 1000 / TIMER_HZ);
}

/* ---- Uxlash va uyg'otish ---------------------------------------------------- */

void proc_sleep_on(const void *channel)
{
    ASSERT(!cpu_interrupts_enabled());
    ASSERT(current != idle);            /* idle hech qachon uxlamasligi kerak */
    current->wait_channel = channel;
    current->state = PROC_BLOCKED;
    schedule();
    current->wait_channel = NULL;       /* uyg'ondik */
}

void proc_wakeup(const void *channel)
{
    uint64_t flags = irq_save();
    for (int i = 0; i < MAX_PROCS; i++) {
        struct process *p = &procs[i];
        if (p->state == PROC_BLOCKED && p->wait_channel == channel)
            p->state = PROC_READY;
    }
    irq_restore(flags);
}

void proc_sleep_ms(uint64_t ms)
{
    uint64_t ticks = (ms * TIMER_HZ + 999) / 1000;  /* yuqoriga yaxlitlash */
    uint64_t flags = irq_save();
    current->wake_tick = timer_ticks() + (ticks ? ticks : 1);
    proc_sleep_on(&sleep_channel);
    irq_restore(flags);
}

/* ---- Tugash va kutish ------------------------------------------------------- */

/* Jarayon tugaganda barcha ochiq fayllarni yopamiz - aks holda ular
 * (va ular egallagan xotira) abadiy "oqib" qoladi. */
static void proc_close_all_files(struct process *p)
{
    for (int fd = 0; fd < MAX_FDS; fd++) {
        if (p->files[fd]) {
            file_close(p->files[fd]);
            p->files[fd] = NULL;
        }
    }
}

void proc_exit(int code)
{
    ASSERT(current != idle);
    cpu_cli();

    struct process *me = current;
    me->exit_code = code;
    proc_close_all_files(me);

    /* User manzil maydonini DARHOL qaytaramiz (xotira ko'p bo'lishi mumkin).
     * Avval yadro maydoniga o'tamiz - joriy CR3 ni yo'q qilib bo'lmaydi. */
    if (me->is_user && me->pml4 != vmm_kernel_pml4()) {
        uint64_t old = me->pml4;
        me->pml4 = vmm_kernel_pml4();
        vmm_switch(me->pml4);
        vmm_destroy_address_space(old);
    }

    /* Bolalarimizni "yetim" qilamiz: tugaganlarini hozir tozalaymiz, qolganlari
     * tugaganda idle tozalaydi. */
    for (int i = 1; i < MAX_PROCS; i++) {
        struct process *child = &procs[i];
        if (child->state != PROC_UNUSED && child->parent == me) {
            child->parent = NULL;
            if (child->state == PROC_ZOMBIE)
                proc_free(child);
        }
    }

    me->state = PROC_ZOMBIE;
    if (me->parent)
        proc_wakeup(me->parent);        /* ota-ona wait() da o'z manzili ustida uxlaydi */
    schedule();                         /* ZOMBIE hech qachon tanlanmaydi - qaytmaymiz */
    panic("proc_exit: zombie jarayon qayta ishga tushdi");
}

int proc_wait(int pid, int *exit_code, bool nohang)
{
    uint64_t flags = irq_save();
    for (;;) {
        bool have_child = false;
        for (int i = 1; i < MAX_PROCS; i++) {
            struct process *p = &procs[i];
            if (p->state == PROC_UNUSED || p->parent != current)
                continue;
            if (pid != -1 && p->pid != pid)
                continue;
            have_child = true;
            if (p->state == PROC_ZOMBIE) {
                int found = p->pid;
                if (exit_code)
                    *exit_code = p->exit_code;
                proc_free(p);           /* yadro stekini endi xavfsiz bo'shatish mumkin */
                irq_restore(flags);
                return found;
            }
        }
        if (!have_child) {
            irq_restore(flags);
            return -1;                  /* bunday bola yo'q - abadiy kutmaymiz */
        }
        if (nohang) {
            irq_restore(flags);
            return 0;                   /* bolalar bor, lekin hali tugamagan */
        }
        if (current->killed) {          /* bizni kill() qilishdi - kutishni to'xtatamiz, */
            irq_restore(flags);         /* aks holda uyg'onib, yana uxlab qolardik */
            return -1;
        }
        proc_sleep_on(current);         /* bola exit() qilganda bizni uyg'otadi */
    }
}

int proc_kill(int pid)
{
    uint64_t flags = irq_save();
    for (int i = 1; i < MAX_PROCS; i++) {
        struct process *p = &procs[i];
        if (p->pid == pid && p->state != PROC_UNUSED && p->state != PROC_ZOMBIE) {
            /* Boshqa jarayonni to'g'ridan-to'g'ri o'ldirib bo'lmaydi - u yadro
             * ichida qulf yoki resurs ushlab turgan bo'lishi mumkin. Shuning
             * uchun faqat BELGI qo'yamiz; jarayon user rejimiga qaytish
             * arafasida buni ko'rib, o'zi exit() qiladi. */
            p->killed = true;
            if (p->state == PROC_BLOCKED)
                p->state = PROC_READY;  /* uyg'otamiz, toki belgini ko'rsin */
            irq_restore(flags);
            return 0;
        }
    }
    irq_restore(flags);
    return -1;
}

/* Jarayon ishlatayotgan user xotira sahifalari. */
static uint64_t user_pages(const struct process *p)
{
    if (!p->is_user || p->pml4 == vmm_kernel_pml4())
        return 0;
    return vmm_count_user_pages(p->pml4);
}

int proc_list(struct myos_proc_info *out, int max)
{
    int n = 0;
    uint64_t flags = irq_save();
    for (int i = 0; i < MAX_PROCS && n < max; i++) {
        struct process *p = &procs[i];
        if (p->state == PROC_UNUSED)
            continue;
        out[n].pid = p->pid;
        out[n].ppid = p->parent ? p->parent->pid : -1;
        out[n].state = p->state;
        out[n].is_user = p->is_user;
        out[n].cpu_ticks = p->cpu_ticks;
        out[n].mem_pages = user_pages(p);
        strlcpy(out[n].name, p->name, sizeof(out[n].name));
        n++;
    }
    irq_restore(flags);
    return n;
}
