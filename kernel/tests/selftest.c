/* =============================================================================
 *  tests/selftest.c - yadro komponentlarining unit testlari
 * =============================================================================
 *
 *  NEGA YADRO ICHIDA TEST:
 *    Yadro kodini oddiy Linux dasturi sifatida test qilish qiyin - u apparatga,
 *    sahifa jadvallariga, uzilishlarga bog'liq. Shuning uchun testlarni yadroning
 *    O'ZIDA, haqiqiy muhitda ishga tushiramiz. Natija serial portga chiqadi va
 *    tools/test.sh uni tekshiradi ("SELFTEST: ... PASSED" satrini qidiradi).
 *
 *  Har bir qatlam o'z testini qo'shadi. Kuchli jamoalarda qoida: "testsiz kod -
 *  tugallanmagan kod". Ayniqsa xotira allocatorlari kabi hamma narsa tayanadigan
 *  qismlar uchun.
 * ============================================================================= */
#include "tests/selftest.h"

#include "arch/cpu.h"
#include "drivers/pit.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/heap.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "proc/process.h"

static int tests_run;
static int tests_failed;

/* Test tekshiruvi: xato bo'lsa panic QILMAYDI - xabar berib, davom etadi,
 * shunda bitta xato boshqa testlarni yashirmaydi. */
#define CHECK(expr)                                                          \
    do {                                                                     \
        tests_run++;                                                         \
        if (!(expr)) {                                                       \
            tests_failed++;                                                  \
            kprintf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, #expr);      \
        }                                                                    \
    } while (0)

/* ---- PMM testlari ----------------------------------------------------------- */
static void test_pmm(void)
{
    kprintf("[test] pmm...\n");
    size_t free_before = pmm_free_frames_count();

    /* Bitta freym: tekislangan, nol emas, 1 MB dan yuqori. */
    uint64_t a = pmm_alloc_frame();
    CHECK(a != 0);
    CHECK(a % PAGE_SIZE == 0);
    CHECK(a >= 0x100000);
    CHECK(pmm_free_frames_count() == free_before - 1);

    /* Ikkinchi freym birinchisidan farq qilishi kerak. */
    uint64_t b = pmm_alloc_frame();
    CHECK(b != 0 && b != a);

    /* Ajratilgan freymga yozib o'qish mumkin (identity mapping orqali). */
    volatile uint64_t *p = (volatile uint64_t *)(uintptr_t)a;
    p[0] = 0xDEADBEEFCAFEBABEULL;
    p[511] = 0x1234;
    CHECK(p[0] == 0xDEADBEEFCAFEBABEULL && p[511] == 0x1234);

    pmm_free_frame(a);
    pmm_free_frame(b);
    CHECK(pmm_free_frames_count() == free_before);

    /* Ketma-ket 16 freym: hammasi band bo'lishi va qaytarilishi. */
    uint64_t run = pmm_alloc_frames(16);
    CHECK(run != 0 && run % PAGE_SIZE == 0);
    CHECK(pmm_free_frames_count() == free_before - 16);
    pmm_free_frames(run, 16);
    CHECK(pmm_free_frames_count() == free_before);

    /* Ko'p freymlarni ajratib, hammasi har xil ekanini tekshirish. */
    static uint64_t frames[256];
    for (int i = 0; i < 256; i++)
        frames[i] = pmm_alloc_frame();
    int unique = 1;
    for (int i = 0; i < 256 && unique; i++)
        for (int j = i + 1; j < 256; j++)
            if (frames[i] == frames[j] || frames[i] == 0) {
                unique = 0;
                break;
            }
    CHECK(unique);
    for (int i = 0; i < 256; i++)
        pmm_free_frame(frames[i]);
    CHECK(pmm_free_frames_count() == free_before);
}

