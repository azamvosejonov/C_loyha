/* =============================================================================
 *  user/bin/forktest.c - fork, copy-on-write, exec va demand paging testlari
 * =============================================================================
 *
 *  1. COW: fork dan keyin bola global o'zgaruvchi va heap'ni o'zgartiradi -
 *     ota o'zining ESKI qiymatlarini ko'rishi kerak (xotira alohida!).
 *  2. Ko'p bola: 20 ta bola har biri o'z kodi bilan tugaydi; ota hammasini
 *     yig'ib oladi.
 *  3. exec: bola "echo" ga aylanadi.
 *  4. Demand paging: 32 MB mmap - bo'sh xotira deyarli O'ZGARMAYDI. Faqat
 *     tegilgan sahifalar ajratiladi.
 * ============================================================================= */
#include "ulib.h"

static int global_value = 100;
static int failures;

static void check(int ok, const char *what)
{
    printf("  [%s] %s\n", ok ? "OK" : "FAIL", what);
    if (!ok)
        failures++;
}

static uint64_t free_kb(void)
{
    struct myos_meminfo m;
    meminfo(&m);
    return m.free_pages * (m.page_size / 1024);
}

int main(void)
{
    printf("forktest: pid %d\n", getpid());

    /* --- 1. Copy-on-write --- */
    int *heap = malloc(sizeof(int));
    *heap = 7;
    int pid = fork();
    if (pid == 0) {
        global_value = 999;             /* bolaning NUSXASIGA yoziladi */
        *heap = 888;
        exit(global_value == 999 && *heap == 888 && getppid() > 0 ? 0 : 1);
    }
    int st = -1;
    wait(pid, &st, 0);
    check(st == 0, "bola o'z nusxasini o'zgartirdi");
    check(global_value == 100 && *heap == 7, "ota eski qiymatlarni ko'radi (COW)");

    /* --- 2. Ko'p bola --- */
    int pids[20], sum = 0;
    for (int i = 0; i < 20; i++) {
        pids[i] = fork();
        if (pids[i] == 0)
            exit(i + 1);                /* har bir bola o'z kodi bilan */
    }
    for (int i = 0; i < 20; i++) {
        wait(pids[i], &st, 0);
        sum += st;
    }
    check(sum == 210, "20 ta bola, chiqish kodlari yig'indisi 210");

    /* --- 3. exec --- */
    pid = fork();
    if (pid == 0) {
        char *argv[] = { "echo", "  exec: bola echo ga aylandi", NULL };
        exec("echo", argv);
        exit(99);                       /* bu yerga kelmasligi kerak */
    }
    wait(pid, &st, 0);
    check(st == 0, "exec ishladi");
    check(exec("mavjud_emas", NULL) < 0, "mavjud bo'lmagan dastur uchun exec -1 qaytaradi");

    /* --- 4. Demand paging --- */
    uint64_t before = free_kb();
    size_t len = 32 * 1024 * 1024;
    char *big = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS);
    check(big != MAP_FAILED, "32 MB mmap");
    uint64_t after_map = free_kb();
    printf("  mmap dan keyin: %lu KB ishlatildi (sahifalar hali yo'q)\n", before - after_map);
    check(before - after_map < 64, "mmap darhol xotira egallamaydi");
    for (size_t off = 0; off < len; off += 1024 * 1024)
        big[off] = 1;                   /* har 1 MB da bittadan sahifaga tegamiz */
    uint64_t after_touch = free_kb();
    printf("  32 ta sahifaga tekkandan keyin: %lu KB\n", before - after_touch);
    check(before - after_touch >= 32 * 4 && before - after_touch < 32 * 4 + 256,
          "faqat tegilgan sahifalar ajratildi (~128 KB)");
    munmap(big, len);
    check(free_kb() + 64 >= before, "munmap xotirani qaytardi");

    if (failures == 0)
        printf("forktest: PASSED\n");
    else
        printf("forktest: %d FAIL\n", failures);
    return failures;
}
