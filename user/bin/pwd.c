/* user/bin/pwd.c - joriy papka (yadro uni har bir jarayon uchun saqlaydi). */
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    char buf[PATH_MAX];
    if (!getcwd(buf, sizeof(buf))) {
        perror("pwd");
        return 1;
    }
    puts(buf);
    return 0;
}
