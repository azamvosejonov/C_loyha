/* =============================================================================
 *  proc/signal.c - SIGNALLAR: jarayonlarga asinxron xabarlar
 * =============================================================================
 *
 *  Signal - bu "dasturiy uzilish". Apparat uzilishi CPU'ni to'xtatib handler'ga
 *  sakratganidek, signal ham JARAYONNI qayerda bo'lmasin to'xtatib, uning
 *  handler funksiyasiga sakratadi. Manbalari:
 *
 *    Ctrl-C (terminal)     -> SIGINT  -> oldingi plandagi guruhga
 *    kill 123              -> SIGTERM
 *    NULL ga yozish (#PF)  -> SIGSEGV -> xato qilgan jarayonning o'ziga
 *    bola tugadi           -> SIGCHLD -> otaga
 *    o'quvchisiz pipe      -> SIGPIPE
 *    alarm(5)              -> SIGALRM (5 soniyadan keyin)
 *
 *  UCH BOSQICH:
 *    1. YUBORISH (signal_send_locked): sig_pending ga bit qo'yiladi, jarayon
 *       uxlayotgan bo'lsa - uyg'otiladi (read() -EINTR bilan qaytadi).
 *    2. KUTISH: signal darhol ishlamaydi. Jarayon yadrodan user rejimiga
 *       QAYTAYOTGANDA (syscall yoki uzilish oxirida) tekshiriladi - bu yadroda
 *       hech qanday qulf ushlanmagan xavfsiz nuqta.
 *    3. YETKAZISH (signal_deliver): standart amal (tugash / to'xtash /
 *       e'tiborsiz), yoki handler: user stekiga "signal freymi" yoziladi
 *       (to'xtatilgan joydagi BARCHA registrlar) va RIP = handler.
 *
 *  HANDLER'DAN QAYTISH:
 *
 *      user steki (pastga o'sadi)
 *      ┌──────────────────────┐
 *      │ ... dastur steki ... │ <- to'xtatilgan RSP
 *      │ 128 bayt "red zone"  │    (x86-64 ABI: funksiya undan ogohlantirishsiz foydalanadi)
 *      ├──────────────────────┤
 *      │ saqlangan registrlar │ <- sigframe.ctx (RIP, RSP, RAX ... niqob)
 *      │ qaytish manzili      │ <- __restore_rt (libc)   <- handler kirishidagi RSP
 *      └──────────────────────┘
 *
 *  handler `ret` qiladi -> libc'dagi __restore_rt -> sigreturn() syscall ->
 *  yadro saqlangan registrlarni freymga qaytaradi -> dastur to'xtagan joyidan
 *  davom etadi, hech narsa sezmasdan. Linux x86-64 da ham aynan shunday.
 * ============================================================================= */
#include "proc/signal.h"

#include "arch/interrupts.h"
#include "drivers/pit.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "proc/process.h"
#include "sys/uaccess.h"

#define BIT(s)        (1u << (s))
#define UNBLOCKABLE   (BIT(SIGKILL) | BIT(SIGSTOP))
#define STOP_SIGNALS  (BIT(SIGSTOP) | BIT(SIGTSTP) | BIT(SIGTTIN) | BIT(SIGTTOU))

/* Qaysi syscall'lar -EINTR dan keyin qayta boshlanishi mumkin (SA_RESTART). */
#define FORCE_IRET_VECTOR 0x81          /* syscall_entry.asm: sysret emas, iretq bilan qaytish */

static bool default_ignore(int sig)
{
    return sig == SIGCHLD || sig == SIGCONT || sig == SIGWINCH || sig == 23 /* SIGURG */;
}

static bool default_stop(int sig)
{
    return (STOP_SIGNALS & BIT(sig)) != 0;
}

/* ---- Yuborish --------------------------------------------------------------- */

