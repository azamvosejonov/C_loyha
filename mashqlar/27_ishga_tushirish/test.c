#include <signal.h>

#include "test.h"
#include "mashq.h"

#define ARGV(...) ((char *const[]){ __VA_ARGS__, NULL })

int main(void)
{
    TEST_BOSHLA();
    BOLIM("chiqish kodlari");
    CHECK_INT(ishga_tushir(ARGV("true")), 0);
    CHECK_INT(ishga_tushir(ARGV("false")), 1);
    CHECK_INT(ishga_tushir(ARGV("sh", "-c", "exit 42")), 42);
    CHECK_INT(ishga_tushir(ARGV("sh", "-c", "exit 255")), 255);
    BOLIM("argumentlar uzatiladi");
    CHECK_INT(ishga_tushir(ARGV("test", "abc", "=", "abc")), 0);
    CHECK_INT(ishga_tushir(ARGV("test", "abc", "=", "abd")), 1);
    BOLIM("dastur topilmadi -> 127");
    CHECK_INT(ishga_tushir(ARGV("bunday_dastur_yoq_xyz")), 127);
    BOLIM("signal bilan o'ldi -> 128 + signal");
    CHECK_INT(ishga_tushir(ARGV("sh", "-c", "kill -TERM $$")), 128 + SIGTERM);
    CHECK_INT(ishga_tushir(ARGV("sh", "-c", "kill -KILL $$")), 128 + SIGKILL);
    BOLIM("ketma-ket 50 marta (zombi jarayonlar qolmasin)");
    int ok = 1;
    for (int i = 0; i < 50; i++)
        ok &= ishga_tushir(ARGV("true")) == 0;
    CHECK(ok);
    TEST_TUGADI();
}
