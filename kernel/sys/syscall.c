/* =============================================================================
 *  sys/syscall.c - TIZIM CHAQIRUVLARI: user dasturlar va yadro orasidagi eshik
 * =============================================================================
 *
 *  QANDAY ISHLAYDI:
 *    User dastur (ring 3) apparatga ham, yadro xotirasiga ham tega olmaydi.
 *    Biror narsa kerak bo'lsa, u "iltimos" qiladi:
 *
 *        mov rax, SYS_WRITE     ; qaysi xizmat
 *        mov rdi, 1             ; argumentlar
 *        mov rsi, buf
 *        mov rdx, len
 *        int 0x80               ; >>> CPU ring 0 ga o'tadi, TSS.rsp0 stekiga <<<
 *
 *    isr.asm registrlarni saqlaydi -> interrupt_dispatch -> syscall_handler.
 *    Natijani frame->rax ga yozamiz; iretq qaytganda user uni RAX da ko'radi.
 *
 *  XAVFSIZLIK - ENG MUHIM QOIDA:
 *    User bergan HAR BIR qiymat - dushman qiymati deb hisoblanadi.
 *      * Ko'rsatkichlar: vmm_user_range_ok() bilan tekshiriladi. Aks holda
 *        dastur read(0, 0x100000, 100) qilib, YADRO KODINI ustidan yozdirardi.
 *      * Satrlar: sahifama-sahifa, uzunlik chegarasi bilan nusxalanadi.
 *      * Fd raqamlari: diapazon tekshiriladi (files[-5] -> xotira buzilishi).
 *    Real yadrolardagi xavfsizlik zaifliklarining katta qismi aynan shu
 *    tekshiruvlarning birortasi unutilgan joyda paydo bo'ladi.
 *
 *  BIZDA YADRO "NON-PREEMPTIBLE": syscall IF=0 holda bajariladi (interrupt
 *  gate). Syscall ichida faqat ixtiyoriy uxlash (read, wait, sleep) boshqa
 *  jarayonga o'tkazadi. Bu sodda va xavfsiz; Linux esa yadro ichida ham
 *  preemption qiladi (ancha murakkab qulflash talab qiladi).
 * ============================================================================= */
#include "sys/syscall.h"

#include "arch/interrupts.h"
#include "acpi/acpi.h"
#include "arch/cpu.h"
#include "arch/gdt.h"
#include "arch/io.h"
#include "drivers/pit.h"
#include "arch/percpu.h"
#include "arch/tsc.h"
#include "boot/bootinfo.h"
#include "drivers/console.h"
#include "drivers/pci.h"
#include "fs/file.h"
#include "fs/tarfs.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/slab.h"
#include "mm/mm.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "myos/abi.h"
#include "proc/process.h"

#define PATH_MAX      128
#define ARG_MAX_LEN   128
#define IO_CHUNK_MAX  (64 * 1024)        /* bitta read/write chegarasi */

/* ---- User xotirasi bilan xavfsiz ishlash ------------------------------------ */

/* User bergan [addr, addr+len) xotirasi haqiqatan jarayonniki va (kerak bo'lsa)
 * yozish mumkinmi? DEMAND PAGING tufayli sahifalar hali yaratilmagan bo'lishi
 * mumkin - mm_prefault ularni hozir yaratadi (va COW ni buzadi). Shundan keyin
 * yadro bu xotiraga to'g'ridan-to'g'ri page fault'siz murojaat qila oladi. */
static bool user_ok(uint64_t addr, size_t len, bool write)
{
    return mm_prefault(current->mm, addr, len, write);
}

/* User satrini yadro buferiga nusxalash. Qaytaradi: uzunlik yoki -1.
 * Har bir yangi sahifaga o'tganda uni qayta tekshiramiz - satr xaritalanmagan
 * sahifaga "cho'zilib" ketishi mumkin. */
static long copy_string_from_user(char *dst, uint64_t src, size_t max)
{
    for (size_t i = 0; i < max; i++) {
        if (i == 0 || ((src + i) & (PAGE_SIZE - 1)) == 0)
            if (!user_ok(src + i, 1, false))
                return -1;
        dst[i] = *(const char *)(uintptr_t)(src + i);
        if (dst[i] == '\0')
            return (long)i;
    }
    return -1;                          /* juda uzun */
}

static struct file *get_file(int64_t fd)
{
    if (fd < 0 || fd >= MAX_FDS)
        return NULL;
    return current->files[fd];
}

/* ---- Alohida syscall'lar ---------------------------------------------------- */

static int64_t sys_write(int64_t fd, uint64_t buf, uint64_t len)
{
    struct file *f = get_file(fd);
    if (!f || len > IO_CHUNK_MAX || !user_ok(buf, len, false))
        return -1;
    return file_write(f, (const void *)(uintptr_t)buf, len);
}

