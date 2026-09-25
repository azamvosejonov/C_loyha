/* =============================================================================
 *  drivers/rtc.c - CMOS REAL VAQT SOATI
 * =============================================================================
 *
 *  Kompyuter o'chiq bo'lsa ham vaqtni hisoblaydigan kichik soat (anakartdagi
 *  batareyka tufayli). 0x70 portiga registr raqami, 0x71 dan qiymat:
 *     0x00 soniya  0x02 daqiqa  0x04 soat  0x07 kun  0x08 oy  0x09 yil
 *     0x0A holat A (7-bit: "hozir yangilanmoqda" - o'qimang!)
 *     0x0B holat B (2-bit: ikkilik rejim, aks holda BCD; 1-bit: 24 soatlik)
 *
 *  BCD (Binary Coded Decimal): 0x59 = "59" (har 4 bit bitta o'nlik raqam).
 *  Ko'p BIOS lar BCD ishlatadi - shuning uchun aylantirish kerak.
 *
 *  Boot vaqtida bir marta o'qiymiz, keyin taymer tiklari bilan hisoblaymiz.
 *  RTC odatda MAHALLIY vaqtda (Windows an'anasi) yoki UTC da (Linux) bo'ladi -
 *  biz UTC deb hisoblaymiz.
 * ============================================================================= */
#include "drivers/rtc.h"

#include <stdbool.h>

#include "arch/io.h"
#include "drivers/pit.h"
#include "lib/kprintf.h"

static uint64_t boot_epoch;             /* boot paytidagi Unix vaqti */
static uint64_t boot_ticks;

static uint8_t cmos_read(uint8_t reg)
{
    outb(0x70, reg | 0x80);             /* 7-bit: NMI ni o'chirib qo'yish (an'ana) */
    return inb(0x71);
}

static bool updating(void)
{
    return cmos_read(0x0A) & 0x80;
}

static uint8_t bcd(uint8_t v)
{
    return (uint8_t)((v & 0x0F) + (v >> 4) * 10);
}

/* Sana -> Unix vaqti (kunlarni sanash, kabisa yillar bilan). */
static uint64_t to_epoch(int year, int mon, int day, int h, int m, int s)
{
    static const int mdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    uint64_t days = 0;
    for (int y = 1970; y < year; y++)
        days += (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 366 : 365;
    for (int i = 0; i < mon - 1; i++) {
        days += (uint64_t)mdays[i];
        if (i == 1 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)))
            days++;                     /* kabisa yilida fevral 29 kun */
    }
    days += (uint64_t)(day - 1);
    return days * 86400 + (uint64_t)h * 3600 + (uint64_t)m * 60 + (uint64_t)s;
}

void rtc_init(void)
{
    uint8_t s, m, h, d, mo, y, s2, m2;
    /* Ikki marta ketma-ket bir xil qiymat olguncha o'qiymiz - yangilanish
     * o'rtasida o'qib qolmaslik uchun (masalan 12:59:59 -> 13:00:00). */
    do {
        while (updating())
            ;
        s = cmos_read(0x00), m = cmos_read(0x02), h = cmos_read(0x04);
        d = cmos_read(0x07), mo = cmos_read(0x08), y = cmos_read(0x09);
        while (updating())
            ;
        s2 = cmos_read(0x00), m2 = cmos_read(0x02);
    } while (s != s2 || m != m2);

    uint8_t regb = cmos_read(0x0B);
    bool pm = h & 0x80;
    h &= 0x7F;
    if (!(regb & 0x04)) {               /* BCD rejimi */
        s = bcd(s), m = bcd(m), h = bcd(h), d = bcd(d), mo = bcd(mo), y = bcd(y);
    }
    if (!(regb & 0x02) && pm)           /* 12 soatlik rejim */
        h = (uint8_t)((h % 12) + 12);
    int year = 2000 + y;                /* 2000-2099 */
    boot_epoch = to_epoch(year, mo, d, h, m, s);
    boot_ticks = timer_ticks();
    kprintf("[rtc]  Sana: %04d-%02u-%02u %02u:%02u:%02u (UTC)\n", year, mo, d, h, m, s);
}

uint64_t time_now(void)
{
    return boot_epoch + (timer_ticks() - boot_ticks) / TIMER_HZ;
}
