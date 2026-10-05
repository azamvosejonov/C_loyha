/*
 * birlik.c — emulyatorning MASHQ funksiyalarini birma-bir tekshiradi (har biriga aniq xabar).
 *
 * Har guruh: "[ OK ] E1 imm_*: 12/12" yoki "[XATO] E5 jadval_yurish: 3/9" va qaysi holat buzilgani:
 *     imm_b(0x80731063) [bne t1, t2, -4096]  ->  olindi 0x00000000, kutilgan 0xfffff000
 * Test vektorlari (kutilgan qiymatlar) — haqiqiy assembler (clang) chiqishidan olingan.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alu.h"
#include "c_juftlar.h"
#include "dekod.h"
#include "mashina.h"
#include "siqilgan.h"

static const char *g_nom;
static int g_jami, g_xato, umumiy_xato;

#define BOSHLA(nom) (g_nom = (nom), g_jami = 0, g_xato = 0)
#define TENG(ifoda, kutilgan, izoh)                                                                     \
    do {                                                                                                \
        uint32_t t_o = (uint32_t)(ifoda), t_k = (uint32_t)(kutilgan);                                   \
        g_jami++;                                                                                       \
        if (t_o != t_k) {                                                                               \
            g_xato++;                                                                                   \
            if (g_xato <= 6)                                                                            \
                printf("        %-46s %-28s -> olindi 0x%08x, kutilgan 0x%08x\n", #ifoda, izoh, t_o, t_k); \
        }                                                                                               \
    } while (0)
#define TUGAT()                                                                                         \
    do {                                                                                                \
        printf("  [%s] %s: %d/%d\n", g_xato ? "XATO" : " OK ", g_nom, g_jami - g_xato, g_jami);         \
        umumiy_xato += g_xato;                                                                          \
    } while (0)

/* ---- sinov "kompyuteri": 1 MB RAM ---- */
static struct mashina M;

static void mashina_tayyorla(void)
{
    free(M.ram);
    memset(&M, 0, sizeof(M));
    M.ram_hajm = 1u << 20;
    M.ram = calloc(1, M.ram_hajm);
    M.cpu.stimecmp = UINT64_MAX;
    M.clint.mtimecmp = UINT64_MAX;
}

static void ram_yoz(uint32_t manzil, uint32_t q) { shina_yoz(&M, manzil, 4, q); }
static uint32_t ram_oqi(uint32_t manzil)
{
    uint32_t q = 0;
    shina_oqi(&M, manzil, 4, &q);
    return q;
}

static void e1_dekod(void)
{
    BOSHLA("E1 imm_i/s/b/u/j (dekod.c)");
    TENG(imm_i(0xFFF50513), 0xFFFFFFFF, "addi a0, a0, -1");
    TENG(imm_i(0x80050513), 0xFFFFF800, "addi a0, a0, -2048");
    TENG(imm_i(0x7FF50513), 0x000007FF, "addi a0, a0, 2047");
    TENG(imm_s(0x00B12423), 8, "sw a1, 8(sp)");
    TENG(imm_s(0x80B12023), 0xFFFFF800, "sw a1, -2048(sp)");
    TENG(imm_s(0x7E552FA3), 0x7FF, "sw t0, 2047(a0)");
    TENG(imm_b(0xFEB50EE3), 0xFFFFFFFC, "beq a0, a1, -4");
    TENG(imm_b(0x80731063), 0xFFFFF000, "bne t1, t2, -4096");
    TENG(imm_b(0x7EB50FE3), 0x00000FFE, "beq a0, a1, +4094");
    TENG(imm_u(0x12345537), 0x12345000, "lui a0, 0x12345");
    TENG(imm_u(0xFFFFF537), 0xFFFFF000, "lui a0, 0xfffff");
    TENG(imm_j(0x001000EF), 0x00000800, "jal ra, +2048");
    TENG(imm_j(0x7FFFF0EF), 0x000FFFFE, "jal ra, +1048574");
    TENG(imm_j(0x8000006F), 0xFFF00000, "jal zero, -1048576");
    TUGAT();
}

