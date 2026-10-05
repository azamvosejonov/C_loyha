/*
 * disasm.c — disassembler: 32 bitli sonni odam o'qiy oladigan buyruqqa aylantiradi.
 *   0x00150513  ->  "addi a0, a0, 1"
 * Emulyatorning --trace rejimi shundan foydalanadi: protsessor aynan nima qilayotganini ko'rasiz.
 * dekod.h dagi funksiyalarni QAYTA ishlatadi — dekoderda xato bo'lsa, disassembler ham xato ko'rsatadi
 * (testlar uni ham shu sababli tekshiradi).
 *
 * Registr nomlari — ABI (chaqirish qoidasi) nomlari: x1 = ra (qaytish manzili), x2 = sp (stek), x10..x17 =
 * a0..a7 (argumentlar va natija), x8..x9, x18..x27 = s0..s11 (saqlanadigan), qolganlari t0..t6 (vaqtinchalik).
 */
#include <stdio.h>

#include "dekod.h"
#include "mashina.h"
#include "siqilgan.h"

const char *const registr_nomi[32] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
};

static const char *csr_nomi(uint32_t r)
{
    switch (r) {
    case CSR_SSTATUS: return "sstatus";
    case CSR_SIE: return "sie";
    case CSR_STVEC: return "stvec";
    case CSR_SCOUNTEREN: return "scounteren";
    case CSR_SSCRATCH: return "sscratch";
    case CSR_SEPC: return "sepc";
    case CSR_SCAUSE: return "scause";
    case CSR_STVAL: return "stval";
    case CSR_SIP: return "sip";
    case CSR_STIMECMP: return "stimecmp";
    case CSR_STIMECMPH: return "stimecmph";
    case CSR_SATP: return "satp";
    case CSR_CYCLE: return "cycle";
    case CSR_TIME: return "time";
    case CSR_INSTRET: return "instret";
    case CSR_TIMEH: return "timeh";
    case CSR_MSTATUS: return "mstatus";
    case CSR_MISA: return "misa";
    case CSR_MEDELEG: return "medeleg";
    case CSR_MIDELEG: return "mideleg";
    case CSR_MIE: return "mie";
    case CSR_MTVEC: return "mtvec";
    case CSR_MCOUNTEREN: return "mcounteren";
    case CSR_MSCRATCH: return "mscratch";
    case CSR_MEPC: return "mepc";
    case CSR_MCAUSE: return "mcause";
    case CSR_MTVAL: return "mtval";
    case CSR_MIP: return "mip";
    case CSR_MHARTID: return "mhartid";
    case CSR_MENVCFGH: return "menvcfgh";
    default: return NULL;
    }
}

static void disasm32(uint32_t b, uint32_t pc, char *s, size_t n);

/* 16 bitli buyruq bo'lsa (past 2 bit != 11): avval 32 bitliga kengaytirib, "c." prefiksi bilan ko'rsatamiz */
void disasm(uint32_t b, uint32_t pc, char *s, size_t n)
{
    if ((b & 3u) != 3u) {
        uint32_t k = c_kengaytir((uint16_t)b);
        if (!k) {
            snprintf(s, n, "? (c: 0x%04x)", b & 0xFFFFu);
            return;
        }
        char ichki[56];
        disasm32(k, pc, ichki, sizeof(ichki));
        snprintf(s, n, "c.%s", ichki);
        return;
    }
    disasm32(b, pc, s, n);
}

