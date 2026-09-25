/* =============================================================================
 *  arch/idt.c - Interrupt Descriptor Table (IDT)
 * =============================================================================
 *
 *  IDT - 256 ta yozuvli jadval: "N-uzilish kelsa, qaysi manzilga sakrash kerak".
 *    0..31   - CPU exception'lari (0 ga bo'lish, page fault, ...)
 *    32..47  - apparat uzilishlari (IRQ 0..15) - PIC ularni shu yerga ko'chiradi
 *    0x80    - bizning syscall vektorimiz (user dasturlar `int 0x80` qiladi)
 *
 *  64-BITLI IDT YOZUVI (16 bayt):
 *    offset_low   (16) - handler manzilining 0..15 bitlari
 *    selector     (16) - qaysi kod segmentida bajarilsin (yadro kodi 0x08)
 *    ist          (8)  - 0 = oddiy stek, 1..7 = TSS.ist[n-1] stekiga o'tish
 *    type_attr    (8)  - P (7-bit) | DPL (5-6) | tur (0..3): 0xE = interrupt gate
 *    offset_mid   (16) - 16..31 bitlar
 *    offset_high  (32) - 32..63 bitlar
 *    reserved     (32)
 *
 *  DPL MUHIM: `int N` instruksiyasini ring 3 dan chaqirish faqat yozuvning
 *  DPL=3 bo'lsa mumkin. Aks holda #GP (General Protection) xatosi. Shuning
 *  uchun faqat 0x80 ga DPL=3 beramiz - user dastur `int 14` qilib soxta
 *  page fault yarata olmasin.
 *
 *  INTERRUPT GATE vs TRAP GATE: interrupt gate kirishda IF=0 qiladi. Biz
 *  hamma joyda interrupt gate ishlatamiz - handler ichida boshqa uzilish
 *  kelmasligi mantiqni ancha soddalashtiradi.
 * ============================================================================= */
#include "arch/idt.h"

#include "arch/gdt.h"

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_pointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

#define IDT_INTERRUPT_GATE 0x8E         /* P=1, DPL=0, tur=0xE */
#define IDT_USER_GATE      0xEE         /* P=1, DPL=3, tur=0xE */

static struct idt_entry idt[256];

/* isr.asm dagi 256 ta stub manzili. */
extern uint64_t isr_stub_table[256];

static void idt_set_gate(int vector, uint64_t handler, uint8_t ist, uint8_t type_attr)
{
    struct idt_entry *e = &idt[vector];
    e->offset_low = handler & 0xFFFF;
    e->selector = GDT_KERNEL_CODE;      /* handler har doim yadro kodida (ring 0) */
    e->ist = ist;
    e->type_attr = type_attr;
    e->offset_mid = (handler >> 16) & 0xFFFF;
    e->offset_high = (uint32_t)(handler >> 32);
    e->reserved = 0;
}

void idt_init(void)
{
    for (int v = 0; v < 256; v++)
        idt_set_gate(v, isr_stub_table[v], 0, IDT_INTERRUPT_GATE);

    /* 8 - Double Fault: IST1 dagi toza stekda ishlasin (gdt.c ga qarang). */
    idt_set_gate(8, isr_stub_table[8], 1, IDT_INTERRUPT_GATE);

    /* 0x80 - syscall: user rejimidan chaqirish mumkin. */
    idt_set_gate(0x80, isr_stub_table[0x80], 0, IDT_USER_GATE);

    idt_load();
}

void idt_load(void)
{
    struct idt_pointer ptr = { .limit = sizeof(idt) - 1, .base = (uint64_t)idt };
    __asm__ volatile("lidt %0" : : "m"(ptr) : "memory");   /* IDTR registriga yuklash */
}
