/* =============================================================================
 *  proc/process.c - jarayonlar, SMP scheduler, uxlash/uyg'otish, exit/wait
 * =============================================================================
 *
 *  SMP (ko'p yadroli) SCHEDULER - xv6 (MIT) modeli:
 *
 *    Har bir CPU'da o'z SCHEDULER TSIKLI bor (scheduler_loop, CPU'ning idle
 *    stekida). Jarayonlar to'g'ridan-to'g'ri bir-biriga emas, har doim scheduler
 *    orqali o'tadi:
 *
 *        jarayon A --sched()--> CPU scheduler tsikli --context_switch--> jarayon B
 *
 *    YAGONA QULF: proc_lock. Jarayonning holati (state), kanali, ota-onasi faqat
 *    shu qulf ostida o'zgaradi. Muhim hiyla: qulf context_switch DAVOMIDA
 *    USHLANGAN bo'ladi va uni "narigi tomon" qo'yib yuboradi:
 *
 *      A: lock(proc_lock); A.state = READY; sched() ---+
 *                                                      | context_switch
 *      scheduler: ... <--------------------------------+
 *                 unlock(proc_lock)
 *
 *    Nega shunday? Aks holda A ning holati READY bo'lgan, lekin u hali o'z
 *    stekida turgan paytda BOSHQA CPU uni olib, ikkalasi BITTA stekda ishlab
 *    ketardi - tizim shu zahoti buziladi. Qulf "A stekdan to'liq chiqmaguncha
 *    uni hech kim olmasin" kafolatini beradi.
 *
 *  UXLASH/UYG'OTISH (proc_sleep): shartni himoya qiluvchi qulf `lk` bilan
 *    chaqiriladi. proc_lock ni olib, keyin `lk` ni qo'yib yuboramiz. wakeup ham
 *    proc_lock ni oladi - demak "shart bajarilmadi -> uxlayapman" orasiga
 *    uyg'otish tusha olmaydi (lost wakeup yo'q).
 *
 *  YADRO PREEMPTIV: taymer uzilishi yadro kodini ham to'xtatishi mumkin -
 *    lekin faqat IF=1 bo'lganda, ya'ni hech qanday spinlock ushlanmaganda
 *    (spin_lock uzilishlarni o'chiradi). Shuning uchun xavfsiz.
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

_Static_assert(PROC_EMBRYO == MYOS_PROC_EMBRYO && PROC_READY == MYOS_PROC_READY &&
               PROC_RUNNING == MYOS_PROC_RUNNING && PROC_BLOCKED == MYOS_PROC_BLOCKED &&
               PROC_ZOMBIE == MYOS_PROC_ZOMBIE, "proc_state va ABI mos emas");

/* switch.asm */
extern void context_switch(uint64_t *old_rsp, uint64_t new_rsp);
extern void new_proc_start(void);
extern void kthread_start(void);

spinlock_t proc_lock = SPINLOCK_INIT("proc");

static struct process procs[MAX_PROCS];
static struct process idle_procs[MAX_CPUS];
static int next_pid = 1;
static int rr_next;                     /* round-robin: qidiruvni shu indeksdan boshlash */

static const char sleep_channel;        /* taymer bo'yicha uxlovchilar kanali */

static inline uint64_t kstack_top(const struct process *p)
{
    return p->kstack_base + KSTACK_PAGES * PAGE_SIZE;
}

/* ---- Yaratish --------------------------------------------------------------- */

struct process *proc_alloc(const char *name)
{
    struct process *p = NULL;
    spin_lock(&proc_lock);
    for (int i = 0; i < MAX_PROCS; i++) {
        if (procs[i].state == PROC_UNUSED) {
            p = &procs[i];
            break;
        }
    }
    if (!p) {
        spin_unlock(&proc_lock);
        return NULL;
    }
    memset(p, 0, sizeof(*p));
    p->state = PROC_EMBRYO;             /* joy band, lekin scheduler hali ko'rmaydi */
    p->pid = next_pid++;
    p->parent = current;
    spin_unlock(&proc_lock);

    strlcpy(p->name, name, sizeof(p->name));
    /* Yadro steki vmalloc'dan: atrofida HIMOYA SAHIFALARI. Stek to'lsa - darhol
     * page fault (keyin double fault), boshqa xotirani jim buzmaydi. */
    p->kstack_base = (uint64_t)vmalloc(KSTACK_PAGES * PAGE_SIZE);
    if (!p->kstack_base) {
        spin_lock(&proc_lock);
        p->state = PROC_UNUSED;
        spin_unlock(&proc_lock);
        return NULL;
    }
    p->pml4 = vmm_kernel_pml4();
    p->quantum_left = SCHED_QUANTUM;
    p->last_cpu = -1;
    return p;
}

