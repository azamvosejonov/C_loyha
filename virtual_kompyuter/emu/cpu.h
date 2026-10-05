/*
 * cpu.h — protsessor (CPU) holati: registrlar, imtiyoz rejimi, CSR'lar, TLB.
 *
 * RISC-V NIMA?
 *   RISC-V — ochiq protsessor arxitekturasi (buyruqlar to'plami, ISA). Bizning protsessor — "RV32IMAC":
 *     RV32 — 32 bitli registrlar va manzillar;
 *     I    — asosiy butun son buyruqlari (qo'shish, solishtirish, sakrash, xotira);
 *     M    — ko'paytirish va bo'lish (mul, div, rem);
 *     A    — atomik buyruqlar (lr/sc, amoadd ...): ko'p oqimli dasturlar va qulflar uchun;
 *     C    — siqilgan (16 bitli) buyruqlar: kod 25-30% kichikroq bo'ladi.
 *   Qo'shimcha: Zicsr (CSR registrlari), Zifencei va "imtiyozli arxitektura" (M, S, U rejimlari,
 *   trap'lar, trap delegatsiyasi, Sv32 virtual xotira, Sstc taymeri).
 *
 * EMULYATOR QANDAY ISHLAYDI (umumiy)
 *   Haqiqiy protsessor abadiy shu siklni bajaradi:
 *       1) uzilish kutayaptimi va ruxsat bormi? bo'lsa — trap'ga o'tish;
 *       2) pc manzilidan buyruqni o'qish (FETCH) — 2 yoki 4 bayt;
 *       3) buyruqni maydonlarga ajratish (DECODE);
 *       4) bajarish (EXECUTE): registrlar/xotirani o'zgartirish;
 *       5) pc ni keyingi buyruqqa o'tkazish.
 *   Bizning cpu_qadam() aynan shu besh qadamni bitta buyruq uchun bajaradi.
 *
 * IMTIYOZ REJIMLARI (privilege modes) — xavfsizlikning asosi
 *   U (User, 0)       — oddiy dasturlar: CSR'larga, qurilmalarga, boshqa jarayon xotirasiga tegolmaydi.
 *   S (Supervisor, 1) — operatsion tizim yadrosi: sahifa jadvallari, jarayonlar, tizim chaqiruvlari.
 *   M (Machine, 3)    — eng yuqori: firmware (OpenSBI kabi). Hamma narsaga kira oladi, apparatni boshqaradi,
 *                       yadroga "xizmatlar" (taymer, quvvatni o'chirish) taqdim etadi.
 *   Yoqilganda protsessor M rejimda boshlanadi. Firmware hamma narsani sozlab, `mret` bilan yadroni S
 *   rejimda ishga tushiradi; yadro esa `sret` bilan dasturlarni U rejimda.
 */
#ifndef CPU_H
#define CPU_H

#include "turlar.h"

enum rejim {
    REJIM_U = 0,                                /* raqamlar spetsifikatsiyadagidek (2 — band, ishlatilmaydi) */
    REJIM_S = 1,
    REJIM_M = 3,
};

/* xotiraga murojaat turi: sahifa ruxsatlari va istisno raqami shunga bog'liq */
enum kirish {
    KIRISH_OQISH,                               /* load: lw, lb, lr.w ... */
    KIRISH_YOZISH,                              /* store: sw, sb, sc.w, amo* ... */
    KIRISH_BAJARISH,                            /* fetch: buyruqni o'qish */
};

/*
 * TRAP SABABLARI (mcause/scause registriga yoziladigan qiymatlar).
 * "Trap" — protsessor oddiy oqimni to'xtatib, trap ishlovchisiga (mtvec yoki stvec manziliga) o'tishi:
 *   - ISTISNO (exception): aynan shu buyruq sabab — noto'g'ri buyruq, sahifa xatosi, ecall ...
 *   - UZILISH (interrupt): tashqi hodisa — taymer, qurilma. Sababning 31-biti = 1.
 */
enum sabab {
    SABAB_BUYRUQ_TEKIS_EMAS = 0,                /* pc 2 ga (C bo'lmasa 4 ga) karrali emas */
    SABAB_BUYRUQ_KIRISH = 1,                    /* instruction access fault: pc da xotira yo'q */
    SABAB_NOTOGRI_BUYRUQ = 2,                   /* illegal instruction */
    SABAB_BREAKPOINT = 3,                       /* ebreak */
    SABAB_OQISH_TEKIS_EMAS = 4,                 /* lw manzili 4 ga karrali emas */
    SABAB_OQISH_KIRISH = 5,                     /* load access fault */
    SABAB_YOZISH_TEKIS_EMAS = 6,                /* store/AMO address misaligned */
    SABAB_YOZISH_KIRISH = 7,                    /* store/AMO access fault */
    SABAB_ECALL_U = 8,                          /* U rejimdan ecall — tizim chaqiruvi */
    SABAB_ECALL_S = 9,                          /* S rejimdan ecall — yadro firmware'dan xizmat so'raydi (SBI) */
    SABAB_ECALL_M = 11,                         /* M rejimdan ecall */
    SABAB_BUYRUQ_SAHIFA = 12,                   /* instruction page fault */
    SABAB_OQISH_SAHIFA = 13,                    /* load page fault */
    SABAB_YOZISH_SAHIFA = 15,                   /* store/AMO page fault */
};

