/* user/bin/touch.c - fayl yo'q bo'lsa bo'sh fayl yaratadi. (Vaqtni yangilash -
 * utimes syscall'i kerak; hozircha yo'q - mashq.) */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "ishlatish: touch <fayl>...\n");
        return 1;
    }
    int status = 0;
    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_WRONLY | O_CREAT, 0644);   /* O_TRUNC YO'Q - mazmun saqlanadi */
        if (fd < 0) {
            fprintf(stderr, "touch: %s: %s\n", argv[i], strerror(errno));
            status = 1;
            continue;
        }
        close(fd);
    }
    return status;
}