/* Resurslarni bo'shatish. Hech qanday spinlock USHLANMAGAN holda chaqiriladi:
 * vfree boshqa CPU'larga TLB tozalash so'rovini yuborib, javob kutishi mumkin. */
static void proc_release_resources(struct process *p)
{
    if (p->kstack_base) {
        vfree((void *)p->kstack_base);
        p->kstack_base = 0;
    }
    if (p->is_user && p->pml4 && p->pml4 != vmm_kernel_pml4()) {
        vmm_destroy_address_space(p->pml4);
        p->pml4 = 0;
    }
}

void proc_free(struct process *p)
{
    proc_release_resources(p);
    spin_lock(&proc_lock);
    p->state = PROC_UNUSED;
    spin_unlock(&proc_lock);
}

void proc_make_ready(struct process *p)
{
    spin_lock(&proc_lock);
    p->state = PROC_READY;
    spin_unlock(&proc_lock);
}

/* Yangi jarayon birinchi marta ishga tushganda (switch.asm: new_proc_start). */
void proc_first_run(void);
void proc_first_run(void)
{
    spin_unlock(&proc_lock);            /* scheduler uni ushlab turgan edi */
}

int proc_create_kernel_thread(const char *name, int (*fn)(void *), void *arg)
{
    struct process *p = proc_alloc(name);
    if (!p)
        return -1;
    /* Stek: go'yo bu oqim context_switch() ichida to'xtab turgan va `ret`
     * new_proc_start ga qaytadi. R13 = davom (kthread_start), R12 = arg, R14 = fn.
     * Qaytish manzili (top - 24) da: `ret` dan keyin RSP 16 ga tekis bo'ladi. */
    uint64_t *sp = (uint64_t *)kstack_top(p);
    *--sp = 0;
    *--sp = 0;
    *--sp = (uint64_t)new_proc_start;
    *--sp = 0;                          /* RBX */
    *--sp = 0;                          /* RBP */
    *--sp = (uint64_t)arg;              /* R12 */
    *--sp = (uint64_t)kthread_start;    /* R13 */
    *--sp = (uint64_t)fn;               /* R14 */
    *--sp = 0;                          /* R15 */
    p->kernel_rsp = (uint64_t)sp;
    int pid = p->pid;
    proc_make_ready(p);
    return pid;
}

/* ---- Scheduler -------------------------------------------------------------- */

/* Joriy jarayondan scheduler tsikliga o'tish. proc_lock ushlangan, BOSHQA
 * qulflar ushlanmagan (ncli == 1) va holat allaqachon RUNNING emas bo'lishi shart. */
static void sched(void)
{
    struct cpu *c = this_cpu();
    struct process *p = current;
    if (!spin_holding(&proc_lock))
        panic("sched: proc_lock ushlanmagan");
    if (c->ncli != 1)
        panic("sched: boshqa qulflar ham ushlangan (ncli=%d) - uxlash taqiqlangan", c->ncli);
    if (p->state == PROC_RUNNING)
        panic("sched: jarayon hali RUNNING");
    if (cpu_interrupts_enabled())
        panic("sched: uzilishlar yoqilgan");

    /* intena - CPU'ga emas, JARAYONGA tegishli: jarayon boshqa CPU'da davom
     * etishi mumkin, shuning uchun uni o'z stekida saqlab, qaytganda tiklaymiz. */
    bool intena = c->intena;
    context_switch(&p->kernel_rsp, c->scheduler_rsp);
    this_cpu()->intena = intena;
}

/* Keyingi tayyor jarayon (proc_lock ushlangan). */
static struct process *pick_next(void)
{
    for (int n = 0; n < MAX_PROCS; n++) {
        int i = (rr_next + n) % MAX_PROCS;
        if (procs[i].state == PROC_READY) {
            rr_next = (i + 1) % MAX_PROCS;
            return &procs[i];
        }
    }
    return NULL;
}

/* Ota-onasi yo'q zombie'larni tozalash (faqat CPU0 bo'sh turganda). */
static void reap_orphans(void)
{
    for (int i = 0; i < MAX_PROCS; i++) {
        struct process *p = &procs[i];
        spin_lock(&proc_lock);
        bool orphan = p->state == PROC_ZOMBIE && p->parent == NULL;
        if (orphan)
            p->state = PROC_DEAD;
        spin_unlock(&proc_lock);
        if (orphan)
            proc_free(p);
    }
}

