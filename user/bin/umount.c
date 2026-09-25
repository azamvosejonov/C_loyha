/* user/bin/umount.c - fayl tizimini uzish (ichida ochiq fayl bo'lsa - EBUSY). */
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "myos.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "ishlatish: umount <papka>\n");
        return 1;
    }
    if (umount(argv[1]) < 0) {
        fprintf(stderr, "umount: %s: %s\n", argv[1], strerror(errno));
        return 1;
    }
    return 0;
}
