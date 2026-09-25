/* =============================================================================
 *  user/bin/rm.c - fayllarni o'chirish: rm [-r] [-f] <yo'l>...
 * =============================================================================
 *
 *  unlink() nomni papkadan olib tashlaydi. Fayl ma'lumotlari esa oxirgi nom
 *  VA oxirgi ochiq deskriptor yo'qolgandagina bo'shatiladi (inode refcount).
 *  -r: papkani rekursiv - avval ichidagilarni, keyin o'zini (rmdir).
 *  -f: yo'q fayl uchun xato chiqarmaslik.
 * ============================================================================= */
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int recursive, force;

static int rm_path(const char *path)
{
    struct stat st;
    if (stat(path, &st) < 0) {
        if (force && errno == ENOENT)
            return 0;
        fprintf(stderr, "rm: %s: %s\n", path, strerror(errno));
        return 1;
    }
    if (S_ISDIR(st.st_mode)) {
        if (!recursive) {
            fprintf(stderr, "rm: %s: bu papka (-r kerak)\n", path);
            return 1;
        }
        DIR *d = opendir(path);
        if (!d) {
            fprintf(stderr, "rm: %s: %s\n", path, strerror(errno));
            return 1;
        }
        /* Avval barcha nomlarni YIG'AMIZ, keyin o'chiramiz. O'qish davomida
         * o'chirish xavfli: readdir pozitsiyasi (tmpfs'da - indeks) siljiydi va
         * ba'zi yozuvlar o'tkazib yuboriladi. */
        char **names = NULL;
        size_t n = 0, cap = 0;
        struct dirent *de;
        while ((de = readdir(d)) != NULL) {
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
                continue;
            if (n == cap) {
                cap = cap ? cap * 2 : 16;
                names = realloc(names, cap * sizeof(*names));
            }
            names[n++] = strdup(de->d_name);
        }
        closedir(d);
        int status = 0;
        for (size_t i = 0; i < n; i++) {
            char child[PATH_MAX];
            snprintf(child, sizeof(child), "%s/%s", path, names[i]);
            status |= rm_path(child);
            free(names[i]);
        }
        free(names);
        if (rmdir(path) < 0) {
            fprintf(stderr, "rm: %s: %s\n", path, strerror(errno));
            return 1;
        }
        return status;
    }
    if (unlink(path) < 0) {
        fprintf(stderr, "rm: %s: %s\n", path, strerror(errno));
        return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    int i = 1;
    for (; i < argc && argv[i][0] == '-' && argv[i][1]; i++) {
        for (const char *o = argv[i] + 1; *o; o++) {
            if (*o == 'r' || *o == 'R')
                recursive = 1;
            else if (*o == 'f')
                force = 1;
            else {
                fprintf(stderr, "rm: noma'lum bayroq -%c\n", *o);
                return 1;
            }
        }
    }
    if (i >= argc) {
        if (force)
            return 0;
        fprintf(stderr, "ishlatish: rm [-rf] <yo'l>...\n");
        return 1;
    }
    int status = 0;
    for (; i < argc; i++)
        status |= rm_path(argv[i]);
    return status;
}