void signal_send_locked(struct process *p, int sig)
{
    if (sig <= 0 || sig >= NSIG || !p->is_user || p->is_idle ||
        p->state == PROC_UNUSED || p->state == PROC_ZOMBIE || p->state == PROC_DEAD)
        return;
    uint32_t bit = BIT(sig);

    if (sig == SIGKILL)
        p->killed = true;
    if (sig == SIGCONT) {
        /* SIGCONT har qanday holatda (hatto e'tiborsiz qoldirilgan bo'lsa ham)
         * to'xtagan jarayonni davom ettiradi va kutayotgan to'xtash signallarini bekor qiladi. */
        p->sig_pending &= ~STOP_SIGNALS;
        if (p->state == PROC_STOPPED)
            p->state = PROC_READY;
    }
    if (default_stop(sig))
        p->sig_pending &= ~BIT(SIGCONT);

    uint64_t h = p->sig_actions[sig].sa_handler;
    if (!(bit & UNBLOCKABLE) && !(p->sig_blocked & bit)) {
        /* E'tiborsiz qoldiriladigan signal umuman navbatga qo'yilmaydi (POSIX). */
        if (h == SIG_IGN || (h == SIG_DFL && default_ignore(sig)))
            return;
    }
    p->sig_pending |= bit;
    if (!(p->sig_blocked & bit) || (bit & UNBLOCKABLE)) {
        if (p->state == PROC_BLOCKED)
            proc_wake_locked(p);        /* read/wait/sleep ichidan -EINTR bilan chiqadi */
        else if (p->state == PROC_STOPPED && sig == SIGKILL)
            p->state = PROC_READY;      /* o'lishi uchun ishga tushishi kerak */
    }
}

int signal_send_pgrp_locked(int pgid, int sig)
{
    int n = 0;
    if (pgid <= 0)
        return 0;
    for (int i = 0; i < MAX_PROCS; i++) {
        struct process *p = proc_slot(i);
        if (p->state != PROC_UNUSED && p->is_user && p->pgid == pgid) {
            signal_send_locked(p, sig);
            n++;
        }
    }
    return n;
}

void signal_send_pgrp(int pgid, int sig)
{
    spin_lock(&proc_lock);
    signal_send_pgrp_locked(pgid, sig);
    spin_unlock(&proc_lock);
}

static bool alive(const struct process *p)
{
    return p->state != PROC_UNUSED && p->state != PROC_ZOMBIE && p->state != PROC_DEAD;
}

int signal_kill(int pid, int sig)
{
    if (sig < 0 || sig >= NSIG)
        return -EINVAL;
    int err = -ESRCH;
    spin_lock(&proc_lock);
    if (pid > 0) {
        struct process *p = proc_find_locked(pid);
        if (p && alive(p)) {
            if (!p->is_user) {
                err = -EPERM;           /* yadro oqimlariga signal yo'q */
            } else {
                if (sig)
                    signal_send_locked(p, sig);
                err = 0;
            }
        }
    } else {
        int pgid = pid == 0 ? current->pgid : -pid;
        for (int i = 0; i < MAX_PROCS; i++) {
            struct process *p = proc_slot(i);
            if (!alive(p) || !p->is_user)
                continue;
            if (pid == -1) {
                /* "hamma" - o'zimiz va init (yadro ishga tushirgan birinchi dastur) bundan mustasno */
                if (p == current || !p->parent || !p->parent->is_user)
                    continue;
            } else if (p->pgid != pgid) {
                continue;
            }
            if (sig)
                signal_send_locked(p, sig);
            err = 0;
        }
    }
    spin_unlock(&proc_lock);
    return err;
}

bool signal_interrupted(const struct process *p)
{
    return p->killed || (p->sig_pending & ~p->sig_blocked) != 0;
}

/* Keyingi yetkaziladigan signal (eng kichik raqam; SIGKILL birinchi). */
static int next_signal(const struct process *p)
{
    uint32_t ready = p->sig_pending & ~p->sig_blocked;
    if (p->killed)
        return SIGKILL;
    if (!ready)
        return 0;
    if (ready & BIT(SIGKILL))
        return SIGKILL;
    return __builtin_ctz(ready);
}

/* ---- Yetkazish -------------------------------------------------------------- */

/* Handler'ga o'tishda user stekiga yoziladigan freym. sigreturn uni o'qiydi. */
struct sigframe {
    uint64_t retaddr;                   /* sa_restorer: handler `ret` qilganda shu yerga */
    struct sigctx {
        uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
        uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
        uint64_t rip, rflags, rsp;
        uint64_t mask;                  /* handlerdan oldingi bloklash niqobi */
        uint64_t sig;
    } ctx;
};

