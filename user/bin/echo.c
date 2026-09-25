/* user/bin/echo.c - argumentlarni bitta qatorga chiqaradi. -n: oxirida '\n' yo'q. */
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    int i = 1, newline = 1;
    if (argc > 1 && strcmp(argv[1], "-n") == 0) {
        newline = 0;
        i++;
    }
    for (int first = i; i < argc; i++)
        printf(i > first ? " %s" : "%s", argv[i]);
    if (newline)
        putchar('\n');
    return 0;
}
