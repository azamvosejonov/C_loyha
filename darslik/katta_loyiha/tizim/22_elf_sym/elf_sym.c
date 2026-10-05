/* elf_sym.c - ELF belgilar jadvali (.symtab) o'quvchisi: mini `nm -S` va "manzil qaysi funksiyada?" qidiruvi */
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *fayl;                     /* butun fayl xotirada */
static size_t fayl_hajm;

static unsigned char *oqi_fayl(const char *nom)
{
    FILE *f = fopen(nom, "rb");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long h = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *b = malloc((size_t)h);
    if (b && fread(b, 1, (size_t)h, f) != (size_t)h) {
        free(b);
        b = NULL;
    }
    fclose(f);
    fayl_hajm = (size_t)h;
    return b;
}

static const char *tur_nomi(unsigned t)
{
    switch (t) {
    case STT_NOTYPE: return "NOTYPE";
    case STT_OBJECT: return "OBJECT";
    case STT_FUNC: return "FUNC";
    case STT_SECTION: return "SECTION";
    case STT_FILE: return "FILE";
    default: return "?";
    }
}

static const char *bog_nomi(unsigned b)
{
    switch (b) {
    case STB_LOCAL: return "LOCAL";
    case STB_GLOBAL: return "GLOBAL";
    case STB_WEAK: return "WEAK";
    default: return "?";
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "foydalanish: %s fayl.o [manzil_hex]\n", argv[0]);
        return 2;
    }
    fayl = oqi_fayl(argv[1]);
    if (!fayl || fayl_hajm < sizeof(Elf64_Ehdr)) {
        fprintf(stderr, "%s: o'qib bo'lmadi\n", argv[1]);
        return 1;
    }

    const Elf64_Ehdr *eh = (const Elf64_Ehdr *)fayl;
    if (memcmp(eh->e_ident, ELFMAG, SELFMAG) != 0 || eh->e_ident[EI_CLASS] != ELFCLASS64) {
        fprintf(stderr, "%s: 64 bitli ELF emas\n", argv[1]);
        return 1;
    }

    const Elf64_Shdr *bolimlar = (const Elf64_Shdr *)(fayl + eh->e_shoff);   /* bo'limlar jadvali */
    const char *bolim_nomlari = (const char *)(fayl + bolimlar[eh->e_shstrndx].sh_offset);

    const Elf64_Shdr *symtab = NULL;
    for (int i = 0; i < eh->e_shnum; i++)
        if (bolimlar[i].sh_type == SHT_SYMTAB)
            symtab = &bolimlar[i];
    if (!symtab) {
        fprintf(stderr, "%s: .symtab yo'q (strip qilingan)\n", argv[1]);
        return 1;
    }
    const Elf64_Sym *belgilar = (const Elf64_Sym *)(fayl + symtab->sh_offset);
    size_t soni = symtab->sh_size / sizeof(Elf64_Sym);
    const char *nomlar = (const char *)(fayl + bolimlar[symtab->sh_link].sh_offset);   /* sh_link: nomlar jadvali */

    if (argc == 2) {
        printf("%-3s %-8s %-5s %-7s %-7s %-9s %s\n", "#", "qiymat", "hajm", "tur", "bog'", "bo'lim", "nom");
        for (size_t i = 0; i < soni; i++) {
            const Elf64_Sym *s = &belgilar[i];
            const char *bolim = s->st_shndx == SHN_UNDEF ? "UND" :
                                s->st_shndx >= SHN_LORESERVE ? "ABS" : bolim_nomlari + bolimlar[s->st_shndx].sh_name;
            printf("%-3zu %08lx %-5lu %-7s %-7s %-9s %s\n", i, (unsigned long)s->st_value, (unsigned long)s->st_size,
                   tur_nomi(ELF64_ST_TYPE(s->st_info)), bog_nomi(ELF64_ST_BIND(s->st_info)), bolim,
                   nomlar + s->st_name);
        }
        return 0;
    }

    /* manzil bo'yicha qidirish: qaysi FUNC belgining [qiymat, qiymat+hajm) oralig'iga tushadi? */
    unsigned long m = strtoul(argv[2], NULL, 16);
    for (size_t i = 0; i < soni; i++) {
        const Elf64_Sym *s = &belgilar[i];
        if (ELF64_ST_TYPE(s->st_info) == STT_FUNC && s->st_shndx != SHN_UNDEF && m >= s->st_value &&
            m < s->st_value + s->st_size) {
            printf("0x%lx: %s + %lu\n", m, nomlar + s->st_name, m - s->st_value);
            return 0;
        }
    }
    printf("0x%lx: hech qaysi funksiyaga tegishli emas\n", m);
    return 1;
}
