/* guard.c - mmap + mprotect bilan qurilgan ajratuvchi. Har ajratish uchun alohida sahifalar + himoyalangan "qo'riqchi" sahifa */
#define _GNU_SOURCE
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "guard.h"

#define MAKS_BLOK 64

struct blok {
    char *taban;                                /* mmap qaytargan boshlanish */
    size_t uzunlik;                             /* butun mmap hajmi (sahifalarda karrali) */
    void *foydalanuvchi;                        /* foydalanuvchiga berilgan manzil */
    int bosh;                                   /* 1 - jadvalda band */
    int bo_shatilgan;                           /* 1 - gfree qilingan (karantinda: hamma ruxsat olib tashlangan) */
};

static struct blok jadval[MAKS_BLOK];

void *gmalloc(size_t n, enum guard_rejim rejim)
{
    size_t sahifa = (size_t)sysconf(_SC_PAGESIZE);
    size_t malumot_sahifalar = (n + sahifa - 1) / sahifa;
    if (malumot_sahifalar == 0)
        malumot_sahifalar = 1;
    size_t jami = (malumot_sahifalar + 1) * sahifa;         /* + 1 ta qo'riqchi sahifa */

    struct blok *b = NULL;
    for (int i = 0; i < MAKS_BLOK; i++)
        if (!jadval[i].bosh) {
            b = &jadval[i];
            break;
        }
    if (!b)
        return NULL;

    char *taban = mmap(NULL, jami, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (taban == MAP_FAILED)
        return NULL;

    char *foy;
    if (rejim == GUARD_OXIRI) {
        char *qoriqchi = taban + malumot_sahifalar * sahifa;    /* ma'lumot sahifalaridan KEYIN */
        mprotect(qoriqchi, sahifa, PROT_NONE);                  /* unga tegsangiz - SIGSEGV */
        foy = qoriqchi - n;                                     /* xotira qo'riqchi sahifaga aynan taqalgan */
    } else {
        mprotect(taban, sahifa, PROT_NONE);                     /* qo'riqchi sahifa BOSHIDA */
        foy = taban + sahifa;
    }

    b->taban = taban;
    b->uzunlik = jami;
    b->foydalanuvchi = foy;
    b->bosh = 1;
    b->bo_shatilgan = 0;
    return foy;
}

int gfree(void *p)
{
    for (int i = 0; i < MAKS_BLOK; i++) {
        struct blok *b = &jadval[i];
        if (b->bosh && b->foydalanuvchi == p) {
            if (b->bo_shatilgan)
                return -1;                      /* allaqachon free qilingan: ikki marta free */
            mprotect(b->taban, b->uzunlik, PROT_NONE);          /* hamma ruxsat olinadi: keyingi har qanday tegish qulatadi */
            b->bo_shatilgan = 1;                /* munmap QILMAYMIZ: manzil boshqa narsaga berilib ketmasin (karantin) */
            return 0;
        }
    }
    return -1;                                  /* bunday ko'rsatkich bizniki emas */
}
