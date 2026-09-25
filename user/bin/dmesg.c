/* user/bin/dmesg.c - yadro logini chiqarish (boot xabarlari va boshqalar). */
#include <stdio.h>
#include <unistd.h>

#include "myos.h"

static char buf[64 * 1024];

int main(void)
{
    long n = dmesg(buf, sizeof(buf));
    if (n < 0) {
        perror("dmesg");
        return 1;
    }
    write(STDOUT_FILENO, buf, (size_t)n);
    return 0;
}
