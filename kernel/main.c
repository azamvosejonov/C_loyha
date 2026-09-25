/* =============================================================================
 *  main.c - yadroning C tilidagi kirish nuqtasi
 * =============================================================================
 *
 *  boot.asm yuqori yarimga o'tib, kmain(magic, mbi_phys) ni chaqiradi.
 *  ISHGA TUSHIRISH TARTIBI - har bir qadam oldingisiga tayanadi:
 *
 *    serial konsol          <- xatolarni ko'rish uchun eng birinchi
 *    bootinfo               <- GRUB ma'lumotini nusxalash (keyin uning joyi band emas)
 *    CPU imkoniyatlari      <- NX, 1 GB sahifa, PAT ...
 *    GDT/IDT                <- exception bo'lsa, tushunarli xabar chiqsin
 *    memblock               <- ilk allocator
 *    vmm                    <- yadroning doimiy sahifa jadvallari (identity yo'qoladi)
 *    buddy (pmm)            <- asosiy fizik allocator
 *    slab, vmalloc          <- kmalloc, himoyalangan yadro steklari, ioremap
 *    ekran                  <- framebuffer yoki VGA matn; log tarixi qayta chiqadi
 *    ACPI, TSC, Local APIC, IO APIC
 *    jarayonlar, fayllar, syscall, taymer, klaviatura
 *    SMP: qolgan CPU yadrolari uyg'otiladi
 *    init (pid 1) -> user shell; har bir CPU o'z scheduler tsiklida
 * ============================================================================= */
#include <stdint.h>

#include "acpi/acpi.h"
#include "arch/apic.h"
#include "arch/cpu.h"
#include "arch/interrupts.h"
#include "arch/percpu.h"
#include "arch/smp.h"
#include "arch/tsc.h"
#include "boot/bootinfo.h"
#include "drivers/console.h"
#include "drivers/fbcon.h"
#include "drivers/keyboard.h"
#include "drivers/pci.h"
#include "drivers/pit.h"
#include "drivers/vga.h"
#include "fs/tarfs.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/string.h"
#include "mm/memblock.h"
#include "mm/pmm.h"
#include "mm/slab.h"
#include "mm/vmalloc.h"
#include "mm/vmm.h"
#include "proc/process.h"
#include "sys/syscall.h"
#include "tests/crashdemo.h"
#include "tests/selftest.h"

static void maybe_run_crashdemo(void)
{
    char name[32];
    if (cmdline_get("demo", name, sizeof(name)))
        crashdemo_run(name);
}

/* ---- Namoyish: bir vaqtda ishlaydigan yadro oqimlari (APPEND=threads) ---- */
static int ticker_thread(void *arg)
{
    const char *label = arg;
    for (int i = 1; i <= 3; i++) {
        kprintf("[%s] %d-marta, tik=%lu\n", label, i, timer_ticks());
        proc_sleep_ms(label[0] == 'A' ? 200 : 300);
    }
    return 0;
}

/* pid 1: "init" yadro oqimi. Kerakli ishlarni bajaradi, keyin birinchi USER
 * dasturni - shell'ni ishga tushiradi va u tugasa qayta ishga tushiradi. */
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

static void screen_init(void)
{
    const struct screen_ops *ops = fbcon_init(&boot_info.fb);
    if (!ops && boot_info.fb.present && boot_info.fb.text_mode)
        ops = vga_text_init();
    if (!ops) {
        kprintf("[con]  Ekran yo'q - faqat serial port\n");
        return;
    }
    console_attach_screen(ops);         /* shu paytgacha yozilgan log ekranda paydo bo'ladi */
    kprintf("[con]  Ekran: %s\n", ops->name);
}

void kmain(uint32_t magic, uint32_t mbi_phys);

void kmain(uint32_t magic, uint32_t mbi_phys)
{
    percpu_init_bsp();                  /* gs:0 - spinlock'lar (push_off) buni ishlatadi */
    console_init_early();
    kprintf("\nMyOS - C tilida noldan yozilgan 64-bitli yadro\n");

    bootinfo_parse(magic, mbi_phys);
    cpu_detect();
    interrupts_init();                  /* GDT, TSS, IDT, PIC (uzilishlar hali o'chiq) */
    cpu_enable_features();
    bootinfo_dump();

    /* ---- Xotira ---- */
    memblock_init(&boot_info);
    vmm_init();
    pmm_init();
    vmm_late_init();
    slab_init();
    vmalloc_init();

    /* ---- Ekran ---- */
    screen_init();

    /* ---- Apparat: ACPI, vaqt, APIC ---- */
    acpi_init();
    tsc_calibrate();
    lapic_init_bsp();
    ioapic_init();

    /* ---- Jarayonlar va fayllar ---- */
    proc_init();
    tarfs_init(&boot_info);
    syscall_init();

    /* ---- Qurilmalar ---- */
    timer_init();
    keyboard_init();
    console_enable_serial_input();

    /* ---- PCI qurilmalari ---- */
    pci_init();

    /* ---- Boshqa CPU yadrolari ---- */
    smp_init();

    proc_create_kernel_thread("init", init_thread, NULL);
    kprintf("[int]  Uzilishlar yoqilmoqda (taymer %d Hz, %d CPU)\n", TIMER_HZ, ncpus);
    scheduler_loop();                   /* kmain BSP ning scheduler tsikliga aylanadi */
}
