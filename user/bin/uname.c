/* user/bin/uname.c - tizim haqida ma'lumot. */
#include <stdio.h>
#include "myos.h"

int main(void)
{
    struct myos_sysinfo s;
    if (sysinfo(&s) < 0)
        return 1;
    uint64_t sec = s.uptime_ms / 1000;
    printf("MyOS x86_64 | CPU: %s (%s), %u yadro, %lu MHz\n", s.cpu_brand, s.cpu_vendor, s.ncpus,
           s.tsc_khz / 1000);
    printf("Yuklovchi: %s | ishlash vaqti: %lu:%02lu:%02lu\n", s.bootloader, sec / 3600,
           (sec / 60) % 60, sec % 60);
    return 0;
}
