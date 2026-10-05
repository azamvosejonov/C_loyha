/*
 * clint.c — CLINT (taymer va dasturiy uzilish) va qurilma signallarini PLIC ga yetkazish.
 *
 * VAQT NIMA?
 *   mtime — doim o'sib boradigan 64 bitli hisoblagich. Haqiqiy kompyuterda u kvars generatoridan
 *   (masalan 10 MHz) o'sadi. Bizda mtime = bajarilgan buyruqlar soni: emulyator DETERMINISTIK —
 *   taymer uzilishi har ishga tushirishda aynan bir xil buyruqda keladi, testlar natijasi o'zgarmaydi.
 *
 * RV32 da 64 bitli registrni o'qish muammosi:
 *   protsessor 32 bitli, mtime esa 64 bitli — ikki marta o'qish kerak (past va yuqori yarmi). Ikki
 *   o'qish orasida past yarmi 0xFFFFFFFF dan 0 ga o'tsa, natija noto'g'ri bo'ladi! Drayverlar shuning
 *   uchun "yuqori - past - yuqori" usulini qo'llaydi: yuqori yarmi o'zgargan bo'lsa — qaytadan o'qiydi.
 */
#include "mashina.h"

#define CLINT_MSIP 0x0000u
#define CLINT_MTIMECMP 0x4000u
#define CLINT_MTIME 0xBFF8u

uint32_t clint_oqi(struct mashina *m, uint32_t s)
{
    struct clint *k = &m->clint;
    switch (s) {
    case CLINT_MSIP: return k->msip & 1u;
    case CLINT_MTIMECMP: return (uint32_t)k->mtimecmp;
    case CLINT_MTIMECMP + 4: return (uint32_t)(k->mtimecmp >> 32);
    case CLINT_MTIME: return (uint32_t)m->cpu.instret;
    case CLINT_MTIME + 4: return (uint32_t)(m->cpu.instret >> 32);
    default: return 0;
    }
}

void clint_yoz(struct mashina *m, uint32_t s, uint32_t q)
{
    struct clint *k = &m->clint;
    switch (s) {
    case CLINT_MSIP: k->msip = q & 1u; break;
    case CLINT_MTIMECMP: k->mtimecmp = (k->mtimecmp & 0xFFFFFFFF00000000ull) | q; break;
    case CLINT_MTIMECMP + 4: k->mtimecmp = (k->mtimecmp & 0xFFFFFFFFull) | ((uint64_t)q << 32); break;
    default: break;                             /* mtime ni yozish e'tiborsiz: vaqtni orqaga qaytarib bo'lmaydi */
    }
}

/* Qurilmalar uzilish "simi"ning holatini PLIC ga beramiz. Haqiqiy apparatda bu doimiy elektr signal;
   bizda har buyruqdan oldin bir marta tekshiriladi. */
void qurilmalar_yangila(struct mashina *m)
{
    plic_signal(&m->plic, IRQ_UART, uart_uzilish(&m->uart));
    plic_signal(&m->plic, IRQ_DISK, m->disk.uzilish);
}
