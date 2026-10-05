/*
 * alu.h — arifmetik-mantiqiy qurilma (ALU): buyruqlarning "hisob" qismi.
 * Xotira va registrlar bilan ishlamaydi — faqat son oladi, son qaytaradi. Shuning uchun ularni
 * alohida (host'da, testlar/birlik/birlik.c) sinash oson.
 */
#ifndef ALU_H
#define ALU_H

#include "turlar.h"

/* RV32I asosiy amallari (add, sub, sll, slt, sltu, xor, srl, sra, or, and).
   funct3 — amal; alt — funct7 ning 5-biti (sub/sra uchun), registr — 1 bo'lsa R-tur (add/sub farqlanadi).
   *xato = 1 — bunday amal yo'q (noto'g'ri buyruq). */
uint32_t alu_asosiy(uint32_t funct3, int alt, int registr, uint32_t a, uint32_t b, int *xato);

/* Shartli sakrash: beq(0) bne(1) blt(4) bge(5) bltu(6) bgeu(7). 1 — sakrash kerak, 0 — kerak emas,
   -1 — bunday funct3 yo'q (2 va 3) */
int shart_bajarildimi(uint32_t funct3, uint32_t a, uint32_t b);

/* Yuklangan qiymatni 32 bitga kengaytirish: lb(0) lh(1) lw(2) lbu(4) lhu(5).
   "u" — ishorasiz (nol bilan to'ldirish), aks holda ishora bilan. */
uint32_t yuklash_kengaytir(uint32_t funct3, uint32_t qiymat);

/* M kengaytmasi: mul(0) mulh(1) mulhsu(2) mulhu(3) div(4) divu(5) rem(6) remu(7).
   Nolga bo'lish va toshish holatlari spetsifikatsiyadagidek (alu.c dagi jadvalga qarang) */
uint32_t m_amal(uint32_t funct3, uint32_t a, uint32_t b);

#endif