static void e2_e3_e4_alu(void)
{
    BOSHLA("E2 shart_bajarildimi (alu.c)");
    TENG(shart_bajarildimi(0, 5, 5), 1, "beq teng");
    TENG(shart_bajarildimi(0, 5, 6), 0, "beq teng emas");
    TENG(shart_bajarildimi(1, 5, 6), 1, "bne");
    TENG(shart_bajarildimi(4, 0xFFFFFFFF, 1), 1, "blt -1 < 1 (ishorali)");
    TENG(shart_bajarildimi(6, 0xFFFFFFFF, 1), 0, "bltu 0xffffffff < 1 (yo'q)");
    TENG(shart_bajarildimi(5, 7, 7), 1, "bge teng");
    TENG(shart_bajarildimi(5, 0x80000000, 0), 0, "bge INT_MIN >= 0 (yo'q)");
    TENG(shart_bajarildimi(7, 0x80000000, 0), 1, "bgeu 0x80000000 >= 0");
    TENG(shart_bajarildimi(2, 1, 1), (uint32_t)-1, "funct3=2 — mavjud emas");
    TENG(shart_bajarildimi(3, 1, 1), (uint32_t)-1, "funct3=3 — mavjud emas");
    TUGAT();

    BOSHLA("E3 yuklash_kengaytir (alu.c)");
    TENG(yuklash_kengaytir(0, 0x7F), 0x7F, "lb 0x7f");
    TENG(yuklash_kengaytir(0, 0x80), 0xFFFFFF80, "lb 0x80 (manfiy)");
    TENG(yuklash_kengaytir(0, 0x12345680), 0xFFFFFF80, "lb faqat past bayt");
    TENG(yuklash_kengaytir(1, 0x8000), 0xFFFF8000, "lh 0x8000");
    TENG(yuklash_kengaytir(1, 0x7FFF), 0x7FFF, "lh 0x7fff");
    TENG(yuklash_kengaytir(4, 0x80), 0x80, "lbu 0x80");
    TENG(yuklash_kengaytir(5, 0xFFFF8000), 0x8000, "lhu faqat past 16 bit");
    TENG(yuklash_kengaytir(2, 0xDEADBEEF), 0xDEADBEEF, "lw o'zgarmaydi");
    TUGAT();

    BOSHLA("E4 m_amal: mul/div/rem (alu.c)");
    TENG(m_amal(0, 7, (uint32_t)-3), (uint32_t)-21, "mul 7 * -3");
    TENG(m_amal(1, 0x80000000, 0x80000000), 0x40000000, "mulh (-2^31)^2");
    TENG(m_amal(3, 0xFFFFFFFF, 0xFFFFFFFF), 0xFFFFFFFE, "mulhu max^2");
    TENG(m_amal(2, 0xFFFFFFFF, 0xFFFFFFFF), 0xFFFFFFFF, "mulhsu -1 * max");
    TENG(m_amal(1, 0xFFFFFFFF, 0xFFFFFFFF), 0, "mulh -1 * -1");
    TENG(m_amal(4, (uint32_t)-7, 2), (uint32_t)-3, "div -7/2 (nolga qarab)");
    TENG(m_amal(6, (uint32_t)-7, 2), (uint32_t)-1, "rem -7%2");
    TENG(m_amal(5, (uint32_t)-7, 2), 0x7FFFFFFC, "divu");
    TENG(m_amal(7, (uint32_t)-7, 2), 1, "remu");
    TENG(m_amal(4, 5, 0), 0xFFFFFFFF, "div x/0 = -1");
    TENG(m_amal(5, 5, 0), 0xFFFFFFFF, "divu x/0 = max");
    TENG(m_amal(6, 5, 0), 5, "rem x%0 = x");
    TENG(m_amal(7, 5, 0), 5, "remu x%0 = x");
    TENG(m_amal(4, 0x80000000, 0xFFFFFFFF), 0x80000000, "div INT_MIN/-1 (toshish)");
    TENG(m_amal(6, 0x80000000, 0xFFFFFFFF), 0, "rem INT_MIN%-1");
    TUGAT();
}

