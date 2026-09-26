#pragma once

#include <stddef.h>

struct jarayon {
    int id;
    int kelish;         /* qachon paydo bo'ladi (tik) */
    int ish;            /* qancha CPU vaqti kerak (tik), >= 1 */
};

int round_robin(const struct jarayon *j, size_t n, int kvant, int *tugash);
