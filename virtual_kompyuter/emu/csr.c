/*
 * csr.c — CSR (control and status register) larni o'qish va yozish.
 *
 * CSR NIMA?
 *   Oddiy registrlar (x0..x31) hisob uchun. CSR'lar esa protsessorni BOSHQARADI: uzilishlar yoqilganmi
 *   (mstatus), trap bo'lsa qayerga sakrash (mtvec/stvec), trap sababi (mcause/scause), sahifa jadvali
 *   qayerda (satp) ... Ular bilan faqat maxsus buyruqlar (csrrw, csrrs, csrrc ...) ishlaydi.
 *
 * HIMOYA QOIDASI — CSR raqamining o'zida yozilgan:
 *   bitlar [9:8]   — kamida qaysi rejim kerak: 0 (U), 1 (S), 3 (M);
 *   bitlar [11:10] = 3 — faqat o'qish uchun (yozish urinishi — noto'g'ri buyruq istisnosi).
 *   Masalan satp = 0x180 = 0b0001_1000_0000: [9:8] = 01 — S yoki M rejim. U rejimdagi dastur
 *   `csrw satp, ...` qilsa — istisno: oddiy dastur sahifa jadvalini almashtira olmaydi.
 *
 * KO'RINISHLAR (views) — eng nozik joy
 *   sstatus, sie, sip — ALOHIDA registr EMAS. Ular mstatus, mie, mip ning S rejimga ruxsat etilgan
 *   bitlaridan iborat "deraza": sstatus ga yozish aslida mstatus ning tegishli bitlarini o'zgartiradi.
 *   sie/sip da faqat M rejim S ga "berib qo'ygan" (mideleg) uzilishlar ko'rinadi.
 *
 * VAQT
 *   time = mtime = bajarilgan buyruqlar soni (deterministik). cycle va instret ham shu qiymat.
 */
#include "mashina.h"

/* mip: qurilmalardan keladigan bitlar + dastur yozadigan bitlar */
uint32_t mip_qiymati(struct mashina *m)
{
    struct cpu *c = &m->cpu;
    uint32_t mip = c->mip_dasturiy & MIP_S_BITLAR;              /* SSIP, STIP, SEIP ni M rejim o'zi qo'ya oladi */
    if (m->clint.msip & 1u)
        mip |= MIP_MSIP;
    if (c->instret >= m->clint.mtimecmp)
        mip |= MIP_MTIP;
    if (plic_kutyapti(&m->plic, 0))
        mip |= MIP_MEIP;
    if (plic_kutyapti(&m->plic, 1))
        mip |= MIP_SEIP;
    if ((c->menvcfgh & MENVCFGH_STCE) && c->instret >= c->stimecmp)
        mip |= MIP_STIP;                        /* Sstc yoqilgan: STIP ni stimecmp boshqaradi */
    return mip;
}

/* hisoblagichni (cycle/time/instret) o'qish ruxsati: S uchun mcounteren, U uchun mcounteren VA scounteren */
static int hisoblagich_ruxsat(const struct cpu *c, uint32_t raqam)
{
    uint32_t bit = raqam & 0x1F;                /* 0xC00/0xC80 -> 0 (cycle), 0xC01 -> 1 (time), 0xC02 -> 2 (instret) */
    if (c->rejim == REJIM_M)
        return 1;
    if (!((c->mcounteren >> bit) & 1u))
        return 0;
    if (c->rejim == REJIM_U && !((c->scounteren >> bit) & 1u))
        return 0;
    return 1;
}

