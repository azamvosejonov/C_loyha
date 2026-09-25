/* user/bin/rmdir.c - BO'SH papkani o'chirish (bo'sh bo'lmasa - ENOTEMPTY). */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "ishlatish: rmdir <papka>...\n");
        return 1;
    }
    int status = 0;
    for (int i = 1; i < argc; i++) {
        if (rmdir(argv[i]) < 0) {
            fprintf(stderr, "rmdir: %s: %s\n", argv[i], strerror(errno));
            status = 1;
        }
    }
    return status;
}
