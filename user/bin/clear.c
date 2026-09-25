/* user/bin/clear.c - ekranni tozalash. ANSI ketma-ketligi: ESC[2J (tozalash) + ESC[H
 * (kursor boshiga). Konsolimiz ANSI'ni hali tushunmaydi - terminal emulyatori
 * qo'shilganda ishlaydi; serial terminalda (QEMU -nographic) hozir ham ishlaydi. */
#include <unistd.h>

int main(void)
{
    write(STDOUT_FILENO, "\033[2J\033[H", 7);
    return 0;
}
