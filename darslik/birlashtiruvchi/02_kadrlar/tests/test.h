/* test.h - juda kichik test tizimi. Har TODO uchun BOSHLA/TEKSHIR.../TUGAT guruhi:
   natija "[ OK ] T1 foiz: 5/5 to'g'ri" yoki "[XATO] T1 foiz: 3/5 to'g'ri" ko'rinishida chiqadi. */
#ifndef TEST_H
#define TEST_H

#include <stdio.h>

static const char *g_nom;
static int g_jami, g_xato, t_xato;

#define BOSHLA(nom) do { g_nom = (nom); g_jami = 0; g_xato = 0; } while (0)

/* butun sonlarni solishtiradi; xato bo'lsa nima olinganini va nima kutilganini ko'rsatadi */
#define TEKSHIR(ifoda, kutilgan)                                                                       \
    do {                                                                                               \
        long long t_olingan = (long long)(ifoda), t_kutilgan = (long long)(kutilgan);                  \
        g_jami++;                                                                                      \
        if (t_olingan != t_kutilgan) {                                                                 \
            g_xato++;                                                                                  \
            printf("        %s  ->  olindi %lld, kutilgan %lld   (%s:%d)\n", #ifoda, t_olingan, \
                   t_kutilgan, __FILE__, __LINE__);                                                    \
        }                                                                                              \
    } while (0)

/* shart (rost/yolg'on) tekshiruvi */
#define TEKSHIR_SHART(shart)                                                                           \
    do {                                                                                               \
        g_jami++;                                                                                      \
        if (!(shart)) {                                                                                \
            g_xato++;                                                                                  \
            printf("        shart bajarilmadi: %s   (%s:%d)\n", #shart, __FILE__, __LINE__);      \
        }                                                                                              \
    } while (0)

#define TUGAT()                                                                                        \
    do {                                                                                               \
        printf("  [%s] %s: %d/%d to'g'ri\n", g_xato ? "XATO" : " OK ", g_nom, g_jami - g_xato, g_jami); \
        t_xato += g_xato;                                                                              \
    } while (0)

#define YAKUN() return t_xato ? 1 : 0

#endif
