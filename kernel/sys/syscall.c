/* =============================================================================
 *  sys/syscall.c - TIZIM CHAQIRUVLARI: user dasturlar va yadro orasidagi eshik
 * =============================================================================
 *
 *  QANDAY ISHLAYDI:
 *    User dastur (ring 3) apparatga ham, yadro xotirasiga ham tega olmaydi.
 *    Biror narsa kerak bo'lsa, "iltimos" qiladi:
 *
 *        mov rax, SYS_WRITE     ; qaysi xizmat
 *        mov rdi, 1             ; argumentlar: RDI, RSI, RDX, R10, R8
 *        syscall                ; >>> ring 0, syscall_entry.asm <<<
 *
 *    syscall_entry.asm freym quradi -> syscall_dispatch -> kerakli funksiya.
 *    Natija frame->rax ga; sysret/iretq qaytganda user uni RAX da ko'radi.
 *    Xato = MANFIY errno (-ENOENT ...). libc uni `errno` ga yozadi.
 *
 *  FAYLLAR:  sys_fs.c   - open, read, write, pipe, stat, mkdir ...
 *            uaccess.c  - user ko'rsatkichlarini xavfsiz tekshirish
 *            shu fayl   - jarayonlar, xotira, tizim ma'lumotlari
 * ============================================================================= */
#include "sys/syscall.h"

#include "acpi/acpi.h"
#include "arch/cpu.h"
#include "arch/gdt.h"
#include "arch/interrupts.h"
#include "arch/io.h"
#include "arch/percpu.h"
#include "arch/tsc.h"
#include "boot/bootinfo.h"
#include "drivers/console.h"
#include "drivers/pci.h"
#include "drivers/pit.h"
#include "drivers/rtc.h"
#include "fs/vfs.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/mm.h"
#include "mm/pmm.h"
#include "mm/slab.h"
#include "myos/abi.h"
#include "proc/process.h"
#include "proc/signal.h"
#include "sys/sys_fs.h"
#include "sys/uaccess.h"

#define ARG_MAX_LEN 256

/* ---- Dastur ishga tushirish ---- */

/* User argv massivini yadroga nusxalash. storage - MAX_ARGS * ARG_MAX_LEN bayt.
 * argv berilmagan bo'lsa argv[0] = path. Qaytaradi: argc yoki -errno. */
static int copy_argv_from_user(uint64_t uargv, const char *path, char *storage, char **argv)
{
    int argc = 0;
    if (uargv) {
        for (; argc <= MAX_ARGS; argc++) {
            uint64_t uarg;
            if (copy_from_user(&uarg, uargv + (uint64_t)argc * sizeof(uint64_t), sizeof(uarg)))
                return -EFAULT;
            if (uarg == 0)
                break;                  /* NULL - massiv oxiri */
            if (argc == MAX_ARGS)
                return -E2BIG;
            argv[argc] = storage + argc * ARG_MAX_LEN;
            long r = copy_string_from_user(argv[argc], uarg, ARG_MAX_LEN);
            if (r < 0)
                return r == -ENAMETOOLONG ? -E2BIG : (int)r;
        }
    }
    if (argc == 0) {
        argv[0] = storage;
        strlcpy(argv[0], path, ARG_MAX_LEN);
        argc = 1;
    }
    return argc;
}

/* spawn va exec uchun umumiy: yo'l + argv ni olib, fn ni chaqirish. */
static int64_t with_path_argv(uint64_t upath, uint64_t uargv, struct interrupt_frame *f,
                              bool is_exec)
{
    char *path = kmalloc(PATH_MAX);
    char *storage = kmalloc(MAX_ARGS * ARG_MAX_LEN);
    char *argv[MAX_ARGS];
    int64_t r;
    if (!path || !storage) {
        r = -ENOMEM;
    } else if ((r = copy_string_from_user(path, upath, PATH_MAX)) >= 0) {
        /* argv ni ESKI manzil maydonidan HOZIR nusxalaymiz - exec dan keyin u yo'q. */
        int argc = copy_argv_from_user(uargv, path, storage, argv);
        if (argc < 0)
            r = argc;
        else
            r = is_exec ? proc_exec(f, path, argc, argv) : proc_spawn(path, argc, argv);
    }
    kfree(path);
    kfree(storage);
    return r;
}

static int64_t sys_wait(int64_t pid, uint64_t ustatus, uint64_t flags)
{
    int status = 0;
    int r = proc_wait((int)pid, &status, (int)(flags & (WAIT_NOHANG | WAIT_UNTRACED)));
    if (r > 0 && ustatus) {
        int32_t st = status;
        if (copy_to_user(ustatus, &st, sizeof(st)))
            return -EFAULT;
    }
    return r;
}

static int64_t sys_mmap(uint64_t addr, uint64_t len, uint64_t prot, uint64_t flags)
{
    if (!(flags & MAP_ANONYMOUS) || !(flags & MAP_PRIVATE))
        return -EINVAL;                 /* hozircha faqat anonim xususiy xotira */
    if (len == 0 || len > (1UL << 40))
        return -EINVAL;
    if (!(flags & MAP_FIXED))
        addr = 0;                       /* joyni yadro tanlaydi */
    uint64_t r = mm_map(current->mm, addr, len, (uint32_t)prot & 7, VMA_ANON);
    return r ? (int64_t)r : -ENOMEM;
}

