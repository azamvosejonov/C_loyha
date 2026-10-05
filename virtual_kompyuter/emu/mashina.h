/*
 * mashina.h — butun "kompyuter": protsessor + RAM + qurilmalar. Va ular orasidagi SHINA (bus).
 *
 * SHINA NIMA?
 *   Protsessor "manzil X dan 4 bayt o'qi" deydi. Kim javob beradi — RAM mi, UART mi, disk mi?
 *   Buni shina hal qiladi: manzil qaysi oraliqda ekaniga qarab so'rovni kerakli qurilmaga yuboradi
 *   (xotira xaritasi qurilmalar.h da). Hech kim javob bermasa — "access fault" (kirish xatosi).
 *
 * IKKI XIL MANZIL
 *   virtual — dastur ko'radigan manzil (pc, lw/sw dagi manzil);
 *   fizik   — shinaga boradigan haqiqiy manzil.
 *   Ular orasidagi tarjimani MMU qiladi (mmu.c). satp.MODE = 0 bo'lsa tarjima yo'q: virtual = fizik.
 */
#ifndef MASHINA_H
#define MASHINA_H

#include "cpu.h"
#include "qurilmalar.h"

struct mashina {
    struct cpu cpu;
    uint8_t *ram;
    uint32_t ram_hajm;
    struct uart uart;
    struct disk disk;
    struct clint clint;
    struct plic plic;

    int toxtadi;                                /* quvvat qurilmasi "o'chir" dedi */
    int chiqish_kodi;
    int trace;                                  /* 1 — har bajarilgan buyruq stderr ga chiqadi */
    const char *dtb_fayl;                       /* -D: yaratilgan DTB ni shu faylga ham yozish */
};

/* CLINT registrlari. clint.c */
uint32_t clint_oqi(struct mashina *m, uint32_t siljish);
void clint_yoz(struct mashina *m, uint32_t siljish, uint32_t qiymat);
/* qurilmalar uzilish signallarini PLIC ga yetkazish (har qadamda). clint.c */
void qurilmalar_yangila(struct mashina *m);

/* FIZIK manzil bo'yicha o'qish/yozish (hajm: 1, 2 yoki 4 bayt). 0 — OK, -1 — bu manzilda hech narsa yo'q */
int shina_oqi(struct mashina *m, uint32_t fiz, int hajm, uint32_t *qiymat);
int shina_yoz(struct mashina *m, uint32_t fiz, int hajm, uint32_t qiymat);

/* VIRTUAL manzil bo'yicha (MMU orqali) — protsessor buyruqlari shularni ishlatadi.
   0 — OK, aks holda istisno sababi (enum sabab); *tval ga muammoli manzil */
int xotira_oqi(struct mashina *m, uint32_t va, int hajm, uint32_t *qiymat);
int xotira_yoz(struct mashina *m, uint32_t va, int hajm, uint32_t qiymat);

/* xom tasvirni (masalan Linux Image) RAM ga yuklash manzili */
#define XOM_MANZIL 0x80400000u
int xom_yukla(struct mashina *m, const char *yol, uint32_t manzil);

/* qurilmalar daraxtini (Device Tree Blob) yaratib, RAM oxiriga joylaydi; manzilini qaytaradi. dtb.c */
uint32_t dtb_joylash(struct mashina *m);

/* ELF fayldan dasturni RAM ga yuklaydi. 0 — OK; *kirish_nuqtasi ga boshlanish manzili. elf.c */
int elf_yukla(struct mashina *m, const char *yol, uint32_t *kirish_nuqtasi);

/* buyruqni o'qiladigan matnga aylantiradi ("addi a0, a0, 1"). 16 bitli (C) buyruqlar ham. disasm.c */
void disasm(uint32_t buyruq, uint32_t pc, char *bufer, size_t hajm);
extern const char *const registr_nomi[32];

#endif