/* Sv32: sahifa jadvalini RAM da quramiz va mmu_tarjima orqali tekshiramiz */
#define PTE(fizik, bayroq) ((((fizik) >> 12) << 10) | (bayroq))
enum { V = 1, R = 2, W = 4, X = 8, U = 16, A = 64, D = 128 };

static void e5_mmu(void)
{
    BOSHLA("E5 jadval_yurish: Sv32 (mmu.c)");
    mashina_tayyorla();
    struct cpu *c = &M.cpu;
    uint32_t j1 = RAM_BOSH + 0x10000, j0 = RAM_BOSH + 0x11000, sahifa = RAM_BOSH + 0x20000;
    c->satp = 0x80000000u | (j1 >> 12);
    c->rejim = REJIM_S;
    /* VPN1=0x100 (VA 0x4000_0000) -> j0;  j0[1] -> sahifa (R);  j0[2] -> sahifa (R W);  j0[3] -> W, R yo'q (taqiq) */
    ram_yoz(j1 + 0x100 * 4, PTE(j0, V));
    ram_yoz(j0 + 1 * 4, PTE(sahifa, V | R));
    ram_yoz(j0 + 2 * 4, PTE(sahifa, V | R | W));
    ram_yoz(j0 + 3 * 4, PTE(sahifa, V | W));
    /* VPN1=0x200 (VA 0x8000_0000) -> 4 MB katta sahifa (RAM_BOSH);  VPN1=0x201 -> NOTO'G'RI tekislangan katta sahifa */
    ram_yoz(j1 + 0x200 * 4, PTE(RAM_BOSH, V | R | W | X));
    ram_yoz(j1 + 0x201 * 4, PTE(RAM_BOSH + 0x1000, V | R));
    uint32_t f = 0;
    TENG(mmu_tarjima(&M, 0x40001234, KIRISH_OQISH, &f), 0, "4KB sahifa, o'qish");
    TENG(f, sahifa + 0x234, "fizik = sahifa + siljish");
    TENG(mmu_tarjima(&M, 0x40002ABC, KIRISH_YOZISH, &f), 0, "4KB, yozish ruxsati bor");
    TENG(f, sahifa + 0xABC, "fizik");
    TENG(mmu_tarjima(&M, 0x40001000, KIRISH_YOZISH, &f), SABAB_YOZISH_SAHIFA, "W=0 -> store page fault");
    TENG(mmu_tarjima(&M, 0x40005000, KIRISH_OQISH, &f), SABAB_OQISH_SAHIFA, "V=0 -> load page fault");
    TENG(mmu_tarjima(&M, 0x40005000, KIRISH_BAJARISH, &f), SABAB_BUYRUQ_SAHIFA, "V=0, fetch -> 12");
    TENG(mmu_tarjima(&M, 0x40003000, KIRISH_OQISH, &f), SABAB_OQISH_SAHIFA, "W=1 R=0 -> taqiqlangan PTE");
    TENG(mmu_tarjima(&M, 0x80123456, KIRISH_BAJARISH, &f), 0, "4MB katta sahifa");
    TENG(f, 0x80123456, "katta sahifa: 22 bit siljish");
    TENG(mmu_tarjima(&M, 0x80400000, KIRISH_OQISH, &f), SABAB_OQISH_SAHIFA, "katta sahifa 4MB ga tekis emas");
    TENG(ram_oqi(j0 + 2 * 4) & (A | D), A | D, "yozilgan sahifada A va D");
    TENG(ram_oqi(j0 + 1 * 4) & (A | D), A, "W=0 sahifa: o'qilgan (A), yozish rad etildi (D yo'q)");
    /* ASID: boshqa jarayonning jadvali (boshqa ASID), sfence.vma SIZ. Linux ASID bo'lsa TLB ni tozalamaydi —
       TLB eski jarayonning tarjimasini bermasligi kerak (bu emulyator infratuzilmasi, mmu_tarjima) */
    uint32_t j1b = RAM_BOSH + 0x12000, j0b = RAM_BOSH + 0x13000, sahifa_b = RAM_BOSH + 0x30000;
    ram_yoz(j1b + 0x100 * 4, PTE(j0b, V));
    ram_yoz(j0b + 1 * 4, PTE(sahifa_b, V | R));
    c->satp = 0x80000000u | (1u << 22) | (j1 >> 12);             /* ASID 1 */
    mmu_tarjima(&M, 0x40001000, KIRISH_OQISH, &f);              /* TLB ga tushdi */
    c->satp = 0x80000000u | (2u << 22) | (j1b >> 12);            /* ASID 2, boshqa jadval */
    mmu_tarjima(&M, 0x40001000, KIRISH_OQISH, &f);
    TENG(f, sahifa_b, "boshqa ASID: TLB dagi eski tarjima ishlatilmadi");
    TUGAT();
}

