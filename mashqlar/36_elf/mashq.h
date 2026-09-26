#pragma once

#include <stddef.h>
#include <stdint.h>

/* ELF64 fayl sarlavhasi - fayl boshida (64 bayt). */
struct elf64_sarlavha {
    uint8_t  ident[16];         /* 0x7f 'E' 'L' 'F', sinf, bayt tartibi, versiya, ... */
    uint16_t tur;               /* 2 - ET_EXEC, 3 - ET_DYN (PIE) */
    uint16_t mashina;           /* 62 - x86-64 */
    uint32_t versiya;           /* 1 */
    uint64_t kirish;            /* birinchi bajariladigan buyruq manzili (_start) */
    uint64_t phoff;             /* dastur sarlavhalari jadvali - fayldagi siljish */
    uint64_t shoff;
    uint32_t bayroqlar;
    uint16_t ehsize;
    uint16_t phentsize;         /* bitta dastur sarlavhasining hajmi (56) */
    uint16_t phnum;             /* nechta dastur sarlavhasi */
    uint16_t shentsize, shnum, shstrndx;
};

/* Dastur sarlavhasi (program header) - 56 bayt. */
struct elf64_dastur {
    uint32_t tur;               /* 1 - PT_LOAD (xotiraga yuklanadigan segment) */
    uint32_t bayroqlar;         /* 1 - X (bajarish), 2 - W (yozish), 4 - R (o'qish) */
    uint64_t offset;            /* fayldagi joyi */
    uint64_t vaddr;             /* xotiradagi manzili */
    uint64_t paddr;
    uint64_t filesz;            /* fayldagi hajmi */
    uint64_t memsz;             /* xotiradagi hajmi (>= filesz; qolgani nollanadi - .bss) */
    uint64_t align;
};

#define ELF_MAKS_SEGMENT 16

struct segment {
    uint64_t vaddr, memsz, filesz, offset;
    uint32_t bayroqlar;
};

struct elf_info {
    uint64_t kirish;
    int tur;
    size_t soni;                            /* PT_LOAD segmentlar soni */
    struct segment seg[ELF_MAKS_SEGMENT];
};

int elf_tahlil(const uint8_t *buf, size_t n, struct elf_info *out);
