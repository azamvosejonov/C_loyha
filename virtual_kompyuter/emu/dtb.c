/*
 * dtb.c — "qurilmalar daraxti" (Device Tree Blob, DTB) ni yaratish.
 *
 * MUAMMO
 *   Yadro (masalan Linux) bitta kompyuter uchun yozilmaydi — minglab xil platformada ishlaydi. Qanday
 *   bilsin: RAM qayerda va qancha? UART qaysi manzilda? Protsessor qaysi kengaytmalarni biladi?
 *   x86 da bu ma'lumotni BIOS/ACPI beradi; RISC-V va ARM da — QURILMALAR DARAXTI.
 *
 * DARAXT (matn ko'rinishi — DTS):
 *     / {
 *         #address-cells = <1>;
 *         memory@80000000 { device_type = "memory"; reg = <0x80000000 0x4000000>; };
 *         cpus { cpu@0 { riscv,isa = "rv32imac..."; mmu-type = "riscv,sv32"; ... }; };
 *         soc { serial@10000000 { compatible = "ns16550a"; reg = <...>; interrupts = <10>; }; ... };
 *     };
 *   "compatible" — drayver tanlash kaliti: Linux "ns16550a" ni ko'rib, 8250 UART drayverini ulaydi.
 *
 * BINAR FORMAT (FDT — flattened device tree). Hamma sonlar BIG-ENDIAN (katta bayt birinchi)!
 *   Bizning protsessor little-endian bo'lsa ham — format shunday kelishilgan. Shuning uchun har 32 bitli
 *   sonni be32_yoz() bilan baytma-bayt yozamiz.
 *
 *   +------------------+  0
 *   | sarlavha (40 B)  |  sehrli son 0xD00DFEED, o'lchamlar, bloklar siljishi
 *   +------------------+
 *   | xotira zaxirasi  |  (bizda bo'sh: 16 ta nol bayt — ro'yxat oxiri)
 *   +------------------+
 *   | TUZILMA bloki    |  tokenlar: BEGIN_NODE(1) nom | PROP(3) uzunlik nom_siljishi qiymat | END_NODE(2) | END(9)
 *   +------------------+
 *   | SATRLAR bloki    |  xossa NOMLARI ("reg\0compatible\0..."), har nom bir marta; PROP shu yerdagi siljishni saqlaydi
 *   +------------------+
 *   Har token va qiymat 4 baytga TEKISLANADI (ortiqcha joy nollar bilan to'ldiriladi).
 */
#include <stdio.h>
#include <string.h>

#include "mashina.h"

#define FDT_SEHR 0xD00DFEEDu
#define FDT_BEGIN_NODE 1u
#define FDT_END_NODE 2u
#define FDT_PROP 3u
#define FDT_END 9u

#define TUZILMA_MAKS 8192
#define SATRLAR_MAKS 1024

struct fdt {
    uint8_t tuzilma[TUZILMA_MAKS];
    uint32_t t_uz;
    char satrlar[SATRLAR_MAKS];
    uint32_t s_uz;
};

/* 32 bitli sonni BIG-ENDIAN tartibda 4 baytga yozadi: eng katta bayt (q >> 24) birinchi */
static void be32_yoz(uint8_t *p, uint32_t q)
{
    /*
     * TODO(D1) — O'ZINGIZ YOZING: 32 bitli sonni BIG-endian (katta bayt birinchi) 4 baytga yozing.
     *   - p[0] = q >> 24, p[1] = q >> 16, ... (uint8_t ga o'tkazing)
     *   - DTB doim big-endian — RISC-V esa little-endian!
     * Tekshirish: make test  (birlik testida 'D1' qatori)
     */
    (void)q;
    p[0] = p[1] = p[2] = p[3] = 0;
}

/* tuzilma blokiga baytlar qo'shadi va uzunlikni 4 ga tekislaydi (bo'shliq — nollar) */
static void baytlar(struct fdt *f, const void *b, uint32_t n)
{
    memcpy(f->tuzilma + f->t_uz, b, n);
    f->t_uz += n;
    /*
     * TODO(D2) — O'ZINGIZ YOZING: Tuzilma blokini 4 ga tekislang: t_uz 4 ga karrali bo'lguncha nol bayt qo'shing.
     *   - 4 ga karrali <=> past 2 bit nol: (t_uz & 3) == 0
     * Tekshirish: make test  (birlik testida 'D2' qatori)
     */
}

static void token(struct fdt *f, uint32_t t)
{
    uint8_t b[4];
    be32_yoz(b, t);
    baytlar(f, b, 4);
}

