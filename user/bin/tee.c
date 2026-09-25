/* user/bin/tee.c - stdin ni ham stdout ga, ham faylga (T shaklidagi quvur):
 *   ls | tee ro'yxat.txt | wc -l      (-a: faylga qo'shib yozish) */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_OUT 16

int main(int argc, char **argv)
{
    int flags = O_WRONLY | O_CREAT | O_TRUNC, i = 1;
    if (argc > 1 && strcmp(argv[1], "-a") == 0) {
        flags = O_WRONLY | O_CREAT | O_APPEND;
        i++;
    }
    int fds[MAX_OUT], n = 0, status = 0;
    fds[n++] = STDOUT_FILENO;
    for (; i < argc && n < MAX_OUT; i++) {
        int fd = open(argv[i], flags, 0644);
        if (fd < 0) {
            fprintf(stderr, "tee: %s: %s\n", argv[i], strerror(errno));
            status = 1;
            continue;
        }
        fds[n++] = fd;
    }
    char buf[4096];
    ssize_t r;
    while ((r = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
        for (int k = 0; k < n; k++)
            if (write(fds[k], buf, (size_t)r) != r)
                status = 1;
    return status;
}
