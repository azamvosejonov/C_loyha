#include <elf.h>                                /* tizimning o'z ELF ta'riflari - solishtirish uchun */

#include "test.h"
#include "mashq.h"

static uint8_t *fayl_oqi(const char *yol, size_t *n)
{
    FILE *f = fopen(yol, "rb");
    if (!f)
        return NULL;
    uint8_t *b = malloc(32 << 20);
    *n = fread(b, 1, 32 << 20, f);
    fclose(f);
    return b;
}

int main(void)
{
    TEST_BOSHLA();
    _Static_assert(sizeof(struct elf64_sarlavha) == 64, "sarlavha 64 bayt");
    _Static_assert(sizeof(struct elf64_dastur) == 56, "dastur sarlavhasi 56 bayt");
    size_t n = 0;
    uint8_t *b = fayl_oqi("/proc/self/exe", &n);
    CHECK(b != NULL && n > 1000);
    if (!b)
        TEST_TUGADI();

    /* Kutilgan qiymatlar - tizimning <elf.h> i bilan. */
    Elf64_Ehdr eh;
    memcpy(&eh, b, sizeof(eh));
    size_t kut_soni = 0;
    Elf64_Phdr ph[64];
    for (int i = 0; i < eh.e_phnum && i < 64; i++) {
        memcpy(&ph[kut_soni], b + eh.e_phoff + (size_t)i * sizeof(Elf64_Phdr), sizeof(Elf64_Phdr));
        if (ph[kut_soni].p_type == PT_LOAD)
            kut_soni++;
    }

    BOLIM("haqiqiy fayl: o'zimizning dasturimiz");
    struct elf_info in;
    memset(&in, 0xAA, sizeof(in));
    CHECK_INT(elf_tahlil(b, n, &in), 0);
    CHECK_HEX(in.kirish, eh.e_entry);
    CHECK_INT(in.tur, eh.e_type);
    CHECK_INT(in.soni, kut_soni);
    for (size_t i = 0; i < kut_soni && i < in.soni && i < ELF_MAKS_SEGMENT; i++) {
        CHECK_HEX(in.seg[i].vaddr, ph[i].p_vaddr);
        CHECK_HEX(in.seg[i].memsz, ph[i].p_memsz);
        CHECK_HEX(in.seg[i].filesz, ph[i].p_filesz);
        CHECK_HEX(in.seg[i].offset, ph[i].p_offset);
        CHECK_INT(in.seg[i].bayroqlar, ph[i].p_flags);
    }

    uint8_t *c = malloc(n);
#define BUZ(ifoda, kod)                             \
    do {                                            \
        memcpy(c, b, n);                            \
        ifoda;                                      \
        CHECK_INT(elf_tahlil(c, n, &in), kod);      \
    } while (0)

    Elf64_Ehdr *h = (Elf64_Ehdr *)c;
    BOLIM("ELF emas yoki mos emas -> -1");
    BUZ(c[0] = 0x7e, -1);
    BUZ(c[3] = 'G', -1);
    BUZ(c[EI_CLASS] = ELFCLASS32, -1);
    BUZ(c[EI_DATA] = ELFDATA2MSB, -1);
    BUZ(h->e_machine = 40 /* ARM */, -1);
    BUZ(h->e_type = 1 /* ET_REL */, -1);
    BUZ(h->e_version = 0, -1);

    BOLIM("buzilgan fayl -> -2");
    CHECK_INT(elf_tahlil(b, 40, &in), -2);                         /* sarlavha to'liq emas */
    BUZ(h->e_phentsize = 32, -2);
    BUZ(h->e_phoff = n - 10, -2);                                  /* jadval fayldan chiqadi */
    BUZ(h->e_phoff = 0xFFFFFFFFFFFFFF00ull, -2);                   /* toshish tuzog'i */
    BUZ(h->e_phnum = 0xFFFF, -2);
    size_t birinchi_load = 0;
    for (int i = 0; i < eh.e_phnum; i++) {
        Elf64_Phdr p;
        memcpy(&p, b + eh.e_phoff + (size_t)i * sizeof(p), sizeof(p));
        if (p.p_type == PT_LOAD) {
            birinchi_load = eh.e_phoff + (size_t)i * sizeof(p);
            break;
        }
    }
    Elf64_Phdr *pl = (Elf64_Phdr *)(c + birinchi_load);
    BUZ(pl->p_filesz = pl->p_memsz + 1, -2);                        /* filesz > memsz */
    BUZ((pl->p_offset = n, pl->p_filesz = 1), -2);                 /* segment fayldan tashqarida */
    BUZ((pl->p_offset = 0xFFFFFFFFFFFFFFF0ull, pl->p_filesz = 0x100), -2);   /* toshish */
    free(c);
    free(b);
    TEST_TUGADI();
}
