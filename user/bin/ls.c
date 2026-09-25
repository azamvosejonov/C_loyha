/* =============================================================================
 *  user/bin/ls.c - initrd dagi fayllar ro'yxati
 *
 *  readdir(i) syscall'i yadrodagi tarfs dan i-chi fayl nomi va hajmini oladi.
 * ============================================================================= */
#include "ulib.h"

int main(void)
{
    struct myos_dirent d;
    uint64_t total = 0;
    int i = 0;
    for (; readdir(i, &d) == 0; i++) {
        printf("  %8lu  %s\n", d.size, d.name);
        total += d.size;
    }
    printf("%d ta fayl, jami %lu bayt\n", i, total);
    return 0;
}
