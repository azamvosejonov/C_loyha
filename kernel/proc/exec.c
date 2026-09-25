/* =============================================================================
 *  proc/exec.c - dastur ishga tushirish: spawn, exec, fork
 * =============================================================================
 *
 *  UNIX MODELI - ikki qadam:
 *     fork()  - joriy jarayonning NUSXASI (copy-on-write tufayli arzon)
 *     exec()  - nusxa ichida BOSHQA dasturni yuklash (manzil maydoni almashadi)
 *  Shell buyruqni aynan shunday bajaradi:
 *     pid = fork();
 *     if (pid == 0) exec("ls", argv);     // bola: ls ga aylanadi
 *     else wait(pid);                     // ota: kutadi
 *  Nega ikki qadam? Fork va exec orasida bola o'z muhitini sozlaydi (fayllarni
 *  yo'naltirish: `ls > fayl`, pipe'lar `ls | cat`) - ota jarayonga tegmasdan.
 *
 *  spawn() - ikkalasini bitta chaqiruvda (posix_spawn kabi), init uchun qulay.
 *
 *  YANGI MANZIL MAYDONI (load_image):
 *    1. ELF segmentlari -> VMA lar (kod r-x, ma'lumot rw-), fayl baytlari nusxalanadi
 *    2. heap VMA (brk) - bo'sh, demand paging bilan o'sadi
 *    3. stek VMA - USER_STACK_TOP ostida, 8 MB gacha avtomatik o'sadi
 *    4. argv satrlari stekning tepasiga:
 *
 *    USER_STACK_TOP ┌────────────────────┐
 *                   │ "hello\0" "-v\0"   │ <- argv satrlari
 *                   ├────────────────────┤ (16 ga tekislash)
 *                   │ argv[0], argv[1]   │
 *                   │ NULL               │
 *        RSP ─────► └────────────────────┘   RDI = argc, RSI = &argv[0]
 * ============================================================================= */
#include "proc/process.h"

#include "arch/gdt.h"
#include "arch/interrupts.h"
#include "fs/file.h"
#include "fs/tarfs.h"
#include "lib/common.h"
#include "lib/string.h"
#include "mm/mm.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "sys/elf.h"

extern void interrupt_return(void);    /* isr.asm */
extern void new_proc_start(void);      /* switch.asm */

#define MAX_ARG_BYTES 4096
#define INITIAL_STACK (64 * 1024)       /* argv uchun darhol ajratiladigan qism */

struct image {
    struct mm *mm;
    uint64_t entry;
    uint64_t sp;
    uint64_t argv_va;
};

static uint64_t setup_user_stack(struct mm *mm, int argc, char *const argv[], uint64_t *argv_va)
{
    uint64_t sp = USER_STACK_TOP;
    uint64_t ptrs[MAX_ARGS + 1];
    for (int i = argc - 1; i >= 0; i--) {
        size_t len = strlen(argv[i]) + 1;
        sp -= len;
        if (!vmm_copy_to_space(mm->pml4, sp, argv[i], len))
            return 0;
        ptrs[i] = sp;
    }
    ptrs[argc] = 0;                     /* argv[argc] = NULL (C standarti) */
    sp = ALIGN_DOWN(sp, 16);
    sp -= (uint64_t)(argc + 1) * sizeof(uint64_t);
    sp = ALIGN_DOWN(sp, 16);            /* ABI: _start da RSP 16 ga karrali */
    if (!vmm_copy_to_space(mm->pml4, sp, ptrs, (size_t)(argc + 1) * sizeof(uint64_t)))
        return 0;
    *argv_va = sp;
    return sp;
}

static int load_image(const char *path, int argc, char *const argv[], struct image *out)
{
    if (argc < 0 || argc > MAX_ARGS)
        return -1;
    size_t total = 0;
    for (int i = 0; i < argc; i++)
        total += strlen(argv[i]) + 1;
    if (total > MAX_ARG_BYTES)
        return -1;

    const struct tar_file *tf = tarfs_find(path);
    if (!tf)
        return -2;                      /* fayl topilmadi */

    struct mm *mm = mm_create();
    if (!mm)
        return -3;
    uint64_t entry, image_end;
    int err = elf_load(mm, tf->data, tf->size, &entry, &image_end);
    if (err < 0) {
        mm_destroy(mm);
        return -4;
    }
    /* Heap: ELF dan keyingi sahifadan (1 sahifalik VMA, sbrk bilan o'sadi). */
    mm->brk_start = mm->brk = ALIGN_UP(image_end, PAGE_SIZE);
    uint64_t stack_bottom = USER_STACK_TOP - INITIAL_STACK;
    uint64_t argv_va;
    if (!mm_map(mm, mm->brk_start, PAGE_SIZE, PROT_READ | PROT_WRITE, VMA_HEAP | VMA_ANON) ||
        !mm_map(mm, stack_bottom, INITIAL_STACK, PROT_READ | PROT_WRITE, VMA_STACK | VMA_ANON) ||
        !mm_populate(mm, USER_STACK_TOP - PAGE_SIZE * 2, PAGE_SIZE * 2) ||
        !setup_user_stack(mm, argc, argv, &argv_va)) {
        mm_destroy(mm);
        return -5;
    }
    out->mm = mm;
    out->entry = entry;
    out->sp = argv_va;
    out->argv_va = argv_va;
    return 0;
}

