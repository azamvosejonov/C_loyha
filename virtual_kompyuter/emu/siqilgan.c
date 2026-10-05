/*
 * siqilgan.c — RV32C: 16 bitli buyruqlarni 32 bitliga aylantirish (spetsifikatsiyaning "RVC" jadvali).
 *
 * 32 bitli buyruqni YIG'ISH uchun kichik "quruvchi" funksiyalar (formatlar dekod.h da chizilgan):
 *   r_tur(f7, rs2, rs1, f3, rd, op), i_tur(imm, rs1, f3, rd, op), s_tur(imm, rs2, rs1, f3, op), ...
 * Ular dekoderning TESKARISI: maydonlarni o'z bit joylariga qo'yib, OR bilan birlashtiradi.
 *
 * Siqilgan registrlar: 3 bitli maydon "rd'" (rd shtrix) faqat x8..x15 ni bildiradi (s0, s1, a0..a5) —
 * eng ko'p ishlatiladigan 8 registr. Haqiqiy raqam = 8 + maydon.
 */
#include "siqilgan.h"

/* ---------- 32 bitli buyruq quruvchilar ---------- */
static uint32_t r_tur(uint32_t f7, uint32_t rs2, uint32_t rs1, uint32_t f3, uint32_t rd, uint32_t op)
{
    return f7 << 25 | rs2 << 20 | rs1 << 15 | f3 << 12 | rd << 7 | op;
}

static uint32_t i_tur(uint32_t imm, uint32_t rs1, uint32_t f3, uint32_t rd, uint32_t op)
{
    return (imm & 0xFFFu) << 20 | rs1 << 15 | f3 << 12 | rd << 7 | op;
}

static uint32_t s_tur(uint32_t imm, uint32_t rs2, uint32_t rs1, uint32_t f3, uint32_t op)
{
    return BITLAR(imm, 11, 5) << 25 | rs2 << 20 | rs1 << 15 | f3 << 12 | BITLAR(imm, 4, 0) << 7 | op;
}

static uint32_t b_tur(uint32_t imm, uint32_t rs2, uint32_t rs1, uint32_t f3)
{
    return BIT(imm, 12) << 31 | BITLAR(imm, 10, 5) << 25 | rs2 << 20 | rs1 << 15 | f3 << 12 |
           BITLAR(imm, 4, 1) << 8 | BIT(imm, 11) << 7 | 0x63u;
}

static uint32_t j_tur(uint32_t imm, uint32_t rd)
{
    return BIT(imm, 20) << 31 | BITLAR(imm, 10, 1) << 21 | BIT(imm, 11) << 20 | BITLAR(imm, 19, 12) << 12 |
           rd << 7 | 0x6Fu;
}

static uint32_t u_tur(uint32_t imm, uint32_t rd, uint32_t op)
{
    return (imm & 0xFFFFF000u) | rd << 7 | op;
}

/* ---------- siqilgan buyruqlarning o'zgarmaslari (spetsifikatsiya 16.x jadvallari) ---------- */

/* c.addi4spn: nzuimm[5:4|9:6|2|3] -> bitlar 12..5 */
static uint32_t imm_addi4spn(uint32_t c)
{
    return BITLAR(c, 12, 11) << 4 | BITLAR(c, 10, 7) << 6 | BIT(c, 6) << 2 | BIT(c, 5) << 3;
}

/* c.lw / c.sw: uimm[5:3] -> 12..10, uimm[2|6] -> 6..5 */
static uint32_t imm_lw(uint32_t c)
{
    return BITLAR(c, 12, 10) << 3 | BIT(c, 6) << 2 | BIT(c, 5) << 6;
}

/* c.addi, c.li, c.andi ...: imm[5] -> 12, imm[4:0] -> 6..2 (6 bitli, ishorali) */
static uint32_t imm_ci(uint32_t c)
{
    return ishora_kengaytir(BIT(c, 12) << 5 | BITLAR(c, 6, 2), 6);
}

