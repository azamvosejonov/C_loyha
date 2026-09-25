/* =============================================================================
 *  arch/cpu.c - CPU imkoniyatlarini aniqlash va yoqish
 * =============================================================================
 *
 *  "Eng past kompyuter"da ham ishlash uchun HECH NARSANI taxmin qilmaymiz:
 *  har bir imkoniyat (NX, 1 GB sahifalar, PAT...) CPUID bilan tekshiriladi va
 *  bo'lmasa - yadro usiz ishlaydi (masalan, NX bo'lmasa sahifalar shunchaki
 *  bajariladigan bo'ladi, 1 GB sahifa bo'lmasa 2 MB lik ishlatiladi).
 * ============================================================================= */
#include "arch/cpu.h"

#include "lib/kprintf.h"
#include "lib/string.h"

struct cpu_features cpu_features;

void cpu_detect(void)
{
    struct cpu_features *f = &cpu_features;
    uint32_t a, b, c, d;

    /* leaf 0: eng katta standart leaf + ishlab chiqaruvchi nomi (EBX, EDX, ECX tartibida!) */
    cpuid(0, 0, &a, &b, &c, &d);
    f->max_leaf = a;
    memcpy(f->vendor + 0, &b, 4);
    memcpy(f->vendor + 4, &d, 4);
    memcpy(f->vendor + 8, &c, 4);
    f->vendor[12] = '\0';

    /* leaf 1: asosiy imkoniyatlar */
    cpuid(1, 0, &a, &b, &c, &d);
    f->pge = d & (1u << 13);
    f->pat = d & (1u << 16);
    f->apic = d & (1u << 9);
    f->x2apic = c & (1u << 21);

    /* leaf 7: kengaytirilgan imkoniyatlar */
    if (f->max_leaf >= 7) {
        cpuid(7, 0, &a, &b, &c, &d);
        f->smep = b & (1u << 7);
        f->smap = b & (1u << 20);
    }

    /* 0x8000000x: AMD kengaytmalari (Intel ham qo'llaydi) */
    cpuid(0x80000000, 0, &a, &b, &c, &d);
    f->max_ext_leaf = a;
    if (a >= 0x80000001) {
        cpuid(0x80000001, 0, &a, &b, &c, &d);
        f->nx = d & (1u << 20);
        f->page1gb = d & (1u << 26);
    }
    if (f->max_ext_leaf >= 0x80000004) {
        uint32_t *brand = (uint32_t *)f->brand;
        for (uint32_t i = 0; i < 3; i++)
            cpuid(0x80000002 + i, 0, &brand[i * 4], &brand[i * 4 + 1], &brand[i * 4 + 2],
                  &brand[i * 4 + 3]);
        f->brand[48] = '\0';
    }
    if (f->max_ext_leaf >= 0x80000007) {
        cpuid(0x80000007, 0, &a, &b, &c, &d);
        f->tsc_invariant = d & (1u << 8);
    }

    const char *brand = f->brand;
    while (*brand == ' ')
        brand++;
    kprintf("[cpu]  %s \"%s\"\n", f->vendor, brand);
    kprintf("[cpu]  NX:%s 1GB-sahifa:%s PAT:%s APIC:%s x2APIC:%s SMEP:%s SMAP:%s\n",
            f->nx ? "ha" : "yo'q", f->page1gb ? "ha" : "yo'q", f->pat ? "ha" : "yo'q",
            f->apic ? "ha" : "yo'q", f->x2apic ? "ha" : "yo'q", f->smep ? "ha" : "yo'q",
            f->smap ? "ha" : "yo'q");
}

void cpu_enable_features(void)
{
    /* NX: boot.asm ham yoqadi, lekin AP (boshqa yadrolar) uchun shu yerda. */
    if (cpu_features.nx)
        wrmsr(MSR_EFER, rdmsr(MSR_EFER) | EFER_NXE);

    /* PAT: 1-yozuvni Write-Combining (WC) qilamiz. Sukut bo'yicha:
     *   PA0=WB PA1=WT PA2=UC- PA3=UC (va takror). Biz PA1 = WC qilamiz.
     * Sahifa yozuvida PWT=1, PCD=0, PAT=0 -> PA1 -> WC. Framebuffer uchun WC
     * yozishni 10 barobargacha tezlashtiradi: CPU yozuvlarni to'plab, katta
     * paketlar bilan yuboradi. */
    if (cpu_features.pat)
        wrmsr(MSR_PAT, 0x0007040600070106ULL);

    /* PGE: global sahifalar - yadro sahifalari CR3 almashganda TLB dan
     * o'chirilmaydi (jarayon almashtirish tezlashadi). */
    if (cpu_features.pge)
        cpu_write_cr4(cpu_read_cr4() | (1UL << 7));

    /* SMEP: yadro user sahifasidagi kodni BAJARA OLMAYDI. Klassik hujum:
     * yadrodagi xatodan foydalanib, user joylashtirgan kodga sakrash - SMEP
     * buni apparat darajasida to'xtatadi. */
    if (cpu_features.smep)
        cpu_write_cr4(cpu_read_cr4() | (1UL << 20));
}