void scheduler_loop(void)
{
    struct cpu *c = this_cpu();
    c->cur_proc = c->idle;
    for (;;) {
        cpu_sti();                      /* qurilma uzilishlari kelib tursin */
        spin_lock(&proc_lock);
        struct process *p = pick_next();
        if (p) {
            p->state = PROC_RUNNING;
            p->quantum_left = SCHED_QUANTUM;
            p->last_cpu = c->id;
            c->cur_proc = p;
            tss_set_kernel_stack(kstack_top(p));    /* ring3 -> ring0 va syscall steki */
            vmm_switch(p->pml4);
            context_switch(&c->scheduler_rsp, p->kernel_rsp);
            /* p CPU'ni qaytarib berdi (sched() orqali), proc_lock ushlangan. */
            c->cur_proc = c->idle;
        }
        spin_unlock(&proc_lock);
        if (!p) {
            if (c->id == 0)
                reap_orphans();
            /* Hech kim tayyor emas: keyingi uzilishgacha uxlaymiz. "sti; hlt"
             * atomar juftlik - uzilish ular orasiga tusha olmaydi. */
            __asm__ volatile("sti; hlt" ::: "memory");
        }
    }
}

void proc_yield(void)
{
    spin_lock(&proc_lock);
    current->state = PROC_READY;
    sched();
    spin_unlock(&proc_lock);
}

/* Har bir CPU'ning taymer uzilishida (IF=0, uzilish kontekstida). */
void sched_tick(void)
{
    struct cpu *c = this_cpu();
    c->ticks++;

    if (c->id == 0) {                   /* uxlovchilarni faqat bitta CPU uyg'otadi */
        uint64_t now = timer_ticks();
        spin_lock(&proc_lock);
        for (int i = 0; i < MAX_PROCS; i++) {
            struct process *p = &procs[i];
            if (p->state == PROC_BLOCKED && p->wait_channel == &sleep_channel &&
                p->wake_tick <= now)
                p->state = PROC_READY;
        }
        spin_unlock(&proc_lock);
    }

    struct process *p = current;
    if (p == c->idle) {
        c->idle_ticks++;
        return;
    }
    p->cpu_ticks++;
    if (--p->quantum_left <= 0)         /* kvant tugadi - majburan almashtiramiz */
        proc_yield();
}

void proc_init_cpu(struct cpu *c)
{
    struct process *idle = &idle_procs[c->id];
    memset(idle, 0, sizeof(*idle));
    idle->pid = 0;
    idle->state = PROC_RUNNING;
    idle->is_idle = true;
    idle->last_cpu = c->id;
    idle->pml4 = vmm_kernel_pml4();
    ksnprintf(idle->name, sizeof(idle->name), "idle/%d", c->id);
    c->idle = idle;
    c->cur_proc = idle;
}

void proc_init(void)
{
    proc_init_cpu(this_cpu());
    kprintf("[proc] SMP scheduler tayyor: round-robin, kvant = %d tik (%d ms), max %d jarayon\n",
            SCHED_QUANTUM, SCHED_QUANTUM * 1000 / TIMER_HZ, MAX_PROCS);
}

/* ---- Uxlash va uyg'otish ---------------------------------------------------- */

void proc_sleep(const void *channel, spinlock_t *lk)
{
    struct process *p = current;
    if (p->is_idle)
        panic("proc_sleep: idle uxlay olmaydi");
    /* proc_lock ni olgunimizcha lk ni QO'YMAYMIZ: shunda wakeup (u ham
     * proc_lock ni oladi) bizni o'tkazib yubora olmaydi. */
    if (lk != &proc_lock) {
        spin_lock(&proc_lock);
        spin_unlock(lk);
    }
    p->wait_channel = channel;
    p->state = PROC_BLOCKED;
    sched();
    p->wait_channel = NULL;
    if (lk != &proc_lock) {
        spin_unlock(&proc_lock);
        spin_lock(lk);
    }
}

/* proc_lock ushlangan holda. */
static void wakeup_locked(const void *channel)
{
    for (int i = 0; i < MAX_PROCS; i++) {
        struct process *p = &procs[i];
        if (p->state == PROC_BLOCKED && p->wait_channel == channel)
            p->state = PROC_READY;
    }
}

void proc_wakeup(const void *channel)
{
    spin_lock(&proc_lock);
    wakeup_locked(channel);
    spin_unlock(&proc_lock);
}

void proc_sleep_ms(uint64_t ms)
{
    uint64_t ticks = (ms * TIMER_HZ + 999) / 1000;
    spin_lock(&proc_lock);
    current->wake_tick = timer_ticks() + (ticks ? ticks : 1);
    while (timer_ticks() < current->wake_tick && !current->killed)
        proc_sleep(&sleep_channel, &proc_lock);
    spin_unlock(&proc_lock);
}

