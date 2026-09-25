/* user/bin/wc.c - qatorlar, so'zlar va baytlarni sanash:  ls | wc -l */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int opt_l, opt_w, opt_c;
static unsigned long tl, tw, tc;

static void report(unsigned long l, unsigned long w, unsigned long c, const char *name)
{
    if (opt_l)
        printf("%7lu", l);
    if (opt_w)
        printf("%8lu", w);
    if (opt_c)
        printf("%8lu", c);
    printf("%s%s\n", name ? " " : "", name ? name : "");
}

static int count(int fd, const char *name)
{
    static char buf[8192];
    unsigned long l = 0, w = 0, c = 0;
    int in_word = 0;
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        c += (unsigned long)n;
        for (ssize_t i = 0; i < n; i++) {
            if (buf[i] == '\n')
                l++;
            if (isspace((unsigned char)buf[i])) {
                in_word = 0;
            } else if (!in_word) {      /* so'z boshlandi */
                in_word = 1;
                w++;
            }
        }
    }
    if (n < 0) {
        fprintf(stderr, "wc: %s: %s\n", name ? name : "-", strerror(errno));
        return 1;
    }
    report(l, w, c, name);
    tl += l, tw += w, tc += c;
    return 0;
}

int main(int argc, char **argv)
{
    int i = 1;
    for (; i < argc && argv[i][0] == '-' && argv[i][1]; i++) {
        for (const char *o = argv[i] + 1; *o; o++) {
            if (*o == 'l') opt_l = 1;
            else if (*o == 'w') opt_w = 1;
            else if (*o == 'c') opt_c = 1;
            else {
                fprintf(stderr, "ishlatish: wc [-lwc] [fayl]...\n");
                return 1;
            }
        }
    }
    if (!opt_l && !opt_w && !opt_c)
        opt_l = opt_w = opt_c = 1;
    if (i == argc)
        return count(STDIN_FILENO, NULL);
    int status = 0, files = argc - i;
    for (; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "wc: %s: %s\n", argv[i], strerror(errno));
            status = 1;
            continue;
        }
        status |= count(fd, argv[i]);
        close(fd);
    }
    if (files > 1)
        report(tl, tw, tc, "jami");
    return status;
}
