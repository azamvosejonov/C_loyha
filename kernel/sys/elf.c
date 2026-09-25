/* =============================================================================
 *  sys/elf.c - ELF yuklovchi (loader)
 * =============================================================================
 *
 *  ELF (Executable and Linkable Format) - Linux va ko'plab tizimlarda
 *  bajariladigan fayllar formati. `gcc` va `ld` aynan shuni yaratadi.
 *
 *  ELF TUZILISHI (yuklash uchun kerakli qismi):
 *
 *    ┌─────────────────────┐ 0
 *    │ ELF sarlavhasi      │  magic "\x7fELF", arxitektura, e_entry (kirish nuqtasi),
 *    │ (64 bayt)           │  e_phoff (dastur sarlavhalari qayerda)
 *    ├─────────────────────┤ e_phoff
 *    │ Program header [0]  │  PT_LOAD: "fayldagi [p_offset, +p_filesz) baytlarni
 *    │ Program header [1]  │  xotiradagi p_vaddr ga qo'y, jami p_memsz joy ajrat"
 *    │ ...                 │
 *    ├─────────────────────┤
 *    │ .text, .data, ...   │  haqiqiy kod va ma'lumotlar
 *    └─────────────────────┘
 *
 *  p_memsz > p_filesz bo'lsa, farq - .bss (nollar). Faylda joy olmaydi.
 *  Linux'dagi `readelf -l build/initrd/hello` buyrug'i shu sarlavhalarni
 *  ko'rsatadi - sinab ko'ring!
 *
 *  XAVFSIZLIK: ELF fayl - TASHQI (ishonchsiz) ma'lumot. Har bir maydonni
 *  tekshiramiz: siljishlar fayl ichidami, manzillar user hududidami, butun son
 *  to'lib ketmaydimi. Aks holda buzilgan fayl yadroni buzishi mumkin. Real
 *  yadrolardagi ko'plab zaifliklar aynan shunday "parser"lardan chiqqan.
 * ============================================================================= */
#include "sys/elf.h"

#include "lib/common.h"
#include "lib/string.h"
#include "mm/pmm.h"
#include "mm/vmm.h"

struct elf64_ehdr {
    uint8_t e_ident[16];                /* magic, sinf (32/64), endianness ... */
    uint16_t e_type;                    /* 2 = EXEC (bajariladigan) */
    uint16_t e_machine;                 /* 62 = x86-64 */
    uint32_t e_version;
    uint64_t e_entry;                   /* kirish nuqtasi (virtual manzil) */
    uint64_t e_phoff;                   /* program header'lar siljishi */
    uint64_t e_shoff;                   /* section header'lar (bizga kerak emas) */
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;               /* bitta program header hajmi */
    uint16_t e_phnum;                   /* program header'lar soni */
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed));

struct elf64_phdr {
    uint32_t p_type;                    /* 1 = PT_LOAD */
    uint32_t p_flags;                   /* PF_X=1, PF_W=2, PF_R=4 */
    uint64_t p_offset;                  /* fayldagi siljish */
    uint64_t p_vaddr;                   /* xotiradagi manzil */
    uint64_t p_paddr;
    uint64_t p_filesz;                  /* fayldagi hajm */
    uint64_t p_memsz;                   /* xotiradagi hajm (>= filesz) */
    uint64_t p_align;
} __attribute__((packed));

#define ELF_CLASS64   2
#define ELF_DATA_LE   1
#define ET_EXEC       2
#define EM_X86_64     62
#define PT_LOAD       1
#define PF_W          2

/* Stekning eng katta hajmi va himoya sahifasi uchun joy qoldiramiz. */
#define USER_IMAGE_LIMIT (USER_STACK_TOP - USER_STACK_MAX - PAGE_SIZE)

int elf_load(uint64_t pml4, const uint8_t *data, size_t size, uint64_t *entry,
             uint64_t *image_end)
{
    if (size < sizeof(struct elf64_ehdr))
        return -1;
    const struct elf64_ehdr *eh = (const void *)data;

    /* --- Sarlavhani tekshirish --- */
    if (memcmp(eh->e_ident, "\x7f" "ELF", 4) != 0)
        return -2;                      /* ELF emas */
    if (eh->e_ident[4] != ELF_CLASS64 || eh->e_ident[5] != ELF_DATA_LE)
        return -3;                      /* 64-bit little-endian emas */
    if (eh->e_type != ET_EXEC || eh->e_machine != EM_X86_64)
        return -4;
    if (eh->e_phentsize != sizeof(struct elf64_phdr))
        return -5;
    if (eh->e_phoff > size || (uint64_t)eh->e_phnum * sizeof(struct elf64_phdr) > size - eh->e_phoff)
        return -6;                      /* program header'lar fayldan tashqarida */

    const struct elf64_phdr *ph = (const void *)(data + eh->e_phoff);
    uint64_t highest = USER_SPACE_START;
    bool entry_ok = false;

    for (uint16_t i = 0; i < eh->e_phnum; i++) {
        const struct elf64_phdr *p = &ph[i];
        if (p->p_type != PT_LOAD || p->p_memsz == 0)
            continue;

        /* --- Segmentni tekshirish --- */
        if (p->p_filesz > p->p_memsz)
            return -7;
        if (p->p_offset > size || p->p_filesz > size - p->p_offset)
            return -8;                  /* ma'lumot fayldan tashqarida */
        uint64_t start = p->p_vaddr;
        uint64_t end = p->p_vaddr + p->p_memsz;
        if (end < start || start < USER_SPACE_START || end > USER_IMAGE_LIMIT)
            return -9;                  /* yadro hududiga yoki stekka yuklashga urinish! */

        /* --- Sahifalarni ajratish --- */
        uint64_t flags = PTE_USER | ((p->p_flags & PF_W) ? PTE_WRITABLE : 0);
        for (uint64_t va = ALIGN_DOWN(start, PAGE_SIZE); va < end; va += PAGE_SIZE) {
            uint64_t old_flags;
            if (vmm_translate(pml4, va, &old_flags)) {
                /* Sahifa oldingi segment bilan umumiy: ruxsatlarni birlashtiramiz. */
                vmm_update_flags(pml4, va, old_flags | flags);
                continue;
            }
            if (!vmm_map_anonymous(pml4, va, 1, flags))   /* nollangan freym = .bss tayyor */
                return -10;
        }

        /* --- Fayldagi baytlarni nusxalash (fizik manzillar orqali, chunki bu
         *     manzil maydoni hozir faol emas) --- */
        if (!vmm_copy_to_space(pml4, start, data + p->p_offset, p->p_filesz))
            return -11;

        if (eh->e_entry >= start && eh->e_entry < end)
            entry_ok = true;
        if (end > highest)
            highest = end;
    }

    if (!entry_ok)
        return -12;                     /* kirish nuqtasi yuklangan kod ichida emas */
    *entry = eh->e_entry;
    *image_end = highest;
    return 0;
}