static void disasm32(uint32_t b, uint32_t pc, char *s, size_t n)
{
    const char *const *R = registr_nomi;
    uint32_t rd = d_rd(b), rs1 = d_rs1(b), rs2 = d_rs2(b), f3 = d_funct3(b), f7 = d_funct7(b);
    static const char *const alu_i[8] = { "addi", "slli", "slti", "sltiu", "xori", "srli", "ori", "andi" };
    static const char *const alu_r[8] = { "add", "sll", "slt", "sltu", "xor", "srl", "or", "and" };
    static const char *const m_r[8] = { "mul", "mulh", "mulhsu", "mulhu", "div", "divu", "rem", "remu" };
    static const char *const shart[8] = { "beq", "bne", "?", "?", "blt", "bge", "bltu", "bgeu" };
    static const char *const yukla[8] = { "lb", "lh", "lw", "?", "lbu", "lhu", "?", "?" };
    static const char *const saqla[8] = { "sb", "sh", "sw", "?", "?", "?", "?", "?" };
    static const char *const csrb[8] = { "?", "csrrw", "csrrs", "csrrc", "?", "csrrwi", "csrrsi", "csrrci" };

    switch (d_opcode(b)) {
    case 0x37: snprintf(s, n, "lui %s, 0x%x", R[rd], imm_u(b) >> 12); return;
    case 0x17: snprintf(s, n, "auipc %s, 0x%x", R[rd], imm_u(b) >> 12); return;
    case 0x6F: snprintf(s, n, "jal %s, 0x%x", R[rd], pc + imm_j(b)); return;
    case 0x67: snprintf(s, n, "jalr %s, %d(%s)", R[rd], (int32_t)imm_i(b), R[rs1]); return;
    case 0x63: snprintf(s, n, "%s %s, %s, 0x%x", shart[f3], R[rs1], R[rs2], pc + imm_b(b)); return;
    case 0x03: snprintf(s, n, "%s %s, %d(%s)", yukla[f3], R[rd], (int32_t)imm_i(b), R[rs1]); return;
    case 0x23: snprintf(s, n, "%s %s, %d(%s)", saqla[f3], R[rs2], (int32_t)imm_s(b), R[rs1]); return;
    case 0x13:
        if (f3 == 1 || f3 == 5)
            snprintf(s, n, "%s %s, %s, %u", f3 == 5 && f7 == 0x20 ? "srai" : alu_i[f3], R[rd], R[rs1], rs2);
        else
            snprintf(s, n, "%s %s, %s, %d", alu_i[f3], R[rd], R[rs1], (int32_t)imm_i(b));
        return;
    case 0x33: {
        const char *nom = f7 == 0x01 ? m_r[f3] : f7 == 0x20 ? (f3 == 0 ? "sub" : "sra") : alu_r[f3];
        snprintf(s, n, "%s %s, %s, %s", nom, R[rd], R[rs1], R[rs2]);
        return;
    }
    case 0x0F: snprintf(s, n, f3 == 1 ? "fence.i" : "fence"); return;
    case 0x2F: {
        static const char *const amo[32] = { [0] = "amoadd.w", [1] = "amoswap.w", [2] = "lr.w", [3] = "sc.w", [4] = "amoxor.w",
                                             [8] = "amoor.w", [12] = "amoand.w", [16] = "amomin.w", [20] = "amomax.w",
                                             [24] = "amominu.w", [28] = "amomaxu.w" };
        const char *nom = amo[BITLAR(b, 31, 27)] ? amo[BITLAR(b, 31, 27)] : "amo?";
        snprintf(s, n, "%s %s, %s, (%s)", nom, R[rd], R[rs2], R[rs1]);
        return;
    }
    case 0x73:
        if (f3 == 0) {
            const char *nom = b == 0x73 ? "ecall" : b == 0x00100073 ? "ebreak" : b == 0x10200073 ? "sret"
                            : b == 0x10500073 ? "wfi" : b == 0x30200073 ? "mret" : f7 == 0x09 ? "sfence.vma" : "?";
            snprintf(s, n, "%s", nom);
        } else {
            const char *c = csr_nomi(BITLAR(b, 31, 20));
            char raqam[16];
            if (!c) {
                snprintf(raqam, sizeof(raqam), "0x%x", BITLAR(b, 31, 20));
                c = raqam;
            }
            if (f3 & 4)
                snprintf(s, n, "%s %s, %s, %u", csrb[f3], R[rd], c, rs1);
            else
                snprintf(s, n, "%s %s, %s, %s", csrb[f3], R[rd], c, R[rs1]);
        }
        return;
    }
    snprintf(s, n, "? (0x%08x)", b);
}