#define UZILISH_BITI 0x80000000u                /* mcause[31]: 1 — uzilish, 0 — istisno */

/* Uzilish raqamlari = mip/mie dagi bit raqamlari */
#define IRQ_S_DASTURIY 1u                       /* SSIP */
#define IRQ_M_DASTURIY 3u                       /* MSIP — CLINT msip registri */
#define IRQ_S_TAYMER 5u                         /* STIP */
#define IRQ_M_TAYMER 7u                         /* MTIP — CLINT: mtime >= mtimecmp */
#define IRQ_S_TASHQI 9u                         /* SEIP — PLIC (S konteksti) */
#define IRQ_M_TASHQI 11u                        /* MEIP — PLIC (M konteksti) */

#define MIP_SSIP (1u << IRQ_S_DASTURIY)
#define MIP_MSIP (1u << IRQ_M_DASTURIY)
#define MIP_STIP (1u << IRQ_S_TAYMER)
#define MIP_MTIP (1u << IRQ_M_TAYMER)
#define MIP_SEIP (1u << IRQ_S_TASHQI)
#define MIP_MEIP (1u << IRQ_M_TASHQI)
#define MIP_S_BITLAR (MIP_SSIP | MIP_STIP | MIP_SEIP)
#define MIP_HAMMASI (MIP_S_BITLAR | MIP_MSIP | MIP_MTIP | MIP_MEIP)

/* CSR manzillari (spetsifikatsiyadan) */
#define CSR_SSTATUS 0x100
#define CSR_SIE 0x104
#define CSR_STVEC 0x105
#define CSR_SCOUNTEREN 0x106
#define CSR_SENVCFG 0x10A
#define CSR_SSCRATCH 0x140
#define CSR_SEPC 0x141
#define CSR_SCAUSE 0x142
#define CSR_STVAL 0x143
#define CSR_SIP 0x144
#define CSR_STIMECMP 0x14D                      /* Sstc kengaytmasi: S rejim o'z taymerini firmware'siz boshqaradi */
#define CSR_STIMECMPH 0x15D
#define CSR_SATP 0x180

#define CSR_MSTATUS 0x300
#define CSR_MISA 0x301
#define CSR_MEDELEG 0x302
#define CSR_MIDELEG 0x303
#define CSR_MIE 0x304
#define CSR_MTVEC 0x305
#define CSR_MCOUNTEREN 0x306
#define CSR_MENVCFG 0x30A
#define CSR_MSTATUSH 0x310
#define CSR_MENVCFGH 0x31A
#define CSR_MSCRATCH 0x340
#define CSR_MEPC 0x341
#define CSR_MCAUSE 0x342
#define CSR_MTVAL 0x343
#define CSR_MIP 0x344
#define CSR_PMPCFG0 0x3A0                       /* 0x3A0..0x3A3 */
#define CSR_PMPADDR0 0x3B0                      /* 0x3B0..0x3BF */
#define CSR_MCYCLE 0xB00
#define CSR_MINSTRET 0xB02
#define CSR_MCYCLEH 0xB80
#define CSR_MINSTRETH 0xB82
#define CSR_CYCLE 0xC00
#define CSR_TIME 0xC01
#define CSR_INSTRET 0xC02
#define CSR_CYCLEH 0xC80
#define CSR_TIMEH 0xC81
#define CSR_INSTRETH 0xC82
#define CSR_MVENDORID 0xF11
#define CSR_MARCHID 0xF12
#define CSR_MIMPID 0xF13
#define CSR_MHARTID 0xF14

/* mstatus bitlari. sstatus — mstatus ning S rejimga ko'rinadigan QISMI (alohida registr emas!) */
#define MSTATUS_SIE (1u << 1)                   /* S rejimda uzilishlar yoqilganmi */
#define MSTATUS_MIE (1u << 3)                   /* M rejimda uzilishlar yoqilganmi */
#define MSTATUS_SPIE (1u << 5)                  /* S trap'idan oldingi SIE */
#define MSTATUS_MPIE (1u << 7)                  /* M trap'idan oldingi MIE */
#define MSTATUS_SPP (1u << 8)                   /* S trap'idan oldingi rejim: 0 — U, 1 — S */
#define MSTATUS_MPP_SILJISH 11
#define MSTATUS_MPP (3u << MSTATUS_MPP_SILJISH) /* M trap'idan oldingi rejim: 0, 1 yoki 3 */
#define MSTATUS_MPRV (1u << 17)                 /* M rejimda load/store MPP rejimi nomidan (tarjima bilan) bajariladi */
#define MSTATUS_SUM (1u << 18)                  /* S rejim U-sahifalarni o'qiy/yoza oladimi */
#define MSTATUS_MXR (1u << 19)                  /* bajariladigan sahifani o'qish mumkinmi */
#define MSTATUS_TVM (1u << 20)                  /* S rejimda satp/sfence.vma ni taqiqlash */
#define MSTATUS_TW (1u << 21)                   /* S rejimda wfi ni taqiqlash */
#define MSTATUS_TSR (1u << 22)                  /* S rejimda sret ni taqiqlash */
#define SSTATUS_MASKA (MSTATUS_SIE | MSTATUS_SPIE | MSTATUS_SPP | MSTATUS_SUM | MSTATUS_MXR)
#define MSTATUS_MASKA (SSTATUS_MASKA | MSTATUS_MIE | MSTATUS_MPIE | MSTATUS_MPP | MSTATUS_MPRV | MSTATUS_TVM | \
                       MSTATUS_TW | MSTATUS_TSR)

