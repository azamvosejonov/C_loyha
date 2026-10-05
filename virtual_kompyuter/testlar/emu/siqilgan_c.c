/* siqilgan_c.c — C kengaytmasini sinash: oddiy C dasturini -march=rv32imac bilan yig'amiz (kompilyator ko'p
   siqilgan buyruq ishlatadi) va natijani tekshiramiz. Xato bo'lsa quvvat qurilmasiga test raqami yoziladi. */
#define QUVVAT (*(volatile unsigned *)0x100000)

static void tugat(unsigned kod)
{
    QUVVAT = kod ? (kod << 16) | 0x3333 : 0x5555;
    for (;;)
        ;
}

__attribute__((noinline)) static unsigned fib(unsigned n)
{
    return n < 2 ? n : fib(n - 1) + fib(n - 2);         /* rekursiya: c.addi16sp, c.swsp, c.lwsp, c.jal ... */
}

__attribute__((noinline)) static int saralash(int *a, int n)
{
    for (int i = 0; i < n; i++)                         /* pufakchali saralash: c.lw, c.sw, c.bnez, c.mv ... */
        for (int j = 0; j + 1 < n - i; j++)
            if (a[j] > a[j + 1]) {
                int t = a[j];
                a[j] = a[j + 1];
                a[j + 1] = t;
            }
    int s = 0;
    for (int i = 0; i < n; i++)
        s = s * 3 + a[i];
    return s;
}

__attribute__((noinline)) static unsigned bitlar(unsigned x)
{
    unsigned r = 0;
    r ^= x << 3;                                        /* c.slli */
    r ^= x >> 5;                                        /* c.srli */
    r ^= (unsigned)((int)x >> 7);                       /* c.srai */
    r &= ~0x0F0u;                                       /* c.andi / and */
    r |= x & 0x1F;
    return r - (x ^ 0x55);                              /* c.sub, c.xor */
}

static int massiv[8] = { 5, -3, 9, 0, 12, -7, 1, 4 };

/* kirish nuqtasi assemblyda: C funksiyasi stekdan foydalanishidan OLDIN sp ni o'rnatish kerak */
__asm__(".section .text.boshi\n.globl _start\n_start:\n  li sp, 0x80100000\n  j asosiy\n");

void asosiy(void)
{
    if (fib(20) != 6765)
        tugat(1);
    if (saralash(massiv, 8) != ((((((-7 * 3 - 3) * 3 + 0) * 3 + 1) * 3 + 4) * 3 + 5) * 3 + 9) * 3 + 12)
        tugat(2);
    if (bitlar(0xDEADBEEFu) != ((((0xDEADBEEFu << 3) ^ (0xDEADBEEFu >> 5) ^ (unsigned)((int)0xDEADBEEFu >> 7)) & ~0x0F0u) | (0xDEADBEEFu & 0x1F)) - (0xDEADBEEFu ^ 0x55))
        tugat(3);
    tugat(0);
}
