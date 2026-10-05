/*
 * cpu.c — protsessorning asosiy sikli: bitta buyruqni olish, ajratish va bajarish.
 *
 * Tuzilishi:
 *   cpu_qadam()      — bitta qadam: uzilish? -> fetch (2 yoki 4 bayt) -> bajar() -> pc yangilash
 *   olish()          — FETCH: avval 16 bit; past 2 bit 11 bo'lsa — yana 16 bit (32 bitli buyruq),
 *                      aks holda — siqilgan buyruq: c_kengaytir() bilan 32 bitliga aylantiriladi
 *   bajar()          — opcode bo'yicha katta switch: har buyruq turi o'z bo'limida
 *   atomik()         — A kengaytmasi: lr.w, sc.w, amo*.w
 *   tizim_buyrugi()  — ecall, ebreak, mret, sret, wfi, sfence.vma va CSR buyruqlari
 *
 * ISTISNOLAR QANDAY ISHLAYDI
 *   Har yordamchi 0 (muvaffaqiyat) yoki istisno sababini (enum sabab, 0 dan katta... yoki 0 ham bo'lishi
 *   mumkin: 0 = SABAB_BUYRUQ_TEKIS_EMAS!) qaytaradi. Chalkashmaslik uchun bajar() natijani
 *   `struct natija` da qaytaradi: bor = 1 bo'lsa — istisno sodir bo'ldi.
 *   Muhim qoida: istisno bo'lgan buyruq HECH NARSANI o'zgartirmasligi kerak (registr ham, pc ham) —
 *   yadro sababni bartaraf qilib (masalan sahifani yuklab), AYNAN shu buyruqni qayta bajara olsin.
 */
#include <stdio.h>

#include "alu.h"
#include "dekod.h"
#include "mashina.h"
#include "siqilgan.h"

struct natija {
    int bor;                                    /* 1 — istisno */
    uint32_t sabab, tval;
};

static struct natija istisno(uint32_t sabab, uint32_t tval)
{
    struct natija n = { 1, sabab, tval };
    return n;
}

static const struct natija OK = { 0, 0, 0 };

/* rd ga yozish: x0 ga yozish e'tiborsiz (x0 — doim nol, apparatda "simga ulangan") */
static void rd_yoz(struct cpu *c, uint32_t rd, uint32_t qiymat)
{
    if (rd != 0)
        c->x[rd] = qiymat;
}

/* sakrash manzilini tekshirish: C kengaytmasi borligi uchun buyruqlar 2 ga tekis bo'lishi yetarli */
static int sakrash_tekis(uint32_t manzil)
{
    return (manzil & 1u) == 0;
}

/* SYSTEM opcode (0x73): ecall/ebreak/mret/sret/wfi/sfence.vma va CSR buyruqlari */
static struct natija tizim_buyrugi(struct mashina *m, uint32_t b, uint32_t *keyingi_pc)
{
    struct cpu *c = &m->cpu;
    uint32_t f3 = d_funct3(b), rd = d_rd(b), rs1 = d_rs1(b);

