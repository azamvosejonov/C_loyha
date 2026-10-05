/* fault_lab.c - sahifa xatolarini (page fault) HAQIQIY yadro bilan o'lchash: demand paging, takroriy murojaat, fork va COW, madvise */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#define SAHIFALAR 256                           /* 256 x 4 KB = 1 MB (2 MB li "katta sahifa"ga yetmaydi: oddiy 4 KB sahifalar) */

static long xatolar(void)                       /* shu jarayonning "kichik" page fault'lari soni (diskka murojaatsiz) */
{
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
    return r.ru_minflt;
}

static long sahifa_hajm;

/* bo'lakning har sahifasiga bittadan bayt yozadi */
static void tegib_chiq(char *p, int sahifalar)
{
    for (int i = 0; i < sahifalar; i++)
        p[(long)i * sahifa_hajm] = 1;
}

static void chiqar(const char *nom, long xato, int sahifalar)
{
    double nisbat = (double)xato / sahifalar;
    const char *baho = nisbat < 0.05 ? "deyarli yo'q" : (nisbat > 0.9 && nisbat < 1.2 ? "~1 sahifaga 1 ta" : "boshqacha");
    printf("  %-44s %4ld xato  (%.2f / sahifa: %s)\n", nom, xato, nisbat, baho);
}

int main(void)
{
    sahifa_hajm = sysconf(_SC_PAGESIZE);
    printf("sahifa hajmi: %ld bayt, bo'lak: %d sahifa\n\n", sahifa_hajm, SAHIFALAR);

    printf("1) Demand paging: mmap xotirani 'va'da qiladi', sahifa haqiqatda tegilganda beriladi\n");
    long a0 = xatolar();
    char *p = mmap(NULL, (size_t)SAHIFALAR * (size_t)sahifa_hajm, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    chiqar("mmap qilingandan keyin (hali tegilmadi)", xatolar() - a0, SAHIFALAR);

    long a1 = xatolar();
    tegib_chiq(p, SAHIFALAR);
    chiqar("1-marta har sahifaga yozdik", xatolar() - a1, SAHIFALAR);

    long a2 = xatolar();
    tegib_chiq(p, SAHIFALAR);
    chiqar("2-marta (sahifalar allaqachon xotirada)", xatolar() - a2, SAHIFALAR);

    printf("\n2) madvise(DONTNEED): sahifalarni yadroga qaytaramiz\n");
    madvise(p, (size_t)SAHIFALAR * (size_t)sahifa_hajm, MADV_DONTNEED);
    long a3 = xatolar();
    tegib_chiq(p, SAHIFALAR);
    chiqar("qaytargandan keyin yana yozdik", xatolar() - a3, SAHIFALAR);

    printf("\n3) fork + copy-on-write: bola ota xotirasini 'baham ko'radi', yozganda nusxalaydi\n");
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        long b0 = xatolar();
        long yig = 0;
        for (int i = 0; i < SAHIFALAR; i++)
            yig += p[(long)i * sahifa_hajm];    /* faqat O'QIYMIZ */
        chiqar("bola: faqat o'qidi (ota bilan umumiy sahifalar)", xatolar() - b0, SAHIFALAR);
        long b1 = xatolar();
        tegib_chiq(p, SAHIFALAR);               /* endi YOZAMIZ: har sahifa uchun nusxa kerak */
        chiqar("bola: har sahifaga yozdi (COW nusxalari)", xatolar() - b1, SAHIFALAR);
        fflush(stdout);
        _exit(yig == SAHIFALAR ? 0 : 1);
    }
    int holat;
    waitpid(pid, &holat, 0);
    printf("  bola tugadi, ota esa sahifalarini o'z holicha saqladi: p[0] = %d\n", p[0]);

    munmap(p, (size_t)SAHIFALAR * (size_t)sahifa_hajm);
    return 0;
}
