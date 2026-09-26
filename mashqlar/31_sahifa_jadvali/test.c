#include "test.h"
#include "mashq.h"

/* Simulyatsiya: 64 ta "fizik" sahifa, fizik manzillar 1 MB dan boshlanadi. */
#define FIZ_BOSH 0x100000ull
#define SAHIFALAR 64
static _Alignas(4096) unsigned char xotira[SAHIFALAR][4096];
static int ajratildi, chegara = SAHIFALAR;

uint64_t sahifa_ajrat(void)
{
    if (ajratildi >= chegara)
        return 0;
    memset(xotira[ajratildi], 0, 4096);
    return FIZ_BOSH + (uint64_t)ajratildi++ * 4096;
}

void *fiz_ptr(uint64_t fiz)
{
    if (fiz < FIZ_BOSH || fiz >= FIZ_BOSH + SAHIFALAR * 4096ull || (fiz & 7)) {
        printf("  [XATO] fiz_ptr(0x%llx): bunday fizik manzil yo'q (PTE_ADDR_MASK unutildimi?)\n",
               (unsigned long long)fiz);
        exit(1);
    }
    return xotira[(fiz - FIZ_BOSH) / 4096] + (fiz - FIZ_BOSH) % 4096;
}

/* Testning o'z "CPU"si: oxirgi yozuvni topish (tekshirish uchun). */
static uint64_t *yozuv(uint64_t pml4, uint64_t v)
{
    uint64_t t = pml4;
    for (int sh = 39; sh >= 12; sh -= 9) {
        uint64_t *e = (uint64_t *)fiz_ptr(t) + ((v >> sh) & 511);
        if (sh == 12)
            return e;
        if (!(*e & PTE_PRESENT))
            return NULL;
        t = *e & PTE_ADDR_MASK;
    }
    return NULL;
}

int main(void)
{
    TEST_BOSHLA();
    uint64_t pml4 = sahifa_ajrat();
    uint64_t f = 0;

    BOLIM("oddiy xaritalash va tarjima");
    CHECK_INT(xarita(pml4, 0x400000, 0x200000, PTE_WRITABLE | PTE_USER), 0);
    CHECK_INT(tarjima(pml4, 0x400123, &f), 0);
    CHECK_HEX(f, 0x200123);
    CHECK_INT(ajratildi, 4);                    /* PML4 + PDPT + PD + PT */
    uint64_t *e = yozuv(pml4, 0x400000);
    CHECK(e != NULL);
    if (e)
        CHECK_HEX(*e, 0x200000 | PTE_PRESENT | PTE_WRITABLE | PTE_USER);

    BOLIM("qo'shni sahifa - o'sha PT jadvali ishlatiladi");
    CHECK_INT(xarita(pml4, 0x401000, 0x7000, PTE_PRESENT), 0);
    CHECK_INT(ajratildi, 4);
    CHECK_INT(tarjima(pml4, 0x401fff, &f), 0);
    CHECK_HEX(f, 0x7fff);
    CHECK_INT(tarjima(pml4, 0x400000, &f), 0);
    CHECK_HEX(f, 0x200000);

    BOLIM("yadro manzili (yuqori yarmi) - yangi jadvallar");
    CHECK_INT(xarita(pml4, 0xFFFFFFFF80000000ull, 0x100000, PTE_WRITABLE), 0);
    CHECK_INT(ajratildi, 7);
    CHECK_INT(tarjima(pml4, 0xFFFFFFFF80000abcull, &f), 0);
    CHECK_HEX(f, 0x100abc);
    CHECK_INT(xarita(pml4, 0x7FFFFFFFF000ull, 0x9000, PTE_USER), 0);   /* user maydonining oxiri */
    CHECK_INT(tarjima(pml4, 0x7FFFFFFFF010ull, &f), 0);
    CHECK_HEX(f, 0x9010);

    BOLIM("bog'lanmagan va kanonik bo'lmagan manzillar");
    CHECK_INT(tarjima(pml4, 0x402000, &f), -1);
    CHECK_INT(tarjima(pml4, 0x80000000, &f), -1);
    CHECK_INT(tarjima(pml4, 0x0000800000000000ull, &f), -1);      /* kanonik emas */
    CHECK_INT(tarjima(pml4, 0xFFFF7FFFFFFFF000ull, &f), -1);      /* kanonik emas */

    BOLIM("xatolar");
    CHECK_INT(xarita(pml4, 0x400001, 0x5000, 0), -1);             /* tekislanmagan virt */
    CHECK_INT(xarita(pml4, 0x500000, 0x5001, 0), -1);             /* tekislanmagan fiz */
    CHECK_INT(xarita(pml4, 0x400000, 0x5000, 0), -2);             /* allaqachon bor */

    BOLIM("xaritani_ochir");
    CHECK_INT(xaritani_ochir(pml4, 0x400000), 0);
    CHECK_INT(tarjima(pml4, 0x400000, &f), -1);
    CHECK_INT(tarjima(pml4, 0x401000, &f), 0);                    /* qo'shnisi joyida */
    CHECK_INT(xaritani_ochir(pml4, 0x400000), -1);
    CHECK_INT(xaritani_ochir(pml4, 0x12345000), -1);
    CHECK_INT(xarita(pml4, 0x400000, 0x6000, 0), 0);              /* qayta bog'lash mumkin */

    BOLIM("xotira tugashi");
    chegara = ajratildi + 1;                    /* faqat bitta sahifa qoldi */
    CHECK_INT(xarita(pml4, 0x0000100000000000ull, 0x5000, 0), -3);   /* 3 ta jadval kerak */
    TEST_TUGADI();
}
