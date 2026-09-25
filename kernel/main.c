/* =============================================================================
 *  main.c - yadroning C tilidagi kirish nuqtasi
 * =============================================================================
 *
 *  boot.asm 64-bitli rejimga o'tib, shu kmain() funksiyasini chaqiradi.
 *  Bu yerda tizimning har bir qismi to'g'ri TARTIBDA ishga tushiriladi.
 *  Tartib muhim: masalan, xotira menejerisiz heap ishlamaydi, heap'siz
 *  jarayon yarata olmaymiz.
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
#include "tests/crashdemo.h"
#include "tests/selftest.h"

/* Yadroga berilgan buyruq qatori (masalan "selftest"). */
static const char *kernel_cmdline = "";

/* Buyruq qatorida shu so'z bormi? (juda sodda tekshiruv) */
static int cmdline_has(const char *word)
{
    return strstr(kernel_cmdline, word) != NULL;
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
    kprintf("[boot] 64-bitli Long Mode faol. multiboot_info = %p\n", (void *)mbi);
    if (mbi->flags & MB_INFO_CMDLINE)
        kernel_cmdline = (const char *)(uintptr_t)mbi->cmdline;
    kprintf("[boot] Buyruq qatori: \"%s\"\n", kernel_cmdline);

    /* 2-qadam: uzilishlar. GDT (TSS bilan), IDT, PIC. */
    interrupts_init();
    kprintf("[int]  GDT, TSS, IDT va PIC sozlandi\n");

    /* int3 - "breakpoint" exception'ini ataylab chaqiramiz: IDT to'g'ri
     * ishlayotganini tekshirish uchun. Handler xabar berib, qaytadi. */
    __asm__ volatile("int3");

    /* 3-qadam: fizik xotira. Endi bo'sh RAM freymlarini bera olamiz. */
    pmm_init(mbi);

    /* 4-qadam: virtual xotira. NULL himoyasi, jarayon manzil maydonlari. */
    vmm_init();

    /* 5-qadam: heap (kmalloc). Bu ham selftest ichida sinovdan o'tadi. */
    kprintf("[heap] Slab allocator tayyor (16..1024 bayt sinflar + katta ajratmalar)\n");

    if (cmdline_has("selftest"))
        selftest_run();

    /* "demo=uaf" kabi parametr bo'lsa - xato namoyishini ishga tushiramiz. */
    const char *demo = strstr(kernel_cmdline, "demo=");
    if (demo) {
        char name[32];
        size_t n = 0;
        for (demo += 5; demo[n] && demo[n] != ' ' && n < sizeof(name) - 1; n++)
            name[n] = demo[n];
        name[n] = '\0';
        crashdemo_run(name);
    }

    /* 6-qadam: qurilmalar. */
    pit_init();
    keyboard_init();
    console_enable_serial_input();
    cpu_sti();                          /* Uzilishlarni YOQAMIZ - endi taymer "yuradi" */
    kprintf("[int]  Uzilishlar yoqildi (taymer %d Hz)\n", TIMER_HZ);

    kprintf("Klaviaturada yozing (echo):\n> ");
    for (;;) {
        char c = console_getc();
        if (c == '\n')
            kprintf("\n[tik: %lu] > ", timer_ticks());
        else
            console_putc(c);
    }
}
