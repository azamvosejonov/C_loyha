/*
 * trap.c — trap'ga kirish (delegatsiya bilan), trap'dan qaytish (mret, sret) va uzilishlarni tekshirish.
 *
 * TRAP — operatsion tizimning yuragi. Yadro hech qachon "o'zi" ishlamaydi: uni faqat trap'lar uyg'otadi:
 *   dastur tizim chaqiruvi qildi (ecall) / xato qildi (sahifa xatosi) / taymer "vaqting tugadi" dedi /
 *   qurilma "ish tayyor" dedi.
 *
 * QAYSI REJIMGA? — DELEGATSIYA
 *   Sukut bo'yicha HAR trap M rejimga (firmware ga) boradi. Lekin dasturning sahifa xatosini firmware
 *   emas, YADRO hal qilishi kerak. Shuning uchun M rejim "bu turdagi trap'larni to'g'ridan-to'g'ri S ga
 *   yubor" deb belgilaydi:
 *     medeleg — istisnolar uchun (bit raqami = sabab raqami);
 *     mideleg — uzilishlar uchun.
 *   Qoida: trap S ga boradi, agar (a) u delegatsiya qilingan bo'lsa VA (b) protsessor hozir S yoki U
 *   rejimda bo'lsa. M rejimda sodir bo'lgan trap HECH QACHON pastga tushmaydi.
 *
 * TRAP'GA KIRISHDA APPARAT NIMA QILADI (M uchun; S uchun — xuddi shu, s-registrlar bilan):
 *   1) mepc   <- qaysi buyruqda to'xtadik (istisnoda — aynan shu buyruq; uzilishda — keyingisi)
 *   2) mcause <- sabab (uzilishda 31-bit = 1)
 *   3) mtval  <- qo'shimcha ma'lumot (xato manzil yoki noto'g'ri buyruqning o'zi), aks holda 0
 *   4) mstatus.MPP  <- oldingi rejim;  MPIE <- MIE;  MIE <- 0 (ishlovchi boshida uzilishlar o'chiq)
 *   5) rejim <- M
 *   6) pc <- mtvec (vektorli rejimda uzilishlar uchun: mtvec + 4 * sabab)
 */
#include "mashina.h"

/* tvec dan sakrash manzili: past 2 bit — MODE (0 — hamma trap bitta manzilga, 1 — uzilishlar vektorli) */
static uint32_t vektor(uint32_t tvec, uint32_t sabab)
{
    uint32_t asos = tvec & ~3u;
    if ((tvec & 3u) == 1 && (sabab & UZILISH_BITI))
        return asos + 4 * (sabab & ~UZILISH_BITI);
    return asos;
}

void trap_kirish(struct cpu *c, uint32_t sabab, uint32_t tval)
{
    /*
     * TODO(E6) — O'ZINGIZ YOZING: Trapga kirish: istisno yoki uzilish qaysi rejimga (S yoki M) borishini aniqlang va holatni saqlang.
     *   - raqam = sabab & ~UZILISH_BITI;  deleg = uzilish bo'lsa mideleg, aks holda medeleg
     *   - c->band_bor = 0  (trap lr/sc band qilishini bekor qiladi)
     *   - rejim <= S VA deleg ning 'raqam'-biti 1 -> S ga: sepc=pc, scause=sabab, stval=tval,
     *   -      SPP <- (oldingi rejim S bo'lsa 1, U bo'lsa 0),  SPIE <- SIE,  SIE <- 0,  rejim=S, pc=vektor(stvec, sabab)
     *   - aks holda M ga: mepc, mcause, mtval;  MPP (2 bit, MSTATUS_MPP_SILJISH) <- oldingi rejim;
     *   -      MPIE <- MIE, MIE <- 0, rejim=M, pc=vektor(mtvec, sabab)
     *   - DIQQAT: M rejimdagi trap HECH QACHON S ga tushmaydi (delegatsiya bo'lsa ham)
     *   - kitob: 06-bob, 6.3-6.6
     * Tekshirish: make test  (birlik testida 'E6' qatori)
     */
    (void)c;
    (void)sabab;
    (void)tval;
    (void)vektor;
}

