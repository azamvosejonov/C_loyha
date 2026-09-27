/* =============================================================================
 *  17_assembly.c - inline assembly: rdtsc, cpuid, syscall      (darslik 17-bob)
 * =============================================================================
 *  Ishga tushirish (faqat x86-64):
 *      gcc -Wall -Wextra -g 17_assembly.c -o asm && ./asm
 *  Assembly'ni ko'rish:
 *      gcc -O2 -S -masm=intel 17_assembly.c -o - | less
 *
 *  Kutilgan natija (raqamlar kompyuterga qarab):
 *      qoshish(20, 22) = 42
 *      CPU ishlab chiqaruvchisi: GenuineIntel (yoki AuthenticAMD)
 *      1 000 000 marta qo'shish: ~... CPU takti (rdtsc)
 *      syscall orqali to'g'ridan-to'g'ri yozildi!
 *
 *  Sinab ko'ring:
 *      1) `gcc -O2 -S 17_assembly.c -o - | grep -A3 "^qoshish:"` - lea buyrug'ini toping.
 *      2) `volatile long s` dan volatile ni o'chirib, -O2 bilan yig'ing. Takt soni nega keskin
 *         kamaydi? (Natijasi ishlatilmaydigan siklni kompilyator butunlay olib tashlaydi.)
 *      3) mening_write kabi `mening_getpid()` yozing: rax = 39 (SYS_getpid), argumentsiz.
 *         Natijani getpid() bilan solishtiring.
 * ============================================================================= */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

__attribute__((noinline)) long qoshish(long a, long b)
{
    return a + b;                               /* -O2 da: lea rax, [rdi+rsi]; ret */
}

static inline uint64_t rdtsc(void)
{
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static void cpuid(uint32_t leaf, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d)
{
    __asm__ volatile("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(leaf), "c"(0));
}

/* libc'siz write: rax = 1 (SYS_write), rdi = fd, rsi = buf, rdx = uzunlik */
static long mening_write(int fd, const void *buf, unsigned long n)
{
    long r;
    __asm__ volatile("syscall"
                     : "=a"(r)
                     : "a"(1L), "D"((long)fd), "S"(buf), "d"(n)
                     : "rcx", "r11", "memory");    /* syscall rcx va r11 ni buzadi */
    return r;
}

int main(void)
{
    printf("qoshish(20, 22) = %ld\n", qoshish(20, 22));

    uint32_t a, b, c, d;
    char nom[13];
    cpuid(0, &a, &b, &c, &d);
    memcpy(nom, &b, 4);                         /* ishlab chiqaruvchi nomi: ebx, edx, ecx */
    memcpy(nom + 4, &d, 4);
    memcpy(nom + 8, &c, 4);
    nom[12] = '\0';
    printf("CPU ishlab chiqaruvchisi: %s\n", nom);

    volatile long s = 0;
    uint64_t t0 = rdtsc();
    for (int i = 0; i < 1000000; i++)
        s += i;
    printf("1 000 000 marta qo'shish: %llu CPU takti (rdtsc)\n", (unsigned long long)(rdtsc() - t0));

    fflush(stdout);                             /* printf buferi write'dan oldin chiqsin */
    const char msg[] = "syscall orqali to'g'ridan-to'g'ri yozildi!\n";
    mening_write(1, msg, sizeof(msg) - 1);
    return 0;
}
