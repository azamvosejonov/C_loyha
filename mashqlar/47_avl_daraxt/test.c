#include "test.h"
#include "mashq.h"

/* Daraxtni tekshirish: BST tartibi, saqlangan balandlik to'g'riligi, muvozanat.
 * Qaytaradi: haqiqiy balandlik yoki -1 (xato topilsa). */
static long elementlar;
static int tekshir(const struct avl *t, long pastki, long yuqori)
{
    if (!t)
        return 0;
    elementlar++;
    if (t->kalit <= pastki || t->kalit >= yuqori)
        return -1;
    int hl = tekshir(t->chap, pastki, t->kalit), hr = tekshir(t->ong, t->kalit, yuqori);
    if (hl < 0 || hr < 0)
        return -1;
    int h = 1 + (hl > hr ? hl : hr);
    if (t->balandlik != h || hl - hr > 1 || hr - hl > 1)
        return -1;
    return h;
}

static int togri(const struct avl *t, long kutilgan_soni)
{
    elementlar = 0;
    int h = tekshir(t, LONG_MIN, LONG_MAX);
    return h >= 0 && elementlar == kutilgan_soni;
}

int main(void)
{
    TEST_BOSHLA();
    struct avl *t = NULL;
    BOLIM("kichik daraxt");
    int k[] = { 30, 20, 10, 25, 40, 50, 5, 27, 26 };
    for (int i = 0; i < 9; i++)
        t = avl_qosh(t, k[i]);
    CHECK(togri(t, 9));
    CHECK(t && t->balandlik <= 4);
    CHECK(avl_bormi(t, 27) && avl_bormi(t, 5) && !avl_bormi(t, 28));
    t = avl_qosh(t, 27);                         /* takror - qo'shilmaydi */
    CHECK(togri(t, 9));

    BOLIM("1..100000 tartib bilan (oddiy BST'ning eng yomon holati)");
    for (int i = 1; i <= 100000; i++)
        t = avl_qosh(t, i * 100 + 1);
    CHECK(togri(t, 100009));
    printf("   (balandlik: %d)\n", t ? t->balandlik : 0);
    CHECK(t && t->balandlik <= 25);              /* 1.44 * log2(n) dan kam */
    int ok = 1;
    for (int i = 1; i <= 100000; i += 997)
        ok &= avl_bormi(t, i * 100 + 1) && !avl_bormi(t, i * 100 + 2);
    CHECK(ok);

    BOLIM("o'chirish");
    t = avl_ochir(t, 25);
    t = avl_ochir(t, 30);
    t = avl_ochir(t, 12345);                     /* yo'q */
    CHECK(togri(t, 100007));
    CHECK(!avl_bormi(t, 25) && !avl_bormi(t, 30) && avl_bormi(t, 27));
    for (int i = 1; i <= 100000; i += 2)
        t = avl_ochir(t, i * 100 + 1);
    CHECK(togri(t, 50007));
    CHECK(t && t->balandlik <= 23);
    for (int i = 2; i <= 100000; i += 2)
        t = avl_ochir(t, i * 100 + 1);
    for (int i = 0; i < 9; i++)
        t = avl_ochir(t, k[i]);
    CHECK(t == NULL);
    avl_ozod(t);

    BOLIM("avl_ozod (LeakSanitizer tekshiradi)");
    for (int i = 0; i < 1000; i++)
        t = avl_qosh(t, (i * 7919) % 1000);
    CHECK(togri(t, 1000));
    avl_ozod(t);
    TEST_TUGADI();
}