static int64_t sys_read(int64_t fd, uint64_t buf, uint64_t len)
{
    struct file *f = get_file(fd);
    if (!f || len > IO_CHUNK_MAX || !user_ok(buf, len, true))
        return -1;
    return file_read(f, (void *)(uintptr_t)buf, len);
}

static int64_t sys_open(uint64_t upath)
{
    char path[PATH_MAX];
    if (copy_string_from_user(path, upath, sizeof(path)) < 0)
        return -1;
    for (int fd = 0; fd < MAX_FDS; fd++) {
        if (!current->files[fd]) {
            struct file *f = file_open(path);
            if (!f)
                return -1;
            current->files[fd] = f;
            return fd;
        }
    }
    return -1;                          /* fd jadvali to'la */
}

static int64_t sys_close(int64_t fd)
{
    struct file *f = get_file(fd);
    if (!f)
        return -1;
    file_close(f);
    current->files[fd] = NULL;
    return 0;
}

/* User argv massivini yadroga nusxalash. storage - MAX_ARGS * ARG_MAX_LEN bayt.
 * argv berilmagan bo'lsa argv[0] = path. Qaytaradi: argc yoki -1. */
static int copy_argv_from_user(uint64_t uargv, const char *path, char *storage, char **argv)
{
    int argc = 0;
    if (uargv) {
        for (; argc <= MAX_ARGS; argc++) {
            uint64_t slot = uargv + (uint64_t)argc * sizeof(uint64_t);
            if (!user_ok(slot, sizeof(uint64_t), false))
                return -1;
            uint64_t uarg = *(const uint64_t *)(uintptr_t)slot;
            if (uarg == 0)
                break;                  /* NULL - massiv oxiri */
            if (argc == MAX_ARGS)
                return -1;              /* argumentlar juda ko'p */
            argv[argc] = storage + argc * ARG_MAX_LEN;
            if (copy_string_from_user(argv[argc], uarg, ARG_MAX_LEN) < 0)
                return -1;
        }
    }
    if (argc == 0) {
        argv[0] = storage;
        strlcpy(argv[0], path, ARG_MAX_LEN);
        argc = 1;
    }
    return argc;
}

static int64_t sys_spawn(uint64_t upath, uint64_t uargv)
{
    char path[PATH_MAX];
    if (copy_string_from_user(path, upath, sizeof(path)) < 0)
        return -1;
    char *storage = kmalloc(MAX_ARGS * ARG_MAX_LEN);
    char *argv[MAX_ARGS];
    if (!storage)
        return -1;
    int argc = copy_argv_from_user(uargv, path, storage, argv);
    int64_t pid = argc < 0 ? -1 : proc_spawn(path, argc, argv);
    kfree(storage);
    return pid;
}

/* exec: muvaffaqiyatda qaytmaydi (to'g'rirog'i, yangi dastur boshiga "qaytadi"). */
static int64_t sys_exec(struct interrupt_frame *f, uint64_t upath, uint64_t uargv)
{
    char path[PATH_MAX];
    if (copy_string_from_user(path, upath, sizeof(path)) < 0)
        return -1;
    char *storage = kmalloc(MAX_ARGS * ARG_MAX_LEN);
    char *argv[MAX_ARGS];
    if (!storage)
        return -1;
    /* argv ni ESKI manzil maydonidan HOZIR nusxalaymiz - exec dan keyin u yo'q. */
    int argc = copy_argv_from_user(uargv, path, storage, argv);
    int64_t r = argc < 0 ? -1 : proc_exec(f, path, argc, argv);
    kfree(storage);
    /* Muvaffaqiyat: freym allaqachon yangi dastur boshiga ko'rsatadi va 0 ni
     * RAX ga yozish zararsiz (yangi dasturda RAX baribir 0). */
    return r;
}

static int64_t sys_mmap(uint64_t addr, uint64_t len, uint64_t prot, uint64_t flags)
{
    if (!(flags & MAP_ANONYMOUS) || !(flags & MAP_PRIVATE))
        return -1;                      /* hozircha faqat anonim xususiy xotira */
    if (len == 0 || len > (1UL << 40))
        return -1;
    if (!(flags & MAP_FIXED))
        addr = 0;                       /* joyni yadro tanlaydi */
    uint64_t r = mm_map(current->mm, addr, len, (uint32_t)prot & 7, VMA_ANON);
    return r ? (int64_t)r : -1;
}

static int64_t sys_wait(int64_t pid, uint64_t ustatus, uint64_t flags)
{
    if (ustatus && !user_ok(ustatus, sizeof(int32_t), true))
        return -1;
    int code = 0;
    int r = proc_wait((int)pid, &code, (flags & WAIT_NOHANG) != 0);
    if (r > 0 && ustatus)
        *(int32_t *)(uintptr_t)ustatus = code;
    return r;
}

