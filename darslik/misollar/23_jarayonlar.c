/* =============================================================================
 *  23_jarayonlar.c - vaqtni bo'lish va kontekst almashishni ko'rish   (23-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -O2 23_jarayonlar.c -o jarayonlar && ./jarayonlar
 *
 *  Nima bo'ladi: CPU yadrolaridan KO'P jarayon (2 x yadrolar) bir xil ish qiladi.
 *  Scheduler ularga vaqtni bo'lib beradi: har biri devor soati bo'yicha uzoqroq
 *  ishlaydi (navbat kutadi), majburiy kontekst almashishlar (preemption) ko'payadi.
 *
 *  Kutilgan natija (raqamlar kompyuterga qarab):
 *      yadrolar: 4, jarayonlar: 8
 *      bola 0: CPU vaqti 0.50 s, devor vaqti 1.00 s, majburiy almashishlar: 120, ixtiyoriy: 1
 *      ...
 *      (CPU vaqti ~ bir xil; devor vaqti ~ jarayonlar/yadrolar marta katta)
 *
 *  Sinab ko'ring: nice -n 19 ./jarayonlar - past ustuvorlik; boshqa terminalda top.
 * ============================================================================= */
#include <stdio.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static double hozir(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

int main(void)
{
    long yadro = sysconf(_SC_NPROCESSORS_ONLN);
    int n = (int)yadro * 2;
    printf("yadrolar: %ld, jarayonlar: %d\n", yadro, n);
    fflush(stdout);
    for (int i = 0; i < n; i++) {
        if (fork() == 0) {
            double t0 = hozir();
            volatile unsigned long x = 0;
            for (unsigned long k = 0; k < 200000000UL; k++)   /* sof CPU ishi */
                x += k;
            struct rusage ru;
            getrusage(RUSAGE_SELF, &ru);
            printf("bola %d: CPU vaqti %.2f s, devor vaqti %.2f s, majburiy almashishlar: %ld, ixtiyoriy: %ld\n",
                   i, (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1e6, hozir() - t0,
                   ru.ru_nivcsw, ru.ru_nvcsw);
            fflush(stdout);                     /* _exit buferni TOZALAMAYDI (12-bob) - usiz matn yo'qoladi */
            _exit(0);
        }
    }
    while (wait(NULL) > 0)
        ;
    return 0;
}
