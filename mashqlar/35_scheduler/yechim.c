/* =============================================================================
 *  35 - Scheduler: Round Robin simulyatsiyasi            [6-modul: yadro mexanizmlari]
 * =============================================================================
 *
 *  VAZIFA:
 *    round_robin(j, n, kvant, tugash) - bitta CPU'da n ta jarayonni "Round Robin"
 *    qoidasi bilan bajarishni simulyatsiya qiling:
 *      * tugash[i] ga j[i] jarayonining TUGAGAN vaqtini yozing;
 *      * qaytish qiymati - kontekst almashishlar soni: CPU bir jarayondan BOSHQA
 *        jarayonga o'tgan har bir holat (birinchi ishga tushirish va bo'sh turgandan
 *        keyingi ishga tushirish sanalmaydi; kvant tugab, navbatda boshqa hech kim
 *        bo'lmasa va o'sha jarayon davom etsa - bu ham almashish EMAS).
 *
 *    QOIDALAR (aynan shunday - test shunga tayanadi):
 *      1. Vaqt 0 dan boshlanadi. Tayyor jarayonlar - FIFO navbatda.
 *      2. Jarayon kelish vaqtida navbat OXIRIGA qo'shiladi. Bir vaqtda kelganlar -
 *         kirish massividagi tartibda.
 *      3. CPU navbat boshidagi jarayonni min(kvant, qolgan_ish) tik bajaradi.
 *      4. Kvant tugab, ishi qolgan bo'lsa - navbat oxiriga qaytadi, LEKIN o'sha
 *         paytgacha (shu tik bilan birga) kelgan yangi jarayonlardan KEYIN.
 *      5. Navbat bo'sh bo'lsa - CPU keyingi kelish vaqtigacha bo'sh turadi.
 *
 *  PYTHON'DA:
 *    collections.deque bilan 20 qatorda - C'da navbatni o'zingiz yozasiz
 *    (halqa bufer - 22-mashq!).
 *
 *  NEGA:
 *    Taymer uzilishi har N millisekundda keladi, yadro joriy jarayonni to'xtatib,
 *    navbatdagisiga o'tadi - bu "preemptive multitasking". MyOS scheduler'i aynan
 *    round-robin: kvant = 5 tik (50 ms). kernel/proc/process.c (sched_tick,
 *    scheduler_loop), docs/06-jarayonlar.md.
 *
 *  MASLAHAT:
 *    * `int qolgan[n]` (yoki malloc), navbat - n + 1 o'rinli halqa (jarayon bir vaqtda
 *      navbatda bir marta bo'ladi).
 *    * Kelishlarni vaqt bo'yicha qayta ishlash uchun: "keyingi hali kelmagan jarayon"
 *      indeksi - lekin kirish vaqt bo'yicha SARALANMAGAN bo'lishi mumkin! Avval
 *      indekslarni (kelish, indeks) bo'yicha saralang.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 35
 * ============================================================================= */
#include <stdlib.h>

#include "mashq.h"

int round_robin(const struct jarayon *j, size_t n, int kvant, int *tugash)
{
    /* TODO */
    (void)j; (void)n; (void)kvant; (void)tugash;
    return -1;
}