/* ---- VMM testlari ----------------------------------------------------------- */
static void test_vmm(void)
{
    kprintf("[test] vmm...\n");
    size_t free_before = pmm_free_frames_count();

    uint64_t as = vmm_create_address_space();
    CHECK(as != 0);

    /* Yadro qismi yangi maydonda ham ko'rinadi (umumiy PD). */
    CHECK(vmm_translate(as, 0xB8000, NULL) == 0xB8000);
    /* 0-sahifa xaritalanmagan (NULL himoyasi). */
    CHECK(vmm_translate(as, 0x0, NULL) == 0);

    /* User sahifasini bog'lab, fizik manzil orqali yozamiz... */
    uint64_t va = USER_SPACE_START + 0x5000;
    uint64_t frame = pmm_alloc_frame();
    CHECK(vmm_map_page(as, va, frame, PTE_WRITABLE | PTE_USER));
    CHECK(vmm_translate(as, va + 0x123, NULL) == frame + 0x123);
    *(volatile uint32_t *)(uintptr_t)(frame + 8) = 0xC0FFEE;

    /* ...keyin shu manzil maydoniga O'TIB, VIRTUAL manzil orqali o'qiymiz. */
    uint64_t flags = irq_save();
    vmm_switch(as);
    uint32_t seen = *(volatile uint32_t *)(uintptr_t)(va + 8);
    vmm_switch(vmm_kernel_pml4());
    irq_restore(flags);
    CHECK(seen == 0xC0FFEE);

    /* Yadro maydonida bu virtual manzil umuman yo'q - izolyatsiya! */
    CHECK(vmm_translate(vmm_kernel_pml4(), va, NULL) == 0);

    /* Foydalanuvchi buferini tekshirish (syscall xavfsizligi). */
    CHECK(vmm_user_range_ok(as, va, 100, true));
    CHECK(!vmm_user_range_ok(as, va + PAGE_SIZE, 1, false));   /* xaritalanmagan */
    CHECK(!vmm_user_range_ok(as, 0x100000, 16, false));        /* yadro hududi */
    CHECK(!vmm_user_range_ok(as, va, (size_t)-1, false));      /* overflow */

    /* copy_to_space: ikki sahifa chegarasidan o'tadigan nusxa. */
    CHECK(vmm_map_anonymous(as, va + PAGE_SIZE, 1, PTE_WRITABLE | PTE_USER));
    static const char msg[] = "sahifa chegarasidan o'tuvchi satr";
    uint64_t cross = va + PAGE_SIZE - 10;
    CHECK(vmm_copy_to_space(as, cross, msg, sizeof(msg)));
    char back[sizeof(msg)];
    for (size_t i = 0; i < sizeof(msg); i++)
        back[i] = *(char *)(uintptr_t)vmm_translate(as, cross + i, NULL);
    CHECK(memcmp(back, msg, sizeof(msg)) == 0);

    /* Unmap. */
    CHECK(vmm_unmap_page(as, va) == frame);
    CHECK(vmm_translate(as, va, NULL) == 0);
    pmm_free_frame(frame);

    /* Yo'q qilish: HAMMA freymlar (jadvallar ham) qaytishi kerak - xotira
     * oqishi (memory leak) bo'lmasligi shart. */
    vmm_destroy_address_space(as);
    CHECK(pmm_free_frames_count() == free_before);
}

/* ---- Heap testlari ---------------------------------------------------------- */

/* Oddiy psevdo-tasodifiy sonlar generatori (LCG - Linear Congruential Generator).
 * Testlar TAKRORLANADIGAN bo'lishi uchun doim bir xil urug'dan (seed) boshlaymiz. */
static uint64_t rng_state = 12345;
static uint32_t rng_next(void)
{
    rng_state = rng_state * 6364136223846793005ULL + 1442695040888963407ULL;
    return (uint32_t)(rng_state >> 33);
}

static void test_heap(void)
{
    kprintf("[test] heap...\n");
    struct heap_stats before, after;
    heap_get_stats(&before);

    /* Tekislash: har bir ko'rsatkich 16 ga karrali. */
    void *a = kmalloc(1);
    void *b = kmalloc(17);
    void *c = kmalloc(1000);
    void *d = kmalloc(5000);            /* katta ajratma (bir necha sahifa) */
    CHECK(a && b && c && d);
    CHECK(((uintptr_t)a & 15) == 0 && ((uintptr_t)b & 15) == 0);
    CHECK(((uintptr_t)c & 15) == 0 && ((uintptr_t)d & 15) == 0);
    memset(d, 0xAB, 5000);              /* butun hajmga yozish mumkin */
    kfree(a);
    kfree(b);
    kfree(c);
    kfree(d);

    /* kzalloc nollaydi (hatto ilgari zaharlangan obyektni ham). */
    uint8_t *z = kzalloc(64);
    int all_zero = 1;
    for (int i = 0; i < 64; i++)
        all_zero &= (z[i] == 0);
    CHECK(all_zero);
    kfree(z);

    /* STRESS TEST: 500 ta tasodifiy o'lchamdagi blok, har biriga o'z naqshini
     * yozamiz, keyin hammasini tekshiramiz. Agar ikki blok bir-birining ustiga
     * tushsa (allocator xatosi), naqsh buziladi. */
    enum { N = 500 };
    static uint8_t *ptrs[N];
    static uint16_t sizes[N];
    for (int i = 0; i < N; i++) {
        sizes[i] = (uint16_t)(1 + rng_next() % 3000);
        ptrs[i] = kmalloc(sizes[i]);
        if (ptrs[i])
            memset(ptrs[i], (uint8_t)i, sizes[i]);
    }
    int ok = 1;
    for (int i = 0; i < N && ok; i++) {
        if (!ptrs[i]) {
            ok = 0;
            break;
        }
        for (int j = 0; j < sizes[i]; j++)
            if (ptrs[i][j] != (uint8_t)i) {
                ok = 0;
                break;
            }
    }
    CHECK(ok);

    /* Tasodifiy tartibda bo'shatish (Fisher-Yates aralashtirish). */
    for (int i = N - 1; i > 0; i--) {
        int j = rng_next() % (i + 1);
        uint8_t *tp = ptrs[i];
        ptrs[i] = ptrs[j];
        ptrs[j] = tp;
    }
    for (int i = 0; i < N; i++)
        kfree(ptrs[i]);

    /* Hamma narsa qaytdimi? */
    heap_get_stats(&after);
    CHECK(after.bytes_in_use == before.bytes_in_use);
    CHECK(after.large_pages == before.large_pages);
    CHECK(after.alloc_count - before.alloc_count == after.free_count - before.free_count);
}