/* ---- Tizim ma'lumotlari ---- */

static int64_t sys_meminfo(uint64_t uinfo)
{
    struct heap_stats hs;
    heap_get_stats(&hs);
    struct myos_meminfo mi = {
        .page_size = PAGE_SIZE,
        .total_pages = pmm_total_pages(),
        .free_pages = pmm_free_pages_count(),
        .kheap_bytes_in_use = hs.bytes_in_use,
        .kheap_allocs = hs.alloc_count,
        .kheap_frees = hs.free_count,
        .kheap_slab_pages = hs.slab_pages,
        .kheap_large_pages = hs.large_pages,
    };
    return copy_to_user(uinfo, &mi, sizeof(mi));
}

static int64_t sys_ps(uint64_t ubuf, uint64_t max)
{
    if (max > MAX_PROCS + MAX_CPUS)
        max = MAX_PROCS + MAX_CPUS;
    struct myos_proc_info *tmp = kmalloc(max * sizeof(*tmp));
    if (!tmp)
        return -ENOMEM;
    int n = proc_list(tmp, (int)max);
    int r = copy_to_user(ubuf, tmp, (size_t)n * sizeof(*tmp));
    kfree(tmp);
    return r ? r : n;
}

static int64_t sys_pciinfo(uint64_t index, uint64_t uinfo)
{
    struct pci_device *d = pci_get((int)index);
    if (!d)
        return -ENOENT;
    struct myos_pci_info o;
    memset(&o, 0, sizeof(o));
    o.bus = d->bus, o.dev = d->dev, o.func = d->func;
    o.vendor = d->vendor, o.device = d->device;
    o.class_code = d->class_code, o.subclass = d->subclass, o.prog_if = d->prog_if;
    o.irq_line = d->irq_line;
    strlcpy(o.class_name, pci_class_name(d->class_code, d->subclass), sizeof(o.class_name));
    if (d->driver)
        strlcpy(o.driver, d->driver->name, sizeof(o.driver));
    return copy_to_user(uinfo, &o, sizeof(o));
}

static int64_t sys_dmesg(uint64_t ubuf, uint64_t size)
{
    size = MIN(size, (uint64_t)64 * 1024);
    char *tmp = kmalloc(size ? size : 1);
    if (!tmp)
        return -ENOMEM;
    size_t n = console_read_log(tmp, size);
    int r = copy_to_user(ubuf, tmp, n);
    kfree(tmp);
    return r ? r : (int64_t)n;
}

static int64_t sys_sysinfo(uint64_t uinfo)
{
    struct myos_sysinfo o;
    memset(&o, 0, sizeof(o));
    o.ncpus = (uint32_t)ncpus;
    o.timer_hz = TIMER_HZ;
    o.uptime_ms = timer_ticks() * 1000 / TIMER_HZ;
    o.tsc_khz = tsc_khz;
    strlcpy(o.cpu_vendor, cpu_features.vendor, sizeof(o.cpu_vendor));
    const char *brand = cpu_features.brand;
    while (*brand == ' ')
        brand++;
    strlcpy(o.cpu_brand, brand, sizeof(o.cpu_brand));
    strlcpy(o.bootloader, boot_info.bootloader, sizeof(o.bootloader));
    return copy_to_user(uinfo, &o, sizeof(o));
}

__attribute__((noreturn)) static void sys_shutdown(void)
{
    kprintf("\n[kernel] Fayl tizimlari sinxronlanmoqda...\n");
    vfs_sync();
    kprintf("[kernel] Tizim o'chirilmoqda...\n");
    acpi_poweroff();                    /* haqiqiy kompyuter: ACPI S5 holati */
    outw(0x604, 0x2000);                /* QEMU (yangi versiyalar) */
    outw(0xB004, 0x2000);               /* Bochs va eski QEMU */
    outb(0xf4, 0x00);                   /* QEMU isa-debug-exit (zaxira) */
    for (;;)
        __asm__ volatile("cli; hlt");
}

__attribute__((noreturn)) static void sys_reboot(void)
{
    vfs_sync();
    kprintf("\n[kernel] Qayta yuklanmoqda...\n");
    acpi_reboot();
    for (;;)
        __asm__ volatile("cli; hlt");
}

/* ---- Dispetcher ------------------------------------------------------------- */

