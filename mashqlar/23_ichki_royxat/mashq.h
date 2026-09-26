#pragma once

#include <stdbool.h>
#include <stddef.h>

/* Linux'ning include/linux/list.h dagi kabi "ichki" (intrusive) ro'yxat. */
struct list_head {
    struct list_head *next, *prev;
};

/* A'zo (member) manzilidan uni o'z ichiga olgan strukturaning manziliga. */
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

void list_init(struct list_head *h);
void list_add_tail(struct list_head *yangi, struct list_head *h);
void list_del(struct list_head *e);
bool list_empty(const struct list_head *h);

struct vazifa {
    int id;
    int ustuvorlik;
    struct list_head node;      /* ro'yxatga shu maydon orqali ulanadi */
};

int eng_ustuvor_id(struct list_head *h);
