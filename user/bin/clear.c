/* user/bin/clear.c - ekranni tozalash. ANSI ketma-ketliklari: ESC[2J (butun ekranni
 * tozalash) + ESC[H (kursor chap yuqori burchakka). Terminal emulyatori
 * (kernel/drivers/vt.c) va istalgan Linux terminali tushunadi. */
#include <unistd.h>

int main(void)
{
    write(STDOUT_FILENO, "\033[2J\033[H", 7);
    return 0;
}