    if (f3 == 0) {
        if (b == 0x00000073u) {                 /* ecall: sepc/mepc = shu buyruq (ishlovchi +4 qiladi) */
            static const uint32_t sabablar[4] = { SABAB_ECALL_U, SABAB_ECALL_S, 0, SABAB_ECALL_M };
            return istisno(sabablar[c->rejim], 0);
        }
        if (b == 0x00100073u)                   /* ebreak: to'xtash nuqtasi (debugger uchun) */
            return istisno(SABAB_BREAKPOINT, c->pc);
        if (b == 0x30200073u) {                 /* mret: faqat M rejimda */
            if (c->rejim != REJIM_M)
                return istisno(SABAB_NOTOGRI_BUYRUQ, b);
            trap_qaytish_m(c);
            *keyingi_pc = c->pc;
            return OK;
        }
        if (b == 0x10200073u) {                 /* sret: S (TSR=0 bo'lsa) yoki M rejimda */
            if (c->rejim == REJIM_U || (c->rejim == REJIM_S && (c->mstatus & MSTATUS_TSR)))
                return istisno(SABAB_NOTOGRI_BUYRUQ, b);
            trap_qaytish_s(c);
            *keyingi_pc = c->pc;
            return OK;
        }
        if (b == 0x10500073u) {                 /* wfi: "uzilish kelguncha kut". U da taqiq; S da TW=1 bo'lsa taqiq */
            if (c->rejim == REJIM_U || (c->rejim == REJIM_S && (c->mstatus & MSTATUS_TW)))
                return istisno(SABAB_NOTOGRI_BUYRUQ, b);
            c->kutmoqda = 1;
            return OK;
        }
        if (d_funct7(b) == 0x09 && rd == 0) {   /* sfence.vma: sahifa jadvali o'zgardi — TLB ni tozalash */
            if (c->rejim == REJIM_U || (c->rejim == REJIM_S && (c->mstatus & MSTATUS_TVM)))
                return istisno(SABAB_NOTOGRI_BUYRUQ, b);
            tlb_tozala(c);
            return OK;
        }
        return istisno(SABAB_NOTOGRI_BUYRUQ, b);
    }
    if (f3 == 4)
        return istisno(SABAB_NOTOGRI_BUYRUQ, b);

    /* CSR buyruqlari. raqam — 12 bit (I-tur o'zgarmasi o'rnida, ishorasiz).
       csrrw  rd, csr, rs1 : rd <- csr; csr <- rs1
       csrrs  rd, csr, rs1 : rd <- csr; csr <- csr | rs1   (rs1 = x0 bo'lsa YOZILMAYDI — faqat o'qish)
       csrrc  rd, csr, rs1 : rd <- csr; csr <- csr & ~rs1
       ...i variantlari: rs1 o'rniga 5 bitli o'zgarmas (rs1 maydonining o'zi) */
    uint32_t raqam = BITLAR(b, 31, 20);
    uint32_t manba = (f3 & 4) ? rs1 : c->x[rs1];
    int tur = f3 & 3;                           /* 1 — rw, 2 — rs, 3 — rc */
    uint32_t eski = 0;
    int oqish_kerak = !(tur == 1 && rd == 0);   /* csrrw x0 — o'qimaydi (o'qishning yon ta'sirini oldini olish) */
    int yozish_kerak = !(tur != 1 && rs1 == 0); /* csrrs/csrrc x0 — yozmaydi (faqat o'qiladigan CSR ham ishlaydi) */

    if (oqish_kerak) {
        if (csr_oqi(m, raqam, &eski))
            return istisno(SABAB_NOTOGRI_BUYRUQ, b);
    }
    if (yozish_kerak) {
        /* NOZIK QOIDA (spetsifikatsiya, mip.SEIP): mip ni o'qiganda SEIP = (dasturiy bit) OR (PLIC signali).
           csrrs/csrrc esa "o'qi-o'zgartir-yoz" qiladi: agar o'qilgan qiymatdagi PLIC signali qaytarib yozilsa,
           u DASTURIY bitga "yopishib" qoladi va tashqi uzilish hech qachon o'chmaydi (cheksiz uzilishlar!).
           Shuning uchun bu ikki buyruqda SEIP o'rniga faqat dasturiy bit olinadi. */
        uint32_t asos = eski;
        if (raqam == CSR_MIP && tur != 1)
            asos = (eski & ~MIP_SEIP) | (c->mip_dasturiy & MIP_SEIP);
        uint32_t yangi = tur == 1 ? manba : tur == 2 ? (asos | manba) : (asos & ~manba);
        if (csr_yoz(m, raqam, yangi))
            return istisno(SABAB_NOTOGRI_BUYRUQ, b);
    }
    rd_yoz(c, rd, eski);
    return OK;
}

