/* =============================================================================
 *  tests/crashdemo.c - "buzib ko'r" namoyishlari
 * =============================================================================
 *
 *  Kuchli dasturchi xatoni KO'RGANDA taniy olishi kerak. Bu fayl eng keng
 *  tarqalgan xotira xatolarini ataylab qiladi, siz esa yadroning ularni qanday
 *  ushlashini ko'rasiz:
 *
 *    make run APPEND=demo=null        - NULL ko'rsatkichga yozish   -> page fault
 *    make run APPEND=demo=uaf         - bo'shatilgan xotiraga yozish -> heap panic
 *    make run APPEND=demo=doublefree  - ikki marta kfree            -> heap panic
 *    make run APPEND=demo=badfree     - kmalloc dan kelmagan kfree   -> heap panic
 *    make run APPEND=demo=stack       - cheksiz rekursiya            -> double fault (IST)
 *
 *  Oddiy C dasturida bu xatolarning ko'pchiligi DARHOL sezilmaydi - dastur
 *  "ishlab turadi" va keyinroq tushunarsiz joyda qulaydi. Shuning uchun
 *  AddressSanitizer, Valgrind kabi vositalar va debug allocatorlar mavjud.
 * ============================================================================= */
#include "tests/crashdemo.h"

#include <stdint.h>

#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/heap.h"

/* noinline + volatile: kompilyator rekursiyani tsiklga aylantirib yubormasin.
 * Rekursiya "cheksiz" emas (limit bor) - gcc ogohlantirmasligi uchun, lekin
 * limit shunchalik kattaki, stek undan ancha oldin tugaydi. */
__attribute__((noinline)) static uint64_t recurse_deep(volatile uint64_t depth)
{
    volatile uint8_t big[512];          /* har bir chaqiruv stekdan 512+ bayt oladi */
    big[0] = (uint8_t)depth;
    if (depth > 100000000)
        return big[0];
    return recurse_deep(depth + 1) + big[0];
}

void crashdemo_run(const char *name)
{
    kprintf("[demo] '%s' namoyishi: ataylab xato qilamiz...\n", name);

    if (strcmp(name, "null") == 0) {
        volatile int *p = NULL;
        *p = 42;
    } else if (strcmp(name, "uaf") == 0) {
        char *p = kmalloc(64);
        kfree(p);
        p[20] = 'X';                    /* bo'shatilgan xotiraga yozish! */
        kmalloc(64);                    /* allocator zaharni tekshiradi -> panic */
    } else if (strcmp(name, "doublefree") == 0) {
        void *p = kmalloc(32);
        kfree(p);
        kfree(p);
    } else if (strcmp(name, "badfree") == 0) {
        static char not_heap[128];
        kfree(not_heap + 16);
    } else if (strcmp(name, "stack") == 0) {
        recurse_deep(0);
    } else {
        kprintf("[demo] noma'lum namoyish. Variantlar: null uaf doublefree badfree stack\n");
        return;
    }
    kprintf("[demo] xato USHLANMADI - bu kutilmagan!\n");
}
