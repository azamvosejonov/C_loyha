/* =============================================================================
 *  proc/exec.c - user dasturini ishga tushirish (proc_spawn)
 * =============================================================================
 *
 *  Unix'da yangi dastur ikki qadamda ishga tushadi: fork() (nusxa) + exec()
 *  (almashtirish). Biz soddaroq "spawn" modelini tanladik (Windows'dagi
 *  CreateProcess va POSIX'dagi posix_spawn kabi): bitta chaqiruv yangi
 *  jarayon yaratadi va unga dasturni yuklaydi.
 *
 *  QADAMLAR:
 *    1. initrd dan faylni topish
 *    2. struct process + yadro steki (proc_alloc)
 *    3. yangi manzil maydoni (vmm_create_address_space)
 *    4. ELF segmentlarini yuklash (elf_load)
 *    5. user stekini ajratish va unga argc/argv ni joylash
 *    6. yadro stekiga "soxta uzilish freymi" qo'yish: scheduler bu jarayonga
 *       birinchi marta o'tganda, context_switch -> interrupt_return -> iretq
 *       CPU'ni ring 3 ga, dasturning _start nuqtasiga "qaytaradi".
 *    7. fd 0/1/2 = konsol, READY holatiga o'tkazish
 *
 *  USER STEKI (argv joylashuvi), USER_STACK_TOP dan pastga:
 *
 *    USER_STACK_TOP ┌────────────────────┐
 *                   │ "hello\0" "-v\0"   │ <- argv satrlari
 *                   ├────────────────────┤ (16 ga tekislash)
 *                   │ argv[0] ───────────┼──► "hello"
 *                   │ argv[1] ───────────┼──► "-v"
 *                   │ argv[2] = NULL     │
 *        RSP ─────► └────────────────────┘   RDI = argc, RSI = &argv[0]
 * ============================================================================= */
#include "proc/process.h"

#include "arch/gdt.h"
#include "arch/interrupts.h"
#include "fs/file.h"
#include "fs/tarfs.h"
#include "lib/common.h"
#include "lib/string.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "sys/elf.h"

extern void interrupt_return(void);    /* isr.asm */
extern void new_proc_start(void);      /* switch.asm */

#define MAX_ARG_BYTES 2048              /* argv satrlari uchun jami chegara */

/* argv ni user stekiga joylash. Qaytaradi: yangi RSP (0 = xato). */
static uint64_t setup_user_stack(uint64_t pml4, int argc, char *const argv[], uint64_t *argv_va)
{
    uint64_t sp = USER_STACK_TOP;
    uint64_t ptrs[MAX_ARGS + 1];

    /* Satrlarni teskari tartibda stekka ko'chiramiz. */
    for (int i = argc - 1; i >= 0; i--) {
        size_t len = strlen(argv[i]) + 1;
        sp -= len;
        if (!vmm_copy_to_space(pml4, sp, argv[i], len))
            return 0;
        ptrs[i] = sp;
    }
    ptrs[argc] = 0;                     /* argv[argc] = NULL (C standarti talabi) */

    sp = ALIGN_DOWN(sp, 16);
    sp -= (uint64_t)(argc + 1) * sizeof(uint64_t);
    sp = ALIGN_DOWN(sp, 16);            /* ABI: _start da RSP 16 ga karrali */
    if (!vmm_copy_to_space(pml4, sp, ptrs, (size_t)(argc + 1) * sizeof(uint64_t)))
        return 0;
    *argv_va = sp;
    return sp;
}

int proc_spawn(const char *path, int argc, char *const argv[])
{
    if (argc < 0 || argc > MAX_ARGS)
        return -1;
    size_t total = 0;
    for (int i = 0; i < argc; i++)
        total += strlen(argv[i]) + 1;
    if (total > MAX_ARG_BYTES)
        return -1;

    /* 1. Faylni topamiz. */
    const struct tar_file *tf = tarfs_find(path);
    if (!tf)
        return -2;                      /* "fayl topilmadi" */

    /* 2. Jarayon. Nomi - yo'lning oxirgi qismi. */
    const char *name = path;
    for (const char *s = path; *s; s++)
        if (*s == '/')
            name = s + 1;
    struct process *p = proc_alloc(name);
    if (!p)
        return -3;
    p->is_user = true;

    /* 3. Manzil maydoni. */
    p->pml4 = vmm_create_address_space();
    if (!p->pml4) {
        proc_free(p);
        return -4;
    }

    /* 4. ELF. */
    uint64_t entry, image_end;
    int err = elf_load(p->pml4, tf->data, tf->size, &entry, &image_end);
    if (err < 0) {
        proc_free(p);
        return -5;
    }
    p->heap_start = p->brk = ALIGN_UP(image_end, PAGE_SIZE);

    /* 5. User steki: USER_STACK_TOP ostida USER_STACK_PAGES sahifa. Undan
     * pastdagi sahifa xaritalanmaydi - stek to'lsa page fault (himoya). */
    uint64_t stack_bottom = USER_STACK_TOP - USER_STACK_PAGES * PAGE_SIZE;
    if (!vmm_map_anonymous(p->pml4, stack_bottom, USER_STACK_PAGES, PTE_USER | PTE_WRITABLE)) {
        proc_free(p);
        return -6;
    }
    uint64_t argv_va;
    uint64_t user_sp = setup_user_stack(p->pml4, argc, argv, &argv_va);
    if (!user_sp) {
        proc_free(p);
        return -7;
    }

    /* 6. Yadro stekida soxta uzilish freymi. interrupt_return uni "tiklab",
     * iretq qiladi -> ring 3, RIP = entry. */
    uint64_t kstack_top = p->kstack_base + KSTACK_PAGES * PAGE_SIZE;
    struct interrupt_frame *f = (struct interrupt_frame *)(kstack_top - sizeof(*f));
    memset(f, 0, sizeof(*f));
    f->rip = entry;                     /* dastur shu yerdan boshlanadi (_start) */
    f->cs = GDT_USER_CODE;              /* 0x23: ring 3 kod segmenti */
    f->rflags = 0x202;                  /* IF=1 (uzilishlar yoqilgan) + 1-bit (doim 1) */
    f->rsp = user_sp;                   /* user steki */
    f->ss = GDT_USER_DATA;              /* 0x1B: ring 3 stek segmenti */
    f->rdi = (uint64_t)argc;            /* main(argc, argv) */
    f->rsi = argv_va;

    /* context_switch uchun freym: `ret` -> new_proc_start, u proc_lock ni
     * qo'yib yuborib (scheduler uni ushlab turgan edi!), R13 ga - ya'ni
     * interrupt_return ga sakraydi. Freym 16 ga tekis, shuning uchun `ret`
     * dan keyin RSP ham tekis - call uchun ABI talabi bajariladi. */
    uint64_t *sp = (uint64_t *)f;
    *--sp = (uint64_t)new_proc_start;
    *--sp = 0;                          /* RBX */
    *--sp = 0;                          /* RBP */
    *--sp = 0;                          /* R12 */
    *--sp = (uint64_t)interrupt_return; /* R13 - davom */
    *--sp = 0;                          /* R14 */
    *--sp = 0;                          /* R15 */
    p->kernel_rsp = (uint64_t)sp;

    /* 7. Standart fayllar. */
    p->files[0] = file_console();       /* stdin */
    p->files[1] = file_console();       /* stdout */
    p->files[2] = file_console();       /* stderr */

    int pid = p->pid;
    proc_make_ready(p);
    return pid;
}