/* User rejimiga "qaytish" freymi: ring 3, RIP = entry. */
static void init_user_frame(struct interrupt_frame *f, const struct image *img, int argc)
{
    memset(f, 0, sizeof(*f));
    f->rip = img->entry;
    f->cs = GDT_USER_CODE;              /* 0x23 */
    f->rflags = 0x202;                  /* IF=1 */
    f->rsp = img->sp;
    f->ss = GDT_USER_DATA;              /* 0x1B */
    f->rdi = (uint64_t)argc;            /* main(argc, argv) */
    f->rsi = img->argv_va;
}

/* Yangi jarayonning yadro stekini tayyorlash: tepada f (user freymi), ostida
 * context_switch freymi. Birinchi o'tishda: new_proc_start -> proc_lock ni
 * qo'yib yuborish -> interrupt_return -> iretq -> ring 3. */
static struct interrupt_frame *prepare_kstack(struct process *p)
{
    uint64_t top = p->kstack_base + KSTACK_PAGES * PAGE_SIZE;
    struct interrupt_frame *f = (struct interrupt_frame *)(top - sizeof(*f));
    uint64_t *sp = (uint64_t *)f;
    *--sp = (uint64_t)new_proc_start;
    *--sp = 0;                          /* RBX */
    *--sp = 0;                          /* RBP */
    *--sp = 0;                          /* R12 */
    *--sp = (uint64_t)interrupt_return; /* R13 - davom */
    *--sp = 0;                          /* R14 */
    *--sp = 0;                          /* R15 */
    p->kernel_rsp = (uint64_t)sp;
    return f;
}

static const char *basename(const char *path)
{
    const char *name = path;
    for (const char *s = path; *s; s++)
        if (*s == '/')
            name = s + 1;
    return name;
}

int proc_spawn(const char *path, int argc, char *const argv[])
{
    struct image img;
    int err = load_image(path, argc, argv, &img);
    if (err < 0)
        return err;
    struct process *p = proc_alloc(basename(path));
    if (!p) {
        mm_destroy(img.mm);
        return -6;
    }
    p->is_user = true;
    p->mm = img.mm;
    p->pml4 = img.mm->pml4;
    init_user_frame(prepare_kstack(p), &img, argc);
    p->files[0] = file_console();
    p->files[1] = file_console();
    p->files[2] = file_console();
    int pid = p->pid;
    proc_make_ready(p);
    return pid;
}

int proc_exec(struct interrupt_frame *f, const char *path, int argc, char *const argv[])
{
    struct process *p = current;
    struct image img;
    /* Yangi tasvirni TO'LIQ yuklab bo'lgunimizcha eskisiga tegmaymiz: xato
     * bo'lsa, dastur exec() dan -1 bilan qaytadi va ishlashda davom etadi. */
    int err = load_image(path, argc, argv, &img);
    if (err < 0)
        return err;

    spin_lock(&proc_lock);              /* ps jadvallarni yurayotgan bo'lishi mumkin */
    struct mm *old = p->mm;
    p->mm = img.mm;
    p->pml4 = img.mm->pml4;
    strlcpy(p->name, basename(path), sizeof(p->name));
    spin_unlock(&proc_lock);
    vmm_switch(p->pml4);
    if (old)
        mm_destroy(old);                /* eski dasturning butun xotirasi */

    init_user_frame(f, &img, argc);     /* syscall'dan "qaytish" yangi dastur boshiga */
    return 0;
}

int proc_fork(struct interrupt_frame *f)
{
    struct process *parent = current;
    struct process *child = proc_alloc(parent->name);
    if (!child)
        return -1;
    child->mm = mm_fork(parent->mm);
    if (!child->mm) {
        proc_free(child);
        return -1;
    }
    child->is_user = true;
    child->pml4 = child->mm->pml4;
    for (int fd = 0; fd < MAX_FDS; fd++)
        child->files[fd] = file_dup(parent->files[fd]);

    /* Bola ota bilan AYNAN bir xil joyga qaytadi - faqat RAX = 0. */
    struct interrupt_frame *cf = prepare_kstack(child);
    *cf = *f;
    cf->rax = 0;
    int pid = child->pid;
    proc_make_ready(child);
    return pid;
}
