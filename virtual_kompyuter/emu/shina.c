/*
 * shina.c — manzil bo'yicha so'rovni kerakli joyga (RAM yoki qurilma) yo'naltirish.
 *
 * Ikki daraja:
 *   1) shina_oqi / shina_yoz  — FIZIK manzil bilan. Xotira xaritasiga qarab RAM yoki qurilmani tanlaydi.
 *   2) xotira_oqi / xotira_yoz — VIRTUAL manzil bilan: avval tekislikni tekshiradi, keyin MMU orqali
 *      fizik manzilga tarjima qiladi, keyin 1-darajani chaqiradi. Protsessor buyruqlari (lw, sw ...)
 *      shu darajadan foydalanadi.
 *
 * BAYTLAR TARTIBI (endianness)
 *   RISC-V — little-endian: ko'p baytli sonning PAST bayti kichik manzilda turadi.
 *   0x12345678 soni RAM da: [78] [56] [34] [12]. Shuning uchun 4 baytni o'qiganda:
 *       qiymat = b[0] | b[1] << 8 | b[2] << 16 | b[3] << 24.
 *   Bizning kompyuter (x86) ham little-endian bo'lsa-da, memcpy bilan "tayanib" yozmaymiz —
 *   aniq siljitish bilan yozilgan kod HAR QANDAY kompyuterda to'g'ri ishlaydi.
 */
#include "mashina.h"

/* fizik manzil RAM ichidami? hajm baytning HAMMASI sig'ishi kerak (oxirgi bayt ham RAM da) */
static int ram_ichida(const struct mashina *m, uint32_t fiz, int hajm)
{
    return fiz >= RAM_BOSH && fiz - RAM_BOSH <= m->ram_hajm - (uint32_t)hajm;
}

int shina_oqi(struct mashina *m, uint32_t fiz, int hajm, uint32_t *qiymat)
{
    if (ram_ichida(m, fiz, hajm)) {
        const uint8_t *p = m->ram + (fiz - RAM_BOSH);
        uint32_t v = 0;
        for (int i = hajm - 1; i >= 0; i--)
            v = (v << 8) | p[i];                /* little-endian: oxirgi (yuqori) baytdan boshlab yig'amiz */
        *qiymat = v;
        return 0;
    }
    if (fiz >= UART_MANZIL && fiz < UART_MANZIL + UART_HAJM) {
        *qiymat = uart_oqi(&m->uart, fiz - UART_MANZIL);
        return 0;
    }
    if (fiz >= DISK_MANZIL && fiz < DISK_MANZIL + DISK_HAJM) {
        *qiymat = disk_oqi(&m->disk, fiz - DISK_MANZIL);
        return 0;
    }
    if (fiz >= CLINT_MANZIL && fiz < CLINT_MANZIL + CLINT_HAJM) {
        *qiymat = clint_oqi(m, fiz - CLINT_MANZIL);
        return 0;
    }
    if (fiz >= PLIC_MANZIL && fiz < PLIC_MANZIL + PLIC_HAJM) {
        *qiymat = plic_oqi(&m->plic, fiz - PLIC_MANZIL);
        return 0;
    }
    if (fiz == QUVVAT_MANZIL) {
        *qiymat = 0;
        return 0;
    }
    return -1;                                  /* bu manzilda hech narsa yo'q: "access fault" */
}

int shina_yoz(struct mashina *m, uint32_t fiz, int hajm, uint32_t qiymat)
{
    if (ram_ichida(m, fiz, hajm)) {
        uint8_t *p = m->ram + (fiz - RAM_BOSH);
        for (int i = 0; i < hajm; i++)
            p[i] = (uint8_t)(qiymat >> (8 * i));        /* past bayt — kichik manzilga */
        return 0;
    }
    if (fiz >= UART_MANZIL && fiz < UART_MANZIL + UART_HAJM) {
        uart_yoz(&m->uart, fiz - UART_MANZIL, qiymat);
        return 0;
    }
    if (fiz >= DISK_MANZIL && fiz < DISK_MANZIL + DISK_HAJM) {
        disk_yoz(m, fiz - DISK_MANZIL, qiymat);
        return 0;
    }
    if (fiz >= CLINT_MANZIL && fiz < CLINT_MANZIL + CLINT_HAJM) {
        clint_yoz(m, fiz - CLINT_MANZIL, qiymat);
        return 0;
    }
    if (fiz >= PLIC_MANZIL && fiz < PLIC_MANZIL + PLIC_HAJM) {
        plic_yoz(&m->plic, fiz - PLIC_MANZIL, qiymat);
        return 0;
    }
    if (fiz == QUVVAT_MANZIL) {
        /* QEMU "sifive_test" qurilmasi bilan bir xil kodlar:
             0x5555             — muvaffaqiyatli o'chirish (chiqish kodi 0)
             (kod << 16) | 0x3333 — xato bilan o'chirish (chiqish kodi = kod) */
        if ((qiymat & 0xFFFF) == 0x5555) {
            m->toxtadi = 1;
            m->chiqish_kodi = 0;
        } else if ((qiymat & 0xFFFF) == 0x3333) {
            m->toxtadi = 1;
            m->chiqish_kodi = (int)(qiymat >> 16);
        }
        return 0;
    }
    return -1;
}

