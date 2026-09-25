/* =============================================================================
 *  arch/gdt.c - to'liq GDT va TSS
 * =============================================================================
 *
 *  GDT NIMA UCHUN KERAK (64-bitli rejimda):
 *    32-bitli davrda segmentlar xotirani bo'laklarga ajratardi. 64-bitda baza va
 *    chegara deyarli e'tiborga olinmaydi, lekin GDT hali ham ikkita muhim narsani
 *    belgilaydi:
 *      1) IMTIYOZ DARAJASI (DPL): kod segmenti ring 0 (yadro) yoki ring 3 (user).
 *         CPU joriy darajani CS registridagi segmentdan biladi. User dasturni
 *         ishga tushirish = CS ga ring 3 segmentini yuklash.
 *      2) TSS (Task State Segment) ni ko'rsatish.
 *
 *  TSS NIMA UCHUN KERAK:
 *    User dasturi (ring 3) ishlayotganda uzilish keldi deylik. CPU ring 0 ga
 *    o'tadi, lekin QAYSI stekdan foydalanadi? User stekiga ishonib bo'lmaydi
 *    (u buzilgan yoki yolg'on manzil bo'lishi mumkin - xavfsizlik teshigi!).
 *    CPU yadro stekining manzilini TSS.rsp0 dan oladi. Shuning uchun har bir
 *    jarayonga o'tganda rsp0 ni o'sha jarayonning yadro stekiga o'rnatamiz.
 *
 *    TSS yana IST (Interrupt Stack Table) ni saqlaydi - ma'lum uzilishlar uchun
 *    MAXSUS stek. Biz uni "double fault" uchun ishlatamiz: agar yadro steki
 *    to'lib ketsa (stack overflow), oddiy stekda xato handleri ham ishlay
 *    olmaydi. IST alohida, toza stek beradi va biz hech bo'lmaganda xatoni
 *    ekranga chiqara olamiz.
 *
 *  GDT YOZUVI TUZILISHI (8 bayt, bizga kerakli bitlar):
 *    40..47 - "access" bayti:
 *        bit 47 P   - mavjud (present)
 *        bit 45..46 DPL - imtiyoz darajasi (0 yoki 3)
 *        bit 44 S   - 1 = kod/ma'lumot segmenti, 0 = tizim segmenti (TSS)
 *        bit 43 E   - 1 = kod (bajariladigan), 0 = ma'lumot
 *        bit 41 RW  - kod uchun: o'qish mumkin; ma'lumot uchun: yozish mumkin
 *    52..55 - bayroqlar: bit 53 L = 64-bitli kod segmenti
 *
 *  HAR BIR CPU'NING O'Z GDT VA TSS'I BOR (struct cpu ichida): TSS.rsp0 har
 *  CPU'da boshqa jarayonning yadro stekiga ko'rsatadi, shuning uchun uni
 *  umumiy qilib bo'lmaydi.
 *
 *  YOZUVLAR TARTIBI:
 *    0x00 null | 0x08 yadro kodi | 0x10 yadro ma'lumoti |
 *    0x18 user ma'lumoti | 0x20 user kodi | 0x28 TSS (16 bayt!)
 *    User ma'lumoti user kodidan OLDIN turishi - `syscall/sysret` instruksiyalari
 *    talabi. Biz hozircha `int 0x80` ishlatamiz, lekin kelajakda sysret'ga
 *    o'tish oson bo'lishi uchun shu tartibni saqlaymiz (docs/mashqlar.md).
 * ============================================================================= */
#include "arch/gdt.h"

#include "arch/percpu.h"
#include "lib/string.h"

#define IST_STACK_SIZE 8192

/* LGDT instruksiyasi o'qiydigan 10 baytlik struktura. */
struct gdt_pointer {
    uint16_t limit;                     /* jadval hajmi - 1 */
    uint64_t base;                      /* jadval manzili */
} __attribute__((packed));

/* BSP ning double fault steki (APlarniki smp.c da ajratiladi). */
static uint8_t bsp_ist_stack[IST_STACK_SIZE] __attribute__((aligned(16)));

/* gdt_load.asm da. */
extern void gdt_load(const struct gdt_pointer *ptr, uint64_t code_sel, uint64_t data_sel);
extern void tss_load(uint64_t tss_sel);

/* Oddiy kod/ma'lumot segmenti yozuvini yasash. */
static uint64_t make_segment(uint8_t access, uint8_t flags)
{
    return ((uint64_t)access << 40) | ((uint64_t)flags << 52);
}

void gdt_init_cpu(struct cpu *c)
{
    uint64_t *gdt = c->gdt;
    struct tss *tss = &c->tss;

    /* Access baytlari:
     *   0x9A = 1001 1010: P=1, DPL=0, S=1, E=1 (kod), RW=1  -> yadro kodi
     *   0x92 = 1001 0010: P=1, DPL=0, S=1, E=0 (ma'lumot), RW=1 -> yadro ma'lumoti
     *   0xF2 = 1111 0010: P=1, DPL=3, ... -> user ma'lumoti
     *   0xFA = 1111 1010: P=1, DPL=3, ... -> user kodi
     * Flags: 0x2 = L biti (64-bitli kod). Ma'lumot segmentlarida L = 0. */
    gdt[0] = 0;                                 /* NULL yozuvi - majburiy */
    gdt[1] = make_segment(0x9A, 0x2);           /* 0x08: yadro kodi */
    gdt[2] = make_segment(0x92, 0x0);           /* 0x10: yadro ma'lumoti */
    gdt[3] = make_segment(0xF2, 0x0);           /* 0x18: user ma'lumoti */
    gdt[4] = make_segment(0xFA, 0x2);           /* 0x20: user kodi */

    /* --- TSS: har bir CPU'ning O'Z TSS'i (rsp0 har CPU'da boshqa jarayon steki) --- */
    memset(tss, 0, sizeof(*tss));
    if (!c->ist_stack)
        c->ist_stack = bsp_ist_stack;
    tss->ist[0] = (uint64_t)(c->ist_stack + IST_STACK_SIZE);   /* IST1: double fault */
    tss->iopb_offset = sizeof(*tss);            /* I/O bitmap yo'q */

    /* TSS deskriptori 16 bayt; baza manzili tarixiy sabablarga ko'ra bo'laklangan. */
    uint64_t base = (uint64_t)tss;
    uint64_t limit = sizeof(*tss) - 1;
    gdt[5] = (limit & 0xFFFF)
           | ((base & 0xFFFFFF) << 16)
           | (0x89ULL << 40)                    /* P=1, DPL=0, tur=9 (64-bitli bo'sh TSS) */
           | (((limit >> 16) & 0xF) << 48)
           | (((base >> 24) & 0xFF) << 56);
    gdt[6] = base >> 32;

    struct gdt_pointer ptr = { .limit = GDT_ENTRIES * 8 - 1, .base = (uint64_t)gdt };
    gdt_load(&ptr, GDT_KERNEL_CODE, GDT_KERNEL_DATA);
    tss_load(GDT_TSS);
}

void tss_set_kernel_stack(uint64_t rsp0)
{
    struct cpu *c = this_cpu();
    c->tss.rsp0 = rsp0;
    c->kernel_rsp = rsp0;               /* syscall kirishi ham shu stekni ishlatadi */
}
