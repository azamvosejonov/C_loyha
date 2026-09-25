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
 *  Kuchli jamoalarda qoida: "testsiz kod - tugallanmagan kod". Ayniqsa xotira
 *  allocatorlari kabi hamma narsa tayanadigan qismlar uchun. Har bir test
 *  oxirida "hamma narsa joyiga qaytdimi?" (xotira oqishi yo'qmi) tekshiriladi.
 * ============================================================================= */
#include "tests/selftest.h"

#include "arch/cpu.h"
#include "arch/percpu.h"
#include "drivers/pit.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/pmm.h"
#include "mm/slab.h"
#include "mm/vmalloc.h"
#include "mm/vmm.h"
#include "lib/spinlock.h"
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

/* Takrorlanadigan psevdo-tasodifiy sonlar (LCG). */
static uint64_t rng_state = 12345;
static uint32_t rng_next(void)
{
    rng_state = rng_state * 6364136223846793005ULL + 1442695040888963407ULL;
    return (uint32_t)(rng_state >> 33);
}

/* ---- Buddy allocator -------------------------------------------------------- */
static void test_buddy(void)
{
    kprintf("[test] buddy allocator...\n");
    size_t free_before = pmm_free_pages_count();

    struct page *a = alloc_pages(0, 0);
    struct page *b = alloc_pages(0, 0);
    CHECK(a && b && a != b);
    CHECK(pmm_free_pages_count() == free_before - 2);

    /* 2^3 = 8 sahifalik blok 8 sahifaga TEKISLANGAN bo'lishi kerak (buddy xossasi). */
    struct page *c = alloc_pages(3, 0);
    CHECK(c && (page_to_pfn(c) & 7) == 0);

    /* GFP_ZERO: nollangan. */
    struct page *z = alloc_pages(1, GFP_ZERO);
    uint64_t *zp = page_to_virt(z);
    int zero = 1;
    for (size_t i = 0; i < 2 * PAGE_SIZE / 8; i++)
        zero &= zp[i] == 0;
    CHECK(zero);

    /* GFP_DMA32: 4 GB dan past. */
    struct page *d = alloc_pages(0, GFP_DMA32);
    CHECK(d && page_to_phys(d) < 0x100000000UL);

    free_pages(a, 0);
    free_pages(b, 0);
    free_pages(c, 3);
    free_pages(z, 1);
    free_pages(d, 0);
    CHECK(pmm_free_pages_count() == free_before);

    /* Stress: 300 ta turli tartibdagi blok, har biriga naqsh; ustma-ust tushmasligi. */
    enum { N = 300 };
    static struct page *blocks[N];
    static uint8_t orders[N];
    for (int i = 0; i < N; i++) {
        orders[i] = (uint8_t)(rng_next() % 4);
        blocks[i] = alloc_pages(orders[i], 0);
        if (blocks[i])
            memset(page_to_virt(blocks[i]), i & 0xFF, PAGE_SIZE << orders[i]);
    }
    int ok = 1;
    for (int i = 0; i < N; i++) {
        if (!blocks[i]) {
            ok = 0;
            continue;
        }
        uint8_t *p = page_to_virt(blocks[i]);
        for (size_t j = 0; j < (PAGE_SIZE << orders[i]); j += 97)
            ok &= p[j] == (i & 0xFF);
    }
    CHECK(ok);
    for (int i = N - 1; i >= 0; i -= 2)     /* aralash tartibda qaytaramiz */
        if (blocks[i])
            free_pages(blocks[i], orders[i]);
    for (int i = N - 2; i >= 0; i -= 2)
        if (blocks[i])
            free_pages(blocks[i], orders[i]);
    /* Birlashtirish (coalescing) to'g'ri ishlasa, hisoblagich aynan qaytadi. */
    CHECK(pmm_free_pages_count() == free_before);
}

/* ---- Slab / kmalloc --------------------------------------------------------- */
struct test_obj {
    uint64_t magic;
    char data[40];
};

static void test_obj_ctor(void *p)
{
    ((struct test_obj *)p)->magic = 0x1234;
}

static void test_slab(void)
{
    kprintf("[test] slab / kmalloc...\n");
    struct heap_stats before, after;
    heap_get_stats(&before);

    void *a = kmalloc(1), *b = kmalloc(100), *c = kmalloc(5000), *d = kmalloc(20000);
    CHECK(a && b && c && d);
    CHECK(((uint64_t)a & 7) == 0 && ((uint64_t)b & 7) == 0);
    CHECK(ksize(b) >= 100 && ksize(d) >= 20000);
    memset(d, 0xAB, 20000);
    kfree(a);
    kfree(b);
    kfree(c);
    kfree(d);

    /* Maxsus kesh + konstruktor. */
    struct kmem_cache *cache = kmem_cache_create("test_obj", sizeof(struct test_obj), 16,
                                                 test_obj_ctor);
    CHECK(cache != NULL);
    struct test_obj *objs[50];
    for (int i = 0; i < 50; i++) {
        objs[i] = kmem_cache_alloc(cache);
        CHECK(objs[i] && objs[i]->magic == 0x1234 && ((uint64_t)objs[i] & 15) == 0);
    }
    for (int i = 0; i < 50; i++)
        kmem_cache_free(cache, objs[i]);
    CHECK(cache->active_objs == 0);
    kmem_cache_destroy(cache);

    /* Stress: 500 ta tasodifiy o'lcham, naqsh, tasodifiy tartibda qaytarish. */
    enum { N = 500 };
    static uint8_t *ptrs[N];
    static uint16_t sizes[N];
    for (int i = 0; i < N; i++) {
        sizes[i] = (uint16_t)(1 + rng_next() % 12000);
        ptrs[i] = kmalloc(sizes[i]);
        if (ptrs[i])
            memset(ptrs[i], (uint8_t)i, sizes[i]);
    }
    int ok = 1;
    for (int i = 0; i < N; i++) {
        if (!ptrs[i]) {
            ok = 0;
            continue;
        }
        for (int j = 0; j < sizes[i]; j++)
            ok &= ptrs[i][j] == (uint8_t)i;
    }
    CHECK(ok);
    for (int i = N - 1; i > 0; i--) {
        int j = rng_next() % (i + 1);
        uint8_t *t = ptrs[i];
        ptrs[i] = ptrs[j];
        ptrs[j] = t;
    }
    for (int i = 0; i < N; i++)
        kfree(ptrs[i]);

    heap_get_stats(&after);
    CHECK(after.bytes_in_use == before.bytes_in_use);
    CHECK(after.large_pages == before.large_pages);
}

/* ---- vmalloc ---------------------------------------------------------------- */
static void test_vmalloc(void)
{
    kprintf("[test] vmalloc / ioremap...\n");
    size_t free_before = pmm_free_pages_count();

    uint8_t *v = vmalloc(3 * PAGE_SIZE);
    CHECK(v != NULL && (uint64_t)v >= VMALLOC_START && (uint64_t)v < VMALLOC_END);
    memset(v, 0x5A, 3 * PAGE_SIZE);
    CHECK(v[0] == 0x5A && v[3 * PAGE_SIZE - 1] == 0x5A);
    /* Himoya sahifalari haqiqatan xaritalanmagan. */
    uint64_t k = vmm_kernel_pml4();
    CHECK(vmm_translate(k, (uint64_t)v - PAGE_SIZE, NULL) == 0);
    CHECK(vmm_translate(k, (uint64_t)v + 3 * PAGE_SIZE, NULL) == 0);

    uint8_t *w = vmalloc(PAGE_SIZE);
    CHECK(w && (w >= v + 4 * PAGE_SIZE || w + 2 * PAGE_SIZE <= v));   /* ustma-ust emas */
    vfree(v);
    vfree(w);
    CHECK(vmm_translate(k, (uint64_t)v, NULL) == 0);

    /* ioremap: fizik xotirani ikkinchi manzilga UC rejimida ko'rish. */
    struct page *p = alloc_pages(0, GFP_ZERO);
    ((volatile uint32_t *)page_to_virt(p))[3] = 0xFEEDFACE;
    volatile uint32_t *io = ioremap(page_to_phys(p), 64);
    CHECK(io && io[3] == 0xFEEDFACE);
    iounmap((void *)io);
    free_pages(p, 0);

    CHECK(pmm_free_pages_count() == free_before);
}

/* ---- Manzil maydonlari ------------------------------------------------------ */
static void test_vmm(void)
{
    kprintf("[test] vmm (manzil maydonlari)...\n");
    size_t free_before = pmm_free_pages_count();

    uint64_t as = vmm_create_address_space();
    CHECK(as != 0);
    /* Yadro yuqori yarimi yangi maydonda ham ko'rinadi. */
    CHECK(vmm_translate(as, (uint64_t)test_vmm, NULL) != 0);
    CHECK(vmm_translate(as, 0, NULL) == 0);             /* NULL */

    uint64_t va = USER_SPACE_START + 0x5000;
    CHECK(vmm_map_anonymous(as, va, 2, PTE_USER | PTE_WRITABLE | pte_nx));
    uint64_t phys = vmm_translate(as, va + 0x10, NULL);
    CHECK(phys != 0);
    *(volatile uint32_t *)phys_to_virt(phys) = 0xC0FFEE;

    push_off();                         /* shu orada boshqa jarayonga o'tib ketmaylik (CR3!) */
    vmm_switch(as);
    uint32_t seen = *(volatile uint32_t *)(va + 0x10);  /* user manzil orqali o'qish */
    vmm_switch(vmm_kernel_pml4());
    pop_off();
    CHECK(seen == 0xC0FFEE);
    CHECK(vmm_translate(vmm_kernel_pml4(), va, NULL) == 0);   /* izolyatsiya */

    CHECK(vmm_user_range_ok(as, va, 2 * PAGE_SIZE, true));
    CHECK(!vmm_user_range_ok(as, va, 3 * PAGE_SIZE, false));   /* 3-sahifa yo'q */
    CHECK(!vmm_user_range_ok(as, (uint64_t)test_vmm, 1, false)); /* yadro manzili */
    CHECK(!vmm_user_range_ok(as, va, (size_t)-1, false));        /* overflow */

    static const char msg[] = "sahifa chegarasidan o'tuvchi satr";
    uint64_t cross = va + PAGE_SIZE - 10;
    CHECK(vmm_copy_to_space(as, cross, msg, sizeof(msg)));
    char back[sizeof(msg)];
    for (size_t i = 0; i < sizeof(msg); i++)
        back[i] = *(char *)phys_to_virt(vmm_translate(as, cross + i, NULL));
    CHECK(memcmp(back, msg, sizeof(msg)) == 0);
    CHECK(vmm_count_user_pages(as) == 2);

    vmm_destroy_address_space(as);
    CHECK(pmm_free_pages_count() == free_before);       /* jadvallar ham qaytdi */
}

/* ---- Jarayonlar / scheduler ------------------------------------------------- */
static volatile int shared_counter;
static volatile int flag_from_other_thread;

static int counter_thread(void *arg)
{
    int n = (int)(intptr_t)arg;
    for (int i = 0; i < n; i++) {
        /* `shared_counter++` ATOMAR EMAS: o'qish-qo'shish-yozish. Ikki CPU bir
         * vaqtda bajarsa, ikkala qo'shish ham bitta bo'lib qoladi. `lock xadd`
         * instruksiyasi (__atomic_add_fetch) buni bitta bo'linmas amal qiladi. */
        __atomic_add_fetch(&shared_counter, 1, __ATOMIC_RELAXED);
        if (i % 100 == 0)
            proc_yield();
    }
    return n % 256;                     /* chiqish kodi 8 bit (wait holat so'zi) */
}

/* Spinlock testi: ODDIY (atomar bo'lmagan) ++ ni qulf bilan himoyalaymiz. Qulf
 * noto'g'ri ishlasa, ko'p CPU'da ba'zi qo'shishlar "yo'qoladi". */
static spinlock_t test_lock = SPINLOCK_INIT("selftest");
static volatile uint64_t locked_counter;

static int lock_thread(void *arg)
{
    int n = (int)(intptr_t)arg;
    for (int i = 0; i < n; i++) {
        spin_lock(&test_lock);
        uint64_t v = locked_counter;    /* o'qish */
        for (volatile int d = 0; d < 20; d++)
            ;                           /* poyga oynasini ataylab kengaytiramiz */
        locked_counter = v + 1;         /* yozish */
        spin_unlock(&test_lock);
    }
    return 0;
}

static int sleeper_thread(void *arg)
{
    (void)arg;
    uint64_t t0 = timer_ticks();
    proc_sleep_ms(100);
    return (int)(timer_ticks() - t0);
}

/* Hech qachon o'zi CPU'ni bermaydi: boshqa oqim flag'ni o'rnatsa - preemption ishlaydi. */
static int spinner_thread(void *arg)
{
    (void)arg;
    uint64_t t0 = timer_ticks();
    while (!flag_from_other_thread && timer_ticks() - t0 < 300)
        ;
    return flag_from_other_thread;
}

static int flag_setter_thread(void *arg)
{
    (void)arg;
    flag_from_other_thread = 1;
    return 0;
}

/* proc_wait holat so'zidan exit() kodi (abi.h: WEXITSTATUS). */
#define EXIT_CODE(st) (((st) >> 8) & 0xFF)

static void test_proc(void)
{
    kprintf("[test] jarayonlar / scheduler...\n");
    size_t free_before = pmm_free_pages_count();
    int code;

    shared_counter = 0;
    int p1 = proc_create_kernel_thread("t-count1", counter_thread, (void *)1000);
    int p2 = proc_create_kernel_thread("t-count2", counter_thread, (void *)1000);
    int p3 = proc_create_kernel_thread("t-count3", counter_thread, (void *)1000);
    CHECK(p1 > 0 && p2 > 0 && p3 > 0);
    CHECK(proc_wait(p1, &code, 0) == p1 && EXIT_CODE(code) == 1000 % 256);
    CHECK(proc_wait(p2, &code, 0) == p2 && EXIT_CODE(code) == 1000 % 256);
    CHECK(proc_wait(p3, &code, 0) == p3 && EXIT_CODE(code) == 1000 % 256);
    CHECK(shared_counter == 3000);

    locked_counter = 0;
    int l1 = proc_create_kernel_thread("t-lock1", lock_thread, (void *)20000);
    int l2 = proc_create_kernel_thread("t-lock2", lock_thread, (void *)20000);
    int l3 = proc_create_kernel_thread("t-lock3", lock_thread, (void *)20000);
    proc_wait(l1, &code, 0);
    proc_wait(l2, &code, 0);
    proc_wait(l3, &code, 0);
    CHECK(locked_counter == 60000);

    int ps = proc_create_kernel_thread("t-sleep", sleeper_thread, NULL);
    CHECK(proc_wait(ps, &code, 0) == ps && EXIT_CODE(code) >= 10);

    flag_from_other_thread = 0;
    int sp = proc_create_kernel_thread("t-spin", spinner_thread, NULL);
    int fs = proc_create_kernel_thread("t-flag", flag_setter_thread, NULL);
    CHECK(proc_wait(sp, &code, 0) == sp && EXIT_CODE(code) == 1);
    CHECK(proc_wait(fs, &code, 0) == fs);

    CHECK(proc_wait(-1, &code, 0) == -ECHILD);         /* bolalar qolmadi */
    CHECK(pmm_free_pages_count() == free_before);       /* yadro steklari qaytdi */
}

void selftest_run(void)
{
    kprintf("[test] ===== SELFTEST boshlandi =====\n");
    tests_run = tests_failed = 0;

    test_buddy();
    test_slab();
    test_vmalloc();
    test_vmm();
    test_proc();

    if (tests_failed == 0)
        kprintf("[test] SELFTEST: %d ta tekshiruv, hammasi PASSED\n", tests_run);
    else
        kprintf("[test] SELFTEST: %d / %d FAILED\n", tests_failed, tests_run);
}