static void e6_trap(void)
{
    BOSHLA("E6 trap_kirish: delegatsiya (trap.c)");
    mashina_tayyorla();
    struct cpu *c = &M.cpu;
    c->stvec = 0x80001000;
    c->mtvec = 0x80002001;                      /* vektorli rejim */
    c->medeleg = 1u << SABAB_ECALL_U;
    c->mideleg = MIP_STIP;
    /* 1) U dan ecall, delegatsiya qilingan -> S */
    c->rejim = REJIM_U;
    c->pc = 0x10000;
    c->mstatus = MSTATUS_SIE | MSTATUS_SPP;
    trap_kirish(c, SABAB_ECALL_U, 0);
    TENG(c->rejim, REJIM_S, "U ecall -> S rejim");
    TENG(c->pc, 0x80001000, "pc = stvec");
    TENG(c->sepc, 0x10000, "sepc = trap joyi");
    TENG(c->scause, SABAB_ECALL_U, "scause");
    TENG(c->mstatus & MSTATUS_SPP, 0, "SPP = 0 (U dan keldi)");
    TENG(c->mstatus & (MSTATUS_SIE | MSTATUS_SPIE), MSTATUS_SPIE, "SPIE <- SIE(1), SIE <- 0");
    /* 2) S dan noto'g'ri buyruq, delegatsiya YO'Q -> M */
    c->pc = 0x80000500;
    c->mstatus = MSTATUS_MIE;
    trap_kirish(c, SABAB_NOTOGRI_BUYRUQ, 0xDEAD);
    TENG(c->rejim, REJIM_M, "delegatsiyasiz -> M");
    TENG(c->pc, 0x80002000, "pc = mtvec (istisno: vektorsiz)");
    TENG(c->mepc, 0x80000500, "mepc");
    TENG(c->mtval, 0xDEAD, "mtval");
    TENG((c->mstatus & MSTATUS_MPP) >> MSTATUS_MPP_SILJISH, REJIM_S, "MPP = 1 (S dan keldi)");
    TENG(c->mstatus & (MSTATUS_MIE | MSTATUS_MPIE), MSTATUS_MPIE, "MPIE <- MIE(1), MIE <- 0");
    /* 3) M dan delegatsiya qilingan sabab ham M da qoladi */
    c->medeleg = 0xFFFF;
    trap_kirish(c, SABAB_BREAKPOINT, 0);
    TENG(c->rejim, REJIM_M, "M dagi trap pastga tushmaydi");
    TENG((c->mstatus & MSTATUS_MPP) >> MSTATUS_MPP_SILJISH, REJIM_M, "MPP = 3");
    /* 4) M taymer uzilishi (delegatsiyasiz), vektorli mtvec: pc = asos + 4*7 */
    c->rejim = REJIM_S;
    trap_kirish(c, UZILISH_BITI | IRQ_M_TAYMER, 0);
    TENG(c->pc, 0x80002000 + 4 * 7, "vektorli: mtvec + 4*sabab");
    TENG(c->mcause, 0x80000007, "mcause (31-bit = uzilish)");
    /* 5) delegatsiya qilingan S taymer uzilishi, U dan */
    c->rejim = REJIM_U;
    trap_kirish(c, UZILISH_BITI | IRQ_S_TAYMER, 0);
    TENG(c->rejim, REJIM_S, "STI -> S");
    TENG(c->scause, 0x80000005, "scause");
    TUGAT();
}

