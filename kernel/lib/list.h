/* =============================================================================
 *  lib/list.h - "intrusive" ikki tomonlama halqali ro'yxat (Linux uslubida)
 * =============================================================================
 *
 *  ODDIY RO'YXAT:     node { data; next; }   - har bir element uchun alohida
 *                     tugun ajratiladi (malloc!), ma'lumotga ko'rsatkich orqali.
 *  INTRUSIVE RO'YXAT: struct process { ...; struct list_head node; }
 *                     - bog'lanish maydoni OBYEKTNING ICHIDA. Qo'shimcha xotira
 *                     ajratilmaydi (yadroda bu juda muhim: xotira ajratish
 *                     muvaffaqiyatsiz bo'lishi mumkin, ro'yxatga qo'shish esa
 *                     hech qachon xato bermasligi kerak). Tugundan obyektga
 *                     container_of() orqali qaytamiz.
 *
 *  HALQALI + "BOSH" TUGUN: bo'sh ro'yxatda head.next == head.prev == &head.
 *  Shu tufayli qo'shish/o'chirishda NULL tekshiruvlari umuman kerak emas -
 *  kod qisqa va xatosiz bo'ladi.
 *
 *       ┌──────────────────────────────────────────────┐
 *       ▼                                              │
 *     [head] ⇄ [a.node] ⇄ [b.node] ⇄ [c.node] ─────────┘
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "lib/common.h"

struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

/* Statik e'lon:  static LIST_HEAD(my_list); */
#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

static inline void list_init(struct list_head *h)
{
    h->next = h;
    h->prev = h;
}

static inline bool list_empty(const struct list_head *h)
{
    return h->next == h;
}

/* n ni prev va next orasiga qo'yish (ichki yordamchi). */
static inline void __list_insert(struct list_head *n, struct list_head *prev,
                                 struct list_head *next)
{
    next->prev = n;
    n->next = next;
    n->prev = prev;
    prev->next = n;
}

/* Boshiga qo'shish (stek kabi - LIFO). */
static inline void list_add(struct list_head *n, struct list_head *head)
{
    __list_insert(n, head, head->next);
}

/* Oxiriga qo'shish (navbat kabi - FIFO). */
static inline void list_add_tail(struct list_head *n, struct list_head *head)
{
    __list_insert(n, head->prev, head);
}

/* Ro'yxatdan chiqarish. Tugun keyin "o'z-o'ziga" ko'rsatadi - list_empty(n)
 * true bo'ladi, ikkinchi list_del zararsiz. */
static inline void list_del(struct list_head *n)
{
    n->prev->next = n->next;
    n->next->prev = n->prev;
    list_init(n);
}

/* Tugun hozir biror ro'yxatdami? (list_init/list_del dan keyin - yo'q) */
static inline bool list_linked(const struct list_head *n)
{
    return n->next != n;
}

#define list_entry(ptr, type, member) container_of(ptr, type, member)
#define list_first_entry(head, type, member) list_entry((head)->next, type, member)

/* Ro'yxat bo'ylab yurish. pos - obyekt ko'rsatkichi. */
#define list_for_each_entry(pos, head, member)                                   \
    for (pos = list_entry((head)->next, __typeof__(*pos), member);               \
         &pos->member != (head);                                                 \
         pos = list_entry(pos->member.next, __typeof__(*pos), member))

/* Yurish davomida joriy elementni O'CHIRISH mumkin bo'lgan variant. */
#define list_for_each_entry_safe(pos, tmp, head, member)                         \
    for (pos = list_entry((head)->next, __typeof__(*pos), member),               \
         tmp = list_entry(pos->member.next, __typeof__(*pos), member);           \
         &pos->member != (head);                                                 \
         pos = tmp, tmp = list_entry(tmp->member.next, __typeof__(*tmp), member))
