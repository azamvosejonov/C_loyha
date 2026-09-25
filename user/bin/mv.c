/* =============================================================================
 *  user/bin/mv.c - ko'chirish/qayta nomlash: rename() syscall'i
 * =============================================================================
 *  Bitta fayl tizimi ichida mv ma'lumotlarni NUSXALAMAYDI - faqat papka
 *  yozuvini ko'chiradi (1 GB fayl ham bir zumda). Boshqa fayl tizimiga
 *  ko'chirishda rename EXDEV qaytaradi - unda nusxalab o'chirish kerak (mashq).
 * ============================================================================= */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "ishlatish: mv <manba>... <nishon>\n");
        return 1;
    }
    const char *target = argv[argc - 1];
    struct stat st;
    int target_is_dir = stat(target, &st) == 0 && S_ISDIR(st.st_mode);
    if (argc > 3 && !target_is_dir) {
        fprintf(stderr, "mv: %s: papka emas\n", target);
        return 1;
    }
    int status = 0;
    for (int i = 1; i < argc - 1; i++) {
        char dst[PATH_MAX];
        if (target_is_dir) {
            const char *b = strrchr(argv[i], '/');
            snprintf(dst, sizeof(dst), "%s/%s", target, b ? b + 1 : argv[i]);
        } else {
            snprintf(dst, sizeof(dst), "%s", target);
        }
        if (rename(argv[i], dst) < 0) {
            fprintf(stderr, "mv: %s -> %s: %s\n", argv[i], dst, strerror(errno));
            status = 1;
        }
    }
    return status;
}