int csr_oqi(struct mashina *m, uint32_t raqam, uint32_t *q)
{
    struct cpu *c = &m->cpu;
    if (BITLAR(raqam, 9, 8) > (uint32_t)c->rejim)
        return SABAB_NOTOGRI_BUYRUQ;            /* rejim yetarli emas */

    switch (raqam) {
    /* ---- S rejim ---- */
    case CSR_SSTATUS: *q = c->mstatus & SSTATUS_MASKA; return 0;
    case CSR_SIE: *q = c->mie & c->mideleg; return 0;
    case CSR_SIP: *q = mip_qiymati(m) & c->mideleg; return 0;
    case CSR_STVEC: *q = c->stvec; return 0;
    case CSR_SCOUNTEREN: *q = c->scounteren; return 0;
    case CSR_SENVCFG: *q = 0; return 0;
    case CSR_SSCRATCH: *q = c->sscratch; return 0;
    case CSR_SEPC: *q = c->sepc; return 0;
    case CSR_SCAUSE: *q = c->scause; return 0;
    case CSR_STVAL: *q = c->stval; return 0;
    case CSR_STIMECMP:
    case CSR_STIMECMPH:
        if (c->rejim == REJIM_S && !(c->menvcfgh & MENVCFGH_STCE))
            return SABAB_NOTOGRI_BUYRUQ;        /* M rejim Sstc ni yoqmagan */
        *q = raqam == CSR_STIMECMP ? (uint32_t)c->stimecmp : (uint32_t)(c->stimecmp >> 32);
        return 0;
    case CSR_SATP:
        if (c->rejim == REJIM_S && (c->mstatus & MSTATUS_TVM))
            return SABAB_NOTOGRI_BUYRUQ;
        *q = c->satp;
        return 0;

    /* ---- M rejim ---- */
    case CSR_MSTATUS: *q = c->mstatus; return 0;
    case CSR_MSTATUSH: *q = 0; return 0;
    case CSR_MISA: *q = MISA_QIYMATI; return 0;
    case CSR_MEDELEG: *q = c->medeleg; return 0;
    case CSR_MIDELEG: *q = c->mideleg; return 0;
    case CSR_MIE: *q = c->mie; return 0;
    case CSR_MIP: *q = mip_qiymati(m); return 0;
    case CSR_MTVEC: *q = c->mtvec; return 0;
    case CSR_MCOUNTEREN: *q = c->mcounteren; return 0;
    case CSR_MENVCFG: *q = 0; return 0;
    case CSR_MENVCFGH: *q = c->menvcfgh; return 0;
    case CSR_MSCRATCH: *q = c->mscratch; return 0;
    case CSR_MEPC: *q = c->mepc; return 0;
    case CSR_MCAUSE: *q = c->mcause; return 0;
    case CSR_MTVAL: *q = c->mtval; return 0;
    case CSR_MVENDORID:
    case CSR_MARCHID:
    case CSR_MIMPID:
    case CSR_MHARTID: *q = 0; return 0;         /* bitta yadro, raqami 0 */
    case CSR_MCYCLE:
    case CSR_MINSTRET: *q = (uint32_t)c->instret; return 0;
    case CSR_MCYCLEH:
    case CSR_MINSTRETH: *q = (uint32_t)(c->instret >> 32); return 0;

    /* ---- hisoblagichlar (hamma rejim, ruxsat bilan) ---- */
    case CSR_CYCLE:
    case CSR_TIME:
    case CSR_INSTRET:
        if (!hisoblagich_ruxsat(c, raqam))
            return SABAB_NOTOGRI_BUYRUQ;
        *q = (uint32_t)c->instret;
        return 0;
    case CSR_CYCLEH:
    case CSR_TIMEH:
    case CSR_INSTRETH:
        if (!hisoblagich_ruxsat(c, raqam))
            return SABAB_NOTOGRI_BUYRUQ;
        *q = (uint32_t)(c->instret >> 32);
        return 0;
    }
    if (raqam >= CSR_PMPCFG0 && raqam < CSR_PMPCFG0 + 4) {
        *q = c->pmpcfg[raqam - CSR_PMPCFG0];
        return 0;
    }
    if (raqam >= CSR_PMPADDR0 && raqam < CSR_PMPADDR0 + 16) {
        *q = c->pmpaddr[raqam - CSR_PMPADDR0];
        return 0;
    }
    return SABAB_NOTOGRI_BUYRUQ;                /* bunday CSR yo'q */
}

/* "maska bilan yozish": faqat maskadagi bitlar yangi qiymatdan, qolganlari eski holicha.
   Bu iboraning o'zi bitlar bilan ishlashning eng muhim andozasi:  (eski & ~maska) | (yangi & maska) */
static uint32_t maskali(uint32_t eski, uint32_t yangi, uint32_t maska)
{
    return (eski & ~maska) | (yangi & maska);
}

