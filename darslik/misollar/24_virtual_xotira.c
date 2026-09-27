/* =============================================================================
 *  24_virtual_xotira.c - talab bo'yicha sahifalash, page fault'lar, COW, mmap (24-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -O2 24_virtual_xotira.c -o vx && ./vx
 *
 *  Kutilgan natija (taxminiy):
 *      mmap 100 MB: page fault'lar +0 (hali hech narsa ajratilmagan!)
 *      har 4096-baytga bittadan yozildi: page fault'lar +25600 (har sahifaga bitta)
 *      fork'dan keyin bola 10 sahifaga yozdi: bolada page fault'lar ~10 (COW nusxalash)
 *      ota o'z qiymatini ko'radi: 1 (bolaning o'zgarishi ko'rinmaydi)
 *
 *  Sinab ko'ring:
 *      1) MAP_PRIVATE ni MAP_SHARED ga o'zgartiring. Ota endi 2 ni ko'radi - nega?
 *      2) Yozish o'rniga faqat o'qing: `volatile char c = p[i]; (void)c;`. Fault'lar bo'ladimi?
 *         (Ha: har sahifaga umumiy "nol sahifa" ulanadi. Keyin yozsangiz - yana fault.)
 *      3) Fault'lar 25600 dan ancha kam chiqsa - Transparent Huge Pages (2 MB sahifalar) yoqilgan:
 *         cat /sys/kernel/mm/transparent_hugepage/enabled
 * ============================================================================= */
#include <stdio.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

static long kichik_faultlar(void)
{
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return ru.ru_minflt;                        /* diskka bormagan page fault'lar */
}

int main(void)
{
    size_t hajm = 100u << 20;
    long f0 = kichik_faultlar();
    char *p = mmap(NULL, hajm, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    printf("mmap 100 MB: page fault'lar +%ld (hali hech narsa ajratilmagan!)\n", kichik_faultlar() - f0);

    f0 = kichik_faultlar();
    for (size_t i = 0; i < hajm; i += 4096)
        p[i] = 1;                               /* har sahifaga birinchi murojaat - page fault */
    printf("har 4096-baytga bittadan yozildi: page fault'lar +%ld (har sahifaga bitta)\n",
           kichik_faultlar() - f0);

    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        long b0 = kichik_faultlar();
        for (int i = 0; i < 10; i++)
            p[(size_t)i * 4096] = 2;            /* umumiy sahifaga YOZISH -> COW nusxa */
        printf("fork'dan keyin bola 10 sahifaga yozdi: bolada page fault'lar %ld (COW nusxalash)\n",
               kichik_faultlar() - b0);
        fflush(stdout);                 /* _exit buferni TOZALAMAYDI (12-bob) - usiz matn yo'qoladi */
        _exit(0);
    }
    waitpid(pid, NULL, 0);
    printf("ota o'z qiymatini ko'radi: %d (bolaning o'zgarishi ko'rinmaydi)\n", p[0]);
    munmap(p, hajm);
    return 0;
}
