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

#include "lib/kprintf.h"
#include "mm/pmm.h"

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

void selftest_run(void)
{
    kprintf("[test] ===== SELFTEST boshlandi =====\n");
    tests_run = tests_failed = 0;

    test_pmm();

    if (tests_failed == 0)
        kprintf("[test] SELFTEST: %d ta tekshiruv, hammasi PASSED\n", tests_run);
    else
        kprintf("[test] SELFTEST: %d / %d FAILED\n", tests_failed, tests_run);
}