/* xossa nomining satrlar blokidagi siljishi: bor bo'lsa — eskisini qaytaradi, yo'q bo'lsa — qo'shadi */
static uint32_t satr_siljishi(struct fdt *f, const char *nom)
{
    /*
     * TODO(D3) — O'ZINGIZ YOZING: Xossa nomini satrlar blokidan qidiring: bor bo'lsa — siljishini, yo'q bo'lsa — oxiriga qo'shib, siljishini qaytaring.
     *   - satrlar ketma-ket, har biri '\0' bilan: i += strlen(f->satrlar + i) + 1
     *   - takrorlanmasin: 'compatible' 10 marta ishlatiladi, lekin blokda BIR marta turishi kerak
     * Tekshirish: make test  (birlik testida 'D3' qatori)
     */
    (void)nom;
    return f->s_uz;
}

static void tugun(struct fdt *f, const char *nom)
{
    token(f, FDT_BEGIN_NODE);
    baytlar(f, nom, (uint32_t)strlen(nom) + 1);
}

static void tugun_oxiri(struct fdt *f)
{
    token(f, FDT_END_NODE);
}

static void xossa(struct fdt *f, const char *nom, const void *qiymat, uint32_t uzunlik)
{
    uint8_t b[8];
    token(f, FDT_PROP);
    be32_yoz(b, uzunlik);
    be32_yoz(b + 4, satr_siljishi(f, nom));
    baytlar(f, b, 8);
    if (uzunlik)
        baytlar(f, qiymat, uzunlik);
}

static void xossa_bosh(struct fdt *f, const char *nom) { xossa(f, nom, NULL, 0); }
static void xossa_matn(struct fdt *f, const char *nom, const char *q) { xossa(f, nom, q, (uint32_t)strlen(q) + 1); }

/* bir nechta 32 bitli son ("<1 2 3>") */
static void xossa_sonlar(struct fdt *f, const char *nom, const uint32_t *q, uint32_t n)
{
    uint8_t b[64];
    for (uint32_t i = 0; i < n; i++)
        be32_yoz(b + 4 * i, q[i]);
    xossa(f, nom, b, 4 * n);
}

static void xossa_son(struct fdt *f, const char *nom, uint32_t q) { xossa_sonlar(f, nom, &q, 1); }

/* bir nechta satr ("a", "b") — ichida '\0' bilan ajratilgan. Uzunlikni qo'lda sanash xavfli (xato bo'lsa massivdan
   tashqarini o'qiymiz), shuning uchun makro sizeof bilan oladi: satr literalining hajmi oxirgi '\0' bilan birga */
#define XOSSA_MATNLAR(f, nom, literal) xossa(f, nom, literal, sizeof(literal))

