/* =============================================================================
 *  main.c - yadroning C tilidagi kirish nuqtasi
 * =============================================================================
 *
 *  boot.asm 64-bitli rejimga o'tib, shu kmain() funksiyasini chaqiradi.
 *  Bu yerda tizimning har bir qismi to'g'ri TARTIBDA ishga tushiriladi.
 *  Tartib muhim: masalan, xotira menejerisiz heap ishlamaydi, heap'siz
 *  jarayon yarata olmaymiz.
 *
 *     console -> interrupts -> pmm -> vmm -> (heap) -> proc -> tarfs/syscall
 *        -> qurilmalar -> sti -> "init" yadro oqimi (pid 1) -> user shell
 *        kmain o'zi esa "idle" (pid 0) ga aylanadi
 * ============================================================================= */
#include <stdint.h>

#include "arch/cpu.h"
#include "arch/interrupts.h"
#include "boot/multiboot.h"
#include "drivers/console.h"
#include "drivers/keyboard.h"
#include "drivers/pit.h"
#include "drivers/vga.h"
#include "fs/tarfs.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/string.h"
#include "mm/heap.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "proc/process.h"
#include "sys/syscall.h"
#include "tests/crashdemo.h"
#include "tests/selftest.h"

/* Yadroga berilgan buyruq qatori (masalan "selftest"). */
static const char *kernel_cmdline = "";

/* Buyruq qatorida shu so'z bormi? (juda sodda tekshiruv) */
static int cmdline_has(const char *word)
{
    return strstr(kernel_cmdline, word) != NULL;
}

/* "demo=uaf" kabi parametr bo'lsa - xato namoyishini ishga tushiramiz. */
static void maybe_run_crashdemo(void)
{
    const char *demo = strstr(kernel_cmdline, "demo=");
    if (!demo)
        return;
    char name[32];
    size_t n = 0;
    for (demo += 5; demo[n] && demo[n] != ' ' && n < sizeof(name) - 1; n++)
        name[n] = demo[n];
    name[n] = '\0';
    crashdemo_run(name);
}

/* ---- 6-bosqich namoyishi: bir vaqtda ishlaydigan yadro oqimlari ----
 * make run APPEND=threads */
static int ticker_thread(void *arg)
{
    const char *label = arg;
    for (int i = 1; i <= 3; i++) {
        kprintf("[%s] %d-marta, tik=%lu\n", label, i, timer_ticks());
        proc_sleep_ms(label[0] == 'A' ? 200 : 300);
    }
    return 0;
}

/* pid 1: "init" yadro oqimi. Uzilishlar yoqilgan, scheduler ishlab turgan
 * muhitda kerakli ishlarni bajaradi, keyin birinchi USER dasturni - shell'ni
 * ishga tushiradi va u tugasa, qayta ishga tushiradi (Unix'dagi init/getty
 * kabi). */
static int init_thread(void *arg)
{
    (void)arg;
    if (cmdline_has("selftest"))
        selftest_run();
    maybe_run_crashdemo();
    if (cmdline_has("threads")) {
        int a = proc_create_kernel_thread("ticker-A", ticker_thread, "A");
        int b = proc_create_kernel_thread("ticker-B", ticker_thread, "B");
        proc_wait(a, NULL, false);
        proc_wait(b, NULL, false);
    }

    for (;;) {
        static char sh_name[] = "sh";
        char *argv[] = { sh_name };
        int pid = proc_spawn("sh", 1, argv);
        if (pid < 0)
            panic("init: /sh ni ishga tushirib bo'lmadi (xato %d)", pid);
        int code;
        proc_wait(pid, &code, false);
        kprintf("[init] shell tugadi (kod %d). Qayta ishga tushiramiz...\n", code);
    }
}

void kmain(uint32_t magic, uint32_t multiboot_info_phys);

void kmain(uint32_t magic, uint32_t multiboot_info_phys)
{
    /* 1-qadam: chiqarish. Birinchi navbatda - aks holda xatolarni ko'ra olmaymiz. */
    console_init();

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    kprintf("MyOS - C tilida noldan yozilgan 64-bitli yadro\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        panic("Multiboot magic noto'g'ri: %x", magic);

    /* Identity mapping tufayli fizik manzilni to'g'ridan-to'g'ri ko'rsatkich
     * sifatida ishlatish mumkin (birinchi 1 GB da). */
    struct multiboot_info *mbi = (struct multiboot_info *)(uintptr_t)multiboot_info_phys;
    if (mbi->flags & MB_INFO_CMDLINE)
        kernel_cmdline = (const char *)(uintptr_t)mbi->cmdline;
    kprintf("[boot] 64-bitli Long Mode faol. Buyruq qatori: \"%s\"\n", kernel_cmdline);

    /* 2-qadam: uzilishlar. GDT (TSS bilan), IDT, PIC. Hali IF=0. */
    interrupts_init();
    kprintf("[int]  GDT, TSS, IDT va PIC sozlandi\n");

    /* 3-qadam: fizik xotira. Endi bo'sh RAM freymlarini bera olamiz. */
    pmm_init(mbi);

    /* 4-qadam: virtual xotira. NULL himoyasi, jarayon manzil maydonlari. */
    vmm_init();

    /* 5-qadam: heap (kmalloc) - alohida ishga tushirish shart emas: birinchi
     * kmalloc() chaqiruvida slab'lar o'zi yaratiladi. */
    kprintf("[heap] Slab allocator tayyor (16..1024 bayt sinflar + katta ajratmalar)\n");

    /* 6-qadam: jarayonlar. kmain shu lahzadan 0-jarayon ("idle"). */
    proc_init();

    /* 7-qadam: user rejimi uchun: fayl tizimi (initrd) va syscall'lar. */
    tarfs_init(mbi);
    syscall_init();

    /* 8-qadam: qurilmalar va uzilishlarni yoqish. */
    pit_init();
    keyboard_init();
    console_enable_serial_input();
    proc_create_kernel_thread("init", init_thread, NULL);
    kprintf("[int]  Uzilishlar yoqilmoqda (taymer %d Hz)\n", TIMER_HZ);
    cpu_sti();                          /* Endi taymer "yuradi" va scheduler ishlaydi */

    /* kmain hech qachon qaytmaydi: u idle jarayoniga aylanadi. */
    proc_idle_loop();
}
