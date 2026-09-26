/* =============================================================================
 *  31 - Sahifa jadvali (x86-64, 4 daraja)              [6-modul: yadro mexanizmlari]
 * =============================================================================
 *
 *  VAZIFA - x86-64 CPU virtual manzilni fizikka qanday aylantirsa, xuddi shuni
 *  dasturda qiling. Fizik xotira simulyatsiya qilingan: `sahifa_ajrat()` yangi
 *  nollangan 4 KB sahifaning FIZIK manzilini beradi, `fiz_ptr(fiz)` esa unga
 *  C ko'rsatkichini (yadrodagi direct map kabi). Jadval yozuvi - uint64_t.
 *
 *    xarita(pml4, virt, fiz, bayroqlar)
 *        virt sahifasini fiz sahifasiga bog'lash. Yo'lda yetishmayotgan jadvallarni
 *        (PDPT, PD, PT) sahifa_ajrat() bilan yarating; oraliq yozuvlar:
 *        manzil | PTE_PRESENT | PTE_WRITABLE | PTE_USER. Oxirgi (PT) yozuv:
 *        fiz | bayroqlar | PTE_PRESENT.
 *        Qaytish: 0; virt yoki fiz 4096 ga tekislanmagan -> -1;
 *                 allaqachon bog'langan -> -2; sahifa_ajrat 0 qaytardi -> -3.
 *    tarjima(pml4, virt, &fiz)
 *        CPU kabi jadvallar bo'ylab yurish. Yo'lda birorta yozuvda PTE_PRESENT
 *        yo'q bo'lsa yoki manzil kanonik bo'lmasa -> -1. Aks holda
 *        *fiz = sahifa manzili + sahifa ichidagi siljish (pastki 12 bit), 0.
 *    xaritani_ochir(pml4, virt)
 *        PT yozuvini 0 qilish (jadvallarni bo'shatish shart emas); bog'lanmagan -> -1.
 *
 *  QANDAY ISHLAYDI (docs/04-virtual-xotira.md):
 *    48 bitli virtual manzil 5 qismga bo'linadi:
 *      47..39 - PML4 indeksi (9 bit, 0..511)
 *      38..30 - PDPT indeksi
 *      29..21 - PD indeksi
 *      20..12 - PT indeksi
 *      11..0  - sahifa ichidagi siljish
 *    Har bir jadval - 512 ta uint64_t (aniq 4 KB). Yozuvdagi PTE_ADDR_MASK bitlari -
 *    keyingi jadvalning (yoki oxirida sahifaning) FIZIK manzili.
 *    Kanonik manzil: 63..48-bitlar 47-bit bilan bir xil (0x00007fff... yoki 0xffff8000...).
 *
 *  NEGA BU MUHIM:
 *    Har bir jarayonning o'z manzil maydoni, fork + COW, mmap, himoya (NX, faqat
 *    o'qish), yadroning yuqori yarmi - hammasi shu tuzilma ustida. MyOS:
 *    kernel/mm/vmm.c (vmm_map, vmm_translate) - siz yozgan narsaning haqiqiy nusxasi.
 *
 *  MASLAHAT:
 *    * Indeks: (virt >> 39) & 511, (virt >> 30) & 511, ...
 *    * Jadval ko'rsatkichi: uint64_t *t = fiz_ptr(yozuv & PTE_ADDR_MASK);
 *    * Yordamchi funksiya: "keyingi darajadagi jadvalni ol (kerak bo'lsa yarat)".
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 31
 * ============================================================================= */
#include "mashq.h"

int xarita(uint64_t pml4, uint64_t virt, uint64_t fiz, uint64_t bayroqlar)
{
    /* TODO */
    (void)pml4; (void)virt; (void)fiz; (void)bayroqlar;
    return -1;
}

int tarjima(uint64_t pml4, uint64_t virt, uint64_t *fiz)
{
    /* TODO */
    (void)pml4; (void)virt; (void)fiz;
    return -1;
}

int xaritani_ochir(uint64_t pml4, uint64_t virt)
{
    /* TODO */
    (void)pml4; (void)virt;
    return -1;
}
