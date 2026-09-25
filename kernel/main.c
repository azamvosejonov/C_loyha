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

#include "boot/multiboot.h"
#include "drivers/console.h"
#include "drivers/vga.h"
#include "lib/kprintf.h"
#include "lib/panic.h"

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
    kprintf("[boot] RAM: pastki %u KB, yuqori %u KB\n", mbi->mem_lower, mbi->mem_upper);

    kprintf("Salom, dunyo! Yadro ishlayapti.\n");

    for (;;)
        __asm__ volatile("hlt");
}
