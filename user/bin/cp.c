/* =============================================================================
 *  user/bin/cp.c - faylni nusxalash: cp <manba> <nishon>  yoki  cp <fayllar>... <papka>
 * ============================================================================= */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char buf[32 * 1024];

static const char *base_name(const char *p)
{
    const char *s = strrchr(p, '/');
    return s ? s + 1 : p;
}

static int copy_file(const char *src, const char *dst)
{
    struct stat st;
    int in = open(src, O_RDONLY);
    if (in < 0 || fstat(in, &st) < 0) {
        fprintf(stderr, "cp: %s: %s\n", src, strerror(errno));
        if (in >= 0)
            close(in);
        return 1;
    }
    if (S_ISDIR(st.st_mode)) {
        fprintf(stderr, "cp: %s: papka (tashlab ketildi)\n", src);
        close(in);
        return 1;
    }
    int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode & 0777);
    if (out < 0) {
        fprintf(stderr, "cp: %s: %s\n", dst, strerror(errno));
        close(in);
        return 1;
    }
    int status = 0;
    ssize_t n;
    while ((n = read(in, buf, sizeof(buf))) > 0) {
        for (ssize_t done = 0; done < n;) {
            ssize_t w = write(out, buf + done, (size_t)(n - done));
            if (w < 0) {
                fprintf(stderr, "cp: %s: %s\n", dst, strerror(errno));
                status = 1;
                goto out;
            }
            done += w;
        }
    }
    if (n < 0) {
        fprintf(stderr, "cp: %s: %s\n", src, strerror(errno));
        status = 1;
    }
out:
    close(in);
    close(out);
    return status;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "ishlatish: cp <manba> <nishon>\n       cp <fayl>... <papka>\n");
        return 1;
    }
    const char *target = argv[argc - 1];
    struct stat st;
    int target_is_dir = stat(target, &st) == 0 && S_ISDIR(st.st_mode);
    if (argc > 3 && !target_is_dir) {
        fprintf(stderr, "cp: %s: papka emas\n", target);
        return 1;
    }
    int status = 0;
    for (int i = 1; i < argc - 1; i++) {
        char dst[PATH_MAX];
        if (target_is_dir)
            snprintf(dst, sizeof(dst), "%s/%s", target, base_name(argv[i]));
        else
            snprintf(dst, sizeof(dst), "%s", target);
        status |= copy_file(argv[i], dst);
    }
    return status;
}