/* sbrk: user heap'ini kattalashtirish/kichraytirish. Unix'dagi malloc aynan
 * shu chaqiruv ustiga qurilgan (user/lib/malloc.c ga qarang). Sahifalar DARHOL
 * ajratilmaydi - faqat heap VMA kengayadi, sahifa birinchi murojaatda paydo
 * bo'ladi (demand paging). */
static int64_t sys_sbrk(int64_t increment)
{
    return (int64_t)mm_sbrk(current->mm, increment);
}

static int64_t sys_readdir(uint64_t index, uint64_t udirent)
{
    if (!user_ok(udirent, sizeof(struct myos_dirent), true))
        return -1;
    const struct tar_file *tf = tarfs_get(index);
    if (!tf)
        return -1;                      /* ro'yxat tugadi */
    struct myos_dirent *d = (struct myos_dirent *)(uintptr_t)udirent;
    strlcpy(d->name, tf->name, sizeof(d->name));
    d->size = tf->size;
    return 0;
}

static int64_t sys_meminfo(uint64_t uinfo)
{
    if (!user_ok(uinfo, sizeof(struct myos_meminfo), true))
        return -1;
    struct heap_stats hs;
    heap_get_stats(&hs);
    struct myos_meminfo *mi = (struct myos_meminfo *)(uintptr_t)uinfo;
    mi->page_size = PAGE_SIZE;
    mi->total_pages = pmm_total_pages();
    mi->free_pages = pmm_free_pages_count();
    mi->kheap_bytes_in_use = hs.bytes_in_use;
    mi->kheap_allocs = hs.alloc_count;
    mi->kheap_frees = hs.free_count;
    mi->kheap_slab_pages = hs.slab_pages;
    mi->kheap_large_pages = hs.large_pages;
    return 0;
}

static int64_t sys_ps(uint64_t ubuf, uint64_t max)
{
    if (max > MAX_PROCS)
        max = MAX_PROCS;
    if (!user_ok(ubuf, max * sizeof(struct myos_proc_info), true))
        return -1;
    struct myos_proc_info *tmp = kmalloc(max * sizeof(*tmp));
    if (!tmp)
        return -1;
    int n = proc_list(tmp, (int)max);
    memcpy((void *)(uintptr_t)ubuf, tmp, (size_t)n * sizeof(*tmp));
    kfree(tmp);
    return n;
}

static int64_t sys_kill(int64_t pid)
{
    if (pid <= 1)                       /* idle va init ni o'ldirib bo'lmaydi */
        return -1;
    return proc_kill((int)pid);
}

__attribute__((noreturn)) static void sys_shutdown(void)
{
    kprintf("\n[kernel] Tizim o'chirilmoqda...\n");
    acpi_poweroff();                    /* haqiqiy kompyuter: ACPI S5 holati */
    outw(0x604, 0x2000);                /* QEMU (yangi versiyalar) ACPI: "quvvatni o'chir" */
    outw(0xB004, 0x2000);               /* Bochs va eski QEMU */
    outb(0xf4, 0x00);                   /* QEMU isa-debug-exit qurilmasi (zaxira usul) */
    for (;;)
        __asm__ volatile("cli; hlt");
}

static int64_t sys_pciinfo(uint64_t index, uint64_t uinfo)
{
    if (!user_ok(uinfo, sizeof(struct myos_pci_info), true))
        return -1;
    struct pci_device *d = pci_get((int)index);
    if (!d)
        return -1;
    struct myos_pci_info *o = (struct myos_pci_info *)uinfo;
    memset(o, 0, sizeof(*o));
    o->bus = d->bus, o->dev = d->dev, o->func = d->func;
    o->vendor = d->vendor, o->device = d->device;
    o->class_code = d->class_code, o->subclass = d->subclass, o->prog_if = d->prog_if;
    o->irq_line = d->irq_line;
    strlcpy(o->class_name, pci_class_name(d->class_code, d->subclass), sizeof(o->class_name));
    if (d->driver)
        strlcpy(o->driver, d->driver->name, sizeof(o->driver));
    return 0;
}

static int64_t sys_dmesg(uint64_t ubuf, uint64_t size)
{
    if (size > 64 * 1024)
        size = 64 * 1024;
    if (!user_ok(ubuf, size, true))
        return -1;
    char *tmp = kmalloc(size);
    if (!tmp)
        return -1;
    size_t n = console_read_log(tmp, size);
    memcpy((void *)ubuf, tmp, n);
    kfree(tmp);
    return (int64_t)n;
}