static int setup_frame(struct interrupt_frame *f, int sig, const struct myos_sigaction *act)
{
    /* >>> LAB setup_frame - vazifa: labs/README.md */
    struct process *p = current;
    if (!(act->sa_flags & SA_RESTORER) || !act->sa_restorer)
        return -1;                      /* qaytish yo'li yo'q - handler'ni chaqira olmaymiz */

    struct sigframe fr;
    fr.retaddr = act->sa_restorer;
    fr.ctx.r15 = f->r15, fr.ctx.r14 = f->r14, fr.ctx.r13 = f->r13, fr.ctx.r12 = f->r12;
    fr.ctx.r11 = f->r11, fr.ctx.r10 = f->r10, fr.ctx.r9 = f->r9, fr.ctx.r8 = f->r8;
    fr.ctx.rbp = f->rbp, fr.ctx.rdi = f->rdi, fr.ctx.rsi = f->rsi, fr.ctx.rdx = f->rdx;
    fr.ctx.rcx = f->rcx, fr.ctx.rbx = f->rbx, fr.ctx.rax = f->rax;
    fr.ctx.rip = f->rip, fr.ctx.rflags = f->rflags, fr.ctx.rsp = f->rsp;
    fr.ctx.mask = p->sig_blocked;
    fr.ctx.sig = (uint64_t)sig;

    /* Red zone'dan pastda; handler kirishida RSP % 16 == 8 bo'lishi kerak
     * (xuddi `call` qilingandek: qaytish manzili 16 ga tekis joydan 8 pastda). */
    uint64_t sp = f->rsp - 128 - sizeof(fr);
    sp = (sp & ~15ULL) - 8;
    if (copy_to_user(sp, &fr, sizeof(fr)))
        return -1;

    f->rip = act->sa_handler;
    f->rsp = sp;
    f->rdi = (uint64_t)sig;             /* void handler(int sig) */
    f->rsi = 0;                         /* siginfo_t * - hozircha yo'q */
    f->rdx = 0;
    f->rax = 0;
    f->rflags &= ~((1ULL << 8) | (1ULL << 10));     /* TF va DF o'chiq (ABI) */

    spin_lock(&proc_lock);
    p->sig_blocked |= (uint32_t)act->sa_mask & ~UNBLOCKABLE;
    if (!(act->sa_flags & SA_NODEFER))
        p->sig_blocked |= BIT(sig);     /* handler ichida shu signal qayta kelmasin */
    if (act->sa_flags & SA_RESETHAND)
        p->sig_actions[sig].sa_handler = SIG_DFL;
    spin_unlock(&proc_lock);
    return 0;
    /* <<< LAB setup_frame */
}

void signal_deliver(struct interrupt_frame *f)
{
    struct process *p = current;
    if (!p->is_user)
        return;
    for (;;) {
        if (p->killed)
            proc_exit_signal(SIGKILL);
        spin_lock(&proc_lock);
        int sig = next_signal(p);
        if (!sig) {
            spin_unlock(&proc_lock);
            return;
        }
        p->sig_pending &= ~BIT(sig);
        struct myos_sigaction act = p->sig_actions[sig];
        spin_unlock(&proc_lock);

        if (act.sa_handler == SIG_IGN)
            continue;
        if (act.sa_handler == SIG_DFL) {
            if (default_ignore(sig))
                continue;
            if (default_stop(sig)) {
                proc_stop_self(sig);    /* SIGCONT kelguncha shu yerda */
                continue;
            }
            proc_exit_signal(sig);      /* qolgan hammasi: tugash */
        }
        if (setup_frame(f, sig, &act) < 0) {
            kprintf("[kernel] '%s' (pid %d): signal %d handler'ini chaqirib bo'lmadi\n",
                    p->name, p->pid, sig);
            proc_exit_signal(SIGSEGV);
        }
        return;                         /* bitta handler; qolganlari keyingi qaytishda */
    }
}

bool signal_should_restart(uint64_t nr)
{
    if (nr != SYS_READ && nr != SYS_WRITE && nr != SYS_WAIT && nr != SYS_IOCTL &&
        nr != SYS_SLEEP)
        return false;
    struct process *p = current;
    spin_lock(&proc_lock);
    int sig = next_signal(p);
    bool restart = false;
    if (sig && sig != SIGKILL) {
        const struct myos_sigaction *a = &p->sig_actions[sig];
        if (a->sa_handler == SIG_DFL)
            restart = default_stop(sig);    /* Ctrl-Z: SIGCONT dan keyin davom etadi */
        else if (a->sa_handler != SIG_IGN && nr != SYS_SLEEP)
            restart = (a->sa_flags & SA_RESTART) != 0;  /* sleep handler'dan keyin EINTR (POSIX) */
    }
    spin_unlock(&proc_lock);
    return restart;
}

