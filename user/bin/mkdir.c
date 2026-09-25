/* user/bin/mkdir.c - papka yaratish. -p: yo'ldagi barcha papkalarni, bor bo'lsa xato emas. */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static int mkdir_p(char *path)
{
    /* "/a/b/c" -> "/a", "/a/b", "/a/b/c" ni ketma-ket yaratamiz. */
    for (char *p = path + 1; *p; p++) {
        if (*p != '/')
            continue;
        *p = '\0';
        int r = mkdir(path, 0755);
        *p = '/';
        if (r < 0 && errno != EEXIST)
            return -1;
    }
    if (mkdir(path, 0755) < 0 && errno != EEXIST)
        return -1;
    return 0;
}

int main(int argc, char **argv)
{
    int i = 1, parents = 0;
    if (argc > 1 && strcmp(argv[1], "-p") == 0) {
        parents = 1;
        i++;
    }
    if (i >= argc) {
        fprintf(stderr, "ishlatish: mkdir [-p] <papka>...\n");
        return 1;
    }
    int status = 0;
    for (; i < argc; i++) {
        if ((parents ? mkdir_p(argv[i]) : mkdir(argv[i], 0755)) < 0) {
            fprintf(stderr, "mkdir: %s: %s\n", argv[i], strerror(errno));
            status = 1;
        }
    }
    return status;
}
