/*
 * mmu.c — virtual xotira: Sv32 sahifa jadvallari bo'yicha manzil tarjimasi (24-bob).
 *
 * G'OYA (umumiy)
 *   Har jarayon o'zining 4 GB lik "xayoliy" manzil maydonini ko'radi. Haqiqiy RAM esa bitta.
 *   Qaysi virtual sahifa (4 KB) qaysi fizik sahifaga tushishini SAHIFA JADVALI aytadi. Jadvalni
 *   yadro tuzadi, protsessor (MMU) har murojaatda o'qiydi. Jadvalni almashtirish = boshqa jarayonga o'tish.
 *
 * Sv32 FORMATI
 *   Virtual manzil (32 bit):   | VPN[1] (10 bit) | VPN[0] (10 bit) | siljish (12 bit) |
 *                                31          22   21          12    11            0
 *   Jadval ikki darajali: 1-darajali jadvalning 1024 ta yozuvidan biri VPN[1] bilan tanlanadi,
 *   u 2-darajali jadvalga (yoki to'g'ridan-to'g'ri 4 MB katta sahifaga) ishora qiladi;
 *   2-darajali jadvaldan VPN[0] bilan 4 KB sahifa tanlanadi.
 *
 *   PTE — sahifa jadvali yozuvi (32 bit):
 *     | PPN (22 bit) | RSW (2) | D | A | G | U | X | W | R | V |
 *       31       10    9   8    7   6   5   4   3   2   1   0
 *     V — yozuv yaroqli;  R/W/X — o'qish/yozish/bajarish ruxsati;  U — user rejim kira oladi;
 *     A — sahifaga murojaat bo'lgan (accessed);  D — sahifaga yozilgan (dirty);  PPN — fizik sahifa raqami.
 *     R=W=X=0 bo'lsa — bu yozuv "barg" (leaf) emas, keyingi darajadagi jadvalga ko'rsatkich.
 *
 *   satp registri: | MODE (1 bit) | ASID (9 bit) | PPN (22 bit) |. MODE=1 — Sv32 yoqilgan,
 *   PPN — 1-darajali jadvalning fizik sahifa raqami.
 *
 * TLB
 *   Har murojaatda jadvalni xotiradan o'qish — 2 ta qo'shimcha xotira murojaati: juda sekin.
 *   Shuning uchun protsessorlar oxirgi tarjimalarni TLB keshida saqlaydi. Bizda 64 yozuvli to'g'ridan-to'g'ri
 *   xaritalangan kesh. Yadro jadvalni o'zgartirsa, `sfence.vma` buyrug'i bilan TLB ni tozalashi SHART —
 *   aks holda eski tarjima ishlatilaveradi (haqiqiy yadrolardagi mashhur xato manbai).
 *
 * ASID — "manzil maydoni raqami" (satp ning 30..22-bitlari)
 *   Har jarayonning o'z jadvali bor. Jarayon almashganda TLB ni tozalash qimmat, shuning uchun TLB yozuvi
 *   qaysi ASID ga tegishli ekanini eslab qoladi: boshqa ASID bilan u "topilmagan" hisoblanadi. Linux ASID
 *   borligini ko'rsa (boot log: "ASID allocator using 9 bits"), jarayon almashganda sfence.vma QILMAYDI —
 *   shuning uchun bu tekshiruv to'g'rilik uchun shart. G (global) bitli sahifalar (yadro) — hamma ASID da bir xil.
 */
#include "mashina.h"

#define PTE_V (1u << 0)
#define PTE_R (1u << 1)
#define PTE_W (1u << 2)
#define PTE_X (1u << 3)
#define PTE_U (1u << 4)
#define PTE_G (1u << 5)
#define PTE_A (1u << 6)
#define PTE_D (1u << 7)

/* murojaat turiga mos SAHIFA XATOSI sababi */
static int sahifa_xatosi(enum kirish tur)
{
    return tur == KIRISH_BAJARISH ? SABAB_BUYRUQ_SAHIFA : tur == KIRISH_YOZISH ? SABAB_YOZISH_SAHIFA : SABAB_OQISH_SAHIFA;
}