static void e7_siqilgan(void)
{
    BOSHLA("E7 c_kengaytir: 16 bitli buyruqlar (siqilgan.c)");
    for (size_t i = 0; i < sizeof(c_juftlar) / sizeof(c_juftlar[0]); i++)
        TENG(c_kengaytir(c_juftlar[i].c), c_juftlar[i].kutilgan, c_juftlar[i].matn);
    TENG(c_kengaytir(0x0000), 0, "0x0000 — noto'g'ri");
    TENG(c_kengaytir(0x8002), 0, "c.jr x0 — taqiqlangan");
    TENG(c_kengaytir(0x4002), 0, "c.lwsp x0 — taqiqlangan");
    TENG(c_kengaytir(0x1002), 0, "c.slli shamt[5]=1 (RV32 da yo'q)");
    TENG(c_kengaytir(0x6101), 0, "c.addi16sp 0 — taqiqlangan");
    TUGAT();
}

static void e8_tekis_emas(void)
{
    BOSHLA("E8 tekis bo'lmagan murojaat (shina.c)");
    mashina_tayyorla();
    ram_yoz(RAM_BOSH + 0x100, 0x44332211);
    ram_yoz(RAM_BOSH + 0x104, 0x88776655);
    uint32_t q = 0;
    TENG(xotira_oqi(&M, RAM_BOSH + 0x101, 4, &q), 0, "lw manzil+1: istisno yo'q");
    TENG(q, 0x55443322, "baytlar to'g'ri yig'ildi");
    TENG(xotira_oqi(&M, RAM_BOSH + 0x103, 2, &q), 0, "lh manzil+3");
    TENG(q, 0x5544, "2 bayt, ikki so'z chegarasida");
    TENG(xotira_yoz(&M, RAM_BOSH + 0x102, 4, 0xAABBCCDD), 0, "sw manzil+2");
    TENG(ram_oqi(RAM_BOSH + 0x100), 0xCCDD2211, "past so'z");
    TENG(ram_oqi(RAM_BOSH + 0x104), 0x8877AABB, "yuqori so'z");
    TENG(xotira_oqi(&M, RAM_BOSH + M.ram_hajm - 2, 4, &q), SABAB_OQISH_KIRISH, "RAM oxiridan chiqish");
    TUGAT();
}

