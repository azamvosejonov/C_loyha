/* =============================================================================
 *  main.c - yadroning C tilidagi kirish nuqtasi
 * =============================================================================
 *
 *  boot.asm 64-bitli rejimga o'tib, shu kmain() funksiyasini chaqiradi.
 *  Bu yerda tizimning har bir qismi to'g'ri TARTIBDA ishga tushiriladi.
 *  Tartib muhim: masalan, xotira menejerisiz heap ishlamaydi, heap'siz
 *  jarayon yarata olmaymiz.
 *
 *     console -> interrupts -> pmm -> vmm -> (heap) -> proc -> qurilmalar -> sti
 *        -> "init" yadro oqimi (pid 1) -> kmain o'zi "idle" (pid 0) ga aylanadi
 * ============================================================================= */
#include <stdint.h>

#include "arch/cpu.h"
#include "arch/interrupts.h"
#include "boot/multiboot.h"
#include "drivers/console.h"
#include "drivers/keyboard.h"
#include "drivers/pit.h"
#include "drivers/vga.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/string.h"
#include "mm/heap.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "proc/process.h"
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

/* ---- 6-bosqich namoyishi: bir vaqtda ishlaydigan yadro oqimlari ---- */

static int ticker_thread(void *arg)
{
    const char *label = arg;
    for (int i = 1; i <= 3; i++) {
        kprintf("[%s] %d-marta, tik=%lu\n", label, i, timer_ticks());
        proc_sleep_ms(label[0] == 'A' ? 200 : 300);
    }
    return 0;
}

static int echo_thread(void *arg)
{
    (void)arg;
    kprintf("Klaviaturada yozing (echo oqimi):\n> ");
    for (;;) {
        int c = console_getc();         /* bu oqim uxlaydi - CPU boshqalarga qoladi */
        if (c < 0)
            return 0;
        if (c == '\n')
            kprintf("\n> ");
        else
            console_putc((char)c);
    }
}

/* pid 1: "init" yadro oqimi. Uzilishlar yoqilgan, scheduler ishlab turgan
 * muhitda kerakli ishlarni bajaradi. */
static int init_thread(void *arg)
{
    (void)arg;
    if (cmdline_has("selftest"))
        selftest_run();
    maybe_run_crashdemo();

    proc_create_kernel_thread("ticker-A", ticker_thread, "A");
    proc_create_kernel_thread("ticker-B", ticker_thread, "B");
    proc_create_kernel_thread("echo", echo_thread, NULL);
    return 0;
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

    /* 7-qadam: qurilmalar va uzilishlarni yoqish. */
    pit_init();
    keyboard_init();
    console_enable_serial_input();
    proc_create_kernel_thread("init", init_thread, NULL);
    cpu_sti();                          /* Endi taymer "yuradi" va scheduler ishlaydi */
    kprintf("[int]  Uzilishlar yoqildi (taymer %d Hz)\n", TIMER_HZ);

    /* kmain hech qachon qaytmaydi: u idle jarayoniga aylanadi. */
    proc_idle_loop();
}
