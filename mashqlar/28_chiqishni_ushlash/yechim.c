/* =============================================================================
 *  28 - Dastur chiqishini ushlash: pipe + dup2      [5-modul: tizim chaqiruvlari]
 * =============================================================================
 *
 *  VAZIFA:
 *    chiqishni_ol(argv, buf, hajm) - argv dasturini ishga tushirib, uning
 *    STANDART CHIQISHINI (stdout) buf ga yig'ing:
 *      * buf ga ko'pi bilan hajm - 1 bayt yoziladi va oxiriga '\0' qo'yiladi
 *      * dastur bundan KO'P chiqarsa ham, uning hammasini o'qib (tashlab)
 *        yuboring - aks holda dastur to'lib qolgan pipe'da abadiy kutadi!
 *      * qaytish: buf ga yozilgan baytlar soni; xato bo'lsa -1
 *      * bola jarayonni albatta waitpid bilan kuting (zombi qolmasin)
 *
 *  PYTHON'DA:
 *    subprocess.run(argv, capture_output=True).stdout
 *
 *  QANDAY ISHLAYDI:
 *    pipe(fds)        - ikki uchli "quvur": fds[0] - o'qish, fds[1] - yozish.
 *    fork()           - bola ham shu ikkala uchni meros oladi.
 *    Bolada: dup2(fds[1], 1) - 1-deskriptorni (stdout) quvurning yozish uchiga
 *            almashtirish. Keyin ikkala asl uchni yopish va execvp.
 *    Otada:  fds[1] ni YOPISH (muhim!), fds[0] dan EOF gacha o'qish, waitpid.
 *    Shell'dagi `a | b` aynan shunday ishlaydi. MyOS: user/bin/sh.c, kernel/fs/pipe.c.
 *
 *  IKKI KLASSIK XATO (test ikkalasini ham tekshiradi):
 *    1) Otada fds[1] yopilmasa - read() hech qachon EOF (0) qaytarmaydi, chunki
 *       quvurga yozadigan uch hali ochiq (otaning o'zida!). Dastur osilib qoladi.
 *    2) Avval waitpid, keyin read - bola 64 KB dan ko'p yozsa, quvur to'ladi,
 *       bola kutadi, ota ham kutadi -> DEADLOCK. Avval o'qish, keyin kutish.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 28
 * ============================================================================= */
#include <sys/wait.h>
#include <unistd.h>

#include "mashq.h"

long chiqishni_ol(char *const argv[], char *buf, size_t hajm)
{
    /* TODO */
    (void)argv; (void)buf; (void)hajm;
    return -1;
}
