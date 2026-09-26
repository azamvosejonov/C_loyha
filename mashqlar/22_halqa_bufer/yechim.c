/* =============================================================================
 *  22 - Halqa bufer (ring buffer)                      [4-modul: yadro uslubidagi C]
 * =============================================================================
 *
 *  VAZIFA (struct halqa - mashq.h da, sig'imi HALQA_HAJMI = 8 bayt):
 *    halqa_init(h)          - bo'sh bufer
 *    halqa_yoz(h, data, n)  - n baytdan SIG'GANCHA yozish; nechta yozilganini qaytarish
 *    halqa_oqi(h, out, n)   - n baytdan BORICHA o'qish (FIFO - birinchi yozilgan
 *                             birinchi o'qiladi); nechta o'qilganini qaytarish
 *
 *  PYTHON'DA:
 *    collections.deque(maxlen=...) ga o'xshash.
 *
 *  NEGA BU YADRO UCHUN:
 *    Pipe (MyOS: kernel/fs/pipe.c), klaviatura navbati (drivers/console.c),
 *    dmesg jurnali, tarmoq kartasi va disk kontrollerlarining navbatlari
 *    (AHCI, NVMe, xHCI) - hammasi halqa bufer. Xotira qayta ajratilmaydi,
 *    ma'lumot surilmaydi - faqat indekslar aylanadi.
 *
 *  MASLAHAT:
 *    * Yozish joyi: (bosh + soni) % HALQA_HAJMI.
 *    * Oxiriga yetganda boshiga "o'ralish" - test aynan shuni tekshiradi.
 *    * `const void *data` ni `const unsigned char *` ga aylantirib ishlating.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 22
 * ============================================================================= */
#include "mashq.h"

void halqa_init(struct halqa *h)
{
    /* TODO */
    (void)h;
}

size_t halqa_yoz(struct halqa *h, const void *data, size_t n)
{
    /* TODO */
    (void)h; (void)data; (void)n;
    return 0;
}

size_t halqa_oqi(struct halqa *h, void *out, size_t n)
{
    /* TODO */
    (void)h; (void)out; (void)n;
    return 0;
}