/* ---- Jarayon / scheduler testlari ------------------------------------------- */

static volatile int shared_counter;
static volatile int flag_from_other_thread;

/* Umumiy hisoblagichni n marta oshiradi. Har 100 qadamda yield - navbatni
 * boshqalarga beradi, shunda oqimlar haqiqatan aralashib ishlaydi. */
static int counter_thread(void *arg)
{
    int n = (int)(intptr_t)arg;
    for (int i = 0; i < n; i++) {
        uint64_t f = irq_save();        /* ++ atomar emas: o'qish-qo'shish-yozish! */
        shared_counter++;
        irq_restore(f);
        if (i % 100 == 0)
            proc_yield();
    }
    return n;
}

static int sleeper_thread(void *arg)
{
    (void)arg;
    uint64_t t0 = timer_ticks();
    proc_sleep_ms(100);                 /* 100 ms = 10 tik */
    return (int)(timer_ticks() - t0);
}

/* HECH QACHON yield qilmaydi va uxlamaydi. Agar boshqa oqim flag ni o'rnata
 * olsa - demak taymer bizni MAJBURAN to'xtatgan (preemption ishlayapti). */
static int spinner_thread(void *arg)
{
    (void)arg;
    uint64_t t0 = timer_ticks();
    while (!flag_from_other_thread && timer_ticks() - t0 < 300)
        ;                               /* band kutish (busy wait) */
    return flag_from_other_thread;
}

static int flag_setter_thread(void *arg)
{
    (void)arg;
    flag_from_other_thread = 1;
    return 0;
}

static void test_proc(void)
{
    kprintf("[test] proc/scheduler...\n");
    size_t free_before = pmm_free_frames_count();
    int code;

    /* 3 ta oqim umumiy hisoblagichni oshiradi. */
    shared_counter = 0;
    int p1 = proc_create_kernel_thread("t-count1", counter_thread, (void *)1000);
    int p2 = proc_create_kernel_thread("t-count2", counter_thread, (void *)1000);
    int p3 = proc_create_kernel_thread("t-count3", counter_thread, (void *)1000);
    CHECK(p1 > 0 && p2 > 0 && p3 > 0);
    CHECK(proc_wait(p1, &code, false) == p1 && code == 1000);
    CHECK(proc_wait(p2, &code, false) == p2 && code == 1000);
    CHECK(proc_wait(p3, &code, false) == p3 && code == 1000);
    CHECK(shared_counter == 3000);

    /* Uxlash: kamida 10 tik o'tishi kerak. */
    int ps = proc_create_kernel_thread("t-sleep", sleeper_thread, NULL);
    CHECK(proc_wait(ps, &code, false) == ps && code >= 10);

    /* Preemption: spinner hech qachon CPU'ni o'zi bermaydi. */
    flag_from_other_thread = 0;
    int sp = proc_create_kernel_thread("t-spin", spinner_thread, NULL);
    int fs = proc_create_kernel_thread("t-flag", flag_setter_thread, NULL);
    CHECK(proc_wait(sp, &code, false) == sp && code == 1);
    CHECK(proc_wait(fs, &code, false) == fs);

    /* Bola yo'q - wait darhol -1 qaytaradi (abadiy osilib qolmaydi). */
    CHECK(proc_wait(-1, &code, false) == -1);

    /* Barcha yadro steklari qaytarildimi? */
    CHECK(pmm_free_frames_count() == free_before);
}

void selftest_run(void)
{
    kprintf("[test] ===== SELFTEST boshlandi =====\n");
    tests_run = tests_failed = 0;

    test_pmm();
    test_vmm();
    test_heap();
    test_proc();

    if (tests_failed == 0)
        kprintf("[test] SELFTEST: %d ta tekshiruv, hammasi PASSED\n", tests_run);
    else
        kprintf("[test] SELFTEST: %d / %d FAILED\n", tests_failed, tests_run);
}
