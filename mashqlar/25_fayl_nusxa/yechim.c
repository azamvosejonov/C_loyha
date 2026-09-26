/* =============================================================================
 *  25 - Faylni nusxalash (cp)                     [5-modul: tizim chaqiruvlari]
 * =============================================================================
 *
 *  VAZIFA:
 *    nusxala(manba, nishon) - manba faylni nishon faylga nusxalash.
 *      FAQAT tizim chaqiruvlari: open, read, write, close (fopen/fread EMAS).
 *      nishon bo'lmasa - yaratilsin (0644), bor bo'lsa - ustidan yozilsin (O_TRUNC).
 *      Muvaffaqiyat: 0.  Xato (manba yo'q, o'qish/yozish xatosi): -1.
 *      Xato bo'lsa ham ochilgan fayllarni YOPING (deskriptor sizib chiqmasin).
 *
 *  PYTHON'DA:
 *    shutil.copyfile(manba, nishon)
 *
 *  BU NIMA - TIZIM CHAQIRUVI (SYSCALL):
 *    open/read/write/close - bular kutubxona funksiyasi emas, YADROGA so'rov.
 *    Dastur `syscall` instruksiyasini bajaradi, CPU yadro rejimiga o'tadi,
 *    yadro ishni qilib qaytadi. MyOS'da buning HAMMA qismini ko'rasiz:
 *    user/libc/unistd.c (so'rov) -> kernel/arch/syscall_entry.asm (kirish) ->
 *    kernel/sys/syscall.c (tarqatish) -> kernel/fs/vfs.c (bajarish).
 *    Fayl deskriptori (fd) - jarayonning ochiq fayllar jadvalidagi indeks (0, 1, 2 - stdin/out/err).
 *
 *  TUZOQLAR:
 *    * read() so'ralgandan KAMROQ qaytarishi mumkin - bu xato emas. 0 - fayl oxiri.
 *    * write() ham so'ralgandan kamroq yozishi mumkin ("qisman yozish")! Hamma
 *      bayt yozilguncha takrorlang. Bu - klassik xato manbai.
 *    * -1 qaytsa, sabab `errno` da (perror() uni odam tiliga o'giradi).
 *
 *  MASLAHAT:
 *    Bufer: `char buf[4096];` - bir vaqtning o'zida 4 KB (bitta sahifa) o'qish.
 *    Kerakli sarlavhalar: <fcntl.h> (open, O_*), <unistd.h> (read, write, close).
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 25
 * ============================================================================= */
#include <fcntl.h>
#include <unistd.h>

#include "mashq.h"

int nusxala(const char *manba, const char *nishon)
{
    /* TODO */
    (void)manba; (void)nishon;
    return -1;
}
