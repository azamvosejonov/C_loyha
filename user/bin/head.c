/* user/bin/head.c - birinchi N qator (sukut 10):  head [-n N] [fayl]... */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int head(FILE *f, long lines)
{
    int c;
    while (lines > 0 && (c = fgetc(f)) != EOF) {
        putchar(c);
        if (c == '\n')
            lines--;
    }
    return 0;
}

int main(int argc, char **argv)
{
    long lines = 10;
    int i = 1;
    if (i + 1 < argc && strcmp(argv[i], "-n") == 0) {
        lines = atoi(argv[i + 1]);
        i += 2;
    } else if (i < argc && argv[i][0] == '-' && argv[i][1] >= '0' && argv[i][1] <= '9') {
        lines = atoi(argv[i] + 1);      /* head -5 */
        i++;
    }
    if (i == argc)
        return head(stdin, lines);
    int status = 0;
    for (; i < argc; i++) {
        FILE *f = fopen(argv[i], "r");
        if (!f) {
            fprintf(stderr, "head: %s: %s\n", argv[i], strerror(errno));
            status = 1;
            continue;
        }
        head(f, lines);
        fclose(f);
    }
    return status;
}
