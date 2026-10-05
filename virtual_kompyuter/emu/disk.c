/*
 * disk.c — oddiy blok qurilma: kompyuterimizdagi fayl (disk tasviri) "disk" vazifasini bajaradi.
 *
 * DMA (direct memory access) g'oyasi: drayver baytlarni bittalab ko'chirmaydi. U qurilmaga faqat
 * "SEKTOR raqami", "RAM dagi MANZIL" va "BUYRUQ" ni aytadi — 512 baytni qurilmaning o'zi RAM ga yozadi
 * (yoki RAM dan oladi). Protsessor esa shu vaqtda boshqa ish qilishi mumkin. Haqiqiy disklar shunday.
 * Soddalashtirish: bizda ko'chirish DARHOL (buyruq yozilgan zahoti) tugaydi. Uzilish (IRQ 1) ixtiyoriy:
 * yoqilgan bo'lsa, haqiqiy drayverlar kabi "ish tugadi" xabarini uzilish orqali olish mumkin.
 *
 * Diqqat: MANZIL — FIZIK manzil. Qurilma MMU ni bilmaydi! Yadro virtual manzilni fizikka o'zi aylantirib
 * berishi kerak (haqiqiy drayverlardagi kabi).
 */
#include <stdio.h>

#include "mashina.h"

int disk_ulash(struct disk *d, const char *yol)
{
    d->fayl = fopen(yol, "r+b");
    if (!d->fayl)
        return -1;
    fseek(d->fayl, 0, SEEK_END);
    long hajm = ftell(d->fayl);
    d->sektorlar = hajm > 0 ? (uint32_t)(hajm / DISK_SEKTOR) : 0;
    return 0;
}

uint32_t disk_oqi(struct disk *d, uint32_t siljish)
{
    switch (siljish) {
    case DISK_R_SEKTOR: return d->sektor;
    case DISK_R_MANZIL: return d->manzil;
    case DISK_R_HOLAT:
        d->uzilish = 0;                         /* holat o'qildi — "uzilishni qabul qildim" */
        return d->holat;
    case DISK_R_UZILISH: return (uint32_t)d->uzilish_yoqilgan;
    case DISK_R_SONI: return d->fayl ? d->sektorlar : 0;
    default: return 0;
    }
}

/* buyruqni bajarish: 512 baytni fayl <-> RAM o'rtasida ko'chirish */
static void bajar(struct mashina *m, uint32_t buyruq)
{
    struct disk *d = &m->disk;
    d->holat = 1;                               /* sukut: xato. Hammasi to'g'ri bo'lsagina 0 */
    if (!d->fayl || d->sektor >= d->sektorlar)
        return;
    if (d->manzil < RAM_BOSH || d->manzil - RAM_BOSH > m->ram_hajm - DISK_SEKTOR)
        return;                                 /* bufer to'liq RAM ichida bo'lishi kerak */
    uint8_t *ram = m->ram + (d->manzil - RAM_BOSH);
    if (fseek(d->fayl, (long)d->sektor * DISK_SEKTOR, SEEK_SET) != 0)
        return;
    if (buyruq == DISK_BUYRUQ_OQI) {
        if (fread(ram, 1, DISK_SEKTOR, d->fayl) != DISK_SEKTOR)
            return;
        d->oqildi++;
    } else if (buyruq == DISK_BUYRUQ_YOZ) {
        if (fwrite(ram, 1, DISK_SEKTOR, d->fayl) != DISK_SEKTOR)
            return;
        fflush(d->fayl);
        d->yozildi++;
    } else {
        return;
    }
    d->holat = 0;
}

void disk_yoz(struct mashina *m, uint32_t siljish, uint32_t qiymat)
{
    struct disk *d = &m->disk;
    switch (siljish) {
    case DISK_R_SEKTOR: d->sektor = qiymat; break;
    case DISK_R_MANZIL: d->manzil = qiymat; break;
    case DISK_R_BUYRUQ:
        bajar(m, qiymat);
        if (d->uzilish_yoqilgan)
            d->uzilish = 1;                     /* ish tugadi: PLIC ga signal (keyingi qadamda) */
        break;
    case DISK_R_UZILISH: d->uzilish_yoqilgan = (int)(qiymat & 1); break;
    default: break;
    }
}