static void e9_uzilish(void)
{
    BOSHLA("E9 uzilish_tekshir (trap.c)");
    mashina_tayyorla();
    struct cpu *c = &M.cpu;
    c->mtvec = 0x80000100;
    c->stvec = 0x80000200;
    c->instret = 1000;
    /* M taymer: kutyapti va yoqilgan; protsessor S da, MIE = 0 -> baribir qabul (past rejim) */
    M.clint.mtimecmp = 500;
    c->mie = MIP_MTIP;
    c->rejim = REJIM_S;
    c->pc = 0x80001000;
    TENG(uzilish_tekshir(&M), 1, "MTI, S rejimda, MIE=0 -> qabul");
    TENG(c->mcause, 0x80000007, "mcause = M taymer");
    /* M rejimda MIE = 0 -> qabul qilinmaydi */
    c->mstatus &= ~MSTATUS_MIE;
    TENG(uzilish_tekshir(&M), 0, "M rejim, MIE=0 -> yo'q");
    /* STI delegatsiya qilingan: S rejimda SIE = 0 -> yo'q; U rejimda -> ha */
    M.clint.mtimecmp = UINT64_MAX;
    c->mideleg = MIP_STIP;
    c->mie = MIP_STIP;
    c->mip_dasturiy = MIP_STIP;
    c->rejim = REJIM_S;
    c->mstatus = 0;
    TENG(uzilish_tekshir(&M), 0, "STI, S rejim, SIE=0 -> yo'q");
    c->rejim = REJIM_U;
    TENG(uzilish_tekshir(&M), 1, "STI, U rejim -> ha");
    TENG(c->rejim, REJIM_S, "S ga keldi");
    /* M rejimda delegatsiya qilingan uzilish hech qachon qabul qilinmaydi */
    c->rejim = REJIM_M;
    c->mstatus = MSTATUS_MIE | MSTATUS_SIE;
    TENG(uzilish_tekshir(&M), 0, "M rejimda S uzilishi -> yo'q");
    /* ustuvorlik: SSI va STI birga -> SSI (1) birinchi */
    c->mideleg = MIP_STIP | MIP_SSIP;
    c->mie = MIP_STIP | MIP_SSIP;
    c->mip_dasturiy = MIP_STIP | MIP_SSIP;
    c->rejim = REJIM_U;
    uzilish_tekshir(&M);
    TENG(c->scause, 0x80000001, "ustuvorlik: SSI > STI");
    /* wfi: kutayotgan, lekin ruxsatsiz uzilish ham uyg'otadi */
    c->rejim = REJIM_S;
    c->mstatus = 0;
    c->kutmoqda = 1;
    uzilish_tekshir(&M);
    TENG(c->kutmoqda, 0, "wfi: uzilish kutayotgan -> uyg'onadi");
    TUGAT();
}

/* bitta buyruqni pc da bajarish */
static void bitta(uint32_t buyruq)
{
    ram_yoz(RAM_BOSH, buyruq);
    M.cpu.pc = RAM_BOSH;
    cpu_qadam(&M);
}

static void e10_amo(void)
{
    BOSHLA("E10 AMO amallari (cpu.c)");
    mashina_tayyorla();
    struct cpu *c = &M.cpu;
    c->rejim = REJIM_M;
    uint32_t m = RAM_BOSH + 0x400;
    static const struct { uint32_t buyruq; uint32_t eski, manba, kutilgan; const char *matn; } t[] = {
        { 0x00B5252F, 10, 5, 15, "amoadd.w a0, a1, (a0)" },
        { 0x08B5252F, 10, 5, 5, "amoswap.w" },
        { 0x20B5252F, 0xF0, 0xFF, 0x0F, "amoxor.w" },
        { 0x60B5252F, 0xF0, 0x3C, 0x30, "amoand.w" },
        { 0x40B5252F, 0xF0, 0x0F, 0xFF, "amoor.w" },
        { 0x80B5252F, 7, (uint32_t)-5, (uint32_t)-5, "amomin.w (ishorali)" },
        { 0xA0B5252F, 7, (uint32_t)-5, 7, "amomax.w" },
        { 0xC0B5252F, 7, (uint32_t)-5, 7, "amominu.w (ishorasiz)" },
        { 0xE0B5252F, 7, (uint32_t)-5, (uint32_t)-5, "amomaxu.w" },
    };
    for (size_t i = 0; i < sizeof(t) / sizeof(t[0]); i++) {
        ram_yoz(m, t[i].eski);
        c->x[10] = m;
        c->x[11] = t[i].manba;
        bitta(t[i].buyruq);
        TENG(ram_oqi(m), t[i].kutilgan, t[i].matn);
        TENG(c->x[10], t[i].eski, "rd = eski qiymat");
    }
    TUGAT();
}

