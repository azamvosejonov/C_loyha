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
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/pmm.h"
#include "mm/vmm.h"

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

void selftest_run(void)
{
    kprintf("[test] ===== SELFTEST boshlandi =====\n");
    tests_run = tests_failed = 0;

    test_pmm();
    test_vmm();

    if (tests_failed == 0)
        kprintf("[test] SELFTEST: %d ta tekshiruv, hammasi PASSED\n", tests_run);
    else
        kprintf("[test] SELFTEST: %d / %d FAILED\n", tests_failed, tests_run);
}
