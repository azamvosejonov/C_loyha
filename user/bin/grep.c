/* =============================================================================
 *  user/bin/grep.c - qatorlarda matn qidirish:  grep [-i] [-v] [-n] [-c] <naqsh> [fayl]...
 *
 *  Naqsh - oddiy satr (regex emas). Qo'shimcha: '^' boshida - qator boshi,
 *  '$' oxirida - qator oxiri. To'liq regex (Thompson NFA) - yaxshi mashq.
 * ============================================================================= */
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool opt_i, opt_v, opt_n, opt_c;
static const char *pattern;
static size_t plen;
static bool anchor_start, anchor_end;

static bool eq(char a, char b)
{
    return opt_i ? tolower((unsigned char)a) == tolower((unsigned char)b) : a == b;
}

static bool match_at(const char *s)
{
    for (size_t i = 0; i < plen; i++)
        if (!s[i] || !eq(s[i], pattern[i]))
            return false;
    return !anchor_end || s[plen] == '\0';
}

static bool matches(const char *line)
{
    if (anchor_start)
        return match_at(line);
    for (const char *s = line;; s++) {
        if (match_at(s))
            return true;
        if (!*s)
            return false;
    }
}

static int grep(FILE *f, const char *name, bool show_name)
{
    char line[2048];
    long lineno = 0, count = 0;
    while (fgets(line, sizeof(line), f)) {
        lineno++;
        size_t len = strlen(line);
        bool nl = len && line[len - 1] == '\n';
        if (nl)
            line[len - 1] = '\0';
        if (matches(line) == opt_v)
            continue;
        count++;
        if (opt_c)
            continue;
        if (show_name)
            printf("%s:", name);
        if (opt_n)
            printf("%ld:", lineno);
        printf("%s\n", line);
    }
    if (opt_c)
        printf(show_name ? "%s:%ld\n" : "%.0s%ld\n", name, count);
    return count ? 0 : 1;               /* grep an'anasi: 0 = topildi, 1 = topilmadi */
}

int main(int argc, char **argv)
{
    int i = 1;
    for (; i < argc && argv[i][0] == '-' && argv[i][1]; i++) {
        for (const char *o = argv[i] + 1; *o; o++) {
            switch (*o) {
            case 'i': opt_i = true; break;
            case 'v': opt_v = true; break;
            case 'n': opt_n = true; break;
            case 'c': opt_c = true; break;
            default:
                fprintf(stderr, "grep: noma'lum bayroq -%c\n", *o);
                return 2;
            }
        }
    }
    if (i >= argc) {
        fprintf(stderr, "ishlatish: grep [-ivnc] <naqsh> [fayl]...\n");
        return 2;
    }
    pattern = argv[i++];
    if (*pattern == '^') {
        anchor_start = true;
        pattern++;
    }
    plen = strlen(pattern);
    if (plen && pattern[plen - 1] == '$') {
        anchor_end = true;
        plen--;
    }
    if (i == argc)
        return grep(stdin, "-", false);
    int status = 1;
    bool many = argc - i > 1;
    for (; i < argc; i++) {
        FILE *f = fopen(argv[i], "r");
        if (!f) {
            fprintf(stderr, "grep: %s: %s\n", argv[i], strerror(errno));
            status = 2;
            continue;
        }
        if (grep(f, argv[i], many) == 0 && status != 2)
            status = 0;
        fclose(f);
    }
    return status;
}
