/* =============================================================================
 *  user/bin/cat.c - fayllar tarkibini chiqarish
 *
 *  Klassik o'qish tsikli: open -> read (bo'lak-bo'lak, 0 qaytguncha) -> close.
 *  Katta faylni ham kichik (256 bayt) bufer bilan o'qiymiz - xotira tejaladi.
 * ============================================================================= */
#include "ulib.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("ishlatish: cat <fayl>...\n");
        return 1;
    }
    int status = 0;
    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i]);
        if (fd < 0) {
            printf("cat: %s: topilmadi\n", argv[i]);
            status = 1;
            continue;
        }
        char buf[256];
        long n;
        while ((n = read(fd, buf, sizeof(buf))) > 0)
            write(STDOUT, buf, (size_t)n);
        close(fd);
    }
    return status;
}
