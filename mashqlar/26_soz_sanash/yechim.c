/* =============================================================================
 *  26 - wc: qator, so'z va baytlarni sanash        [5-modul: tizim chaqiruvlari]
 * =============================================================================
 *
 *  VAZIFA:
 *    sana(yol, &n) - fayldagi '\n' lar soni, so'zlar soni va baytlar sonini
 *    hisoblang (Linux'dagi `wc` kabi). So'z - bo'shliq belgilari (' ', '\t',
 *    '\n', '\r', '\v', '\f') bilan ajratilgan, bo'sh bo'lmagan ketma-ketlik.
 *    Faylni open + read bilan 4096 baytli bo'laklarda o'qing (hammasini birdan
 *    xotiraga yuklamang). 0 - muvaffaqiyat, -1 - xato (fayl yo'q).
 *
 *  PYTHON'DA:
 *    t = open(yol, 'rb').read(); t.count(b'\n'), len(t.split()), len(t)
 *
 *  ASOSIY TUZOQ:
 *    So'z ikki bo'lak CHEGARASIDA bo'linib qolishi mumkin: "...sal" | "om ...".
 *    Agar har bir bo'lakni alohida sanasangiz, bu so'z ikki marta sanaladi.
 *    Yechim: "hozir so'z ichidamanmi?" holatini bo'laklar orasida SAQLASH.
 *    Bu - "holat mashinasi" (state machine) - drayverlar, tarmoq protokollari
 *    va terminal emulyatori (MyOS: kernel/drivers/vt.c) shunday ishlaydi.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 26
 * ============================================================================= */
#include <fcntl.h>
#include <unistd.h>

#include "mashq.h"

int sana(const char *yol, struct wc *natija)
{
    /* TODO */
    (void)yol; (void)natija;
    return -1;
}