/* ---- Tugash va kutish ------------------------------------------------------- */

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
    struct process *me = current;
    if (me->is_idle)
        panic("proc_exit: idle tugay olmaydi");
    cpu_sti();                          /* resurslarni bo'shatish qulfsiz, IF=1 da */
    proc_close_all_files(me);

    /* User xotirasini DARHOL qaytaramiz (ko'p bo'lishi mumkin). Avval yadro
     * maydoniga o'tamiz: joriy CR3 ni yo'q qilib bo'lmaydi. */
    if (me->is_user && me->pml4 != vmm_kernel_pml4()) {
        /* pml4 ni QULF OSTIDA almashtiramiz: proc_list (ps) jadvallarni qulf
         * ostida yuradi va bo'shatilgan jadvallarga tegib qolmasligi kerak. */
        spin_lock(&proc_lock);
        uint64_t old = me->pml4;
        me->pml4 = vmm_kernel_pml4();
        spin_unlock(&proc_lock);
        vmm_switch(me->pml4);
        vmm_destroy_address_space(old);
    }

    spin_lock(&proc_lock);
    me->exit_code = code;
    /* Bolalarimizni yetim qilamiz; zombie bo'lganlarini CPU0 tozalaydi. */
    for (int i = 0; i < MAX_PROCS; i++)
        if (procs[i].state != PROC_UNUSED && procs[i].parent == me)
            procs[i].parent = NULL;
    me->state = PROC_ZOMBIE;
    if (me->parent)
        wakeup_locked(me->parent);      /* ota-ona wait() da o'z manzili ustida uxlaydi */
    sched();                            /* ZOMBIE hech qachon tanlanmaydi */
    panic("proc_exit: zombie qayta ishga tushdi");
}

int proc_wait(int pid, int *exit_code, bool nohang)
{
    spin_lock(&proc_lock);
    for (;;) {
        bool have_child = false;
        struct process *found = NULL;
        for (int i = 0; i < MAX_PROCS; i++) {
            struct process *p = &procs[i];
            if (p->state == PROC_UNUSED || p->parent != current)
                continue;
            if (pid != -1 && p->pid != pid)
                continue;
            have_child = true;
            if (p->state == PROC_ZOMBIE) {
                found = p;
                break;
            }
        }
        if (found) {
            int fpid = found->pid;
            if (exit_code)
                *exit_code = found->exit_code;
            found->state = PROC_DEAD;   /* hech kim tegmasin */
            spin_unlock(&proc_lock);
            proc_free(found);           /* qulfsiz: vfree TLB shootdown qilishi mumkin */
            return fpid;
        }
        if (!have_child || current->killed) {
            spin_unlock(&proc_lock);
            return -1;
        }
        if (nohang) {
            spin_unlock(&proc_lock);
            return 0;
        }
        proc_sleep(current, &proc_lock);
    }
}

int proc_kill(int pid)
{
    spin_lock(&proc_lock);
    for (int i = 0; i < MAX_PROCS; i++) {
        struct process *p = &procs[i];
        if (p->pid == pid && p->state != PROC_UNUSED && p->state != PROC_ZOMBIE &&
            p->state != PROC_DEAD) {
            /* Boshqa jarayonni zo'rlab to'xtatmaymiz (u yadroda resurs ushlab turgan
             * bo'lishi mumkin) - faqat BELGI. U user rejimiga qaytishda o'zi chiqadi. */
            p->killed = true;
            if (p->state == PROC_BLOCKED)
                p->state = PROC_READY;
            spin_unlock(&proc_lock);
            return 0;
        }
    }
    spin_unlock(&proc_lock);
    return -1;
}

static void fill_info(struct myos_proc_info *o, const struct process *p)
{
    o->pid = p->pid;
    o->ppid = p->parent ? p->parent->pid : -1;
    o->state = p->state == PROC_DEAD ? PROC_ZOMBIE : p->state;
    o->is_user = p->is_user;
    o->cpu_ticks = p->cpu_ticks;
    o->mem_pages = 0;
    strlcpy(o->name, p->name, sizeof(o->name));
}

int proc_list(struct myos_proc_info *out, int max)
{
    int n = 0;
    spin_lock(&proc_lock);
    for (int i = 0; i < ncpus && n < max; i++) {
        if (cpus[i] && cpus[i]->idle) {
            fill_info(&out[n], cpus[i]->idle);
            out[n++].cpu_ticks = cpus[i]->idle_ticks;
        }
    }
    for (int i = 0; i < MAX_PROCS && n < max; i++) {
        struct process *p = &procs[i];
        if (p->state == PROC_UNUSED)
            continue;
        fill_info(&out[n], p);
        /* Sahifa jadvallarini qulf OSTIDA yuramiz: proc_exit pml4 ni faqat
         * qulf ostida almashtiradi, shuning uchun jadval bo'shatilib ketmaydi. */
        if (p->is_user && p->pml4 != vmm_kernel_pml4())
            out[n].mem_pages = vmm_count_user_pages(p->pml4);
        n++;
    }
    spin_unlock(&proc_lock);
    return n;
}