/* misa: MXL=1 (32 bit) va kengaytmalar harflari bitlari (A=0, B=1, C=2, ... harf - 'A') */
#define MISA_QIYMATI ((1u << 30) | (1u << ('A' - 'A')) | (1u << ('C' - 'A')) | (1u << ('I' - 'A')) | \
                      (1u << ('M' - 'A')) | (1u << ('S' - 'A')) | (1u << ('U' - 'A')))

/* menvcfg (64 bit): 63-bit STCE — Sstc yoqilgan (S rejim stimecmp dan foydalana oladi) */
#define MENVCFGH_STCE (1u << 31)

/* TLB — "translation lookaside buffer": virtual -> fizik tarjimalar keshi (mmu.c ga qarang) */
#define TLB_HAJM 64

struct tlb_yozuv {
    uint32_t vpn;                               /* virtual sahifa raqami (manzil >> 12) */
    uint32_t pte;                               /* shu sahifaning PTE si (ruxsatlar va fizik sahifa) */
    uint32_t pte_manzil;                        /* PTE xotirada qayerda (A/D bitlarini yangilash uchun) */
    uint32_t asid;                              /* qaysi manzil maydoniga (jarayonga) tegishli — satp.ASID */
    int daraja;                                 /* 1 — 4 MB katta sahifa, 0 — 4 KB oddiy sahifa */
    int bor;                                    /* yozuv to'ldirilganmi */
};

struct cpu {
    uint32_t x[32];                             /* umumiy registrlar x0..x31. x0 doim 0 (yozish e'tiborsiz) */
    uint32_t pc;                                /* keyingi bajariladigan buyruq manzili */
    enum rejim rejim;

    /* M rejim CSR'lari */
    uint32_t mstatus, medeleg, mideleg, mie, mtvec, mscratch, mepc, mcause, mtval, mcounteren;
    uint32_t mip_dasturiy;                      /* mip ning dastur yozadigan bitlari: SSIP, STIP, SEIP */
    uint32_t menvcfgh;
    uint32_t pmpcfg[4], pmpaddr[16];            /* PMP: saqlanadi, lekin TEKSHIRILMAYDI (soddalashtirish) */

    /* S rejim CSR'lari (sstatus, sie, sip — mstatus/mie/mip ning ko'rinishlari, alohida joy yo'q) */
    uint32_t stvec, sscratch, sepc, scause, stval, satp, scounteren;
    uint64_t stimecmp;                          /* Sstc: time >= stimecmp bo'lsa STIP */

    uint64_t instret;                           /* bajarilgan buyruqlar soni. Bizda mtime = time = cycle = instret */
    int kutmoqda;                               /* wfi bajarildi: uzilish kelguncha "uxlaydi" */

    /* A kengaytmasi: lr.w qo'ygan "band qilish" (reservation) */
    int band_bor;
    uint32_t band_manzil;

    struct tlb_yozuv tlb[TLB_HAJM];
    uint64_t tlb_topildi, tlb_topilmadi;        /* statistika: -s bilan chiqariladi */
};

struct mashina;

/* bitta buyruqni bajaradi (yoki trap'ga o'tadi). cpu.c */
void cpu_qadam(struct mashina *m);

/* trap'ga kirish: delegatsiyaga qarab M yoki S rejimga o'tadi. trap.c */
void trap_kirish(struct cpu *c, uint32_t sabab, uint32_t tval);
void trap_qaytish_s(struct cpu *c);             /* sret */
void trap_qaytish_m(struct cpu *c);             /* mret */
/* uzilish kutayaptimi va qabul qilinadimi? Ha bo'lsa — trap'ga kiradi va 1 qaytaradi. trap.c */
int uzilish_tekshir(struct mashina *m);

/* CSR o'qish/yozish. 0 — OK, aks holda SABAB_NOTOGRI_BUYRUQ. csr.c */
int csr_oqi(struct mashina *m, uint32_t raqam, uint32_t *qiymat);
int csr_yoz(struct mashina *m, uint32_t raqam, uint32_t qiymat);
/* mip ning joriy qiymati: qurilmalar (CLINT, PLIC, Sstc) va dasturiy bitlar birga. csr.c */
uint32_t mip_qiymati(struct mashina *m);

/* virtual -> fizik manzil (Sv32). 0 — OK, aks holda istisno sababi. mmu.c */
int mmu_tarjima(struct mashina *m, uint32_t va, enum kirish tur, uint32_t *fiz);
void tlb_tozala(struct cpu *c);

#endif
