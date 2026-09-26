#include "test.h"
#include "mashq.h"

int main(void)
{
    TEST_BOSHLA();
    size_t n = 3000001;
    int *a = malloc(n * sizeof(int));
    long long kutilgan = 0;
    for (size_t i = 0; i < n; i++) {
        a[i] = (int)(i % 1000) - 300;
        kutilgan += a[i];
    }
    BOLIM("parallel_yigindi");
    CHECK_INT(parallel_yigindi(a, n, 1), kutilgan);
    CHECK_INT(parallel_yigindi(a, n, 2), kutilgan);
    CHECK_INT(parallel_yigindi(a, n, 4), kutilgan);
    CHECK_INT(parallel_yigindi(a, n, 7), kutilgan);
    CHECK_INT(parallel_yigindi(a, 5, 8), a[0] + a[1] + a[2] + a[3] + a[4]);   /* oqim > element */
    free(a);
    BOLIM("hisoblagich (poyga holati)");
    CHECK_INT(hisoblagich(1, 1000), 1000);
    CHECK_INT(hisoblagich(8, 200000), 1600000);
    CHECK_INT(hisoblagich(16, 50000), 800000);
    TEST_TUGADI();
}
