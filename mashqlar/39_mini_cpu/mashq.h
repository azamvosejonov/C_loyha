#pragma once

#include <stddef.h>
#include <stdint.h>

enum op {
    HALT,       /* to'xtash (muvaffaqiyat)                                */
    LI,         /* r[a] = imm                                             */
    MOV,        /* r[a] = r[b]                                            */
    ADD,        /* r[a] = r[b] + r[d]                                     */
    SUB,        /* r[a] = r[b] - r[d]                                     */
    MUL,        /* r[a] = r[b] * r[d]                                     */
    JMP,        /* pc = imm                                               */
    JZ,         /* agar r[a] == 0: pc = imm                               */
    JNZ,        /* agar r[a] != 0: pc = imm                               */
    LOAD,       /* r[a] = xotira[r[b]]                                    */
    STORE,      /* xotira[r[b]] = r[a]                                    */
    PUSH,       /* sp--; xotira[sp] = r[a]                                */
    POP,        /* r[a] = xotira[sp]; sp++                                */
    CALL,       /* PUSH(pc + 1); pc = imm                                 */
    RET,        /* pc = POP()                                             */
    SYS,        /* tizim chaqiruvi: r[0] = 1 -> chiqarish r[1]; 2 -> chiqish(r[1]) */
    OP_SONI
};

struct buyruq {
    uint8_t op;
    uint8_t a, b, d;            /* registr raqamlari (0..7) */
    int64_t imm;                /* o'zgarmas son yoki manzil */
};

#define XOTIRA 256
#define CHIQISH_MAKS 64

struct cpu {
    int64_t r[8];
    int64_t pc;                 /* keyingi buyruq indeksi */
    int64_t sp;                 /* stek tepasi; bo'sh stek: sp == XOTIRA */
    int64_t xotira[XOTIRA];
    int64_t chiqish[CHIQISH_MAKS];
    int chiqish_soni;
    int64_t chiqish_kodi;
};

int bajar(struct cpu *m, const struct buyruq *dastur, size_t n, long maks_qadam);
