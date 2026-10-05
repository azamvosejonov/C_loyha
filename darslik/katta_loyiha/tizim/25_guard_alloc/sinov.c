/* sinov.c - har xato turini alohida jarayonda sinaydi va ushlanganini yoki ushlanmaganini ko'rsatadi */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "guard.h"

typedef void (*sinov_fn)(void);

static void togri(void)
{
    char *p = gmalloc(64, GUARD_OXIRI);
    memset(p, 'a', 64);                         /* aynan 64 bayt: chegaradan chiqmaymiz */
    gfree(p);
}

static void overflow_oxiri(void)
{
    char *p = gmalloc(64, GUARD_OXIRI);
    p[64] = 'X';                                /* 65-bayt: chegaradan 1 bayt tashqarida */
}

static void underflow_oxiri(void)
{
    char *p = gmalloc(64, GUARD_OXIRI);
    p[-1] = 'X';                                /* boshidan oldin: bu rejimda qo'riqchi yo'q */
}

static void underflow_boshi(void)
{
    char *p = gmalloc(64, GUARD_BOSHI);
    p[-1] = 'X';
}

static void free_dan_keyin(void)
{
    char *p = gmalloc(64, GUARD_OXIRI);
    gfree(p);
    p[0] = 'X';                                 /* bo'shatilgan xotiraga yozish */
}

static void ikki_free(void)
{
    char *p = gmalloc(64, GUARD_OXIRI);
    gfree(p);
    if (gfree(p) != 0)
        exit(3);                                /* ajratuvchi o'zi aniqladi: maxsus kod bilan chiqamiz */
}

static void oddiy_malloc_overflow(void)
{
    char *p = malloc(64);
    p[64] = 'X';                                /* heap da jim buzilish: hech narsa qulamaydi */
    (void)p;
}

static void bajar(const char *nom, sinov_fn f, int xato_bor)    /* xato_bor: sinovda ataylab xato bormi */
{
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        f();
        _exit(0);
    }
    int holat;
    waitpid(pid, &holat, 0);
    const char *natija;
    if (WIFSIGNALED(holat) && WTERMSIG(holat) == SIGSEGV)
        natija = "USHLANDI: SIGSEGV (darhol, xato yuz bergan qatorda)";
    else if (WIFEXITED(holat) && WEXITSTATUS(holat) == 3)
        natija = "USHLANDI: ajratuvchi xatoni o'zi aniqladi";
    else if (WIFEXITED(holat) && WEXITSTATUS(holat) == 0)
        natija = xato_bor ? "ushlanmadi (xato bor, lekin dastur xatosiz tugagandek ko'rindi!)" : "xatosiz tugadi (to'g'ri)";
    else
        natija = "kutilmagan natija";
    printf("  %-36s %s\n", nom, natija);
}

int main(void)
{
    printf("Har sinov alohida jarayonda (xato bo'lsa faqat o'sha jarayon o'ladi):\n");
    bajar("to'g'ri ishlatish", togri, 0);
    bajar("overflow +1 bayt (oxiri rejimi)", overflow_oxiri, 1);
    bajar("underflow -1 bayt (oxiri rejimi)", underflow_oxiri, 1);
    bajar("underflow -1 bayt (boshi rejimi)", underflow_boshi, 1);
    bajar("free dan keyin yozish", free_dan_keyin, 1);
    bajar("ikki marta free", ikki_free, 1);
    bajar("oddiy malloc: overflow +1 bayt", oddiy_malloc_overflow, 1);
    return 0;
}