static void p1_plic(void)
{
    BOSHLA("P1 PLIC eng_ustuvor (plic.c)");
    struct plic p;
    memset(&p, 0, sizeof(p));
    p.ustuvorlik[3] = 2;
    p.ustuvorlik[5] = 5;
    p.ustuvorlik[7] = 5;
    p.ustuvorlik[9] = 1;
    p.yoqilgan[1] = (1u << 3) | (1u << 5) | (1u << 9);
    plic_signal(&p, 3, 1);
    plic_signal(&p, 5, 1);
    plic_signal(&p, 7, 1);                      /* yoqilmagan */
    plic_signal(&p, 9, 1);
    TENG(plic_kutyapti(&p, 1), 1, "kontekst 1 da kutayotgan bor");
    TENG(plic_kutyapti(&p, 0), 0, "kontekst 0 da hech narsa yoqilmagan");
    TENG(plic_oqi(&p, 0x200004 + 0x1000), 5, "claim: eng ustuvor (5), 7 yoqilmagan");
    TENG(plic_oqi(&p, 0x200004 + 0x1000), 3, "keyingisi: 3 (ustuvorlik 2)");
    p.chegara[1] = 1;
    TENG(plic_oqi(&p, 0x200004 + 0x1000), 0, "9 ning ustuvorligi 1 <= chegara 1 -> yo'q");
    plic_yoz(&p, 0x200004 + 0x1000, 5);         /* complete */
    plic_signal(&p, 5, 1);
    TENG(plic_oqi(&p, 0x200004 + 0x1000), 5, "complete dan keyin yana claim qilish mumkin");
    TUGAT();
}

/* big-endian 32 bitli son o'qish. DIQQAT: uint8_t avval int ga ko'tariladi — 0xD0 << 24 int uchun toshish (UB),
   shuning uchun har baytni uint32_t ga o'tkazamiz (kitob, 1.8) */
static uint32_t be32(const uint8_t *p)
{
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3];
}

static void d_dtb(const char *kutilgan_yol)
{
    BOSHLA("D1-D3 qurilmalar daraxti (dtb.c)");
    mashina_tayyorla();
    free(M.ram);
    M.ram_hajm = 64u << 20;
    M.ram = calloc(1, M.ram_hajm);
    uint32_t manzil = dtb_joylash(&M);
    const uint8_t *b = M.ram + (manzil - RAM_BOSH);
    TENG(manzil, RAM_BOSH + M.ram_hajm - 65536, "DTB RAM oxirgi 64 KB ida");
    TENG(be32(b), 0xD00DFEED, "sehrli son BIG-endian (D1)");
    uint32_t hajm = be32(b + 4);
    uint32_t tuzilma_uz = be32(b + 36);
    TENG(tuzilma_uz & 3u, 0, "tuzilma bloki 4 ga tekislangan (D2)");
    uint32_t s_off = be32(b + 12);
    uint32_t s_uz = be32(b + 32);
    int compatible_soni = 0;
    for (uint32_t i = 0; i < s_uz; i += (uint32_t)strlen((const char *)b + s_off + i) + 1)
        compatible_soni += strcmp((const char *)b + s_off + i, "compatible") == 0;
    TENG(compatible_soni, 1, "\"compatible\" satrlar blokida BIR marta (D3)");
    FILE *f = fopen(kutilgan_yol, "rb");
    if (f) {
        static uint8_t k[65536];
        size_t n = fread(k, 1, sizeof(k), f);
        fclose(f);
        size_t farq = 0;
        while (farq < n && farq < hajm && k[farq] == b[farq])
            farq++;
        TENG(hajm, n, "DTB hajmi kutilgandek");
        TENG(farq, n, "birinchi farq qiluvchi bayt siljishi (kutilgan = hajm: farq yo'q)");
    } else {
        printf("        (kutilgan DTB fayli topilmadi: %s)\n", kutilgan_yol);
        g_xato++;
    }
    TUGAT();
}

int main(int argc, char **argv)
{
    e1_dekod();
    e2_e3_e4_alu();
    e5_mmu();
    e6_trap();
    e7_siqilgan();
    e8_tekis_emas();
    e9_uzilish();
    e10_amo();
    p1_plic();
    d_dtb(argc > 1 ? argv[1] : "testlar/birlik/kutilgan.dtb");
    if (umumiy_xato)
        printf("  ba'zi mashqlar hali to'g'ri emas (yuqoridagi [XATO] qatorlar)\n");
    return umumiy_xato != 0;
}
