/* user/bin/sync.c - keshdagi o'zgarishlarni diskka yozish. */
#include <unistd.h>

int main(void)
{
    sync();
    return 0;
}
