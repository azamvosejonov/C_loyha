#include "test.h"
#include "mashq.h"

#define TUGASH(...)                                                   \
    do {                                                              \
        int kut_[] = { __VA_ARGS__ };                                   \
        for (size_t i_ = 0; i_ < sizeof(kut_) / sizeof(kut_[0]); i_++)    \
            CHECK_INT(t[i_], kut_[i_]);                                 \
    } while (0)

int main(void)
{
    TEST_BOSHLA();
    int t[16];

    BOLIM("klassik misol (kvant = 2)");
    struct jarayon a[] = { { 1, 0, 5 }, { 2, 1, 3 }, { 3, 2, 1 }, { 4, 3, 2 }, { 5, 4, 3 } };
    memset(t, 0, sizeof(t));
    CHECK_INT(round_robin(a, 5, 2, t), 8);
    TUGASH(13, 12, 5, 9, 14);

    BOLIM("katta kvant = FCFS (birinchi kelgan - birinchi)");
    memset(t, 0, sizeof(t));
    CHECK_INT(round_robin(a, 5, 100, t), 4);
    TUGASH(5, 8, 9, 11, 14);

    BOLIM("kvant = 1");
    struct jarayon b[] = { { 1, 0, 3 }, { 2, 0, 3 } };
    memset(t, 0, sizeof(t));
    CHECK_INT(round_robin(b, 2, 1, t), 5);
    TUGASH(5, 6);

    BOLIM("CPU bo'sh turadi (kelishlar orasida bo'shliq)");
    struct jarayon c[] = { { 1, 2, 2 }, { 2, 10, 3 }, { 3, 11, 1 } };
    memset(t, 0, sizeof(t));
    CHECK_INT(round_robin(c, 3, 2, t), 2);
    TUGASH(4, 14, 13);

    BOLIM("yolg'iz jarayon - almashish yo'q");
    struct jarayon d[] = { { 7, 3, 10 } };
    memset(t, 0, sizeof(t));
    CHECK_INT(round_robin(d, 1, 3, t), 0);
    TUGASH(13);

    BOLIM("kirish vaqt bo'yicha saralanmagan");
    struct jarayon e[] = { { 1, 6, 2 }, { 2, 0, 4 }, { 3, 1, 2 } };
    memset(t, 0, sizeof(t));
    CHECK_INT(round_robin(e, 3, 2, t), 3);
    TUGASH(8, 6, 4);

    BOLIM("kvant tugashi va yangi kelish bir vaqtda (4-qoida)");
    struct jarayon f[] = { { 1, 0, 4 }, { 2, 2, 2 } };
    memset(t, 0, sizeof(t));
    CHECK_INT(round_robin(f, 2, 2, t), 2);
    TUGASH(6, 4);                            /* t=2 da: 2 kelgan -> navbatda 1 dan oldin */
    TEST_TUGADI();
}