/* c.j / c.jal: offset[11|4|9:8|10|6|7|3:1|5] -> bitlar 12..2 */
static uint32_t imm_cj(uint32_t c)
{
    /*
     * TODO(E7a) — O'ZINGIZ YOZING: c.j / c.jal siljishi: 16 bitli buyruqdagi bitlar tartibi: offset[11|4|9:8|10|6|7|3:1|5] (12..2 bitlarda).
     *   - ya'ni: 12-bit -> offset[11], 11 -> [4], 10..9 -> [9:8], 8 -> [10], 7 -> [6], 6 -> [7], 5..3 -> [3:1], 2 -> [5]
     *   - 12 bitli son — ishora_kengaytir(v, 12)
     *   - tekshirish: c_juftlar.h dagi 'c.j' va 'c.jal' juftlari
     * Tekshirish: make test  (birlik testida 'E7' qatori)
     */
    (void)c;
    return 0;
}

/* c.beqz / c.bnez: offset[8|4:3] -> 12..10, offset[7:6|2:1|5] -> 6..2 */
static uint32_t imm_cb(uint32_t c)
{
    /*
     * TODO(E7b) — O'ZINGIZ YOZING: c.beqz / c.bnez siljishi: offset[8|4:3] -> 12..10 bitlar, offset[7:6|2:1|5] -> 6..2 bitlar.
     *   - 12 -> [8], 11..10 -> [4:3], 6..5 -> [7:6], 4..3 -> [2:1], 2 -> [5]
     *   - 9 bitli son — ishora_kengaytir(v, 9)
     * Tekshirish: make test  (birlik testida 'E7' qatori)
     */
    (void)c;
    return 0;
}

/* c.addi16sp: nzimm[9] -> 12, nzimm[4|6|8:7|5] -> 6..2 */
static uint32_t imm_addi16sp(uint32_t c)
{
    uint32_t v = BIT(c, 12) << 9 | BIT(c, 6) << 4 | BIT(c, 5) << 6 | BITLAR(c, 4, 3) << 7 | BIT(c, 2) << 5;
    return ishora_kengaytir(v, 10);
}

/* c.lwsp: uimm[5] -> 12, uimm[4:2|7:6] -> 6..2 */
static uint32_t imm_lwsp(uint32_t c)
{
    return BIT(c, 12) << 5 | BITLAR(c, 6, 4) << 2 | BITLAR(c, 3, 2) << 6;
}

/* c.swsp: uimm[5:2|7:6] -> 12..7 */
static uint32_t imm_swsp(uint32_t c)
{
    return BITLAR(c, 12, 9) << 2 | BITLAR(c, 8, 7) << 6;
}

