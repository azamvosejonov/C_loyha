/* =============================================================================
 *  lib/panic.c - "yadro vahimasi" (kernel panic)
 * =============================================================================
 *
 *  Linux'dagi "Kernel panic" yoki Windows'dagi "ko'k ekran" ning analogi.
 *  Xabarni chiqaramiz, "backtrace" (qaysi funksiyalar orqali bu yerga
 *  kelganimiz)ni chiqaramiz va CPU'ni abadiy to'xtatamiz.
 *
 *  BACKTRACE QANDAY ISHLAYDI (-fno-omit-frame-pointer tufayli):
 *    Har bir funksiya boshida kompilyator shunday kod qo'yadi:
 *        push rbp        ; chaqiruvchining rbp sini saqlash
 *        mov rbp, rsp    ; rbp = joriy freym boshi
 *    Demak xotirada zanjir hosil bo'ladi:
 *        [rbp]     -> oldingi funksiyaning rbp si
 *        [rbp + 8] -> qaytish manzili (bizni chaqirgan joy)
 *    Shu zanjir bo'ylab yurib, har bir qaytish manzilini chiqaramiz. boot.asm
 *    rbp = 0 qo'ygan, shuning uchun zanjir o'sha yerda tugaydi.
 *    Manzilni funksiya nomiga aylantirish:  addr2line -f -e build/kernel.elf 0x...
 * ============================================================================= */
#include "lib/panic.h"

#include <stdarg.h>
#include <stdint.h>

#include "arch/cpu.h"
#include "drivers/console.h"
#include "lib/kprintf.h"
#include "lib/spinlock.h"
#include "arch/smp.h"

static void print_backtrace(void)
{
    uint64_t *rbp;
    __asm__ volatile("mov %%rbp, %0" : "=r"(rbp));  /* joriy freym ko'rsatkichi */

    kprintf("Backtrace (addr2line -f -e build/kernel.elf <manzil>):\n");
    for (int depth = 0; rbp && depth < 16; depth++) {
        /* Xavfsizlik: freym ko'rsatkichi aqlga sig'adigan manzilda bo'lishi kerak.
         * Buzilgan stekda zanjir istalgan joyga ko'rsatishi mumkin. */
        if ((uint64_t)rbp < 0x1000 || ((uint64_t)rbp & 7))
            break;
        uint64_t return_addr = rbp[1];
        if (return_addr == 0)
            break;
        kprintf("  #%d  %p\n", depth, (void *)return_addr);
        rbp = (uint64_t *)rbp[0];       /* zanjirning keyingi halqasi */
    }
}

void panic(const char *fmt, ...)
{
    cpu_cli();                          /* Boshqa hech narsa ishlamasin - uzilishlar ham */
    /* Qulflarni "buzamiz": panic konsol qulfi ushlangan paytda sodir bo'lgan
     * bo'lishi mumkin - aks holda xabarni chiqara olmay qotib qolardik. */
    spinlocks_busted = true;
    smp_stop_others();                  /* boshqa CPU'lar ham to'xtasin */

    console_set_color(COLOR_WHITE, COLOR_RED);
    kprintf("\n*** KERNEL PANIC ***\n");
    va_list ap;
    va_start(ap, fmt);
    kvprintf(fmt, ap);
    va_end(ap);
    kprintf("\n");
    print_backtrace();
    kprintf("Tizim to'xtatildi.\n");

    for (;;)                            /* abadiy to'xtash */
        cpu_hlt();
}
