/* =============================================================================
 *  arch/interrupts.c - barcha uzilishlarning markaziy "dispetcheri"
 * =============================================================================
 *
 *  isr.asm har qanday uzilishda interrupt_dispatch(frame) ni chaqiradi. Bu yerda
 *  vektor raqamiga qarab qaror qilamiz:
 *
 *     0..31    CPU exception    -> handle_exception(): tushuntirib, to'xtatamiz
 *    32..47    apparat IRQ      -> ro'yxatdan o'tgan drayver handleri + EOI
 *    boshqalar                  -> handlers[] jadvalidan (masalan, 0x80 syscall)
 *
 *  "Handler jadvali" yondashuvi drayverlarni bir-biridan ajratadi: klaviatura
 *  drayveri o'z handlerini irq_register() bilan qo'shadi, bu fayl klaviatura
 *  haqida hech narsa bilmaydi. Linux'dagi request_irq() ham shunday ishlaydi.
 * ============================================================================= */
#include "arch/interrupts.h"

#include "arch/cpu.h"
#include "arch/gdt.h"
#include "arch/idt.h"
#include "drivers/pic.h"
#include "lib/kprintf.h"
#include "lib/panic.h"

static interrupt_handler_t handlers[256];

/* Intel SDM 3A, 6.15-bo'lim: exception nomlari. */
static const char *const exception_names[32] = {
    "Divide Error (0 ga bo'lish)",       /* 0  #DE */
    "Debug",                             /* 1  #DB */
    "Non-Maskable Interrupt",            /* 2  NMI */
    "Breakpoint (int3)",                 /* 3  #BP */
    "Overflow",                          /* 4  #OF */
    "Bound Range Exceeded",              /* 5  #BR */
    "Invalid Opcode (noto'g'ri instruksiya)", /* 6 #UD */
    "Device Not Available",              /* 7  #NM */
    "Double Fault",                      /* 8  #DF */
    "Coprocessor Segment Overrun",       /* 9 */
    "Invalid TSS",                       /* 10 #TS */
    "Segment Not Present",               /* 11 #NP */
    "Stack-Segment Fault",               /* 12 #SS */
    "General Protection Fault",          /* 13 #GP */
    "Page Fault (sahifa xatosi)",        /* 14 #PF */
    "Reserved",                          /* 15 */
    "x87 Floating-Point",                /* 16 #MF */
    "Alignment Check",                   /* 17 #AC */
    "Machine Check",                     /* 18 #MC */
    "SIMD Floating-Point",               /* 19 #XM */
    "Virtualization",                    /* 20 #VE */
    "Control Protection",                /* 21 #CP */
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Hypervisor Injection",              /* 28 */
    "VMM Communication",                 /* 29 */
    "Security",                          /* 30 */
    "Reserved",                          /* 31 */
};

void dump_frame(const struct interrupt_frame *f)
{
    kprintf("  RIP=%016lx  CS=%04lx  RFLAGS=%016lx\n", f->rip, f->cs, f->rflags);
    kprintf("  RSP=%016lx  SS=%04lx  ERR=%lx\n", f->rsp, f->ss, f->error_code);
    kprintf("  RAX=%016lx RBX=%016lx RCX=%016lx\n", f->rax, f->rbx, f->rcx);
    kprintf("  RDX=%016lx RSI=%016lx RDI=%016lx\n", f->rdx, f->rsi, f->rdi);
    kprintf("  RBP=%016lx R8 =%016lx R9 =%016lx\n", f->rbp, f->r8, f->r9);
    kprintf("  R10=%016lx R11=%016lx R12=%016lx\n", f->r10, f->r11, f->r12);
    kprintf("  R13=%016lx R14=%016lx R15=%016lx\n", f->r13, f->r14, f->r15);
}

/* Page fault xato kodining bitlari (Intel SDM 4.7) - nima uchun xato bo'lganini
 * aytadi. Debug qilishda juda qimmatli. */
static void explain_page_fault(uint64_t err)
{
    kprintf("  Manzil (CR2) = %p\n", (void *)cpu_read_cr2());
    kprintf("  Sabab: %s, %s, %s%s\n",
            (err & 1) ? "ruxsat buzildi (sahifa bor)" : "sahifa mavjud emas",
            (err & 2) ? "YOZISH" : "O'QISH",
            (err & 4) ? "user rejimida" : "yadro rejimida",
            (err & 16) ? ", instruksiya o'qishda" : "");
}

/* Faqat stub - 7-bosqichda user dasturdagi xato butun tizimni emas, faqat
 * o'sha jarayonni o'ldiradigan qilamiz. */
static void handle_exception(struct interrupt_frame *frame)
{
    uint64_t v = frame->vector;

    /* int3 (breakpoint) - xato emas, debug uchun. Xabar berib davom etamiz. */
    if (v == 3) {
        kprintf("[int3] Breakpoint: RIP=%p - davom etamiz\n", (void *)frame->rip);
        return;
    }

    kprintf("\n!!! EXCEPTION %lu: %s\n", v, exception_names[v]);
    if (v == 14)
        explain_page_fault(frame->error_code);
    dump_frame(frame);
    panic("Yadroda qayta ishlanmagan exception (%s)", exception_names[v]);
}

/* isr.asm dan chaqiriladi. Bu funksiya uzilishlar O'CHIQ holda ishlaydi. */
void interrupt_dispatch(struct interrupt_frame *frame);
void interrupt_dispatch(struct interrupt_frame *frame)
{
    uint64_t v = frame->vector;

    if (v < 32) {
        handle_exception(frame);
        return;
    }

    if (v >= PIC_IRQ_BASE && v < PIC_IRQ_BASE + 16) {
        uint8_t irq = (uint8_t)(v - PIC_IRQ_BASE);
        if (pic_is_spurious(irq))
            return;
        /* EOI ni handlerdan OLDIN yuboramiz. Sabab: taymer handleri boshqa
         * jarayonga o'tib ketishi (context switch) mumkin va bu yerga ancha
         * vaqt qaytmaydi. Agar EOI yuborilmagan bo'lsa, PIC shu vaqt davomida
         * boshqa taymer uzilishlarini yubormaydi - tizim "qotadi".
         * Xavfsiz, chunki IF=0: yangi uzilish biz iretq qilmagunimizcha yoki
         * sti qilmagunimizcha kelmaydi. */
        pic_send_eoi(irq);
    }

    if (handlers[v])
        handlers[v](frame);
    else if (v >= PIC_IRQ_BASE && v < PIC_IRQ_BASE + 16)
        ;                               /* handleri yo'q IRQ - e'tiborsiz qoldiramiz */
    else
        kprintf("[int] Kutilmagan uzilish: vektor %lu\n", v);
}

void interrupt_register(uint8_t vector, interrupt_handler_t handler)
{
    handlers[vector] = handler;
}

void irq_register(uint8_t irq, interrupt_handler_t handler)
{
    handlers[PIC_IRQ_BASE + irq] = handler;
    pic_unmask(irq);
}

void interrupts_init(void)
{
    gdt_init();                         /* to'liq GDT + TSS */
    idt_init();                         /* 256 ta darvoza */
    pic_init();                         /* IRQ'larni 32..47 ga ko'chirish, hammasini niqoblash */
}