void trap_qaytish_s(struct cpu *c)
{
    c->rejim = (c->mstatus & MSTATUS_SPP) ? REJIM_S : REJIM_U;
    if (c->mstatus & MSTATUS_SPIE)              /* SIE <- SPIE */
        c->mstatus |= MSTATUS_SIE;
    else
        c->mstatus &= ~MSTATUS_SIE;
    c->mstatus |= MSTATUS_SPIE;                 /* spetsifikatsiya: SPIE <- 1, SPP <- U */
    c->mstatus &= ~MSTATUS_SPP;
    c->mstatus &= ~MSTATUS_MPRV;                /* M dan pastga qaytilsa MPRV o'chadi */
    c->band_bor = 0;
    c->pc = c->sepc;
}

void trap_qaytish_m(struct cpu *c)
{
    enum rejim oldingi = (enum rejim)((c->mstatus & MSTATUS_MPP) >> MSTATUS_MPP_SILJISH);
    c->rejim = oldingi;
    if (c->mstatus & MSTATUS_MPIE)              /* MIE <- MPIE */
        c->mstatus |= MSTATUS_MIE;
    else
        c->mstatus &= ~MSTATUS_MIE;
    c->mstatus |= MSTATUS_MPIE;                 /* MPIE <- 1, MPP <- U (eng past qo'llab-quvvatlangan rejim) */
    c->mstatus &= ~MSTATUS_MPP;
    if (oldingi != REJIM_M)
        c->mstatus &= ~MSTATUS_MPRV;
    c->band_bor = 0;
    c->pc = c->mepc;
}

/*
 * Uzilish qabul qilinadimi?
 *   kutayotgan = mip & mie (sodir bo'lgan VA yoqilgan).
 *   M ga tegishli (delegatsiya QILINMAGAN) uzilishlar: protsessor M dan past rejimda bo'lsa — DOIM;
 *     M rejimda — faqat mstatus.MIE = 1 bo'lsa.
 *   S ga tegishli (delegatsiya qilingan) uzilishlar: U rejimda — doim; S rejimda — SIE = 1 bo'lsa;
 *     M rejimda — HECH QACHON (yuqori rejim pastki rejimning uzilishi bilan to'xtatilmaydi).
 *   Ustuvorlik (spetsifikatsiya): MEI > MSI > MTI > SEI > SSI > STI.
 *   wfi: biror uzilish KUTAYOTGAN bo'lsa (ruxsat bo'lmasa ham) protsessor uyg'onadi.
 */
int uzilish_tekshir(struct mashina *m)
{
    struct cpu *c = &m->cpu;
    /*
     * TODO(E9) — O'ZINGIZ YOZING: Uzilishni qabul qilish: qaysi uzilish HOZIR protsessorni to'xtatadi? (yuqoridagi izohni o'qing)
     *   - kutayotgan = mip_qiymati(m) & c->mie;  nol bo'lsa 0 qaytaring
     *   - kutayotgan bor -> c->kutmoqda = 0 (wfi uyg'onadi — ruxsat bo'lmasa ham)
     *   - M qismi = kutayotgan & ~mideleg;  S qismi = kutayotgan & mideleg
     *   - M ruxsat: rejim < M  yoki  mstatus.MIE;   S ruxsat: rejim < S  yoki  (rejim == S va mstatus.SIE)
     *   - qabul = ruxsat berilgan qismlar;  ustuvorlik tartibida (MEI, MSI, MTI, SEI, SSI, STI) birinchisi uchun
     *   -      trap_kirish(c, UZILISH_BITI | irq, 0) va return 1
     * Tekshirish: make test  (birlik testida 'E9' qatori)
     */
    (void)c;
    return 0;
}