static int64_t sys_sysinfo(uint64_t uinfo)
{
    if (!user_ok(uinfo, sizeof(struct myos_sysinfo), true))
        return -1;
    struct myos_sysinfo *o = (struct myos_sysinfo *)uinfo;
    memset(o, 0, sizeof(*o));
    o->ncpus = (uint32_t)ncpus;
    o->timer_hz = TIMER_HZ;
    o->uptime_ms = timer_ticks() * 1000 / TIMER_HZ;
    o->tsc_khz = tsc_khz;
    strlcpy(o->cpu_vendor, cpu_features.vendor, sizeof(o->cpu_vendor));
    const char *brand = cpu_features.brand;
    while (*brand == ' ')
        brand++;
    strlcpy(o->cpu_brand, brand, sizeof(o->cpu_brand));
    strlcpy(o->bootloader, boot_info.bootloader, sizeof(o->bootloader));
    return 0;
}

__attribute__((noreturn)) static void sys_reboot(void)
{
    kprintf("\n[kernel] Qayta yuklanmoqda...\n");
    acpi_reboot();
    for (;;)
        __asm__ volatile("cli; hlt");
}

/* ---- Dispetcher ------------------------------------------------------------- */

/* `syscall` instruksiyasi (syscall_entry.asm) va `int 0x80` - ikkalasi shu yerga. */
void syscall_dispatch(struct interrupt_frame *f);
void syscall_dispatch(struct interrupt_frame *f)
{
    uint64_t a1 = f->rdi, a2 = f->rsi, a3 = f->rdx;
    int64_t ret;

    /* Syscall davomida uzilishlar YOQIQ: taymer ishlaydi, boshqa jarayonlar
     * navbat oladi, TLB tozalash so'rovlari (IPI) javobsiz qolmaydi. Umumiy
     * ma'lumotlar spinlock'lar bilan himoyalangan. */
    cpu_sti();

    switch (f->rax) {
    case SYS_EXIT:     proc_exit((int)a1);                              /* qaytmaydi */
    case SYS_WRITE:    ret = sys_write((int64_t)a1, a2, a3); break;
    case SYS_READ:     ret = sys_read((int64_t)a1, a2, a3); break;
    case SYS_OPEN:     ret = sys_open(a1); break;
    case SYS_CLOSE:    ret = sys_close((int64_t)a1); break;
    case SYS_SPAWN:    ret = sys_spawn(a1, a2); break;
    case SYS_WAIT:     ret = sys_wait((int64_t)a1, a2, a3); break;
    case SYS_GETPID:   ret = current->pid; break;
    case SYS_YIELD:    proc_yield(); ret = 0; break;
    case SYS_SLEEP:    proc_sleep_ms(a1); ret = 0; break;
    case SYS_SBRK:     ret = sys_sbrk((int64_t)a1); break;
    case SYS_READDIR:  ret = sys_readdir(a1, a2); break;
    case SYS_MEMINFO:  ret = sys_meminfo(a1); break;
    case SYS_PS:       ret = sys_ps(a1, a2); break;
    case SYS_KILL:     ret = sys_kill((int64_t)a1); break;
    case SYS_UPTIME:   ret = (int64_t)(timer_ticks() * 1000 / TIMER_HZ); break;
    case SYS_SHUTDOWN: sys_shutdown();
    case SYS_PCIINFO:  ret = sys_pciinfo(a1, a2); break;
    case SYS_REBOOT:   sys_reboot();
    case SYS_DMESG:    ret = sys_dmesg(a1, a2); break;
    case SYS_SYSINFO:  ret = sys_sysinfo(a1); break;
    case SYS_FORK:     ret = proc_fork(f); break;
    case SYS_EXEC:     ret = sys_exec(f, a1, a2); break;
    case SYS_MMAP:     ret = sys_mmap(a1, a2, a3, f->r10); break;
    case SYS_MUNMAP:   ret = mm_unmap(current->mm, a1, a2); break;
    case SYS_GETPPID:  ret = current->parent ? current->parent->pid : 0; break;
    default:           ret = -1; break;         /* noma'lum syscall */
    }
    f->rax = (uint64_t)ret;             /* natija user'ning RAX registriga qaytadi */

    /* User rejimiga qaytish arafasi - kill() belgisini tekshirish joyi. */
    if (current->killed)
        proc_exit(-1);
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
     * IF (uzilishlar - GS almashguncha kelmasin), DF, TF, AC. */
    wrmsr(MSR_SFMASK, (1u << 9) | (1u << 10) | (1u << 8) | (1u << 18));
}

void syscall_init(void)
{
    interrupt_register(VECTOR_SYSCALL, syscall_dispatch);   /* int 0x80 ham ishlaydi */
    syscall_init_cpu();
}