/*
 * A kengaytmasi — atomik amallar (opcode 0x2F, funct3 = 2: 32 bitli).
 *   lr.w rd, (rs1)       — o'qiydi va manzilni "band qiladi" (reservation)
 *   sc.w rd, rs2, (rs1)  — band qilish hali kuchda bo'lsa yozadi va rd = 0; aks holda yozmaydi, rd = 1
 *   amoXXX.w rd, rs2, (rs1) — BITTA bo'linmas qadamda: rd <- xotira; xotira <- xotira (amal) rs2
 * Qulflar (spinlock), hisoblagichlar, navbatlar shular ustiga quriladi (15, 26-boblar).
 * Bitta yadroli emulyatorda "bo'linmaslik" o'z-o'zidan bor; lekin lr/sc semantikasi (band qilish,
 * trap'da bekor bo'lishi) to'liq modellashtiriladi — yadro kodi haqiqiy apparatdagidek ishlashi uchun.
 * Manzil 4 ga tekis bo'lishi SHART (aks holda istisno; AMO lar uchun yozish kodlari ishlatiladi).
 */
static struct natija atomik(struct mashina *m, uint32_t b)
{
    struct cpu *c = &m->cpu;
    uint32_t rd = d_rd(b), funct5 = BITLAR(b, 31, 27);
    uint32_t manzil = c->x[d_rs1(b)], manba = c->x[d_rs2(b)];
    if (d_funct3(b) != 2)
        return istisno(SABAB_NOTOGRI_BUYRUQ, b);

    if (funct5 == 0x02) {                       /* lr.w */
        if (d_rs2(b) != 0)
            return istisno(SABAB_NOTOGRI_BUYRUQ, b);
        uint32_t q;
        int xato = xotira_oqi(m, manzil, 4, &q);
        if (xato)
            return istisno((uint32_t)xato, manzil);
        c->band_bor = 1;
        c->band_manzil = manzil;
        rd_yoz(c, rd, q);
        return OK;
    }
    if (manzil & 3u)
        return istisno(SABAB_YOZISH_TEKIS_EMAS, manzil);
    if (funct5 == 0x03) {                       /* sc.w */
        if (!c->band_bor || c->band_manzil != manzil) {
            c->band_bor = 0;
            rd_yoz(c, rd, 1);                   /* muvaffaqiyatsiz: hech narsa yozilmadi */
            return OK;
        }
        int xato = xotira_yoz(m, manzil, 4, manba);
        if (xato)
            return istisno((uint32_t)xato, manzil);
        c->band_bor = 0;
        rd_yoz(c, rd, 0);
        return OK;
    }

    /* AMO: avval YOZISH ruxsatini tekshiramiz (o'qish ham, yozish ham kerak) — aks holda yarim bajarilib qoladi */
    uint32_t fiz, eski;
    int xato = mmu_tarjima(m, manzil, KIRISH_YOZISH, &fiz);
    if (xato)
        return istisno((uint32_t)xato, manzil);
    if (shina_oqi(m, fiz, 4, &eski) != 0)
        return istisno(SABAB_YOZISH_KIRISH, manzil);
    uint32_t yangi;
    /*
     * TODO(E10) — O'ZINGIZ YOZING: AMO: xotiradagi 'eski' va registrdagi 'manba' dan 'yangi' qiymatni hisoblang (funct5 bo'yicha).
     *   - 0x01 swap: manba;  0x00 add;  0x04 xor;  0x0C and;  0x08 or
     *   - 0x10 min / 0x14 max — ISHORALI (int32_t);  0x18 minu / 0x1C maxu — ishorasiz
     *   - boshqa funct5 -> return istisno(SABAB_NOTOGRI_BUYRUQ, b)
     * Tekshirish: make test  (birlik testida 'E1' qatori)
     */
    (void)funct5;
    (void)manba;
    yangi = eski;
    if (shina_yoz(m, fiz, 4, yangi) != 0)
        return istisno(SABAB_YOZISH_KIRISH, manzil);
    rd_yoz(c, rd, eski);
    return OK;
}

/* Bitta (32 bitli yoki kengaytirilgan) buyruqni bajaradi. uz — asl buyruq uzunligi (2 yoki 4):
   jal/jalr ning qaytish manzili pc + uz. *keyingi_pc — sukut bo'yicha pc + uz, sakrashlar o'zgartiradi. */