/* daraxtni quradi (QEMU "virt" mashinasiga o'xshash) va to'liq DTB ni chiq ga yozadi. Hajmini qaytaradi */
static uint32_t daraxt_qur(struct mashina *m, uint8_t *chiq, uint32_t sigim)
{
    static struct fdt f;
    memset(&f, 0, sizeof(f));
    enum { CPU_INTC = 1, PLIC_PH = 2 };         /* phandle: boshqa tugunlarga "ko'rsatkich" raqami */

    tugun(&f, "");                              /* ildiz tugun nomi — bo'sh satr */
    xossa_son(&f, "#address-cells", 1);         /* manzil = 1 ta 32 bitli son */
    xossa_son(&f, "#size-cells", 1);            /* hajm = 1 ta 32 bitli son */
    xossa_matn(&f, "compatible", "riscv-virtio");
    xossa_matn(&f, "model", "virtual-kompyuter,rv32");

    tugun(&f, "chosen");
    xossa_matn(&f, "stdout-path", "/soc/serial@10000000");
    tugun_oxiri(&f);

    char nom[32];
    snprintf(nom, sizeof(nom), "memory@%x", RAM_BOSH);
    tugun(&f, nom);
    xossa_matn(&f, "device_type", "memory");
    uint32_t reg[2] = { RAM_BOSH, m->ram_hajm };
    xossa_sonlar(&f, "reg", reg, 2);
    tugun_oxiri(&f);

    tugun(&f, "cpus");
    xossa_son(&f, "#address-cells", 1);
    xossa_son(&f, "#size-cells", 0);
    xossa_son(&f, "timebase-frequency", 10000000);      /* mtime 1 soniyada 10 mln marta o'sadi (bizda: 10 mln buyruq) */
    tugun(&f, "cpu@0");
    xossa_matn(&f, "device_type", "cpu");
    xossa_son(&f, "reg", 0);
    xossa_matn(&f, "status", "okay");
    xossa_matn(&f, "compatible", "riscv");
    xossa_matn(&f, "riscv,isa", "rv32imac_zicsr_zifencei");               /* eski format: bitta satr */
    xossa_matn(&f, "riscv,isa-base", "rv32i");                             /* yangi format: asos ... */
    XOSSA_MATNLAR(&f, "riscv,isa-extensions", "i\0m\0a\0c\0zicsr\0zifencei\0zicntr");  /* ... va kengaytmalar ro'yxati */
    xossa_matn(&f, "mmu-type", "riscv,sv32");
    tugun(&f, "interrupt-controller");          /* protsessorning o'z uzilish "kirishlari" (mip bitlari) */
    xossa_son(&f, "#interrupt-cells", 1);
    xossa_bosh(&f, "interrupt-controller");
    xossa_matn(&f, "compatible", "riscv,cpu-intc");
    xossa_son(&f, "phandle", CPU_INTC);
    tugun_oxiri(&f);
    tugun_oxiri(&f);
    tugun_oxiri(&f);

    tugun(&f, "soc");
    xossa_son(&f, "#address-cells", 1);
    xossa_son(&f, "#size-cells", 1);
    xossa_matn(&f, "compatible", "simple-bus");
    xossa_bosh(&f, "ranges");

    tugun(&f, "clint@2000000");
    XOSSA_MATNLAR(&f, "compatible", "sifive,clint0\0riscv,clint0");
    uint32_t clint_reg[2] = { CLINT_MANZIL, CLINT_HAJM };
    xossa_sonlar(&f, "reg", clint_reg, 2);
    uint32_t clint_irq[4] = { CPU_INTC, IRQ_M_DASTURIY, CPU_INTC, IRQ_M_TAYMER };
    xossa_sonlar(&f, "interrupts-extended", clint_irq, 4);
    tugun_oxiri(&f);

    tugun(&f, "plic@c000000");
    XOSSA_MATNLAR(&f, "compatible", "sifive,plic-1.0.0\0riscv,plic0");
    uint32_t plic_reg[2] = { PLIC_MANZIL, PLIC_HAJM };
    xossa_sonlar(&f, "reg", plic_reg, 2);
    xossa_son(&f, "#address-cells", 0);
    xossa_son(&f, "#interrupt-cells", 1);
    xossa_bosh(&f, "interrupt-controller");
    uint32_t plic_irq[4] = { CPU_INTC, IRQ_M_TASHQI, CPU_INTC, IRQ_S_TASHQI };     /* kontekst 0 — M, 1 — S */
    xossa_sonlar(&f, "interrupts-extended", plic_irq, 4);
    xossa_son(&f, "riscv,ndev", PLIC_MANBALAR - 1);
    xossa_son(&f, "phandle", PLIC_PH);
    tugun_oxiri(&f);

    tugun(&f, "serial@10000000");
    xossa_matn(&f, "compatible", "ns16550a");
    uint32_t uart_reg[2] = { UART_MANZIL, 0x100 };
    xossa_sonlar(&f, "reg", uart_reg, 2);
    xossa_son(&f, "clock-frequency", 3686400);
    xossa_son(&f, "interrupt-parent", PLIC_PH);
    xossa_son(&f, "interrupts", IRQ_UART);
    tugun_oxiri(&f);

    tugun_oxiri(&f);                            /* soc */
    tugun_oxiri(&f);                            /* ildiz */
    token(&f, FDT_END);

    /* yig'ish: sarlavha | zaxira (16 B nol) | tuzilma | satrlar */
    uint32_t off_zaxira = 40, off_tuzilma = off_zaxira + 16, off_satr = off_tuzilma + f.t_uz;
    uint32_t jami = off_satr + f.s_uz;
    if (jami > sigim)
        return 0;
    memset(chiq, 0, jami);
    uint32_t sarlavha[10] = { FDT_SEHR, jami, off_tuzilma, off_satr, off_zaxira, 17, 16, 0, f.s_uz, f.t_uz };
    for (int i = 0; i < 10; i++)
        be32_yoz(chiq + 4 * i, sarlavha[i]);
    memcpy(chiq + off_tuzilma, f.tuzilma, f.t_uz);
    memcpy(chiq + off_satr, f.satrlar, f.s_uz);
    return jami;
}

/* DTB ni RAM ning oxirgi 64 KB iga joylaydi (yadro uni ko'rib, kerakli joyga ko'chiradi) */
uint32_t dtb_joylash(struct mashina *m)
{
    uint32_t joy = 64 * 1024;
    uint32_t manzil = RAM_BOSH + m->ram_hajm - joy;
    uint32_t hajm = daraxt_qur(m, m->ram + (manzil - RAM_BOSH), joy);
    if (!hajm) {
        fprintf(stderr, "emulyator: DTB sig'madi\n");
        return 0;
    }
    if (m->dtb_fayl) {                          /* -D FAYL: tekshirish uchun diskka ham yozamiz (dtc bilan o'qish mumkin) */
        FILE *f = fopen(m->dtb_fayl, "wb");
        if (f) {
            fwrite(m->ram + (manzil - RAM_BOSH), 1, hajm, f);
            fclose(f);
        }
    }
    return manzil;
}
