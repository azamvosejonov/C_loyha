/* =============================================================================
 *  user/bin/ls.c - papka tarkibi
 * =============================================================================
 *
 *  ls [-l] [-a] [yo'l]...
 *    -l  batafsil: tur va ruxsatlar, havolalar, hajm, o'zgartirilgan vaqt
 *    -a  yashirin fayllar ham ('.' bilan boshlanadiganlar, jumladan . va ..)
 *
 *  Ish tartibi: opendir -> readdir (getdents syscall) -> har bir nom uchun
 *  stat (batafsil ma'lumot) -> nom bo'yicha saralash -> chiqarish.
 *  Papka ichidagi nomlar tartibi fayl tizimiga bog'liq, shuning uchun
 *  haqiqiy ls kabi saralaymiz.
 * ============================================================================= */
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static bool opt_long, opt_all;

struct entry {
    char *name;
    struct stat st;
};

static int cmp_entry(const void *a, const void *b)
{
    return strcmp(((const struct entry *)a)->name, ((const struct entry *)b)->name);
}

/* "drwxr-xr-x" ko'rinishi: birinchi harf - tur, keyin 3 guruh (egasi, guruh, boshqalar). */
static void mode_string(unsigned mode, char out[11])
{
    out[0] = S_ISDIR(mode) ? 'd' : S_ISCHR(mode) ? 'c' : S_ISBLK(mode) ? 'b'
           : S_ISFIFO(mode) ? 'p' : ((mode & S_IFMT) == S_IFLNK) ? 'l' : '-';
    const char *rwx = "rwxrwxrwx";
    for (int i = 0; i < 9; i++)
        out[1 + i] = (mode & (0400 >> i)) ? rwx[i] : '-';
    out[10] = '\0';
}

static void print_entry(const struct entry *e)
{
    if (!opt_long) {
        printf("%s%s\n", e->name, S_ISDIR(e->st.st_mode) ? "/" : "");
        return;
    }
    char mode[11];
    mode_string(e->st.st_mode, mode);
    struct tm tm;
    time_t t = (time_t)e->st.st_mtime;
    gmtime_r(&t, &tm);
    if (S_ISCHR(e->st.st_mode) || S_ISBLK(e->st.st_mode))   /* qurilma: hajm o'rniga major, minor */
        printf("%s %2u %4u, %4u ", mode, e->st.st_nlink, e->st.st_rdev >> 20,
               e->st.st_rdev & 0xFFFFF);
    else
        printf("%s %2u %10lu ", mode, e->st.st_nlink, e->st.st_size);
    printf("%04d-%02d-%02d %02d:%02d %s%s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
           tm.tm_hour, tm.tm_min, e->name, S_ISDIR(e->st.st_mode) ? "/" : "");
}

static int list_dir(const char *path)
{
    DIR *d = opendir(path);
    if (!d) {
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        return 1;
    }
    struct entry *ents = NULL;
    size_t n = 0, cap = 0;
    struct dirent *de;
    while ((de = readdir(d)) != NULL) {
        if (de->d_name[0] == '.' && !opt_all)
            continue;
        if (n == cap) {
            cap = cap ? cap * 2 : 32;
            ents = realloc(ents, cap * sizeof(*ents));
            if (!ents) {
                fprintf(stderr, "ls: xotira yetmadi\n");
                return 1;
            }
        }
        char full[PATH_MAX];
        snprintf(full, sizeof(full), "%s/%s", path, de->d_name);
        ents[n].name = strdup(de->d_name);
        if (stat(full, &ents[n].st) < 0)
            memset(&ents[n].st, 0, sizeof(ents[n].st));
        n++;
    }
    closedir(d);
    qsort(ents, n, sizeof(*ents), cmp_entry);
    uint64_t total = 0;
    for (size_t i = 0; i < n; i++) {
        print_entry(&ents[i]);
        total += ents[i].st.st_blocks;
        free(ents[i].name);
    }
    if (opt_long)
        printf("jami %lu KB\n", (total + 1) / 2);     /* 512 baytli bloklar -> KB (yuqoriga yaxlitlab) */
    free(ents);
    return 0;
}

int main(int argc, char **argv)
{
    int first = 1;
    for (; first < argc && argv[first][0] == '-' && argv[first][1]; first++) {
        for (const char *o = argv[first] + 1; *o; o++) {
            if (*o == 'l')
                opt_long = true;
            else if (*o == 'a')
                opt_all = true;
            else {
                fprintf(stderr, "ls: noma'lum bayroq -%c\nishlatish: ls [-la] [yo'l]...\n", *o);
                return 2;
            }
        }
    }
    if (first == argc)
        return list_dir(".");

    int status = 0;
    for (int i = first; i < argc; i++) {
        struct stat st;
        if (stat(argv[i], &st) < 0) {
            fprintf(stderr, "ls: %s: %s\n", argv[i], strerror(errno));
            status = 1;
            continue;
        }
        if (!S_ISDIR(st.st_mode)) {     /* oddiy fayl - o'zini ko'rsatamiz */
            struct entry e = { argv[i], st };
            print_entry(&e);
            continue;
        }
        if (argc - first > 1)
            printf("%s:\n", argv[i]);
        status |= list_dir(argv[i]);
    }
    return status;
}
