/* =============================================================================
 *  user/bin/mount.c - fayl tizimini papkaga ulash
 *
 *    mount                              - ulangan fayl tizimlari (/proc/mounts o'rniga
 *                                          hozircha yadro logi: dmesg | grep mount)
 *    mount -t <tur> <qurilma> <papka>   - masalan: mount -t ext2 /dev/sda1 /mnt
 *                                          mount -t tmpfs none /tmp
 * ============================================================================= */
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "myos.h"

int main(int argc, char **argv)
{
    if (argc == 5 && strcmp(argv[1], "-t") == 0) {
        if (mount(argv[3], argv[4], argv[2]) < 0) {
            fprintf(stderr, "mount: %s -> %s: %s\n", argv[3], argv[4], strerror(errno));
            return 1;
        }
        return 0;
    }
    fprintf(stderr, "ishlatish: mount -t <tur> <qurilma> <papka>\n");
    return 1;
}
