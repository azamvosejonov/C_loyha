/* =============================================================================
 *  27 - Dasturni ishga tushirish: fork + exec + wait  [5-modul: tizim chaqiruvlari]
 * =============================================================================
 *
 *  VAZIFA:
 *    ishga_tushir(argv) - argv[0] dasturini (PATH bo'yicha qidirib) argv
 *    argumentlari bilan ishga tushiring, tugashini kuting va natija qaytaring:
 *      * normal tugadi       -> uning chiqish kodi (0..255)
 *      * dastur topilmadi    -> 127 (shell ham shunday qiladi)
 *      * signal bilan o'ldi  -> 128 + signal raqami (SIGTERM = 15 -> 143)
 *      * fork xatosi         -> -1
 *
 *  PYTHON'DA:
 *    subprocess.run(argv).returncode
 *
 *  UNIX'NING ENG MASHHUR G'OYASI:
 *    fork()   - joriy jarayonning NUSXASINI yaratadi. Ikkala jarayon ham
 *               fork() dan QAYTADI: otada bolaning pid'i, bolada 0.
 *    execvp() - joriy jarayonning dasturini boshqasiga ALMASHTIRADI (xotira,
 *               kod - hammasi yangi; pid va ochiq fayllar - o'sha). Muvaffaqiyatli
 *               bo'lsa, HECH QACHON qaytmaydi.
 *    waitpid()- ota bolani kutadi va uning holatini (status) oladi.
 *    Shell har bir buyruqni aynan shunday bajaradi. MyOS'da buning yadro
 *    tomonini ko'rasiz: kernel/proc/process.c (fork + COW), kernel/proc/exec.c.
 *
 *  MASLAHAT:
 *    * Bolada: execvp(argv[0], argv); qaytsa - demak xato: _exit(127).
 *      (exit emas, _exit - bolada ota'ning stdio buferlarini ikkinchi marta
 *      chiqarmaslik uchun.)
 *    * Otada: waitpid(pid, &status, 0), keyin WIFEXITED(status) /
 *      WEXITSTATUS(status) / WIFSIGNALED(status) / WTERMSIG(status).
 *    * Sarlavhalar: <unistd.h>, <sys/wait.h>.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 27
 * ============================================================================= */
#include <sys/wait.h>
#include <unistd.h>

#include "mashq.h"

int ishga_tushir(char *const argv[])
{
    /* TODO */
    (void)argv;
    return -1;
}
