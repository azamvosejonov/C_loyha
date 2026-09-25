/* =============================================================================
 *  user/bin/tail.c - oxirgi N qator (sukut 10):  tail [-n N] [fayl]
 *
 *  Kirish pipe bo'lishi mumkin (orqaga lseek qilib bo'lmaydi), shuning uchun
 *  hammasini o'qiymiz va faqat oxirgi N qatorni HALQA buferda saqlaymiz.
 * ============================================================================= */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE 1024

int main(int argc, char **argv)
{
    long n = 10;
    int i = 1;
    if (i + 1 < argc && strcmp(argv[i], "-n") == 0) {
        n = atoi(argv[i + 1]);
        i += 2;
    } else if (i < argc && argv[i][0] == '-' && argv[i][1] >= '0' && argv[i][1] <= '9') {
        n = atoi(argv[i] + 1);
        i++;
    }
    if (n <= 0)
        return 0;
    FILE *f = stdin;
    if (i < argc && !(f = fopen(argv[i], "r"))) {
        fprintf(stderr, "tail: %s: %s\n", argv[i], strerror(errno));
        return 1;
    }
    char (*ring)[LINE] = calloc((size_t)n, LINE);
    if (!ring) {
        fprintf(stderr, "tail: xotira yetmadi\n");
        return 1;
    }
    long count = 0;
    while (fgets(ring[count % n], LINE, f))
        count++;
    for (long k = count > n ? count - n : 0; k < count; k++)
        fputs(ring[k % n], stdout);
    return 0;
}
