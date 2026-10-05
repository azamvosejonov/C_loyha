/*
 * qurilmalar.h — "kompyuter"ning tashqi qurilmalari va xotira xaritasi.
 *
 * XOTIRA XARITASI (memory map)
 *   Protsessor qurilmalar bilan ham XOTIRA kabi gaplashadi: maxsus manzilga yozsa — qurilmaga buyruq
 *   bo'ladi (MMIO — memory-mapped I/O). Qaysi manzil nimaga tegishli — shu jadval. Manzillar ataylab
 *   QEMU ning "virt" mashinasi bilan BIR XIL: bu yerda o'rgangan kod haqiqiy RISC-V emulyatorlarida ham ishlaydi.
 *
 *     0x0010_0000  quvvat (sifive_test)  — 0x5555 yozilsa kompyuter o'chadi
 *     0x0200_0000  CLINT                 — taymer (mtime, mtimecmp) va dasturiy uzilish (msip)
 *     0x0C00_0000  PLIC                  — tashqi uzilishlar kontrolleri: qaysi qurilma "chaqirdi"
 *     0x1000_0000  UART (16550)          — konsol; uzilish raqami (PLIC manbasi) 10
 *     0x1000_1000  disk                  — oddiy blok qurilma (bizning dizayn); uzilish raqami 1
 *     0x8000_0000  RAM                   — operativ xotira (sukut 64 MB, -m bilan). Firmware shu yerdan boshlanadi.
 */
#ifndef QURILMALAR_H
#define QURILMALAR_H

#include <stdio.h>

#include "turlar.h"

#define QUVVAT_MANZIL 0x00100000u
#define CLINT_MANZIL 0x02000000u
#define CLINT_HAJM 0x10000u
#define PLIC_MANZIL 0x0C000000u
#define PLIC_HAJM 0x400000u
#define UART_MANZIL 0x10000000u
#define UART_HAJM 8u
#define DISK_MANZIL 0x10001000u
#define DISK_HAJM 0x20u
#define RAM_BOSH 0x80000000u

#define IRQ_DISK 1                              /* PLIC manba raqamlari */
#define IRQ_UART 10

/* ---------------- CLINT (core-local interruptor) ----------------
 * Har protsessor yadrosining shaxsiy taymeri va "qo'ng'irog'i":
 *   +0x0000 msip      (32 bit) — 0-bit: M dasturiy uzilish (boshqa yadroni "uyg'otish" uchun)
 *   +0x4000 mtimecmp  (64 bit) — mtime >= mtimecmp bo'lsa M taymer uzilishi (MTIP)
 *   +0xBFF8 mtime     (64 bit) — doim o'sib boradigan vaqt hisoblagichi (bizda = bajarilgan buyruqlar soni)
 */
struct clint {
    uint32_t msip;
    uint64_t mtimecmp;
};

/* ---------------- PLIC (platform-level interrupt controller) ----------------
 * Ko'p qurilma bitta protsessorga uzilish yuboradi. PLIC ular orasida "dispetcher":
 *   - har manbaning USTUVORLIGI (0 — o'chiq, 1..7);
 *   - har KONTEKST (kim qabul qiladi: 0 — M rejim, 1 — S rejim) uchun YOQILGAN manbalar va CHEGARA;
 *   - CLAIM: ishlovchi "kim chaqirdi?" deb o'qiydi — eng ustuvor manba raqami qaytadi va u "ishlanmoqda"
 *     deb belgilanadi; ish tugagach shu raqam COMPLETE ga yoziladi.
 * Registrlar (QEMU virt bilan bir xil joylashuv):
 *   +0x000000 + 4*manba          ustuvorlik
 *   +0x001000                    kutayotgan manbalar (bitlar, faqat o'qish)
 *   +0x002000 + 0x80*kontekst    yoqilgan manbalar (bitlar)
 *   +0x200000 + 0x1000*kontekst  chegara (threshold)
 *   +0x200004 + 0x1000*kontekst  claim (o'qish) / complete (yozish)
 */
#define PLIC_MANBALAR 32
#define PLIC_KONTEKSTLAR 2

struct plic {
    uint32_t ustuvorlik[PLIC_MANBALAR];
    uint32_t kutmoqda;                          /* manbalar bitlari: qurilma signal bergan, hali claim qilinmagan */
    uint32_t ishlanmoqda;                       /* claim qilingan, complete kutilyapti */
    uint32_t yoqilgan[PLIC_KONTEKSTLAR];
    uint32_t chegara[PLIC_KONTEKSTLAR];
};

/* qurilma signal darajasini bildiradi (1 — uzilish kerak). Har qadamda chaqiriladi */
void plic_signal(struct plic *p, int manba, int daraja);
/* kontekstga uzilish kutayaptimi (MEIP / SEIP manbai) */
int plic_kutyapti(const struct plic *p, int kontekst);
uint32_t plic_oqi(struct plic *p, uint32_t siljish);
void plic_yoz(struct plic *p, uint32_t siljish, uint32_t qiymat);

