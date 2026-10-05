/*
 * asosiy.c — firmware ning asosiy qismi: apparatni sozlash, yadroni ishga tushirish, M trap ishlovchisi.
 */
#include "fw.h"

#define MSTATUS_MPP_S (1u << 11)
#define MSTATUS_MPP_MASKA (3u << 11)
#define MIE_MTIE (1u << 7)

/* yadroga topshiriladigan trap'lar (medeleg): sahifa xatolari, tekislik, noto'g'ri buyruq, breakpoint,
   U rejimdan ecall — bularning hammasini Linux O'ZI hal qiladi. S rejimdan ecall (9) — SBI, M da qoladi. */
#define DELEG_ISTISNOLAR ((1u << 0) | (1u << 1) | (1u << 2) | (1u << 3) | (1u << 4) | (1u << 5) | (1u << 6) | \
                          (1u << 7) | (1u << 8) | (1u << 12) | (1u << 13) | (1u << 15))
#define DELEG_UZILISHLAR ((1u << 1) | (1u << 5) | (1u << 9))   /* SSI, STI, SEI */

void firmware_asosiy(uint32_t hartid, uint32_t dtb)
{
    konsol_matn("\n[vk-sbi] Virtual kompyuter firmware'i (SBI v2.0), M rejim\n");
    konsol_matn("[vk-sbi] hart ");
    konsol_son(hartid);
    konsol_matn(", DTB ");
    konsol_hex(dtb);
    konsol_matn(", yadro ");
    konsol_hex(YADRO_MANZIL);
    konsol_matn("\n");

    /*
     * TODO(F3) — O'ZINGIZ YOZING: Yadroga o'tishdan oldin: delegatsiya, hisoblagichlar, M taymer, va mret uchun MPP/mepc.
     *   - csr_yoz(medeleg, DELEG_ISTISNOLAR);  csr_yoz(mideleg, DELEG_UZILISHLAR)
     *   - csr_yoz(mcounteren, 7) — S rejim rdtime qila olsin;  csr_yoz(mie, MIE_MTIE)
     *   - mstatus.MPP <- S (MSTATUS_MPP_MASKA, MSTATUS_MPP_S);  mepc <- YADRO_MANZIL
     * Tekshirish: make test  (birlik testida 'F3' qatori)
     */

    konsol_matn("[vk-sbi] yadroga o'tyapman (mret -> S rejim)\n\n");
    register uint32_t a0 __asm__("a0") = hartid;
    register uint32_t a1 __asm__("a1") = dtb;
    __asm__ volatile("mret" ::"r"(a0), "r"(a1));
    for (;;)
        ;
}

/* M rejim trap ishlovchisi (trap.S chaqiradi) */
void m_trap(struct kadr *k)
{
    uint32_t sabab = csr_oqi(mcause);
    if (sabab == 0x80000007u) {                 /* M taymer uzilishi -> yadroga S taymer */
        sbi_taymer_uzilishi();
        return;
    }
    if (sabab == 9) {                           /* S rejimdan ecall -> SBI */
        sbi_ecall(k);
        return;
    }
    /* kutilmagan trap: ma'lumot chiqarib, to'xtaymiz */
    konsol_matn("\n[vk-sbi] KUTILMAGAN TRAP: mcause=");
    konsol_hex(sabab);
    konsol_matn(" mepc=");
    konsol_hex(k->mepc);
    konsol_matn(" mtval=");
    konsol_hex(csr_oqi(mtval));
    konsol_matn("\n");
    quvvatni_och(1);
}
