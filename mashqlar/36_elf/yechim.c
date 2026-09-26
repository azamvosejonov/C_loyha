/* =============================================================================
 *  36 - ELF fayl tahlilchisi                            [6-modul: yadro mexanizmlari]
 * =============================================================================
 *
 *  VAZIFA:
 *    elf_tahlil(buf, n, &info) - xotiradagi ELF fayl (buf, n bayt) ni tekshirib,
 *    yuklash uchun kerakli ma'lumotni info ga yozing:
 *      info.kirish, info.tur, va HAR BIR PT_LOAD (tur == 1) segmenti uchun
 *      seg[i] = { vaddr, memsz, filesz, offset, bayroqlar } (fayldagi tartibda).
 *    Qaytish:
 *       0 - to'g'ri
 *      -1 - ELF emas yoki mos emas: sehrli baytlar 7f 45 4c 46 emas; ident[4] != 2
 *           (64 bit emas); ident[5] != 1 (little-endian emas); mashina != 62;
 *           versiya != 1; tur 2 yoki 3 emas
 *      -2 - buzilgan: sarlavha n ga sig'maydi; phentsize != 56; dastur sarlavhalari
 *           jadvali fayldan tashqariga chiqadi; PT_LOAD segmentda
 *           offset + filesz > n yoki filesz > memsz
 *      -3 - PT_LOAD segmentlar ELF_MAKS_SEGMENT dan ko'p
 *    Tuzilmalar mashq.h da (ular aynan diskdagi formatga mos).
 *
 *  NEGA BU YADRO UCHUN:
 *    `exec` tizim chaqiruvi aynan shunday qiladi: faylni o'qiydi, sarlavhani
 *    TEKSHIRADI, har bir PT_LOAD segmentni jarayon manzil maydoniga xaritalaydi
 *    (filesz bayt fayldan, qolgan memsz - filesz - nollar: .bss), keyin rip = kirish.
 *    MyOS: kernel/sys/elf.c (elf_load lab'i). Fayl - DUSHMAN kirishi: har bir
 *    maydon tekshirilmasa, buzilgan fayl yadroni qulatadi yoki hujum yo'li bo'ladi.
 *
 *  TUZOQLAR:
 *    * TOSHISH: phoff + phnum * phentsize, offset + filesz - katta qiymatlarda 64 bitda
 *      ham toshishi mumkin! (13-bob: hisoblashdan OLDIN tekshirish: offset > n - filesz)
 *    * buf ni to'g'ridan-to'g'ri struct ko'rsatkichiga aylantirish tekislanmagan
 *      murojaatga olib kelishi mumkin; xavfsiz yo'l - memcpy bilan lokal struct'ga
 *      nusxalash.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 36
 *    (test haqiqiy dasturni - o'zining bajariladigan faylini - o'qiydi)
 * ============================================================================= */
#include <string.h>

#include "mashq.h"

int elf_tahlil(const uint8_t *buf, size_t n, struct elf_info *out)
{
    /* TODO */
    (void)buf; (void)n; (void)out;
    return -1;
}