/* murojaat turiga mos KIRISH XATOSI sababi (PTE ning o'zini o'qib bo'lmasa) */
static int kirish_xatosi(enum kirish tur)
{
    return tur == KIRISH_BAJARISH ? SABAB_BUYRUQ_KIRISH : tur == KIRISH_YOZISH ? SABAB_YOZISH_KIRISH : SABAB_OQISH_KIRISH;
}

void tlb_tozala(struct cpu *c)
{
    for (int i = 0; i < TLB_HAJM; i++)
        c->tlb[i].bor = 0;
}

/*
 * ruxsat_bormi — barg PTE bu murojaatga ruxsat beradimi?
 *   1) U-bit: U rejim faqat U=1 sahifalarga kira oladi. S rejim U=1 sahifadan BUYRUQ BAJARA OLMAYDI
 *      (hujumchi user xotirasiga kod qo'yib, yadroni unga sakratmasin), o'qish/yozish esa faqat
 *      sstatus.SUM=1 bo'lsa (yadro user buferiga ataylab murojaat qilayotganini bildiradi).
 *   2) Tur bo'yicha: o'qish — R (yoki MXR=1 bo'lsa X ham yetadi), yozish — W, bajarish — X.
 */
static int ruxsat_bormi(const struct cpu *c, enum rejim rejim, uint32_t pte, enum kirish tur)
{
    if (rejim == REJIM_U && !(pte & PTE_U))
        return 0;
    if (rejim == REJIM_S && (pte & PTE_U)) {
        if (tur == KIRISH_BAJARISH)
            return 0;
        if (!(c->mstatus & MSTATUS_SUM))
            return 0;
    }
    switch (tur) {
    case KIRISH_OQISH:
        return (pte & PTE_R) || ((c->mstatus & MSTATUS_MXR) && (pte & PTE_X));
    case KIRISH_YOZISH:
        return (pte & PTE_W) != 0;
    case KIRISH_BAJARISH:
        return (pte & PTE_X) != 0;
    }
    return 0;
}

/* barg PTE va daraja bo'yicha yakuniy fizik manzil */
static uint64_t fizik_manzil(uint32_t pte, int daraja, uint32_t va)
{
    uint64_t ppn = pte >> 10;                   /* 22 bitli fizik sahifa raqami */
    if (daraja == 1)                            /* 4 MB sahifa: VPN[0] va siljish (22 bit) virtual manzildan olinadi */
        return ((ppn >> 10) << 22) | (va & 0x3FFFFF);
    return (ppn << 12) | (va & 0xFFF);
}

/*
 * jadval_yurish — sahifa jadvali bo'ylab yurib, barg PTE ni topadi ("page table walk").
 * Qaytaradi: 0 — topildi (*pte, *daraja, *pte_manzil to'ldirilgan), aks holda istisno sababi.
 */
static int jadval_yurish(struct mashina *m, uint32_t va, enum kirish tur, uint32_t *pte_chiq, int *daraja_chiq,
                         uint32_t *pte_manzil_chiq)
{
    /*
     * TODO(E5) — O'ZINGIZ YOZING: Sv32 sahifa jadvali bo'ylab yurish (page table walk). Eng muhim mashqlardan biri.
     *   - jadval = (satp & 0x3FFFFF) << 12  — 1-darajali jadvalning FIZIK manzili (uint64_t: 34 bit bo'lishi mumkin)
     *   - vpn[1] = va ning 31..22 bitlari, vpn[0] = 21..12 bitlari
     *   - daraja = 1 dan 0 gacha: pte_manzil = jadval + vpn[daraja]*4;  shina_oqi(m, pte_manzil, 4, &pte)
     *   -   o'qib bo'lmasa (yoki manzil > 0xFFFFFFFF)  -> return kirish_xatosi(tur)
     *   -   V = 0  yoki  (R = 0 va W = 1)            -> return sahifa_xatosi(tur)
     *   -   R yoki X bor — bu BARG: daraja 1 da PPN[0] (pte ning 19..10 bitlari) nol bo'lmasa -> sahifa_xatosi
     *   -         aks holda *pte_chiq, *daraja_chiq, *pte_manzil_chiq ni to'ldirib 0 qaytaring
     *   -   barg emas — keyingi jadval: (pte >> 10) << 12
     *   - tsikldan chiqdingiz (0-darajada ham barg yo'q) -> sahifa_xatosi(tur)
     *   - kitob: 07-bob, 7.2. Test: make test -> E5
     * Tekshirish: make test  (birlik testida 'E5' qatori)
     */
    (void)m;
    (void)va;
    (void)pte_chiq;
    (void)daraja_chiq;
    (void)pte_manzil_chiq;
    (void)kirish_xatosi;
    return sahifa_xatosi(tur);
}

