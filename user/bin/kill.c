/* user/bin/kill.c - jarayonni to'xtatish: kill <pid> */
#include "ulib.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("ishlatish: kill <pid>\n");
        return 1;
    }
    int pid = atoi(argv[1]);
    if (kill(pid) < 0) {
        printf("kill: pid %d topilmadi yoki uni o'ldirib bo'lmaydi\n", pid);
        return 1;
    }
    return 0;
}
