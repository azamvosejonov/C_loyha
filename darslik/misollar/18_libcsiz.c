/* =============================================================================
 *  18_libcsiz.c - standart kutubxonasiz (freestanding) dastur  (darslik 18-bob)
 * =============================================================================
 *  Bu dasturda printf ham, main ham YO'Q - xuddi yadrodagi kabi. Kirish nuqtasi
 *  _start, chiqish - to'g'ridan-to'g'ri syscall.
 *
 *  Ishga tushirish (faqat x86-64 Linux):
 *      gcc -Wall -Wextra -O2 -ffreestanding -nostdlib -static -fno-stack-protector \
 *          18_libcsiz.c -o libcsiz && ./libcsiz; echo "chiqish kodi: $?"
 *
 *  Kutilgan natija:
 *      libc yo'q, printf yo'q - faqat syscall! 7 * 6 = 42
 *      chiqish kodi: 42
 *
 *  Sinab ko'ring:
 *      ls -l libcsiz      - oddiy "salom"dan qanchalik kichik?
 *      nm libcsiz         - faqat sizning belgilaringiz (printf, malloc yo'q)
 *      -nostdlib ni olib tashlang - _start ikki marta ta'riflangan xatosi (libc'niki bilan)
 * ============================================================================= */
static long sys_write(int fd, const char *buf, unsigned long n)
{
    long r;
    __asm__ volatile("syscall" : "=a"(r) : "a"(1L), "D"((long)fd), "S"(buf), "d"(n)
                     : "rcx", "r11", "memory");
    return r;
}

__attribute__((noreturn)) static void sys_exit(int kod)
{
    __asm__ volatile("syscall" : : "a"(60L), "D"((long)kod) : "rcx", "r11", "memory");
    __builtin_unreachable();
}

static unsigned long uzunlik(const char *s)     /* strlen'ni o'zimiz yozamiz - libc yo'q */
{
    unsigned long n = 0;
    while (s[n])
        n++;
    return n;
}

static void son_yoz(long x)                     /* printf("%ld") o'rniga (11-mashq!) */
{
    char b[24];
    int i = 23;
    b[i] = '\0';
    do {
        b[--i] = (char)('0' + x % 10);
        x /= 10;
    } while (x);
    sys_write(1, b + i, uzunlik(b + i));
}

void _start(void)
{
    const char *m = "libc yo'q, printf yo'q - faqat syscall! 7 * 6 = ";
    sys_write(1, m, uzunlik(m));
    son_yoz(7 * 6);
    sys_write(1, "\n", 1);
    sys_exit(42);
}
