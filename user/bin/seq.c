/* user/bin/seq.c - sonlar ketma-ketligi:  seq 5  |  seq 2 10  |  seq 0 5 100 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    long first = 1, step = 1, last;
    if (argc == 2) {
        last = atoi(argv[1]);
    } else if (argc == 3) {
        first = atoi(argv[1]);
        last = atoi(argv[2]);
    } else if (argc == 4) {
        first = atoi(argv[1]);
        step = atoi(argv[2]);
        last = atoi(argv[3]);
    } else {
        fprintf(stderr, "ishlatish: seq [boshi [qadam]] oxiri\n");
        return 1;
    }
    if (step == 0)
        return 1;
    for (long i = first; step > 0 ? i <= last : i >= last; i += step)
        printf("%ld\n", i);
    return 0;
}
