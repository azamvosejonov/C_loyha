#include "test.h"
#include "mashq.h"

/* Ro'yxatni id'lar satriga aylantirish (va halqa to'g'riligini tekshirish). */
static const char *korinish(struct list_head *h)
{
    static char buf[256];
    size_t k = 0;
    buf[0] = '\0';
    int i = 0;
    for (struct list_head *p = h->next; p && p != h && i < 20; p = p->next, i++) {
        if (p->next && p->next->prev != p)
            return "(prev ko'rsatkichi buzilgan)";
        struct vazifa *v = container_of(p, struct vazifa, node);
        k += (size_t)snprintf(buf + k, sizeof(buf) - k, k ? " %d" : "%d", v->id);
    }
    return buf;
}

int main(void)
{
    TEST_BOSHLA();
    struct list_head h = { NULL, NULL };
    BOLIM("list_init / list_empty");
    list_init(&h);
    CHECK(h.next == &h && h.prev == &h);
    if (h.next != &h)
        TEST_TUGADI();
    CHECK(list_empty(&h));
    CHECK_INT(eng_ustuvor_id(&h), -1);

    BOLIM("list_add_tail");
    struct vazifa v[5];
    int ust[5] = { 3, 9, 1, 9, 5 };
    for (int i = 0; i < 5; i++) {
        v[i].id = 100 + i;
        v[i].ustuvorlik = ust[i];
        list_add_tail(&v[i].node, &h);
    }
    CHECK(!list_empty(&h));
    CHECK_STR(korinish(&h), "100 101 102 103 104");
    CHECK(h.prev == &v[4].node);

    BOLIM("eng_ustuvor_id (container_of)");
    CHECK_INT(eng_ustuvor_id(&h), 101);

    BOLIM("list_del");
    list_del(&v[1].node);
    CHECK(v[1].node.next == NULL && v[1].node.prev == NULL);
    CHECK_STR(korinish(&h), "100 102 103 104");
    CHECK_INT(eng_ustuvor_id(&h), 103);
    list_del(&v[0].node);                   /* birinchisi */
    list_del(&v[4].node);                   /* oxirgisi */
    CHECK_STR(korinish(&h), "102 103");
    CHECK(h.prev == &v[3].node);
    list_del(&v[2].node);
    list_del(&v[3].node);
    CHECK(list_empty(&h));
    TEST_TUGADI();
}
