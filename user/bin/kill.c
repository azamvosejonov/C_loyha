/* user/bin/kill.c - jarayonni to'xtatish: kill [-SIGNAL] <pid>... */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    int sig = SIGTERM, i = 1;
    if (argc > 1 && argv[1][0] == '-') {
        sig = atoi(argv[1] + 1);
        i++;
    }
    if (i >= argc) {
        fprintf(stderr, "ishlatish: kill [-SIGNAL] <pid>...\n");
        return 1;
    }
    int status = 0;
    for (; i < argc; i++) {
        int pid = atoi(argv[i]);
        if (kill(pid, sig) < 0) {
            fprintf(stderr, "kill: (%d): %s\n", pid, strerror(errno));
            status = 1;
        }
    }
    return status;
}
