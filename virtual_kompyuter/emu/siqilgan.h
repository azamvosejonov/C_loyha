/*
 * siqilgan.h — C kengaytmasi: 16 bitli ("siqilgan") buyruqlarni 32 bitli ekvivalentiga aylantirish.
 *
 * NEGA KERAK?
 *   Dasturlarda eng ko'p uchraydigan buyruqlar (addi sp, sp, -16 / lw a0, 8(sp) / ret / mv a0, a1 ...)
 *   ko'pincha kichik o'zgarmas va "mashhur" registrlar (a0..a5, s0, s1, sp) bilan ishlaydi. Ular uchun
 *   32 bit ortiqcha: C kengaytmasi ularni 16 bitga sig'diradi. Natija: kod 25-30% kichik, kesh samaraliroq.
 *
 * QANDAY TANILADI?
 *   Buyruqning pastki 2 biti: 11 — oddiy 32 bitli buyruq; 00, 01, 10 — 16 bitli buyruq ("kvadrant" 0, 1, 2).
 *   Shuning uchun protsessor avval 16 bit o'qiydi, past ikki bitiga qarab qolgan 16 bitni o'qish kerakmi —
 *   hal qiladi.
 *
 * BIZNING USUL — "kengaytirish" (haqiqiy protsessorlar ham ko'pincha shunday qiladi):
 *   16 bitli buyruqni AYNAN shu ishni qiladigan 32 bitli buyruqqa aylantiramiz va oddiy bajaruvchiga beramiz.
 *   Masalan c.addi a0, 1 (0x0505) -> addi a0, a0, 1 (0x00150513). Bajaruvchiga faqat "buyruq uzunligi 2"
 *   ekanini aytamiz (pc + 2, jal/jalr dagi qaytish manzili ham pc + 2).
 *
 * BIT JUMBOQLARI
 *   C buyruqlarda o'zgarmas sonning bitlari juda ARALASH joylashgan (masalan c.j: imm[11|4|9:8|10|6|7|3:1|5]).
 *   Ularni to'g'ri yig'ish — bitlar bilan ishlashning eng yaxshi mashqi. siqilgan.c dagi har funksiya
 *   spetsifikatsiyadagi jadvaldan BIR qator.
 */
#ifndef SIQILGAN_H
#define SIQILGAN_H

#include "turlar.h"

/* 16 bitli buyruqni 32 bitliga aylantiradi. 0 — noto'g'ri (yoki ajratilgan/"reserved") buyruq */
uint32_t c_kengaytir(uint16_t c);

#endif
