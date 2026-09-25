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
 *  YOZUVLAR TARTIBI:
 *    0x00 null | 0x08 yadro kodi | 0x10 yadro ma'lumoti |
 *    0x18 user ma'lumoti | 0x20 user kodi | 0x28 TSS (16 bayt!)
 *    User ma'lumoti user kodidan OLDIN turishi - `syscall/sysret` instruksiyalari
 *    talabi. Biz hozircha `int 0x80` ishlatamiz, lekin kelajakda sysret'ga
 *    o'tish oson bo'lishi uchun shu tartibni saqlaymiz (docs/mashqlar.md).
 * ============================================================================= */
#include "arch/gdt.h"

#include "lib/string.h"

/* 64-bitli TSS strukturasi (Intel SDM 3A, 8.7-bo'lim). packed - bo'sh joysiz. */
struct tss {
    uint32_t reserved0;
    uint64_t rsp0;                      /* ring 0 ga o'tganda ishlatiladigan stek */
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];                    /* IST1..IST7 - maxsus uzilish steklari */
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;               /* I/O ruxsat bitmap'i siljishi (bizda yo'q) */
} __attribute__((packed));

/* LGDT instruksiyasi o'qiydigan 10 baytlik struktura. */
struct gdt_pointer {
    uint16_t limit;                     /* jadval hajmi - 1 */
    uint64_t base;                      /* jadval manzili */
} __attribute__((packed));

/* 7 ta 8-baytlik yozuv: 5 ta segment + TSS (2 ta joy egallaydi). */
static uint64_t gdt[7];
static struct tss tss;

/* Double fault uchun alohida stek (IST1). 16 ga tekislangan. */
static uint8_t double_fault_stack[8192] __attribute__((aligned(16)));

/* gdt_load.asm da. */
extern void gdt_load(const struct gdt_pointer *ptr, uint64_t code_sel, uint64_t data_sel);
extern void tss_load(uint64_t tss_sel);

/* Oddiy kod/ma'lumot segmenti yozuvini yasash. */
static uint64_t make_segment(uint8_t access, uint8_t flags)
{
    return ((uint64_t)access << 40) | ((uint64_t)flags << 52);
}

void gdt_init(void)
{
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

    /* --- TSS --- */
    memset(&tss, 0, sizeof(tss));
    tss.ist[0] = (uint64_t)(double_fault_stack + sizeof(double_fault_stack));  /* IST1, steklar pastga o'sadi */
    tss.iopb_offset = sizeof(tss);              /* bitmap yo'q: TSS oxiriga ko'rsatamiz */

    /* TSS deskriptori 16 bayt ("tizim segmenti" 64-bitda kengaytirilgan).
     * Baza manzili bir nechta bo'lakka bo'lingan - tarixiy sabablar tufayli. */
    uint64_t base = (uint64_t)&tss;
    uint64_t limit = sizeof(tss) - 1;
    gdt[5] = (limit & 0xFFFF)                   /* chegara 0..15 bitlari */
           | ((base & 0xFFFFFF) << 16)          /* baza 0..23 bitlari */
           | (0x89ULL << 40)                    /* P=1, DPL=0, tur=9 (64-bitli bo'sh TSS) */
           | (((limit >> 16) & 0xF) << 48)      /* chegara 16..19 bitlari */
           | (((base >> 24) & 0xFF) << 56);     /* baza 24..31 bitlari */
    gdt[6] = base >> 32;                        /* baza 32..63 bitlari (ikkinchi 8 bayt) */

    static struct gdt_pointer ptr;
    ptr.limit = sizeof(gdt) - 1;
    ptr.base = (uint64_t)gdt;

    gdt_load(&ptr, GDT_KERNEL_CODE, GDT_KERNEL_DATA);  /* yangi GDT + segmentlarni qayta yuklash */
    tss_load(GDT_TSS);                                 /* TR registriga TSS selektori */
}

void tss_set_kernel_stack(uint64_t rsp0)
{
    tss.rsp0 = rsp0;
}