/*
 * TEKIS BO'LMAGAN MUROJAAT (misaligned): lw manzili 4 ga karrali emas (masalan 0x1002).
 *   Spetsifikatsiya ikki yo'lga ruxsat beradi: (a) apparat o'zi bajaradi, (b) istisno beradi va firmware
 *   dasturiy ravishda bajaradi (OpenSBI shunday qiladi — sekin). Biz (a) ni tanlaymiz: murojaatni BAYTLARGA
 *   bo'lamiz. Har bayt ALOHIDA tarjima qilinadi — chunki 4 bayt ikki sahifaga bo'linib ketishi mumkin
 *   (0x1FFE..0x2001: ikki bayt bir sahifada, ikkitasi keyingisida)!
 *   Faqat atomik buyruqlar (lr/sc/amo) tekis bo'lishi SHART (cpu.c da tekshiriladi).
 */
static int tekis(uint32_t manzil, int hajm)
{
    return (manzil & (uint32_t)(hajm - 1)) == 0;
}

static int tekis_emas_oqi(struct mashina *m, uint32_t va, int hajm, uint32_t *qiymat)
{
    /*
     * TODO(E8a) — O'ZINGIZ YOZING: Tekis bo'lmagan o'qish: murojaatni BAYTLARGA bo'ling, har baytni ALOHIDA tarjima qiling.
     *   - i = 0..hajm-1: mmu_tarjima(m, va + i, KIRISH_OQISH, &fiz) — xato bo'lsa o'sha xatoni qaytaring
     *   - shina_oqi(m, fiz, 1, &bayt) — muvaffaqiyatsiz bo'lsa SABAB_OQISH_KIRISH
     *   - little-endian: q |= bayt << (8 * i);  oxirida *qiymat = q, return 0
     * Tekshirish: make test  (birlik testida 'E8' qatori)
     */
    (void)m;
    (void)va;
    (void)hajm;
    (void)qiymat;
    return SABAB_OQISH_TEKIS_EMAS;
}

static int tekis_emas_yoz(struct mashina *m, uint32_t va, int hajm, uint32_t qiymat)
{
    /*
     * TODO(E8b) — O'ZINGIZ YOZING: Tekis bo'lmagan yozish: baytlarga bo'lib yozing — lekin AVVAL hamma baytlar tarjimasini tekshiring.
     *   - nega? 2 bayt yozilib, 3-baytda sahifa xatosi bo'lsa — xotira 'yarim' o'zgarib qoladi. Bu taqiqlangan
     *   - 1-tsikl: fiz[i] = tarjima (KIRISH_YOZISH), xato bo'lsa qaytaring;  2-tsikl: shina_yoz(m, fiz[i], 1, (qiymat >> 8*i) & 0xFF)
     *   - shina_yoz xatosi -> SABAB_YOZISH_KIRISH
     * Tekshirish: make test  (birlik testida 'E8' qatori)
     */
    (void)m;
    (void)va;
    (void)hajm;
    (void)qiymat;
    return SABAB_YOZISH_TEKIS_EMAS;
}

int xotira_oqi(struct mashina *m, uint32_t va, int hajm, uint32_t *qiymat)
{
    if (!tekis(va, hajm))
        return tekis_emas_oqi(m, va, hajm, qiymat);
    uint32_t fiz;
    int xato = mmu_tarjima(m, va, KIRISH_OQISH, &fiz);
    if (xato)
        return xato;                            /* sahifa xatosi (13) yoki PTE ni o'qishda kirish xatosi */
    if (shina_oqi(m, fiz, hajm, qiymat) != 0)
        return SABAB_OQISH_KIRISH;
    return 0;
}

int xotira_yoz(struct mashina *m, uint32_t va, int hajm, uint32_t qiymat)
{
    if (!tekis(va, hajm))
        return tekis_emas_yoz(m, va, hajm, qiymat);
    uint32_t fiz;
    int xato = mmu_tarjima(m, va, KIRISH_YOZISH, &fiz);
    if (xato)
        return xato;
    if (shina_yoz(m, fiz, hajm, qiymat) != 0)
        return SABAB_YOZISH_KIRISH;
    return 0;
}
