/* =============================================================================
 *  user/bin/cat.c - fayllarni ketma-ket chiqarish (con-CAT-enate)
 * =============================================================================
 *
 *  Klassik Unix o'qish tsikli: open -> read (0 qaytguncha) -> close.
 *  Argument berilmasa yoki "-" bo'lsa - standart kirishdan (stdin) o'qiydi.
 *  Shuning uchun cat pipe o'rtasida ham ishlaydi:  ls | cat | wc
 *
 *  stdio (printf) ishlatilmaydi: baytlarni o'zgartirmasdan, katta bo'laklarda
 *  to'g'ridan-to'g'ri write() bilan uzatamiz - bu eng tez yo'l.
 * ============================================================================= */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char buf[16 * 1024];

/* Hammasini yozish: write() qisman yozishi mumkin (masalan, pipe to'lsa). */
static int write_all(int fd, const char *p, size_t len)
{
    while (len) {
        ssize_t n = write(fd, p, len);
        if (n < 0)
            return -1;
        p += n;
        len -= (size_t)n;
    }
    return 0;
}

static int cat_fd(int fd, const char *name)
{
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        if (write_all(STDOUT_FILENO, buf, (size_t)n) < 0) {
            perror("cat: yozish");
            return 1;
        }
    }
    if (n < 0) {
        fprintf(stderr, "cat: %s: %s\n", name, strerror(errno));
        return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return cat_fd(STDIN_FILENO, "-");
    int status = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-") == 0) {
            status |= cat_fd(STDIN_FILENO, "-");
            continue;
        }
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "cat: %s: %s\n", argv[i], strerror(errno));
            status = 1;
            continue;
        }
        status |= cat_fd(fd, argv[i]);
        close(fd);
    }
    return status;
}
