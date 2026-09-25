/* user/bin/dmesg.c - yadro logini chiqarish (boot xabarlari va boshqalar). */
#include "ulib.h"

static char buf[64 * 1024];

int main(void)
{
    long n = dmesg(buf, sizeof(buf));
    if (n > 0)
        write(STDOUT, buf, (size_t)n);
    return n < 0;
}