bool signal_force(int sig)
{
    struct process *p = current;
    spin_lock(&proc_lock);
    /* Xato tufayli signalni e'tiborsiz qoldirib yoki bloklab bo'lmaydi -
     * aks holda xato qilgan instruksiya cheksiz qayta bajarilardi. */
    if (p->sig_actions[sig].sa_handler == SIG_IGN || (p->sig_blocked & BIT(sig))) {
        p->sig_actions[sig].sa_handler = SIG_DFL;
        p->sig_blocked &= ~BIT(sig);
    }
    p->sig_pending |= BIT(sig);
    bool dfl = p->sig_actions[sig].sa_handler == SIG_DFL;
    spin_unlock(&proc_lock);
    return dfl;
}

void signal_exec_reset(struct process *p)
{
    for (int s = 1; s < NSIG; s++) {
        if (p->sig_actions[s].sa_handler != SIG_IGN) {
            memset(&p->sig_actions[s], 0, sizeof(p->sig_actions[s]));
        }
    }
}

/* ---- Syscall'lar ------------------------------------------------------------ */

int64_t sys_sigaction(uint64_t sig, uint64_t uact, uint64_t uold)
{
    if (sig == 0 || sig >= NSIG)
        return -EINVAL;
    struct process *p = current;
    struct myos_sigaction act;
    if (uact && copy_from_user(&act, uact, sizeof(act)))
        return -EFAULT;
    if (uact && (sig == SIGKILL || sig == SIGSTOP))
        return -EINVAL;                 /* bular ustidan nazorat - faqat yadroda */
    spin_lock(&proc_lock);
    struct myos_sigaction old = p->sig_actions[sig];
    if (uact) {
        act.sa_mask &= ~(uint64_t)UNBLOCKABLE;
        p->sig_actions[sig] = act;
        if (act.sa_handler == SIG_IGN || (act.sa_handler == SIG_DFL && default_ignore((int)sig)))
            p->sig_pending &= ~BIT(sig);    /* kutayotgani ham bekor */
    }
    spin_unlock(&proc_lock);
    if (uold && copy_to_user(uold, &old, sizeof(old)))
        return -EFAULT;
    return 0;
}

int64_t sys_sigprocmask(uint64_t how, uint64_t uset, uint64_t uold)
{
    struct process *p = current;
    uint64_t set = 0;
    if (uset && copy_from_user(&set, uset, sizeof(set)))
        return -EFAULT;
    spin_lock(&proc_lock);
    uint64_t old = p->sig_blocked;
    if (uset) {
        uint32_t s = (uint32_t)set & ~UNBLOCKABLE;
        if (how == SIG_BLOCK)
            p->sig_blocked |= s;
        else if (how == SIG_UNBLOCK)
            p->sig_blocked &= ~s;
        else if (how == SIG_SETMASK)
            p->sig_blocked = s;
        else {
            spin_unlock(&proc_lock);
            return -EINVAL;
        }
    }
    spin_unlock(&proc_lock);
    if (uold && copy_to_user(uold, &old, sizeof(old)))
        return -EFAULT;
    return 0;
}

