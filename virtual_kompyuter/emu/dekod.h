/*
 * dekod.h — 32 bitli RISC-V buyrug'ini MAYDONLARGA ajratish (decode).
 *
 * Har RISC-V buyrug'i 32 bit. Pastki 7 bit — OPCODE: buyruqning umumiy turi (yuklash, saqlash, sakrash ...).
 * Qolgan bitlar formatga qarab turli ma'noga ega. Oltita format bor:
 *
 *   R-tur (registr-registr: add, sub ...):
 *     | funct7 (7) | rs2 (5) | rs1 (5) | funct3 (3) | rd (5) | opcode (7) |
 *       31     25   24   20   19   15   14       12   11   7   6        0
 *   I-tur (o'zgarmas bilan: addi, lw, jalr ...):
 *     |   imm[11:0] (12)     | rs1 (5) | funct3 (3) | rd (5) | opcode (7) |
 *   S-tur (saqlash: sw ...):  o'zgarmas IKKI bo'lakda: [31:25] va [11:7]
 *     | imm[11:5] | rs2 | rs1 | funct3 | imm[4:0] | opcode |
 *   B-tur (shartli sakrash: beq ...): o'zgarmas aralashtirilgan, 0-biti doim 0 (manzillar 2 ga juft)
 *     | imm[12] | imm[10:5] | rs2 | rs1 | funct3 | imm[4:1] | imm[11] | opcode |
 *   U-tur (lui, auipc): yuqori 20 bit
 *     |        imm[31:12] (20)       | rd | opcode |
 *   J-tur (jal): aralashtirilgan 20 bit, 0-bit doim 0
 *     | imm[20] | imm[10:1] | imm[11] | imm[19:12] | rd | opcode |
 *
 * NEGA BITLAR ARALASHTIRILGAN? Apparat uchun qulay: rs1, rs2, rd HAR DOIM bir xil joyda, ishora biti
 * HAR DOIM 31-bitda. Shunda protsessor registrlarni formatni bilmasdan oldindan o'qiy oladi.
 * Dasturiy emulyatorda esa bu bizga "bit jumbog'i": bo'laklarni to'g'ri joyiga yig'ish kerak.
 */
#ifndef DEKOD_H
#define DEKOD_H

#include "turlar.h"

static inline uint32_t d_opcode(uint32_t b) { return BITLAR(b, 6, 0); }
static inline uint32_t d_rd(uint32_t b) { return BITLAR(b, 11, 7); }
static inline uint32_t d_funct3(uint32_t b) { return BITLAR(b, 14, 12); }
static inline uint32_t d_rs1(uint32_t b) { return BITLAR(b, 19, 15); }
static inline uint32_t d_rs2(uint32_t b) { return BITLAR(b, 24, 20); }
static inline uint32_t d_funct7(uint32_t b) { return BITLAR(b, 31, 25); }

/* O'zgarmaslar (immediate). Hammasi ISHORA BILAN 32 bitga kengaytirilgan (U-tur bundan mustasno — u allaqachon 32 bit). dekod.c */
uint32_t imm_i(uint32_t b);
uint32_t imm_s(uint32_t b);
uint32_t imm_b(uint32_t b);
uint32_t imm_u(uint32_t b);
uint32_t imm_j(uint32_t b);

#endif