static struct natija bajar(struct mashina *m, uint32_t b, uint32_t uz, uint32_t *keyingi_pc)
{
    struct cpu *c = &m->cpu;
    uint32_t rd = d_rd(b), rs1 = d_rs1(b), rs2 = d_rs2(b), f3 = d_funct3(b), f7 = d_funct7(b);
    uint32_t a = c->x[rs1], bq = c->x[rs2];     /* manba registrlarning QIYMATLARI */

    switch (d_opcode(b)) {
    case 0x37:                                  /* LUI: rd <- o'zgarmas << 12 (katta son yuklashning birinchi yarmi) */
        rd_yoz(c, rd, imm_u(b));
        return OK;

    case 0x17:                                  /* AUIPC: rd <- pc + (o'zgarmas << 12). Pozitsiyaga bog'liq bo'lmagan kod uchun */
        rd_yoz(c, rd, c->pc + imm_u(b));
        return OK;

    case 0x6F: {                                /* JAL: rd <- pc + uz; pc <- pc + imm. Funksiya chaqiruvi (rd = ra) */
        uint32_t manzil = c->pc + imm_j(b);
        if (!sakrash_tekis(manzil))
            return istisno(SABAB_BUYRUQ_TEKIS_EMAS, manzil);
        rd_yoz(c, rd, c->pc + uz);
        *keyingi_pc = manzil;
        return OK;
    }

    case 0x67: {                                /* JALR: pc <- (rs1 + imm) & ~1. Funksiyadan qaytish (ret = jalr x0, 0(ra)) */
        if (f3 != 0)
            return istisno(SABAB_NOTOGRI_BUYRUQ, b);
        uint32_t manzil = (a + imm_i(b)) & ~1u; /* a ni OLDINDAN o'qidik: rd == rs1 bo'lsa ham to'g'ri */
        if (!sakrash_tekis(manzil))
            return istisno(SABAB_BUYRUQ_TEKIS_EMAS, manzil);
        rd_yoz(c, rd, c->pc + uz);
        *keyingi_pc = manzil;
        return OK;
    }

    case 0x63: {                                /* BRANCH: shart bajarilsa pc <- pc + imm */
        int shart = shart_bajarildimi(f3, a, bq);
        if (shart < 0)
            return istisno(SABAB_NOTOGRI_BUYRUQ, b);
        if (shart) {
            uint32_t manzil = c->pc + imm_b(b);
            if (!sakrash_tekis(manzil))
                return istisno(SABAB_BUYRUQ_TEKIS_EMAS, manzil);
            *keyingi_pc = manzil;
        }
        return OK;
    }

    case 0x03: {                                /* LOAD: rd <- xotira[rs1 + imm] */
        static const int hajmlar[8] = { 1, 2, 4, 0, 1, 2, 0, 0 };     /* 0 — bunday funct3 yo'q */
        int hajm = hajmlar[f3];
        if (!hajm)
            return istisno(SABAB_NOTOGRI_BUYRUQ, b);
        uint32_t manzil = a + imm_i(b), qiymat;
        int xato = xotira_oqi(m, manzil, hajm, &qiymat);
        if (xato)
            return istisno((uint32_t)xato, manzil);
        rd_yoz(c, rd, yuklash_kengaytir(f3, qiymat));
        return OK;
    }

    case 0x23: {                                /* STORE: xotira[rs1 + imm] <- rs2 (pastki 1, 2 yoki 4 bayt) */
        if (f3 > 2)
            return istisno(SABAB_NOTOGRI_BUYRUQ, b);
        uint32_t manzil = a + imm_s(b);
        int xato = xotira_yoz(m, manzil, 1 << f3, bq);
        if (xato)
            return istisno((uint32_t)xato, manzil);
        return OK;
    }

    case 0x13: {                                /* OP-IMM: rd <- rs1 (amal) o'zgarmas */
        uint32_t imm = imm_i(b);
        int alt = 0;
        if (f3 == 1 || f3 == 5) {               /* siljitishlar: o'zgarmasning past 5 biti — miqdor, yuqorisi — funct7 */
            if (f7 != 0x00 && !(f3 == 5 && f7 == 0x20))
                return istisno(SABAB_NOTOGRI_BUYRUQ, b);        /* RV32 da 32 dan katta siljish yo'q */
            alt = f7 == 0x20;                   /* srai */
        }
        int xato;
        uint32_t r = alu_asosiy(f3, alt, 0, a, imm, &xato);
        rd_yoz(c, rd, r);
        return OK;
    }

    case 0x33: {                                /* OP: rd <- rs1 (amal) rs2 */
        if (f7 == 0x01) {                       /* M kengaytmasi */
            rd_yoz(c, rd, m_amal(f3, a, bq));
            return OK;
        }
        if (f7 != 0x00 && !(f7 == 0x20 && (f3 == 0 || f3 == 5)))
            return istisno(SABAB_NOTOGRI_BUYRUQ, b);
        int xato;
        uint32_t r = alu_asosiy(f3, f7 == 0x20, 1, a, bq, &xato);
        if (xato)
            return istisno(SABAB_NOTOGRI_BUYRUQ, b);
        rd_yoz(c, rd, r);
        return OK;
    }

    case 0x0F:                                  /* FENCE / FENCE.I: bitta yadroli emulyatorda tartib doim saqlanadi — hech narsa */
        return OK;

    case 0x2F:
        return atomik(m, b);

    case 0x73:
        return tizim_buyrugi(m, b, keyingi_pc);
    }
    return istisno(SABAB_NOTOGRI_BUYRUQ, b);    /* noma'lum opcode */
}

