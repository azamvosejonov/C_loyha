#pragma once

/* Amal: natijani *natija ga yozadi, 0 - muvaffaqiyat, -1 - hisoblab bo'lmaydi. */
typedef int (*amal_fn)(int a, int b, int *natija);

struct amal {
    const char *nom;
    amal_fn fn;
};

/* yechim.c da ta'riflanadi. Oxirgi element: { NULL, NULL }. */
extern const struct amal amallar[];

int hisobla(const char *nom, int a, int b, int *natija);
