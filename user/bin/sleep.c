/* user/bin/sleep.c - N soniya kutish (jarayon UXLAYDI - CPU ishlatmaydi). */
#include <stdio.h>
#include <stdlib.h>

#include "myos.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "ishlatish: sleep <soniya>\n");
        return 1;
    }
    /* "0.5" kabi kasr sonlarni ham qo'llaymiz (strtod yo'q - qo'lda). */
    char *end;
    long sec = strtol(argv[1], &end, 10);
    long ms = sec * 1000;
    if (*end == '.') {
        long scale = 100;
        for (end++; *end >= '0' && *end <= '9' && scale; end++, scale /= 10)
            ms += (*end - '0') * scale;
    }
    sleep_ms((uint64_t)ms);
    return 0;
}
