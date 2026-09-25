/* =============================================================================
 *  arch/tsc.c - TSC (Time Stamp Counter) kalibrlash va kutish
 * =============================================================================
 *
 *  TSC - CPU ichidagi hisoblagich, har bir taktda oshadi. `rdtsc` bilan
 *  o'qiladi - juda tez va aniq. Lekin uning CHASTOTASI CPU'ga qarab farq
 *  qiladi (1 GHz dan 5 GHz gacha). Bilish uchun ma'lum vaqt oralig'ini boshqa
 *  soat bilan o'lchab solishtiramiz: PIT ning 2-kanali (1 193 182 Hz, doim
 *  bir xil). "Kalibrlash" shu.
 *
 *  PIT 2-KANALI: aslida karnay (speaker) uchun. 0x61 portining 0-biti kanalni
 *  yoqadi (gate), 5-biti - sanash tugaganini bildiradi. Uni busy-wait uchun
 *  ishlatish mumkin - uzilishlar kerak emas.
 * ============================================================================= */
#include "arch/tsc.h"

#include "arch/cpu.h"
#include "arch/io.h"
#include "lib/kprintf.h"

uint64_t tsc_khz;

void pit_wait_ms(uint32_t ms)
{
    while (ms > 0) {
        uint32_t chunk = ms > 50 ? 50 : ms;             /* 16 bitli sanagichga sig'ishi uchun */
        uint16_t count = (uint16_t)(1193182UL * chunk / 1000);
        uint8_t v = inb(0x61);
        outb(0x61, (v & ~0x02) | 0x01);                  /* gate=1, karnay o'chiq */
        outb(0x43, 0xB0);                                /* kanal 2, lo/hi bayt, 0-rejim */
        outb(0x42, count & 0xFF);
        outb(0x42, count >> 8);
        v = inb(0x61) & ~0x01;
        outb(0x61, v);                                   /* gate ni qayta ishga tushirish */
        outb(0x61, v | 0x01);
        while (!(inb(0x61) & 0x20))                      /* OUT2 = 1 bo'lguncha */
            cpu_pause();
        ms -= chunk;
    }
}

void tsc_calibrate(void)
{
    /* 3 marta o'lchab, eng kichigini olamiz (virtualizatsiyada tasodifiy
     * kechikishlar bo'ladi - minimum eng ishonchlisi). */
    uint64_t best = UINT64_MAX;
    for (int i = 0; i < 3; i++) {
        uint64_t t0 = rdtsc();
        pit_wait_ms(10);
        uint64_t d = rdtsc() - t0;
        if (d < best)
            best = d;
    }
    tsc_khz = best / 10;
    kprintf("[tsc]  TSC chastotasi: %lu.%03lu MHz%s\n", tsc_khz / 1000, tsc_khz % 1000,
            cpu_features.tsc_invariant ? " (invariant)" : "");
}

void udelay(uint64_t us)
{
    if (!tsc_khz) {                     /* hali kalibrlanmagan: taxminiy */
        for (uint64_t i = 0; i < us * 100; i++)
            io_wait();
        return;
    }
    uint64_t end = rdtsc() + us * tsc_khz / 1000;
    while (rdtsc() < end)
        cpu_pause();
}

void mdelay(uint64_t ms)
{
    udelay(ms * 1000);
}