int csr_yoz(struct mashina *m, uint32_t raqam, uint32_t q)
{
    struct cpu *c = &m->cpu;
    if (BITLAR(raqam, 9, 8) > (uint32_t)c->rejim)
        return SABAB_NOTOGRI_BUYRUQ;
    if (BITLAR(raqam, 11, 10) == 3)
        return SABAB_NOTOGRI_BUYRUQ;            /* faqat o'qiladigan CSR */

    switch (raqam) {
    case CSR_SSTATUS:
        c->mstatus = maskali(c->mstatus, q, SSTATUS_MASKA);   /* faqat S ga ko'rinadigan bitlar */
        return 0;
    case CSR_SIE:
        c->mie = maskali(c->mie, q, c->mideleg);              /* faqat M bergan (delegatsiya qilgan) bitlar */
        return 0;
    case CSR_SIP:
        c->mip_dasturiy = maskali(c->mip_dasturiy, q, c->mideleg & MIP_SSIP);  /* S faqat SSIP ni yoza oladi */
        return 0;
    case CSR_STVEC: c->stvec = q & ~2u; return 0;            /* MODE: 0 — bitta manzil, 1 — vektorli */
    case CSR_SCOUNTEREN: c->scounteren = q & 7u; return 0;
    case CSR_SENVCFG: return 0;
    case CSR_SSCRATCH: c->sscratch = q; return 0;
    case CSR_SEPC: c->sepc = q & ~1u; return 0;              /* C kengaytmasi bor: 2 ga tekis */
    case CSR_SCAUSE: c->scause = q; return 0;
    case CSR_STVAL: c->stval = q; return 0;
    case CSR_STIMECMP:
    case CSR_STIMECMPH:
        if (c->rejim == REJIM_S && !(c->menvcfgh & MENVCFGH_STCE))
            return SABAB_NOTOGRI_BUYRUQ;
        if (raqam == CSR_STIMECMP)
            c->stimecmp = (c->stimecmp & 0xFFFFFFFF00000000ull) | q;
        else
            c->stimecmp = (c->stimecmp & 0xFFFFFFFFull) | ((uint64_t)q << 32);
        return 0;
    case CSR_SATP:
        if (c->rejim == REJIM_S && (c->mstatus & MSTATUS_TVM))
            return SABAB_NOTOGRI_BUYRUQ;
        /* DIQQAT: satp ni yozish TLB ni TOZALAMAYDI — haqiqiy protsessordagi kabi, yadro sfence.vma qiladi */
        c->satp = q;
        return 0;

    case CSR_MSTATUS: {
        uint32_t yangi = maskali(c->mstatus, q, MSTATUS_MASKA);
        if (((yangi & MSTATUS_MPP) >> MSTATUS_MPP_SILJISH) == 2)
            yangi = (yangi & ~MSTATUS_MPP) | (c->mstatus & MSTATUS_MPP);  /* MPP=2 (H rejim) bizda yo'q: eski qoladi */
        c->mstatus = yangi;
        return 0;
    }
    case CSR_MSTATUSH: return 0;
    case CSR_MISA: return 0;                    /* kengaytmalarni o'chirib bo'lmaydi: yozish e'tiborsiz */
    case CSR_MEDELEG:
        c->medeleg = q & 0xB3FFu & ~(1u << SABAB_ECALL_M);     /* M rejimdan ecall ni S ga berib bo'lmaydi */
        return 0;
    case CSR_MIDELEG: c->mideleg = q & MIP_S_BITLAR; return 0;  /* faqat S uzilishlarini berish mumkin */
    case CSR_MIE: c->mie = q & MIP_HAMMASI; return 0;
    case CSR_MIP: c->mip_dasturiy = maskali(c->mip_dasturiy, q, MIP_S_BITLAR); return 0;
    case CSR_MTVEC: c->mtvec = q & ~2u; return 0;
    case CSR_MCOUNTEREN: c->mcounteren = q & 7u; return 0;
    case CSR_MENVCFG: return 0;
    case CSR_MENVCFGH: c->menvcfgh = q & MENVCFGH_STCE; return 0;
    case CSR_MSCRATCH: c->mscratch = q; return 0;
    case CSR_MEPC: c->mepc = q & ~1u; return 0;
    case CSR_MCAUSE: c->mcause = q; return 0;
    case CSR_MTVAL: c->mtval = q; return 0;
    case CSR_MCYCLE:
    case CSR_MINSTRET:
    case CSR_MCYCLEH:
    case CSR_MINSTRETH:
        return 0;                               /* bizda vaqt = hisoblagich: o'zgartirish e'tiborsiz */
    }
    if (raqam >= CSR_PMPCFG0 && raqam < CSR_PMPCFG0 + 4) {
        c->pmpcfg[raqam - CSR_PMPCFG0] = q;
        return 0;
    }
    if (raqam >= CSR_PMPADDR0 && raqam < CSR_PMPADDR0 + 16) {
        c->pmpaddr[raqam - CSR_PMPADDR0] = q;
        return 0;
    }
    return SABAB_NOTOGRI_BUYRUQ;
}
