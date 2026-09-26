/* =============================================================================
 *  40 - Mini shell (yakuniy loyiha)                     [6-modul: yadro mexanizmlari]
 * =============================================================================
 *
 *  VAZIFA - shell(qator): bitta buyruq qatorini bajarib, OXIRGI buyruqning chiqish
 *  holatini qaytaring. Bu - 15 (split), 27 (fork/exec/wait) va 28 (pipe/dup2)
 *  mashqlarining birlashmasi.
 *
 *  SINTAKSIS (soddalashtirilgan):
 *    * So'zlar bo'shliq/tab bilan ajratilgan. Qo'shtirnoq yo'q.
 *    * `|`, `<`, `>` - alohida so'z bo'lib keladi (atrofida bo'shliq bor).
 *    * `a | b | c` - ko'pi bilan SHELL_MAKS_BUYRUQ ta buyruq; har biri ko'pi bilan
 *      SHELL_MAKS_ARG - 1 ta so'z.
 *    * `< fayl` - shu buyruqning stdin'i fayldan; `> fayl` - stdout faylga
 *      (O_WRONLY | O_CREAT | O_TRUNC, 0644). Buyruqning istalgan joyida kelishi mumkin.
 *
 *  QAYTISH:
 *    * bo'sh qator -> 0
 *    * sintaksis xatosi -> -1 (hech narsa ishga tushirilmaydi): `|` boshida/oxirida,
 *      `| |` (bo'sh buyruq), `<`/`>` dan keyin fayl nomi yo'q, juda ko'p buyruq/so'z
 *    * aks holda oxirgi buyruqning holati: chiqish kodi; topilmadi -> 127;
 *      signal bilan o'ldi -> 128 + signal; `<` fayli ochilmadi -> 1
 *
 *  TALABLAR (test tekshiradi):
 *    * Hamma buyruqlar PARALLEL ishlaydi (avval hammasini ishga tushirib, keyin kutish).
 *    * Otada va har bir bolada keraksiz pipe uchlari YOPILGAN bo'lishi shart -
 *      aks holda `seq | grep | wc` abadiy kutadi (28-mashqni eslang).
 *    * Hamma bolalar kutiladi (zombi qolmaydi), fd'lar sizib chiqmaydi.
 *
 *  NEGA:
 *    Shell - OS bilan birinchi uchrashuv va tizim chaqiruvlarining eng yaxshi
 *    mashqi. Bu mashqdan keyin MyOS'ning user/bin/sh.c ini o'qing - u xuddi shu
 *    g'oyalar ustiga qo'shtirnoq, o'zgaruvchilar, job control, tarix va Tab qo'shadi.
 *    Keyin esa yadroning ICHKI tomoni: kernel/proc/process.c (fork), exec.c,
 *    fs/pipe.c - labs/README.md.
 *
 *  REJA:
 *    1) Qatorni so'zlarga bo'lish (satr nusxasi + strtok_r yoki qo'lda).
 *    2) So'zlarni buyruqlarga ajratish: argv massivlari, har biri uchun kirish/chiqish fayli.
 *       Sintaksisni TO'LIQ tekshirib, keyingina fork qilish.
 *    3) n - 1 ta pipe. i-bola: stdin <- (i-1)-pipe, stdout -> i-pipe; keyin fayl
 *       yo'naltirishlari; keyin HAMMA pipe uchlarini yopish; execvp; _exit(127).
 *    4) Ota: hamma pipe uchlarini yopish, hamma bolalarni waitpid.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 40
 * ============================================================================= */
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mashq.h"

int shell(const char *qator)
{
    /* TODO */
    (void)qator;
    return -1;
}
