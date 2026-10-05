/*
 * main.c — emulyatorni ishga tushirish: "kompyuter"ni yig'ish, dasturlarni yuklash, quvvatni yoqish.
 *
 *   ./vk [parametrlar] birinchi.elf [ikkinchi.elf | -k Image]
 *     -m MB      RAM hajmi (sukut 64)
 *     -d FAYL    disk tasviri
 *     -k FAYL    xom (raw) yadro tasviri (masalan Linux'ning arch/riscv/boot/Image) — 0x8040_0000 ga yuklanadi
 *     -S         firmware'siz rejim: protsessor to'g'ridan-to'g'ri S rejimda boshlanadi, M rejim vazifalarini
 *                (delegatsiya, Sstc taymer) emulyator o'zi sozlaydi. Oddiy testlar va o'rganish uchun.
 *     -t         trace: har bajarilgan buyruq stderr ga (juda ko'p! -n bilan birga ishlating)
 *     -n SON     ko'pi bilan shuncha qadam (cheksiz sikl yoki cheksiz trap'ni to'xtatish uchun)
 *     -s         oxirida statistika: buyruqlar soni, TLB, disk
 *     -i         "odamdek yozish": stdin (fayl) qatorma-qator, dastur jim turganda beriladi (shell testlari uchun)
 *     -D FAYL    yaratilgan qurilmalar daraxtini (DTB) faylga ham yozish (`dtc -I dtb FAYL` bilan o'qish mumkin)
 *
 * Yoqilganda (haqiqiy RISC-V kompyuteridagi kabi):
 *   rejim = M, pc = birinchi ELF ning kirish nuqtasi (odatda firmware), a0 = 0 (yadro raqami — "hart id").
 *   Ikkinchi ELF (yoki -k tasvir) ham xotiraga yuklanadi; uni ishga tushirish — firmware ning vazifasi.
 *
 * Chiqish kodi: dastur quvvat qurilmasiga yozgan kod; -n chegarasi tugasa 124.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mashina.h"

static void foydalanish(void)
{
    fprintf(stderr,
            "foydalanish: vk [bayroqlar] dastur.elf [yadro.elf]\n"
            "  -m MB        RAM hajmi (standart 64)\n"
            "  -d disk.img  disk tasviri (0x10001000 dagi DMA disk)\n"
            "  -k Image     xom yadro tasviri (Linux Image) -> 0x80400000\n"
            "  -S           S rejimda boshlash (firmware'siz: delegatsiya tayyor qilinadi)\n"
            "  -i           klaviatura kirishini 'odamdek' berish (skriptli kirish uchun)\n"
            "  -t           har buyruqni chop etish (trace)\n"
            "  -n N         N qadamdan keyin to'xtash (cheksiz aylanishdan himoya)\n"
            "  -s           oxirida statistika\n"
            "  -D fayl      yaratilgan qurilmalar daraxtini (DTB) faylga yozish\n");
    exit(2);
}

int main(int argc, char **argv)
{
    static struct mashina m;                    /* static: struct katta (TLB), stekka qo'ymaymiz; nollar bilan boshlanadi */
    uint32_t ram_mb = 64;
    const char *disk_yol = NULL, *birinchi = NULL, *ikkinchi = NULL, *xom = NULL;
    int faqat_s = 0, odamdek = 0;
    uint64_t chegara = 0;
    int statistika = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc)
            ram_mb = (uint32_t)atoi(argv[++i]);
        else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc)
            disk_yol = argv[++i];
        else if (strcmp(argv[i], "-t") == 0)
            m.trace = 1;
        else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc)
            chegara = strtoull(argv[++i], NULL, 10);
        else if (strcmp(argv[i], "-s") == 0)
            statistika = 1;
        else if (strcmp(argv[i], "-S") == 0)
            faqat_s = 1;
        else if (strcmp(argv[i], "-i") == 0)
            odamdek = 1;
        else if (strcmp(argv[i], "-D") == 0 && i + 1 < argc)
            m.dtb_fayl = argv[++i];
        else if (strcmp(argv[i], "-k") == 0 && i + 1 < argc)
            xom = argv[++i];
        else if (argv[i][0] == '-' || ikkinchi)
            foydalanish();
        else if (birinchi)
            ikkinchi = argv[i];
        else
            birinchi = argv[i];
    }
    if (!birinchi || ram_mb == 0 || ram_mb > 1024)
        foydalanish();

    m.ram_hajm = ram_mb * 1024u * 1024u;
    m.ram = calloc(1, m.ram_hajm);
    if (!m.ram) {
        fprintf(stderr, "RAM uchun xotira yetmadi\n");
        return 2;
    }
    if (disk_yol && disk_ulash(&m.disk, disk_yol) != 0) {
        perror(disk_yol);
        return 2;
    }

    uint32_t kirish, ikkinchi_kirish = 0;
    if (elf_yukla(&m, birinchi, &kirish) != 0)
        return 2;
    if (ikkinchi && elf_yukla(&m, ikkinchi, &ikkinchi_kirish) != 0)
        return 2;
    if (xom && xom_yukla(&m, xom, XOM_MANZIL) != 0)
        return 2;

    struct cpu *c = &m.cpu;
    c->pc = kirish;
    c->x[10] = 0;                               /* a0 = hart id */
    c->x[11] = dtb_joylash(&m);                 /* a1 = qurilmalar daraxti (DTB) manzili: qanday apparat borligini aytadi */
    c->stimecmp = UINT64_MAX;                   /* taymerlar o'chiq: dastur o'zi sozlaydi */
    m.clint.mtimecmp = UINT64_MAX;
    if (faqat_s) {
        /* Firmware o'rniga: hamma ruxsat etilgan trap va S uzilishlarini S ga topshiramiz, Sstc va hisoblagichlarni yoqamiz */
        c->rejim = REJIM_S;
        c->medeleg = 0xB3FFu;
        c->mideleg = MIP_S_BITLAR;
        c->menvcfgh = MENVCFGH_STCE;
        c->mcounteren = 7;
    } else {
        c->rejim = REJIM_M;
    }

    uart_tayyorla(&m.uart);
    if (odamdek)
        m.uart.vaqt = &c->instret;             /* "odamdek yozish" uchun vaqt manbai */
    uint64_t qadamlar = 0;                      /* instret emas: trap'lar buyruq hisoblanmaydi, cheksiz trap sikli ham to'xtasin */
    while (!m.toxtadi) {
        cpu_qadam(&m);
        if (chegara && ++qadamlar >= chegara) {
            fprintf(stderr, "\nemulyator: %llu ta buyruq chegarasiga yetdi (pc=0x%08x)\n", (unsigned long long)chegara, c->pc);
            m.chiqish_kodi = 124;
            break;
        }
    }
    uart_tugat();

    if (statistika) {
        uint64_t jami = c->tlb_topildi + c->tlb_topilmadi;
        fprintf(stderr, "statistika: %llu ta buyruq bajarildi; TLB: %llu topildi, %llu topilmadi (%.1f%%); disk: %llu o'qish, %llu yozish\n",
                (unsigned long long)c->instret, (unsigned long long)c->tlb_topildi, (unsigned long long)c->tlb_topilmadi,
                jami ? 100.0 * (double)c->tlb_topildi / (double)jami : 0.0, (unsigned long long)m.disk.oqildi,
                (unsigned long long)m.disk.yozildi);
    }
    if (m.disk.fayl)
        fclose(m.disk.fayl);
    free(m.ram);
    return m.chiqish_kodi;
}
