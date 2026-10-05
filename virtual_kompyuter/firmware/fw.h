/*
 * fw.h — firmware (M rejim dasturi) uchun umumiy ta'riflar.
 *
 * FIRMWARE NIMA QILADI? (OpenSBI ning soddalashtirilgan o'xshashi)
 *   1) Kompyuter yoqilganda birinchi ishlaydi (M rejim) va apparatni sozlaydi:
 *      trap delegatsiyasi, hisoblagichlarga ruxsat, taymer.
 *   2) Yadroni (Linux) S rejimda ishga tushiradi: mepc = yadro manzili, MPP = S, mret.
 *   3) Keyin "orqa fonda" yashaydi va yadroga SBI xizmatlarini ko'rsatadi: yadro `ecall` qiladi
 *      (a7 = kengaytma raqami, a6 = funksiya raqami, a0..a5 = argumentlar), firmware bajaradi,
 *      natijani a0 (xato kodi) va a1 (qiymat) da qaytaradi. Bu — yadro va apparat orasidagi "tizim chaqiruvi".
 *   4) M taymer uzilishini qabul qilib, yadroga S taymer uzilishi sifatida "uzatadi".
 */
#ifndef FW_H
#define FW_H

#include <stddef.h>
#include <stdint.h>

#define YADRO_MANZIL 0x80400000u                /* Linux Image shu yerga yuklangan (emulyatorning -k) */
#define UART 0x10000000u
#define CLINT_MTIMECMP 0x02004000u
#define QUVVAT 0x00100000u

/* trap.S saqlagan kadr: x[0..31] registrlar (x[2] = yadroning sp si) va mepc */
struct kadr {
    uint32_t x[32];
    uint32_t mepc;
};

enum { RA = 1, SP = 2, A0 = 10, A1, A2, A3, A4, A5, A6, A7 };

/* CSR bilan ishlash: inline assembly. "r" — istalgan registr; volatile — kompilyator olib tashlamasin */
#define csr_oqi(nom) ({ uint32_t _q; __asm__ volatile("csrr %0, " #nom : "=r"(_q)); _q; })
#define csr_yoz(nom, q) __asm__ volatile("csrw " #nom ", %0" ::"r"((uint32_t)(q)))
#define csr_bit_yoq(nom, q) __asm__ volatile("csrs " #nom ", %0" ::"r"((uint32_t)(q)))
#define csr_bit_och(nom, q) __asm__ volatile("csrc " #nom ", %0" ::"r"((uint32_t)(q)))

/* MMIO: kompilyator bu yozish/o'qishni olib tashlamasligi va tartibini o'zgartirmasligi uchun volatile */
static inline void mmio_yoz32(uint32_t manzil, uint32_t q) { *(volatile uint32_t *)manzil = q; }
static inline uint32_t mmio_oqi32(uint32_t manzil) { return *(volatile uint32_t *)manzil; }
static inline void mmio_yoz8(uint32_t manzil, uint8_t q) { *(volatile uint8_t *)manzil = q; }
static inline uint8_t mmio_oqi8(uint32_t manzil) { return *(volatile uint8_t *)manzil; }

/* konsol.c */
void konsol_belgi(int c);
int konsol_oqi(void);                           /* -1 — bayt yo'q */
void konsol_matn(const char *s);
void konsol_hex(uint32_t q);
void konsol_son(uint32_t q);

/* sbi.c */
void sbi_ecall(struct kadr *k);
void sbi_taymer_uzilishi(void);
void quvvatni_och(uint32_t kod) __attribute__((noreturn));

#endif