/*
 * Qaysi rejim nomidan tekshiramiz ("samarali rejim")?
 *   Buyruq o'qish — har doim joriy rejim.
 *   Load/store — joriy rejim; LEKIN M rejimda mstatus.MPRV = 1 bo'lsa — MPP dagi rejim nomidan.
 *   (Firmware shu orqali yadroning virtual manzilidagi buferni o'qiy oladi.)
 * M rejimda tarjima yo'q: M doim fizik manzillar bilan ishlaydi.
 */
static enum rejim samarali_rejim(const struct cpu *c, enum kirish tur)
{
    if (tur != KIRISH_BAJARISH && c->rejim == REJIM_M && (c->mstatus & MSTATUS_MPRV))
        return (enum rejim)((c->mstatus & MSTATUS_MPP) >> MSTATUS_MPP_SILJISH);
    return c->rejim;
}

int mmu_tarjima(struct mashina *m, uint32_t va, enum kirish tur, uint32_t *fiz)
{
    struct cpu *c = &m->cpu;
    enum rejim rejim = samarali_rejim(c, tur);
    if (rejim == REJIM_M || !(c->satp >> 31)) {         /* M rejim yoki satp.MODE = 0: tarjima yo'q */
        *fiz = va;
        return 0;
    }

    uint32_t vpn = va >> 12, asid = BITLAR(c->satp, 30, 22);
    struct tlb_yozuv *t = &c->tlb[vpn % TLB_HAJM];
    uint32_t pte, pte_manzil;
    int daraja;
    /* 4 MB sahifa TLB ga 4 KB qismlari bo'yicha yoziladi: kalit sifatida doim to'liq VPN ishlatiladi */
    if (t->bor && t->vpn == vpn && ((t->pte & PTE_G) || t->asid == asid) &&
        (tur != KIRISH_YOZISH || (t->pte & PTE_D))) {
        c->tlb_topildi++;
        pte = t->pte;
        daraja = t->daraja;
        pte_manzil = t->pte_manzil;
    } else {
        c->tlb_topilmadi++;
        int xato = jadval_yurish(m, va, tur, &pte, &daraja, &pte_manzil);
        if (xato)
            return xato;
    }

    if (!ruxsat_bormi(c, rejim, pte, tur))
        return sahifa_xatosi(tur);

    /* A va D bitlari: apparat o'zi qo'yadi (Svadu uslubi). Yadro ular orqali qaysi sahifa ishlatilganini
       (sahifani almashtirish uchun) va qaysi biri o'zgarganini (diskka yozish kerakmi) biladi. */
    uint32_t yangi = pte | PTE_A | (tur == KIRISH_YOZISH ? PTE_D : 0);
    if (yangi != pte) {
        shina_yoz(m, pte_manzil, 4, yangi);
        pte = yangi;
    }

    uint64_t f = fizik_manzil(pte, daraja, va);
    if (f > 0xFFFFFFFFu)
        return kirish_xatosi(tur);              /* bizning shina 32 bitli: undan yuqori fizik manzil yo'q */

    t->bor = 1;
    t->vpn = vpn;
    t->pte = pte;
    t->daraja = daraja;
    t->pte_manzil = pte_manzil;
    t->asid = asid;
    *fiz = (uint32_t)f;
    return 0;
}