/* ---------------- UART (16550 ning kichik, lekin haqiqiy drayverlar bilan mos qismi) ----------------
 * Registrlar (UART_MANZIL dan siljish):
 *   0  RBR (o'qish) — kelgan bayt;  THR (yozish) — yuboriladigan bayt
 *   1  IER — uzilishlarni yoqish: 0-bit "bayt keldi", 1-bit "yuborishga tayyor"
 *   2  IIR (o'qish) — qaysi uzilish: 0x04 — bayt keldi, 0x02 — yuborishga tayyor, 0x01 — yo'q
 *   5  LSR — holat: 0-bit "o'qishga bayt bor" (DR), 5-bit "yuborishga tayyor" (THRE), 6-bit (TEMT)
 *   3, 4, 7 — boshqa registrlar: yoziladi/o'qiladi, ta'siri yo'q
 */
#define UART_LSR_DR 0x01u
#define UART_LSR_THRE 0x20u
#define UART_LSR_TEMT 0x40u
#define UART_IER_RX 0x01u
#define UART_IER_TX 0x02u

struct uart {
    unsigned char kirish[4096];                 /* klaviaturadan (stdin dan) kelgan, hali o'qilmagan baytlar */
    size_t bosh, oxir;                          /* bufer: [bosh, oxir) */
    int stdin_tugadi;
    /* "odamdek yozish" (stdin fayl/quvur bo'lsa): butun kirish oldindan o'qiladi, lekin dasturga QATORMA-QATOR
       beriladi — faqat dastur bir muddat (sukunat) hech narsa chiqarmay, oldingi qatorni o'qib bo'lgach.
       Aks holda kirish darhol kelib, terminal uni so'rovdan (prompt) oldin aks ettirib yuborardi. */
    unsigned char *zaxira;
    size_t zaxira_uz, zaxira_bosh;
    const uint64_t *vaqt;                       /* protsessorning instret i (deterministik vaqt) */
    uint64_t oxirgi_chiqish;
    int tx_uzilish;                             /* "yuborishga tayyor" uzilishi kutyapti (IIR o'qilgach o'chadi) */
    int registr[8];
};

uint32_t uart_oqi(struct uart *u, uint32_t siljish);
void uart_yoz(struct uart *u, uint32_t siljish, uint32_t qiymat);
int uart_uzilish(struct uart *u);               /* PLIC ga signal darajasi */
void uart_tayyorla(struct uart *u);
void uart_tugat(void);
int uart_interaktiv(void);

/* ---------------- DISK (oddiy blok qurilma) ----------------
 * O'RGANISH UCHUN eng sodda, lekin haqiqiy g'oyadagi qurilma: drayver registrlarga yozadi,
 * qurilma xotiraga o'zi ko'chiradi (DMA). Ish tugagach (agar yoqilgan bo'lsa) UZILISH yuboradi.
 * Registrlar (DISK_MANZIL dan siljish, hammasi 32 bit):
 *   0x00 SEKTOR   — qaysi sektor (512 bayt)
 *   0x04 MANZIL   — RAM dagi FIZIK manzil
 *   0x08 BUYRUQ   — 1: o'qi (disk -> RAM), 2: yoz (RAM -> disk). Ko'chirish darhol tugaydi
 *   0x0C HOLAT    — 0 — OK, 1 — xato. O'QILGANDA uzilish signali o'chadi ("qabul qildim")
 *   0x10 SONI     — sektorlar soni (faqat o'qish)
 *   0x14 UZILISH  — 1 yozilsa: har buyruq tugaganda PLIC ga uzilish (IRQ 1)
 */
#define DISK_SEKTOR 512u
#define DISK_R_SEKTOR 0x00u
#define DISK_R_MANZIL 0x04u
#define DISK_R_BUYRUQ 0x08u
#define DISK_R_HOLAT 0x0Cu
#define DISK_R_SONI 0x10u
#define DISK_R_UZILISH 0x14u
#define DISK_BUYRUQ_OQI 1u
#define DISK_BUYRUQ_YOZ 2u

struct disk {
    FILE *fayl;                                 /* NULL — disk ulanmagan */
    uint32_t sektorlar;
    uint32_t sektor, manzil, holat;
    int uzilish_yoqilgan, uzilish;
    uint64_t oqildi, yozildi;                   /* statistika */
};

struct mashina;
int disk_ulash(struct disk *d, const char *yol);
uint32_t disk_oqi(struct disk *d, uint32_t siljish);
void disk_yoz(struct mashina *m, uint32_t siljish, uint32_t qiymat);

#endif