uint32_t c_kengaytir(uint16_t c16)
{
    uint32_t c = c16;
    uint32_t f3 = BITLAR(c, 15, 13);
    uint32_t rd = BITLAR(c, 11, 7), rs2 = BITLAR(c, 6, 2);     /* to'liq (5 bitli) registr maydonlari */
    uint32_t rdq = 8 + BITLAR(c, 4, 2), rs1q = 8 + BITLAR(c, 9, 7);     /* siqilgan registrlar: x8..x15 */
    enum { SP = 2, RA = 1 };

    if (c == 0)
        return 0;                               /* 0x0000 — ataylab "noto'g'ri": nollangan xotiraga sakrash darhol ushlanadi */

    switch (BITLAR(c, 1, 0)) {
    case 0:                                     /* ======== kvadrant 0 ======== */
        switch (f3) {
        case 0: {                               /* c.addi4spn rd', sp, nzuimm  ->  addi rd', sp, nzuimm */
            uint32_t imm = imm_addi4spn(c);
            return imm ? i_tur(imm, SP, 0, rdq, 0x13) : 0;
        }
        case 2:                                 /* c.lw rd', uimm(rs1')  ->  lw rd', uimm(rs1') */
            return i_tur(imm_lw(c), rs1q, 2, rdq, 0x03);
        case 6:                                 /* c.sw rs2', uimm(rs1')  ->  sw */
            return s_tur(imm_lw(c), rdq, rs1q, 2, 0x23);
        }
        return 0;

    case 1:                                     /* ======== kvadrant 1 ======== */
        switch (f3) {
        case 0:                                 /* c.addi rd, imm  (c.nop: rd = 0) */
            return i_tur(imm_ci(c), rd, 0, rd, 0x13);
        case 1:                                 /* c.jal offset  ->  jal ra, offset (faqat RV32) */
            return j_tur(imm_cj(c), RA);
        case 2:                                 /* c.li rd, imm  ->  addi rd, x0, imm */
            return i_tur(imm_ci(c), 0, 0, rd, 0x13);
        case 3:
            if (rd == SP) {                     /* c.addi16sp  ->  addi sp, sp, nzimm */
                uint32_t imm = imm_addi16sp(c);
                return imm ? i_tur(imm, SP, 0, SP, 0x13) : 0;
            } else {                            /* c.lui rd, nzimm  ->  lui rd, nzimm (imm[17:12]) */
                uint32_t imm = imm_ci(c) << 12;
                return imm ? u_tur(imm, rd, 0x37) : 0;
            }
        case 4: {                               /* arifmetika: rd' = rs1' */
            uint32_t r = rs1q;
            switch (BITLAR(c, 11, 10)) {
            case 0:                             /* c.srli */
                return BIT(c, 12) ? 0 : i_tur(BITLAR(c, 6, 2), r, 5, r, 0x13);
            case 1:                             /* c.srai: funct7 = 0x20 (o'zgarmasning yuqori qismida) */
                return BIT(c, 12) ? 0 : i_tur(0x400 | BITLAR(c, 6, 2), r, 5, r, 0x13);
            case 2:                             /* c.andi */
                return i_tur(imm_ci(c), r, 7, r, 0x13);
            default:                            /* c.sub c.xor c.or c.and (12-bit = 0) */
                if (BIT(c, 12))
                    return 0;                   /* RV64 dagi c.subw/c.addw — bizda yo'q */
                switch (BITLAR(c, 6, 5)) {
                case 0: return r_tur(0x20, rdq, r, 0, r, 0x33);        /* sub */
                case 1: return r_tur(0, rdq, r, 4, r, 0x33);           /* xor */
                case 2: return r_tur(0, rdq, r, 6, r, 0x33);           /* or */
                default: return r_tur(0, rdq, r, 7, r, 0x33);          /* and */
                }
            }
        }
        case 5:                                 /* c.j offset  ->  jal x0, offset */
            return j_tur(imm_cj(c), 0);
        case 6:                                 /* c.beqz rs1', offset  ->  beq rs1', x0, offset */
            return b_tur(imm_cb(c), 0, rs1q, 0);
        case 7:                                 /* c.bnez */
            return b_tur(imm_cb(c), 0, rs1q, 1);
        }
        return 0;

    default:                                    /* ======== kvadrant 2 (3 — 32 bitli, bu yerga kelmaydi) ======== */
        switch (f3) {
        case 0:                                 /* c.slli rd, shamt */
            return BIT(c, 12) ? 0 : i_tur(BITLAR(c, 6, 2), rd, 1, rd, 0x13);
        case 2:                                 /* c.lwsp rd, uimm(sp)  (rd != 0) */
            return rd ? i_tur(imm_lwsp(c), SP, 2, rd, 0x03) : 0;
        case 4:
            if (!BIT(c, 12)) {
                if (rs2 == 0)                   /* c.jr rs1  ->  jalr x0, 0(rs1)   (rs1 != 0) */
                    return rd ? i_tur(0, rd, 0, 0, 0x67) : 0;
                return r_tur(0, rs2, 0, 0, rd, 0x33);                  /* c.mv rd, rs2  ->  add rd, x0, rs2 */
            }
            if (rs2 == 0) {
                if (rd == 0)
                    return 0x00100073u;         /* c.ebreak */
                return i_tur(0, rd, 0, RA, 0x67);                      /* c.jalr rs1  ->  jalr ra, 0(rs1) */
            }
            return r_tur(0, rs2, rd, 0, rd, 0x33);                     /* c.add rd, rs2  ->  add rd, rd, rs2 */
        case 6:                                 /* c.swsp rs2, uimm(sp) */
            return s_tur(imm_swsp(c), rs2, SP, 2, 0x23);
        }
        return 0;
    }
}