void syscall_dispatch(struct interrupt_frame *f);
void syscall_dispatch(struct interrupt_frame *f)
{
    uint64_t nr = f->rax, a1 = f->rdi, a2 = f->rsi, a3 = f->rdx, a4 = f->r10;
    int64_t ret;

    /* Syscall davomida uzilishlar YOQIQ: taymer ishlaydi, TLB so'rovlari (IPI)
     * javobsiz qolmaydi. Umumiy ma'lumotlar qulflar bilan himoyalangan. */
    cpu_sti();

    bool handled;
    ret = sys_fs_dispatch(nr, a1, a2, a3, a4, &handled);
    if (!handled) {
        switch (nr) {
        case SYS_EXIT:     proc_exit((int)a1);                  /* qaytmaydi */
        case SYS_SPAWN:    ret = with_path_argv(a1, a2, f, false); break;
        case SYS_EXEC:     ret = with_path_argv(a1, a2, f, true); break;
        case SYS_FORK:     ret = proc_fork(f); break;
        case SYS_WAIT:     ret = sys_wait((int64_t)a1, a2, a3); break;
        case SYS_GETPID:   ret = current->pid; break;
        case SYS_GETPPID:  ret = current->parent ? current->parent->pid : 0; break;
        case SYS_YIELD:    proc_yield(); ret = 0; break;
        case SYS_SLEEP: {
            /* Signal uzsa: qolgan vaqtni argumentga yozib qo'yamiz - qayta
             * boshlansa (Ctrl-Z + fg), faqat QOLGAN qismi uxlanadi. */
            uint64_t left = proc_sleep_ms(a1);
            if (left)
                f->rdi = left;
            ret = left ? -EINTR : 0;
            break;
        }
        case SYS_KILL:     ret = signal_kill((int)(int64_t)a1, (int)a2); break;
        case SYS_SIGACTION:   ret = sys_sigaction(a1, a2, a3); break;
        case SYS_SIGPROCMASK: ret = sys_sigprocmask(a1, a2, a3); break;
        case SYS_SIGRETURN:   ret = sys_sigreturn(f); break;
        case SYS_SETPGID:     ret = sys_setpgid((int64_t)a1, (int64_t)a2); break;
        case SYS_GETPGID:     ret = sys_getpgid((int64_t)a1); break;
        case SYS_SETSID:      ret = sys_setsid(); break;
        case SYS_ALARM:       ret = sys_alarm(a1); break;
        case SYS_PAUSE:       ret = sys_pause(); break;
        case SYS_SBRK:     ret = (int64_t)mm_sbrk(current->mm, (int64_t)a1); break;
        case SYS_MMAP:     ret = sys_mmap(a1, a2, a3, a4); break;
        case SYS_MUNMAP:   ret = mm_unmap(current->mm, a1, a2) ? -EINVAL : 0; break;
        case SYS_MEMINFO:  ret = sys_meminfo(a1); break;
        case SYS_PS:       ret = sys_ps(a1, a2); break;
        case SYS_UPTIME:   ret = (int64_t)(timer_ticks() * 1000 / TIMER_HZ); break;
        case SYS_TIME:     ret = (int64_t)time_now(); break;
        case SYS_PCIINFO:  ret = sys_pciinfo(a1, a2); break;
        case SYS_DMESG:    ret = sys_dmesg(a1, a2); break;
        case SYS_SYSINFO:  ret = sys_sysinfo(a1); break;
        case SYS_SHUTDOWN: sys_shutdown();
        case SYS_REBOOT:   sys_reboot();
        default:           ret = -ENOSYS; break;                 /* noma'lum syscall */
        }
    }
    /* Signal bilan uzilgan syscall: SA_RESTART bo'lsa (yoki Ctrl-Z bilan
     * to'xtatilgan bo'lsa) - `syscall` instruksiyasini QAYTA bajaramiz: RIP ni
     * 2 bayt (syscall = 0F 05) orqaga, RAX = asl raqam. Argumentlar
     * (RDI, RSI ...) freymda o'zgarmagan. */
    if (ret == -EINTR && signal_should_restart(nr)) {
        f->rax = nr;
        f->rip -= 2;
    } else {
        f->rax = (uint64_t)ret;         /* natija user'ning RAX registriga */
    }

    /* User rejimiga qaytish arafasi - signallarni yetkazish joyi. */
    signal_deliver(f);
}

extern void syscall_entry(void);        /* syscall_entry.asm */

void syscall_init_cpu(void)
{
    /* EFER.SCE - syscall/sysret ga ruxsat. */
    wrmsr(MSR_EFER, rdmsr(MSR_EFER) | EFER_SCE);
    /* STAR: [47:32] = syscall uchun CS (SS = CS+8):  0x08 yadro kodi, 0x10 ma'lumot.
     *       [63:48] = sysret uchun baza: CS = baza+16 (0x20|3), SS = baza+8 (0x18|3).
     * GDT dagi tartib (user ma'lumoti user kodidan OLDIN) aynan shu uchun. */
    wrmsr(MSR_STAR, ((uint64_t)0x10 << 48) | ((uint64_t)GDT_KERNEL_CODE << 32));
    wrmsr(MSR_LSTAR, (uint64_t)syscall_entry);
    /* SFMASK: syscall paytida RFLAGS dan o'chiriladigan bitlar:
     * IF (GS almashguncha uzilish kelmasin), DF, TF, AC. */
    wrmsr(MSR_SFMASK, (1u << 9) | (1u << 10) | (1u << 8) | (1u << 18));
}

void syscall_init(void)
{
    interrupt_register(VECTOR_SYSCALL, syscall_dispatch);   /* eski `int 0x80` ham ishlaydi */
    syscall_init_cpu();
}
