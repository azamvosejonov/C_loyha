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
#include "arch/io.h"
#include "drivers/pit.h"
#include "fs/file.h"
#include "fs/tarfs.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/heap.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "myos/abi.h"
#include "proc/process.h"

#define PATH_MAX      128
#define ARG_MAX_LEN   128
#define IO_CHUNK_MAX  (64 * 1024)        /* bitta read/write chegarasi */

/* ---- User xotirasi bilan xavfsiz ishlash ------------------------------------ */

static bool user_ok(uint64_t addr, size_t len, bool write)
{
    return vmm_user_range_ok(current->pml4, addr, len, write);
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

static int64_t sys_spawn(uint64_t upath, uint64_t uargv)
{
    char path[PATH_MAX];
    if (copy_string_from_user(path, upath, sizeof(path)) < 0)
        return -1;

    /* argv ni yadroga nusxalaymiz: avval ko'rsatkichlar massivi, keyin satrlar.
     * Barcha satrlar bitta kmalloc buferida saqlanadi. */
    char *storage = kmalloc(MAX_ARGS * ARG_MAX_LEN);
    char *argv[MAX_ARGS];
    int argc = 0;
    if (!storage)
        return -1;
    if (uargv) {
        for (; argc <= MAX_ARGS; argc++) {
            uint64_t slot = uargv + (uint64_t)argc * sizeof(uint64_t);
            if (!user_ok(slot, sizeof(uint64_t), false)) {
                kfree(storage);
                return -1;
            }
            uint64_t uarg = *(const uint64_t *)(uintptr_t)slot;
            if (uarg == 0)
                break;                  /* NULL - massiv oxiri */
            if (argc == MAX_ARGS) {
                kfree(storage);
                return -1;              /* argumentlar juda ko'p */
            }
            argv[argc] = storage + argc * ARG_MAX_LEN;
            if (copy_string_from_user(argv[argc], uarg, ARG_MAX_LEN) < 0) {
                kfree(storage);
                return -1;
            }
        }
    }
    if (argc == 0) {                    /* argv berilmagan: argv[0] = yo'l */
        argv[0] = storage;
        strlcpy(argv[0], path, ARG_MAX_LEN);
        argc = 1;
    }
    int64_t pid = proc_spawn(path, argc, argv);
    kfree(storage);
    return pid;
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
 * shu chaqiruv ustiga qurilgan (user/lib/malloc.c ga qarang). */
static int64_t sys_sbrk(int64_t increment)
{
    struct process *p = current;
    uint64_t old_brk = p->brk;
    uint64_t new_brk = old_brk + (uint64_t)increment;
    uint64_t limit = USER_STACK_TOP - (USER_STACK_PAGES + 1) * PAGE_SIZE;  /* stek + himoya */

    if (increment > 0 && (new_brk < old_brk || new_brk > limit))
        return -1;                      /* to'lib ketish yoki stekka urilish */
    if (increment < 0 && new_brk < p->heap_start)
        return -1;

    uint64_t old_top = ALIGN_UP(old_brk, PAGE_SIZE);
    uint64_t new_top = ALIGN_UP(new_brk, PAGE_SIZE);
    if (new_top > old_top) {
        if (!vmm_map_anonymous(p->pml4, old_top, (new_top - old_top) / PAGE_SIZE,
                               PTE_USER | PTE_WRITABLE)) {
            /* Qisman xaritalanganlarini qaytarib olamiz. */
            for (uint64_t va = old_top; va < new_top; va += PAGE_SIZE) {
                uint64_t phys = vmm_unmap_page(p->pml4, va);
                if (phys)
                    pmm_free_frame(phys);
            }
            return -1;
        }
    } else {
        for (uint64_t va = new_top; va < old_top; va += PAGE_SIZE) {
            uint64_t phys = vmm_unmap_page(p->pml4, va);
            if (phys)
                pmm_free_frame(phys);   /* xotirani tizimga QAYTARAMIZ */
        }
    }
    p->brk = new_brk;
    return (int64_t)old_brk;
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
    mi->total_pages = pmm_total_frames();
    mi->free_pages = pmm_free_frames_count();
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
    outw(0x604, 0x2000);                /* QEMU (yangi versiyalar) ACPI: "quvvatni o'chir" */
    outw(0xB004, 0x2000);               /* Bochs va eski QEMU */
    outb(0xf4, 0x00);                   /* QEMU isa-debug-exit qurilmasi (zaxira usul) */
    for (;;)
        __asm__ volatile("cli; hlt");
}

/* ---- Dispetcher ------------------------------------------------------------- */

static void syscall_handler(struct interrupt_frame *f)
{
    uint64_t a1 = f->rdi, a2 = f->rsi, a3 = f->rdx;
    int64_t ret;

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
    default:           ret = -1; break;         /* noma'lum syscall */
    }
    f->rax = (uint64_t)ret;             /* natija user'ning RAX registriga qaytadi */
}

void syscall_init(void)
{
    interrupt_register(VECTOR_SYSCALL, syscall_handler);
}
