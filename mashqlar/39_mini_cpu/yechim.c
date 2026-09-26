/* =============================================================================
 *  39 - Kichik CPU (virtual mashina)                    [6-modul: yadro mexanizmlari]
 * =============================================================================
 *
 *  VAZIFA - mashq.h dagi o'yinchoq CPU'ni "bajaring": haqiqiy protsessor qiladigan
 *  sikl - buyruqni olish (fetch), dekodlash (decode), bajarish (execute).
 *
 *    bajar(m, dastur, n, maks_qadam):
 *      1. Boshlash: r[0..7] = 0, pc = 0, sp = XOTIRA, chiqish_soni = 0,
 *         chiqish_kodi = 0. m->xotira ga TEGMANG (test uni oldindan to'ldirishi mumkin).
 *      2. Sikl: dastur[pc] ni olib bajarish; pc odatda + 1 (sakrashlarda - imm).
 *      3. Qaytish qiymati:
 *          0  - HALT yoki SYS chiqish (r[0] == 2; chiqish_kodi = r[1])
 *         -1  - noma'lum op yoki registr raqami > 7        ("#UD - invalid opcode")
 *         -2  - LOAD/STORE manzili 0..XOTIRA-1 dan tashqarida  ("#PF - page fault")
 *         -3  - stek to'lishi (sp 0 dan pastga) yoki bo'sh stekdan POP/RET
 *         -4  - pc 0..n-1 dan tashqarida
 *         -5  - maks_qadam ta buyruq bajarildi, hali tugamadi (cheksiz sikl)
 *         -6  - SYS: noma'lum raqam yoki chiqish buferi to'la
 *      SYS r[0] == 1: chiqish[chiqish_soni++] = r[1] (dastur "ekranga" yozadi).
 *
 *  ARIFMETIKA: ADD/SUB/MUL toshganda "aylanib" ketishi kerak (haqiqiy CPU kabi),
 *  lekin int64_t toshishi C'da UB! Ishorasiz turda hisoblang:
 *      r[a] = (int64_t)((uint64_t)r[b] + (uint64_t)r[d]);
 *
 *  NEGA BU YADRO UCHUN:
 *    * CPU aynan shu siklni bajaradi - 17-bobdagi registrlar, pc (rip), sp (rsp),
 *      CALL/RET va stek endi sizning qo'lingizda.
 *    * Xato kodlari - CPU istisnolarining (exceptions) o'xshashi: #UD, #PF, stek
 *      xatosi. Haqiqiy CPU bunday holatda yadroning ishlovchisini chaqiradi
 *      (MyOS: kernel/arch/interrupts.c), yadro esa dasturni SIGSEGV/SIGILL bilan to'xtatadi.
 *    * SYS - `syscall` buyrug'ining o'xshashi: dastur "yadro"dan xizmat so'raydi (14-bob).
 *    * maks_qadam - taymer uzilishining o'xshashi: cheksiz sikldagi dastur butun
 *      tizimni osib qo'ymaydi.
 *    QEMU va Bochs - x86 uchun aynan shunday (juda katta) emulyatorlar.
 *
 *  MASLAHAT:
 *    `switch (b->op)` - har bir buyruq uchun case. Umumiy tekshiruvlarni (registr
 *    raqamlari, pc chegarasi) sikl boshida bir marta qiling.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 39
 * ============================================================================= */
#include "mashq.h"

int bajar(struct cpu *m, const struct buyruq *dastur, size_t n, long maks_qadam)
{
    /* TODO */
    (void)m; (void)dastur; (void)n; (void)maks_qadam;
    return -1;
}
