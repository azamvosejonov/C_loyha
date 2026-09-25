/* =============================================================================
 *  arch/io.h - x86 "port" kiritish/chiqarish instruksiyalari
 * =============================================================================
 *
 *  NIMA BU:
 *    x86'da qurilmalar bilan gaplashishning ikki yo'li bor:
 *      1) Xotira orqali (MMIO) - masalan, VGA ekran 0xB8000 manzilida
 *      2) Alohida "port" manzillari orqali (0..0xFFFF) - IN va OUT instruksiyalari
 *    Serial port (0x3F8), PIC (0x20), taymer (0x40), klaviatura (0x60) - hammasi
 *    port orqali ishlaydi.
 *
 *  NEGA "static inline":
 *    Har biri bitta instruksiya. Funksiya chaqiruvi (call/ret) dan qochish uchun
 *    kompilyator ularni chaqirilgan joyga to'g'ridan-to'g'ri qo'yadi.
 *
 *  INLINE ASM SINTAKSISI (GCC):
 *    __asm__ volatile ("instruksiya" : chiqishlar : kirishlar : buzilganlar);
 *      "a"(x)  - x ni AL/AX/EAX registriga qo'y
 *      "Nd"(p) - p ni 8-bitli konstanta (N) yoki DX registri (d) sifatida ber
 *      "=a"(r) - natijani AL/AX/EAX dan r ga ol
 *      volatile - kompilyator bu instruksiyani o'chirmasin/ko'chirmasin
 *      "memory" - kompilyator xotira haqidagi taxminlarini unutsin (to'siq)
 * ============================================================================= */
#pragma once

#include <stdint.h>

/* Portga 1 bayt yozish. */
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

/* Portdan 1 bayt o'qish. */
static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port) : "memory");
    return value;
}

/* Portga 2 bayt yozish. */
static inline void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

/* Portdan 2 bayt o'qish. */
static inline uint16_t inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port) : "memory");
    return value;
}

/* Portga 4 bayt yozish (PCI konfiguratsiya maydoni uchun). */
static inline void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

/* Portdan 4 bayt o'qish. */
static inline uint32_t inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port) : "memory");
    return value;
}

/* Portdan count ta 2 baytli so'zni ketma-ket xotiraga o'qish ("rep insw").
 * ATA disk sektori (256 so'z) bitta instruksiya bilan o'qiladi: CPU o'zi
 * RDI ni oshirib, RCX ni kamaytirib boradi. */
static inline void insw(uint16_t port, void *buf, uint32_t count)
{
    __asm__ volatile("rep insw" : "+D"(buf), "+c"(count) : "d"(port) : "memory");
}

static inline void outsw(uint16_t port, const void *buf, uint32_t count)
{
    __asm__ volatile("rep outsw" : "+S"(buf), "+c"(count) : "d"(port) : "memory");
}

/* Juda qisqa kutish. Eski qurilmalar (masalan, PIC) buyruqlar orasida biroz vaqt
 * talab qiladi. 0x80 porti - BIOS POST kodlari uchun, unga yozish zararsiz va
 * taxminan 1 mikrosekund oladi. */
static inline void io_wait(void)
{
    outb(0x80, 0);
}