/*
 * FETCH: pc dan buyruqni o'qish. Avval 16 bit (pastki yarmi): uning past 2 biti 11 bo'lsa — bu 32 bitli
 * buyruq, yuqori yarmini pc + 2 dan o'qiymiz (u BOSHQA sahifada bo'lishi mumkin — alohida tarjima!).
 * Aks holda — 16 bitli siqilgan buyruq.
 * Qaytaradi: 0 — OK (*buyruq, *uz to'ldirilgan), aks holda istisno sababi (*tval — muammoli manzil).
 */
static int olish(struct mashina *m, uint32_t pc, uint32_t *buyruq, uint32_t *uz, uint32_t *tval)
{
    uint32_t fiz, past, yuqori;
    *tval = pc;
    int xato = mmu_tarjima(m, pc, KIRISH_BAJARISH, &fiz);
    if (xato)
        return xato;
    if (shina_oqi(m, fiz, 2, &past) != 0)
        return SABAB_BUYRUQ_KIRISH;
    if ((past & 3u) != 3u) {
        *buyruq = past;
        *uz = 2;
        return 0;
    }
    *tval = pc + 2;
    xato = mmu_tarjima(m, pc + 2, KIRISH_BAJARISH, &fiz);
    if (xato)
        return xato;
    if (shina_oqi(m, fiz, 2, &yuqori) != 0)
        return SABAB_BUYRUQ_KIRISH;
    *buyruq = past | (yuqori << 16);
    *uz = 4;
    return 0;
}

static const char REJIM_HARFI[4] = { 'U', 'S', '?', 'M' };

/* trap_kirish + trace (-t): trap'lar ham ko'rinsin — aks holda "pc=0 da abadiy trap" kabi holatlar trace'da
   umuman ko'rinmaydi (buyruq o'qilmaydi, chop etiladigan narsa yo'q) */
static void trap_qil(struct mashina *m, uint32_t sabab, uint32_t tval)
{
    struct cpu *c = &m->cpu;
    uint32_t eski_pc = c->pc;
    enum rejim eski = c->rejim;
    trap_kirish(c, sabab, tval);
    if (m->trace)
        fprintf(stderr, "    ~~ istisno %u (tval=0x%08x) pc=0x%08x [%c] -> pc=0x%08x [%c]\n", sabab, tval, eski_pc,
                REJIM_HARFI[eski], c->pc, REJIM_HARFI[c->rejim]);
}