int64_t sys_sigreturn(struct interrupt_frame *f)
{
    /* >>> LAB sys_sigreturn - vazifa: labs/README.md */
    /* Handler `ret` qildi: RSP endi sigframe.ctx ga ko'rsatadi. */
    struct sigctx c;
    if (copy_from_user(&c, f->rsp, sizeof(c))) {
        kprintf("[kernel] '%s': buzilgan signal freymi\n", current->name);
        proc_exit_signal(SIGSEGV);
    }
    /* Freym USER xotirasida edi - dastur uni o'zgartirgan bo'lishi mumkin.
     * Hech narsaga ishonmaymiz: RIP user manzili bo'lsin, RFLAGS da faqat
     * arifmetik bayroqlar (IOPL, IF ni o'zgartirishga ruxsat yo'q). */
    if (c.rip >= 0x0000800000000000ULL || c.rsp >= 0x0000800000000000ULL) {
        kprintf("[kernel] '%s': sigreturn - noto'g'ri manzil\n", current->name);
        proc_exit_signal(SIGSEGV);
    }
    /* CF PF AF ZF SF DF OF (bitlar 0,2,4,6,7,10,11). TF (8) ham yo'q: dastur
     * sigreturn orqali o'zini "qadamma-qadam" rejimga qo'ya olmasin. */
    const uint64_t user_flags = 0xCD5;
    f->r15 = c.r15, f->r14 = c.r14, f->r13 = c.r13, f->r12 = c.r12;
    f->r11 = c.r11, f->r10 = c.r10, f->r9 = c.r9, f->r8 = c.r8;
    f->rbp = c.rbp, f->rdi = c.rdi, f->rsi = c.rsi, f->rdx = c.rdx;
    f->rcx = c.rcx, f->rbx = c.rbx;
    f->rip = c.rip;
    f->rsp = c.rsp;
    f->rflags = (c.rflags & user_flags) | (1ULL << 9) | (1ULL << 1);    /* IF=1, 1-bit doim 1 */
    f->vector = FORCE_IRET_VECTOR;      /* RCX va R11 ham tiklansin: sysret ularni buzadi */
    spin_lock(&proc_lock);
    current->sig_blocked = (uint32_t)c.mask & ~UNBLOCKABLE;
    spin_unlock(&proc_lock);
    return (int64_t)c.rax;              /* dispatcher buni f->rax ga yozadi */
    /* <<< LAB sys_sigreturn */
}

int64_t sys_setpgid(int64_t pid, int64_t pgid)
{
    struct process *me = current;
    if (pgid < 0)
        return -EINVAL;
    spin_lock(&proc_lock);
    struct process *p = pid == 0 ? me : proc_find_locked((int)pid);
    int64_t err = 0;
    if (!p || !alive(p) || (p != me && p->parent != me))
        err = -ESRCH;                   /* faqat o'zimiz yoki bolamiz */
    else if (p->sid != me->sid || p->sid == p->pid)
        err = -EPERM;                   /* boshqa sessiya yoki sessiya yetakchisi */
    if (!err) {
        int target = pgid ? (int)pgid : p->pid;
        if (target != p->pid) {         /* mavjud guruhga qo'shilish - o'sha sessiyada bo'lsin */
            bool found = false;
            for (int i = 0; i < MAX_PROCS && !found; i++) {
                struct process *q = proc_slot(i);
                found = alive(q) && q->pgid == target && q->sid == me->sid;
            }
            if (!found)
                err = -EPERM;
        }
        if (!err)
            p->pgid = target;
    }
    spin_unlock(&proc_lock);
    return err;
}

int64_t sys_getpgid(int64_t pid)
{
    spin_lock(&proc_lock);
    struct process *p = pid == 0 ? current : proc_find_locked((int)pid);
    int64_t r = (p && alive(p)) ? p->pgid : -ESRCH;
    spin_unlock(&proc_lock);
    return r;
}

int64_t sys_setsid(void)
{
    struct process *me = current;
    spin_lock(&proc_lock);
    for (int i = 0; i < MAX_PROCS; i++) {
        struct process *q = proc_slot(i);
        if (alive(q) && q->pgid == me->pid) {
            spin_unlock(&proc_lock);
            return -EPERM;              /* allaqachon guruh yetakchisi */
        }
    }
    me->sid = me->pgid = me->pid;
    spin_unlock(&proc_lock);
    return me->sid;
}

int64_t sys_alarm(uint64_t seconds)
{
    struct process *me = current;
    uint64_t now = timer_ticks();
    spin_lock(&proc_lock);
    uint64_t left = me->alarm_tick > now ? (me->alarm_tick - now + TIMER_HZ - 1) / TIMER_HZ : 0;
    me->alarm_tick = seconds ? now + seconds * TIMER_HZ : 0;
    spin_unlock(&proc_lock);
    return (int64_t)left;
}

static const char pause_channel;

int64_t sys_pause(void)
{
    spin_lock(&proc_lock);
    while (!signal_interrupted(current))
        proc_sleep(&pause_channel, &proc_lock);
    spin_unlock(&proc_lock);
    return -EINTR;
}

void signal_check_alarms_locked(uint64_t now)
{
    for (int i = 0; i < MAX_PROCS; i++) {
        struct process *p = proc_slot(i);
        if (p->alarm_tick && p->alarm_tick <= now && alive(p)) {
            p->alarm_tick = 0;
            signal_send_locked(p, SIGALRM);
        }
    }
}