void cpu_qadam(struct mashina *m)
{
    struct cpu *c = &m->cpu;

    /* 0) qurilmalarning uzilish signallari PLIC ga */
    qurilmalar_yangila(m);

    /* 1) Uzilish kutayaptimi? Qabul qilinsa — pc trap ishlovchisiga o'tdi, bu qadam tugadi. */
    if (uzilish_tekshir(m)) {
        if (m->trace)
            fprintf(stderr, "    ~~ uzilish %u -> pc=0x%08x [%c]\n",
                    (c->rejim == REJIM_M ? c->mcause : c->scause) & ~UZILISH_BITI, c->pc, REJIM_HARFI[c->rejim]);
        return;
    }

    /* wfi holati: hech narsa bajarmay, keyingi taymer hodisasigacha VAQTNI SURAMIZ (haqiqiy protsessor
       shu vaqt uxlab, quvvat tejaydi). Uyg'otadigan narsa bo'lmasa — abadiy uyqu: to'xtaymiz. */
    if (c->kutmoqda) {
        uint64_t keyingi = UINT64_MAX;
        if ((c->mie & MIP_MTIP) && m->clint.mtimecmp > c->instret)
            keyingi = m->clint.mtimecmp;
        if ((c->mie & MIP_STIP) && (c->menvcfgh & MENVCFGH_STCE) && c->stimecmp > c->instret && c->stimecmp < keyingi)
            keyingi = c->stimecmp;
        if (keyingi != UINT64_MAX) {
            if (uart_interaktiv() && keyingi - c->instret > 100000)
                keyingi = c->instret + 100000;  /* terminalda klaviatura uyg'otishi mumkin: kichik qadamlar */
            c->instret = keyingi;
            return;
        }
        if ((c->mie & MIP_SEIP) && uart_interaktiv()) {
            c->instret += 100000;               /* faqat klaviatura uyg'ota oladi: vaqtni suramiz va kutamiz */
            return;
        }
        fprintf(stderr, "\nemulyator: protsessor 'wfi' da uyg'onmaydigan holda uxladi (pc=0x%08x)\n", c->pc);
        m->toxtadi = 1;
        m->chiqish_kodi = 3;
        return;
    }

    /* 2) FETCH */
    uint32_t b, uz, tval;
    if (!sakrash_tekis(c->pc)) {
        trap_qil(m, SABAB_BUYRUQ_TEKIS_EMAS, c->pc);
        return;
    }
    int xato = olish(m, c->pc, &b, &uz, &tval);
    if (xato) {
        trap_qil(m, (uint32_t)xato, tval);
        return;
    }

    if (m->trace) {
        char matn[64];
        disasm(b, c->pc, matn, sizeof(matn));
        if (uz == 2)
            fprintf(stderr, "[%c] %08x: %04x      %s\n", REJIM_HARFI[c->rejim], c->pc, b, matn);
        else
            fprintf(stderr, "[%c] %08x: %08x  %s\n", REJIM_HARFI[c->rejim], c->pc, b, matn);
    }

    /* 3) DECODE: siqilgan buyruq bo'lsa — 32 bitliga aylantiramiz */
    uint32_t b32 = b;
    if (uz == 2) {
        b32 = c_kengaytir((uint16_t)b);
        if (b32 == 0) {
            trap_qil(m, SABAB_NOTOGRI_BUYRUQ, b);       /* mtval/stval = asl 16 bitli buyruq */
            return;
        }
    }

    /* 4) EXECUTE */
    uint32_t keyingi_pc = c->pc + uz;
    struct natija n = bajar(m, b32, uz, &keyingi_pc);
    if (n.bor) {
        if (n.sabab == SABAB_NOTOGRI_BUYRUQ)
            n.tval = b;                         /* noto'g'ri buyruqda tval = ASL buyruq (16 yoki 32 bit) */
        trap_qil(m, n.sabab, n.tval);           /* pc HALI o'zgarmagan: mepc/sepc = aynan shu buyruq */
        return;
    }

    /* 5) pc ni yangilash va hisoblagich */
    c->pc = keyingi_pc;
    c->x[0] = 0;
    c->instret++;
}
