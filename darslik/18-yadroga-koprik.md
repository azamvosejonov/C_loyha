# 18-bob. Freestanding C: yadroga ko'prik

> **Bu bobda nima o'rganasiz:** operatsion tizimsiz ("yalang'och" apparatda) C qanday ishlashini; yadro kompilyatsiya bayroqlarini; **linker skriptini** (yadro xotirada qayerda turishini belgilash);
> CPU yoqilgandan birinchi C funksiyasigacha nima bo'lishini; MyOS kodini qayerdan va qanday o'qishni. Bu bob darslikni MyOS bilan bog'laydi — bundan keyin siz yadro ichida ishlaysiz.
> **Oldindan nima kerak:** 1-, 5-, 8-, 11-, 14-, 16-, 17-boblar.   **Vaqt:** 5–6 soat.

> **To'liq ishlaydigan misol:** [misollar/18_libcsiz.c](misollar/18_libcsiz.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Hozirgacha hamma dasturlarimiz operatsion tizim **ustida** ishladi: `printf`, `malloc`, fayllar — hammasi tayyor edi. **Yadro** esa OS ning o'zi: uning ostida **hech narsa yo'q**.
Bu bobda "hech narsa yo'q" sharoitida C qanday yashashini o'rganamiz.

**Hayotdan misol: mehmonxona va cho'ldagi chodir.** Oddiy dastur — **mehmonxonada** yashaydi: suv, elektr, oshxona, kir yuvish — hammasi tayyor (libc, operatsion tizim). Yadro esa **cho'lda chodir tikadi**:
suvni o'zi topadi, olovni o'zi yoqadi. `printf` yo'q, `malloc` yo'q, hatto `strlen` ham yo'q — hammasini o'zingiz yozasiz. Chunki yadroning o'zi boshqalarga "mehmonxona" bo'ladi.

| Mehmonxonada | Cho'lda (yadro) |
|---|---|
| suv kranidan | quduq qazish — `kmalloc` ni o'zingiz yozasiz |
| tayyor oshxona | olov yoqish — `printf` o'rniga `kprintf` |
| ma'mur | siz o'zingiz — xato qilsangiz hech kim qutqarmaydi |

## 18.1. Hosted va freestanding

Siz shu paytgacha yozgan dasturlar — **hosted** muhitda: ostida OS bor, `main` ni kimdir chaqiradi, `printf`, `malloc`, fayllar mavjud. Yadro esa **freestanding** muhitda: ostida hech narsa yo'q.
Yadroning o'zi — boshqalar uchun "ost".

| | Hosted (oddiy dastur) | Freestanding (yadro) |
|---|---|---|
| Kirish nuqtasi | `main` (libc'ning `_start` i chaqiradi) | o'zingiz belgilaysiz (MyOS: `boot.asm` → `kmain`) |
| Standart kutubxona | bor | **yo'q** — faqat `<stdint.h>`, `<stddef.h>`, `<stdbool.h>`, `<stdarg.h>`, `<limits.h>` |
| Xotira | `malloc` | o'zingiz yozasiz (buddy, slab) |
| Chiqish | `printf` → `write` | to'g'ridan-to'g'ri ekran xotirasiga / serial portga |
| Xato | segfault → OS dasturni o'ldiradi | page fault → **o'zingizning** ishlovchingiz; ishlovchi yo'q bo'lsa — triple fault, kompyuter qayta yuklanadi |
| Stek | 8 MB, avtomatik o'sadi | 8–16 KB, o'zingiz ajratasiz, to'lsa — qulash |

### Birinchi tajriba: libc'siz yashash

**Bu dastur nima qiladi (umumiy):** `printf`, `strlen`, `main` — hech qaysisi yo'q. Dastur o'zining `strlen` ini yozadi, sonni matnga aylantirishni o'zi qiladi, ekranga chiqarishni to'g'ridan-to'g'ri `syscall`
(17.5) bilan bajaradi, va `main` o'rniga `_start` dan boshlanadi — xuddi yadro kabi. (Faqat x86-64 Linux.)

```c
/* chodir.c - libc'siz: o'z strlen, o'z utoa, to'g'ridan-to'g'ri syscall */
typedef unsigned long size_t;

static long sys_write(int fd, const void *buf, size_t n)
{
    long r;
    __asm__ volatile("syscall" : "=a"(r) : "a"(1L), "D"((long)fd), "S"(buf), "d"(n)
                     : "rcx", "r11", "memory");
    return r;
}

static void sys_exit(int kod)
{
    __asm__ volatile("syscall" : : "a"(60L), "D"((long)kod));
    for (;;) { }                                /* bu yerga hech qachon kelmaydi */
}

static size_t mening_strlen(const char *s)
{
    size_t n = 0;
    while (s[n])
        n++;
    return n;
}

static void yoz(const char *s) { sys_write(1, s, mening_strlen(s)); }

/* Sonni matnga: raqamlarni oxiridan boshlab bufer oxiriga yozamiz */
static void yoz_son(unsigned long x)
{
    char buf[21];
    int i = 20;
    buf[i] = '\0';
    do {
        buf[--i] = (char)('0' + x % 10);
        x /= 10;
    } while (x);
    yoz(&buf[i]);
}

void _start(void)
{
    yoz("Salom, libc'siz dunyo!\n");
    yoz("2 + 3 = ");
    yoz_son(2 + 3);
    yoz("\n1 kunda soniyalar: ");
    yoz_son(24ul * 60 * 60);
    yoz("\n");
    sys_exit(0);
}
```

```console
$ gcc -Wall -Wextra -O2 -ffreestanding -nostdlib -static -fno-stack-protector chodir.c -o chodir
$ ./chodir
Salom, libc'siz dunyo!
2 + 3 = 5
1 kunda soniyalar: 86400
$ ls -l chodir | awk '{print "hajmi:", $5, "bayt"}'
hajmi: 9320 bayt
$ nm chodir | grep -c " U " || true
0
```

**Qismlar (avval vazifasi):**

| Qism | Vazifasi | Tafsilot |
|---|---|---|
| `_start` | dastur **kirish nuqtasi** (`main` o'rniga) | libc yo'q, shuning uchun uni chaqiradigan hech kim yo'q: `_start` — OS yuklagandan keyingi **birinchi** bajariladigan funksiya |
| `sys_write` | ekranga yozish | 17.5 dagi `syscall`: `rax=1`, `rdi=fd`, `rsi=buf`, `rdx=n` |
| `sys_exit` | dasturni tugatish | `syscall` raqami 60. `_start` dan **qaytib bo'lmaydi** (qaytadigan joy yo'q), shuning uchun albatta `exit` kerak |
| `mening_strlen` | `strlen` ni o'zimiz yozdik | 7.4 dagi usul |
| `yoz_son` | sonni matnga aylantirish | raqamlarni **oxiridan** olamiz (`x % 10`), bufer **oxiridan boshlab** yozamiz, oxirida `buf[i]` dan chiqaramiz |
| `-ffreestanding -nostdlib` | "libc yo'q" | 18.2 |

Dastur bir necha kilobayt, tashqaridan kerak bo'lgan belgilar (`U`) esa 0 ta — u hech kimga bog'liq emas. Oddiy `gcc salom.c` bilan yig'ilgan dastur esa libc'dan o'nlab funksiyani so'raydi.

**Sinab ko'ring:** `yoz_son` ga manfiy sonlarni ham qo'llaydigan `yoz_int(long x)` yozing. `-nostdlib` ni olib tashlab yig'ing — linker nima deydi (ikkita `_start`)?

## 18.2. Yadro kompilyatsiya bayroqlari

**Hayotdan misol: cho'l uchun jihozlar.** `-ffreestanding` — "mehmonxona yo'q, hech narsani tayyor deb o'ylama". `-nostdlib` — "libc'ni olib kelma". `-mno-red-zone`, `-mno-sse` — yadroga xos cheklovlar:
uzilish istalgan paytda kelishi mumkin, shuning uchun ba'zi qulayliklardan voz kechiladi.

MyOS `Makefile`'idagi `KERNEL_CFLAGS` — har biri nega kerak (Makefile'da ham izohlangan):

| Bayroq | Nega |
|---|---|
| `-ffreestanding` | Kompilyator libc borligini taxmin qilmaydi (masalan, `printf` → `puts` almashtirmaydi) |
| `-nostdlib` (link) | Hech qanday standart kutubxona va `crt0` ulanmaydi |
| `-fno-stack-protector` | Stek himoyasi glibc'dagi `__stack_chk_fail` ga tayanadi — u yo'q |
| `-mno-red-zone` | Uzilishlar red zone'ni buzadi (17-bob) |
| `-mgeneral-regs-only` | SSE/AVX registrlari ishlatilmaydi — ularni har uzilishda saqlash kerak bo'lardi |
| `-mcmodel=kernel` | Kod manzilning eng yuqori 2 GB ida (`0xFFFFFFFF80000000+`) |
| `-fno-pic -fno-pie` | Yadro aniq manzilga yuklanadi — "joyi o'zgaruvchan" kod kerak emas |
| `-Wall -Wextra -Werror` | Yadroda xato — butun tizimning qulashi |

### Nozik joy: kompilyator o'zi `memset` chaqiruvini qo'shishi mumkin

Siz hech qachon `memset` yozmagan funksiyada kompilyator uni **o'zi** chaqirishi mumkin. Mana oddiy nollash sikli:

```c
/* nolla.c - oddiy sikl */
typedef unsigned long size_t;

void nolla(char *a, size_t n)
{
    for (size_t i = 0; i < n; i++)
        a[i] = 0;
}
```

```console
$ gcc -O2 -c nolla.c -o nolla_oddiy.o && nm nolla_oddiy.o
                 U memset
0000000000000000 T nolla
$ gcc -O2 -ffreestanding -c nolla.c -o nolla_free.o && nm nolla_free.o
0000000000000000 T nolla
```

**Nima ko'rdik:** oddiy kompilyatsiyada kompilyator siklni tanib, `memset` ga **almashtirdi** — `nm` da `U memset` ("kerak, lekin bu yerda yo'q"). `-ffreestanding` bilan bu o'zgartirish bajarilmadi.
Lekin GCC hujjatiga ko'ra, hatto freestanding'da ham kompilyator `memcpy`, `memmove`, `memset`, `memcmp` ni **chaqirishi mumkin** (masalan, katta struct nusxalashda boshqa maqsad arxitekturalarida).
Shuning uchun yadro bu to'rtta funksiyani albatta ta'riflashi kerak (MyOS: `kernel/lib/string.c`).

> **Eslab qoling:** yadroda `memcpy/memmove/memset/memcmp` bo'lishi **shart** — kompilyator ularni yashirincha chaqiradi.

## 18.3. Linker skripti — yadro xotirada qayerda turadi

**Hayotdan misol: qurilish bosh rejasi.** Oddiy dasturda linker binoni o'zi joylashtiradi. Yadro uchun esa siz aytasiz: "poydevor 1 MB manzildan boshlansin, avval kod qavati (`.text`), keyin o'zgarmaslar (`.rodata`),
keyin ma'lumotlar (`.data`)". Yuklovchi (GRUB) yadroni aynan shu reja bo'yicha xotiraga qo'yadi.

**Bu nima?** **Linker skripti** — linker uchun "xarita": qaysi bo'lim (`.text`, `.data`...) qaysi manzildan boshlansin. **Asosiy ishi:** tayyor dastur/yadroning xotiradagi joylashuvini belgilash.

Avval oddiy dasturda bo'limlar qayerda turishini ko'ramiz (11-bobdagi `nm`dan bilamiz: kod `T`, ma'lumot `D`, qiymatsiz `B`):

```c
/* bolimlar.c - bo'limlar xotirada qayerda */
#include <stdio.h>

extern char etext, edata, end;          /* linker o'zi yaratadigan belgilar */

int ma_lumot = 5;                       /* .data  : qiymatli global */
int bos_ma_lumot;                       /* .bss   : qiymatsiz global (0) */
const char matn[] = "faqat o'qish";     /* .rodata: o'zgarmas */

void funksiya(void) {}                  /* .text  : kod */

int main(void)
{
    printf("kod (funksiya)   : %p\n", (void *)funksiya);
    printf("etext (kod oxiri): %p\n", (void *)&etext);
    printf("o'zgarmas matn   : %p\n", (void *)matn);
    printf("data (qiymatli)  : %p\n", (void *)&ma_lumot);
    printf("edata            : %p\n", (void *)&edata);
    printf("bss (qiymatsiz)  : %p\n", (void *)&bos_ma_lumot);
    printf("end              : %p\n", (void *)&end);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -no-pie bolimlar.c -o bolimlar
$ ./bolimlar
kod (funksiya)   : 0x401136
etext (kod oxiri): 0x401231
o'zgarmas matn   : 0x402008
data (qiymatli)  : 0x404018
edata            : 0x40401c
bss (qiymatsiz)  : 0x404020
end              : 0x404028
$ size bolimlar
   text	   data	    bss	    dec	    hex	filename
   1462	    548	     12	   2022	    7e6	bolimlar
```

**Nima ko'rdik:** manzillar o'sish tartibida: **kod** (`0x401...`) → **o'zgarmas matn** (`0x402...`) → **data** (`0x4040...`) → **bss** (undan keyin) → `end` (hammasining oxiri).
`etext`, `edata`, `end` — linker o'zi yaratgan belgilar: "kod qayerda tugaydi", "data qayerda tugaydi", "dastur qayerda tugaydi". `size` bo'limlar hajmini ko'rsatadi.

Yadro uchun esa joylashuvni **o'zingiz** yozasiz. Mana eng kichik linker skripti va uning ta'siri (`chodir` dasturini shu skript bilan yig'amiz):

```c
/* min.ld - kichik linker skripti */
ENTRY(_start)                       /* kirish nuqtasi */
SECTIONS
{
    . = 0x10000000;                 /* "." - joriy manzil: shu manzildan boshla */
    .text   : { *(.text*) }         /* avval kod */
    .rodata : { *(.rodata*) }       /* keyin o'zgarmaslar */
    .data   : { *(.data*) }         /* keyin ma'lumotlar */
    .bss    : { *(.bss*) *(COMMON) }    /* oxirida qiymatsizlar */
}
```

```c
/* manzilli.c - libc'siz, o'zim belgilagan manzilda */
typedef unsigned long size_t;

static long sys_write(int fd, const void *buf, size_t n)
{
    long r;
    __asm__ volatile("syscall" : "=a"(r) : "a"(1L), "D"((long)fd), "S"(buf), "d"(n)
                     : "rcx", "r11", "memory");
    return r;
}

static void sys_exit(int kod)
{
    __asm__ volatile("syscall" : : "a"(60L), "D"((long)kod));
    for (;;) { }
}

void _start(void)
{
    sys_write(1, "men 0x10000000 manzilda turibman\n", 33);
    sys_exit(0);
}
```

```console
$ gcc -O2 -ffreestanding -nostdlib -static -no-pie -fno-stack-protector -T min.ld manzilli.c -o manzilli
$ ./manzilli
men 0x10000000 manzilda turibman
$ readelf -h manzilli | grep Entry
  Entry point address:               0x10000000
$ nm manzilli
0000000010000000 T _start
```

**Nima ko'rdik:** skriptdagi `. = 0x10000000;` bizning tanlovimiz bo'yicha kirish nuqtasi (`Entry point address`) va `_start` aynan `0x10000000` ga tushdi. Oddiy dastur `0x401000` atrofida turardi (yuqoridagi `bolimlar`).
Dastur baribir ishladi — chunki OS uni o'sha manzilga yukladi.

MyOS yadrosining haqiqiy skripti (`kernel/linker.ld`, soddalashtirilgan):

```text
ENTRY(_start)                                   /* birinchi bajariladigan belgi (boot.asm) */
KERNEL_VMA = 0xFFFFFFFF80000000;                /* yadro ishlaydigan virtual manzil */

SECTIONS
{
    . = 1M;                                     /* "." - joriy manzil: fizik 1 MB dan */
    .boot : ALIGN(8) { KEEP(*(.multiboot2)) ... }   /* yuklovchi sarlavhasi + 32 bitli kod */

    . = ALIGN(4K);
    . += KERNEL_VMA;                            /* bundan keyingi hammasi yuqori yarmida */
    __kernel_start = .;
    .text   : AT(ADDR(.text) - KERNEL_VMA)   ALIGN(4K) { ... *(.text*) ... }
    .rodata : AT(ADDR(.rodata) - KERNEL_VMA) ALIGN(4K) { ... }
    .data   : AT(ADDR(.data) - KERNEL_VMA)   ALIGN(4K) { ... }
    .bss    : AT(ADDR(.bss) - KERNEL_VMA)    ALIGN(4K) { ... }
    . = ALIGN(4K);
    __kernel_end = .;                           /* C kodidan: extern char __kernel_end[]; */
}
```

| Qism | Vazifasi |
|---|---|
| `.` | joriy manzil hisoblagichi; `. = 1M;` uni o'rnatadi (`;` — buyruq oxiri, linker skriptida ham) |
| `ALIGN(4K)` | har bir bo'lim yangi sahifadan: shunda `.text` ni "faqat o'qish + bajarish", `.data` ni "o'qish + yozish, bajarib bo'lmaydi" qilib xaritalash mumkin (W^X himoyasi) |
| `AT(...)` | **yuklash** manzili (fizik) va **ishlash** manzili (virtual) farqli: GRUB fizik 1 MB ga yuklaydi, kod esa yuqori yarmida ishlaydi. `boot.asm` sahifa jadvalini shunday sozlaydiki, ikkalasi ham bir xil fizik xotiraga ko'rsatadi |
| `__kernel_end = .;` | skriptdagi belgi C'dan manzil sifatida o'qiladi — "yadro qayerda tugaydi, bo'sh xotira qayerdan boshlanadi" (yuqoridagi `end` kabi) |

To'liq variantni `kernel/linker.ld` da o'qing — har bir qator izohlangan.

> **Eslab qoling:** linker skripti = "qaysi bo'lim qaysi manzilga". `.` — joriy manzil; `ALIGN(4K)` — sahifa chegarasi; `AT` — yuklash manzili; skript belgilari C'da `extern char x[]` bilan o'qiladi.

## 18.4. Birinchi C funksiyasigacha

**Hayotdan misol: bo'sh uyga ko'chib kirish.** C kodi ishlashi uchun stek kerak, `.bss` tozalangan bo'lishi kerak. Yangi uyga ko'chganda avval eshikni o'rnatasiz, chiroqni ulaysiz — keyin mebel olib kirasiz.
Yadroda bu ishni kichik assembly kodi (`_start`) qiladi va shundan keyingina `kmain()` ni chaqiradi.

CPU yoqilganda C ishlay olmaydi: stek yo'q, sahifalar yo'q, hatto 64 bitli rejim ham yo'q. `kernel/boot/boot.asm` shularni tayyorlaydi:

```text
GRUB: 32 bitli himoyalangan rejim, sahifalashsiz, Multiboot2 ma'lumoti ebx da
  → boot.asm: vaqtinchalik stek
  → boot.asm: sahifa jadvallari (identity + yuqori yarmi)
  → boot.asm: CR3, PAE, EFER.LME, CR0.PG → 64 bitli rejim
  → boot.asm: yuqori yarmidagi manzilga sakrash, yangi stek
  → call kmain(magic, mbi_phys)           ← birinchi C funksiyasi (kernel/main.c)
```

Keyin `kmain` qatlamma-qatlam tizimni quradi — `kernel/main.c` ni oching: har bir qator izohlangan, tartib esa muhim (xotirasiz hech narsa ishlamaydi, uzilishlarsiz taymer yo'q...).

## 18.5. Yadroda "libc'siz" yashash — qo'llanma

| Ehtiyoj | MyOS'da |
|---|---|
| `printf` | `kprintf` (`kernel/lib/kprintf.c`) — ekran, serial va `dmesg` buferiga |
| `malloc` | `kmalloc`/`kfree` (slab), `alloc_pages` (buddy), `vmalloc` |
| `assert`/`abort` | `panic("...")` — hamma CPU'larni to'xtatib, xabarni chiqaradi |
| `errno` | yo'q — funksiyalar `-ENOMEM` kabi manfiy kod qaytaradi |
| mutex | `spinlock_t`, `struct mutex` |
| ro'yxatlar | `struct list_head` (`kernel/lib/list.h`) — 23-mashqdagi aynan o'sha |
| `sleep` | `proc_sleep`/`proc_wakeup` — kutish navbatlari |
| vaqt | `timer_ticks()`, `udelay()` (TSC) |
| `memcpy`/`strlen` | `kernel/lib/string.c` |

**Hayotdan misol: `panic` — samolyotdagi favqulodda qo'nish.** Yadro davom etib bo'lmaydigan xatoni ko'rsa, eng xavfsiz yo'l — hamma narsani to'xtatish va nima bo'lganini ekranga yozish. Bir jarayonni o'ldirish mumkin,
yadroning o'zini — yo'q.

## 18.6. Yadroda xato qilish qanday ko'rinadi

- **Page fault** (noto'g'ri manzil) → `kernel/arch/interrupts.c` ishlovchisi: user dasturda bo'lsa — SIGSEGV, yadroda bo'lsa — PANIC: RIP, CR2 (xato manzili), registrlar va stek.
- **Triple fault** — istisno ishlovchisining o'zida istisno → CPU qayta yuklanadi (ekranda hech narsa yo'q, kompyuter shunchaki qayta yonadi). QEMU'da: `-d int,cpu_reset -no-reboot`.
- **Qotib qolish** — cheksiz sikl yoki deadlock. `make debug` + gdb bilan to'xtatib, `bt`.

Debug: `docs/08-test-debug.md` — gdb bilan yadroni qadamma-qadam bajarish, `addr2line` bilan PANIC manzilini qatorga aylantirish.

## 18.7. MyOS kodini qanday o'qish kerak

1. **Kichikdan boshlang.** `git checkout e5906bb` — v0.1: ~6800 qatorli (izohlar bilan) sodda yadro (`docs/01`–`08`). Uni tushunmasdan hozirgi versiyaga o'tmang. Tugatgach: `git checkout -` bilan qayting.
2. **Bosqichma-bosqich.** `git log --oneline --reverse` — har bir commit bitta bosqich. Har birining `git show --stat <commit>` i qaysi fayllar qo'shilganini ko'rsatadi.
3. **Hujjat + kod birga.** `docs/09`–`15` — har biri qaysi faylni o'qishni aytadi.
4. **Faylni yuqoridan pastga.** Har bir fayl boshida katta izoh: nima qiladi, qanday ishlaydi, nega shunday.
5. **Buzing.** Har bir hujjat oxiridagi "Sinab ko'ring" — kodni ataylab buzib, natijani kuzating.
6. **Lab'lar.** `tools/lab.py` — funksiyani o'chirib, qayta yozish (labs/README.md).

## 18.8. Darslikdan keyin — to'liq yo'l

```text
darslik 00-18 + mashqlar 01-30      C tili, xotira, tizim chaqiruvlari
      ↓
mashqlar 31-40                      yadro mexanizmlari ODDIY DASTUR sifatida:
                                    sahifa jadvali, buddy, slab, spinlock, scheduler, ELF, ext2
      ↓
docs/00-15 + MyOS kodi              haqiqiy yadroni o'qish
      ↓
labs (22 ta)                        yadro ICHIDA funksiyalarni qayta yozish
      ↓
QOLLANMA.md 11-bo'lim               yadroni NOLDAN yozish
      ↓
YAKUNIY.md IV qism                  yangi drayverlar va quyi tizimlar (NVMe, USB, tarmoq)
```

## Hayotdan misol va to'liq dastur

**Libc'siz yashash (`chodir.c`).** Bobning to'liq dasturi — 18.1 dagi `chodir.c`: `_start` dan boshlanadi, o'z `strlen`i, o'z son→matn funksiyasi, to'g'ridan-to'g'ri `syscall`.
U yadro dasturchisi uchun "hello world": hech narsa tayyor emas, hammasi o'zingizniki. Keyingi qadam (loyiha) — shu g'oya ustiga o'zingizning `kprintf` ni qurish.

<!-- katta:boshi -->
## Katta loyiha: libk — yadro uchun mini kutubxona

**Umumiy fikr.** Oddiy C dasturi `memcpy`, `strlen`, `printf` kabi funksiyalarni **libc**dan oladi. Lekin **yadro** (OS kernel) ishga tushganda hech qanday libc **yo'q** — yadroning o'zi libc'ni ta'minlashi kerak. Shu bosqichda yadroda kerak bo'ladigan asosiy funksiyalarni **noldan, libc'siz** yozamiz va ularni **haqiqiy libc bilan solishtirib** sinaymiz.

**Hayotiy o'xshatish:** orolda yashash. Do'kon (libc) yo'q: bolta, arra, ipni o'zingiz yasashingiz kerak. Lekin yasaganingizni **do'kondagisi bilan solishtirib** ko'rsangiz, to'g'ri yasaganingizga ishonch hosil qilasiz.

### Bu bosqichda nima qilamiz

Kutubxona (`libk.h` + `libk.c`) to'rt guruh funksiyadan iborat. Nomlar `k` prefiksi bilan (libc nomlari bilan to'qnashmaslik uchun — yadroda bu odat):

| Guruh | Funksiyalar | Nima uchun yadroda kerak |
|---|---|---|
| xotira | `kmemset`, `kmemcpy`, `kmemmove`, `kmemcmp` | sahifalarni nollash, bufer ko'chirish |
| satr | `kstrlen`, `kstrcmp`, `kstrlcpy` | fayl nomlari, buyruq qatori |
| son → matn | `kutoa(son, bufer, asos)` | konsolga son chiqarish (`printf` yo'q!) |
| bitmap | `kbit_yoq`, `kbit_och`, `kbit_bor`, `kbit_bosh_top` | bo'sh sahifalarni kuzatish (1 bit = 1 sahifa) |
| halqa bufer | `kring_*` | klaviatura buferi: bitta yozadi, bitta o'qiydi |

**Interfeys (`libk.h`):**

```c
/* libk.h - yadro uchun mini kutubxona: libc'siz (freestanding) satr/xotira funksiyalari, bitmap, halqa bufer */
#ifndef LIBK_H
#define LIBK_H

#include <stddef.h>
#include <stdint.h>

/* --- xotira va satrlar (libc nomlari bilan to'qnashmasligi uchun "k" prefiksi) --- */
void *kmemset(void *dst, int bayt, size_t n);
void *kmemcpy(void *dst, const void *src, size_t n);       /* ustma-ust tushmasligi shart */
void *kmemmove(void *dst, const void *src, size_t n);      /* ustma-ust tushsa ham to'g'ri */
int kmemcmp(const void *a, const void *b, size_t n);
size_t kstrlen(const char *s);
int kstrcmp(const char *a, const char *b);
size_t kstrlcpy(char *dst, const char *src, size_t hajm); /* dst ga ko'pi bilan hajm-1 belgi + '\0'; src uzunligini qaytaradi */

/* --- sonlarni matnga aylantirish: asos 2..16, bufer yetarlicha katta bo'lishi shart (kamida 65 bayt) --- */
char *kutoa(uint64_t son, char *bufer, int asos);

/* --- bitmap: 1 bit = 1 obyekt (16-bob). bitlar massivi uint8_t bayt qatorida --- */
void kbit_yoq(uint8_t *b, size_t i);
void kbit_och(uint8_t *b, size_t i);
int kbit_bor(const uint8_t *b, size_t i);
long kbit_bosh_top(const uint8_t *b, size_t n);            /* birinchi 0 bitning indeksi yoki -1 */

/* --- halqa bufer: bitta yozuvchi va bitta o'quvchi (klaviatura buferi kabi). sig'im 2 ning darajasi bo'lishi shart --- */
struct kring {
    uint8_t *ma_lumot;
    size_t sigim;                               /* 2 ning darajasi */
    size_t bosh, oxir;                          /* o'qish va yozish hisoblagichlari (cheksiz oshadi, mask bilan indeks olinadi) */
};

void kring_boshla(struct kring *r, uint8_t *xotira, size_t sigim);
int kring_yoz(struct kring *r, uint8_t bayt);              /* 0 - OK, -1 - to'la */
int kring_oqi(struct kring *r, uint8_t *bayt);             /* 0 - OK, -1 - bo'sh */
size_t kring_band(const struct kring *r);

#endif
```

**Eng nozik funksiyalardan uchtasi.**

**1) `kmemmove` — nega `kmemcpy` dan farqli?** Agar manba va nishon **ustma-ust** tushsa, oldinga ko'chirish manbani **o'zimiz buzib yuboramiz**. Shuning uchun `dst` manba **orqasida** bo'lsa, **oxiridan boshlab** ko'chiramiz:

```c
void *kmemmove(void *dst, const void *src, size_t n)
{
    uint8_t *d = dst;
    const uint8_t *s = src;
    if (d < s) {
        while (n--)                             /* oldinga ko'chirish: dst manba oldida */
            *d++ = *s++;
    } else if (d > s) {
        d += n;
        s += n;
        while (n--)                             /* ORQAGA ko'chirish: dst manba orqasida bo'lsa, oxiridan boshlaymiz */
            *--d = *--s;
    }
    return dst;
}
```

**2) `kutoa` — sonni matnga aylantirish.** Son `% asos` — eng **past raqam**, `/ asos` — qolgani. Raqamlar **teskari** chiqadi, shuning uchun vaqtinchalik `teskari[]` ga yozib, so'ng ag'daramiz:

```c
char *kutoa(uint64_t son, char *bufer, int asos)
{
    static const char raqamlar[] = "0123456789abcdef";
    char teskari[65];
    int n = 0;
    if (asos < 2 || asos > 16)
        asos = 10;
    do {
        teskari[n++] = raqamlar[son % (uint64_t)asos];      /* eng past raqam birinchi chiqadi */
        son /= (uint64_t)asos;
    } while (son);
    int i = 0;
    while (n)
        bufer[i++] = teskari[--n];              /* teskari tartibda yozamiz */
    bufer[i] = '\0';
    return bufer;
}
```

Masalan `kutoa(11, b, 2)`: `11 % 2 = 1`, `5 % 2 = 1`, `2 % 2 = 0`, `1 % 2 = 1` → teskari `1101` → ag'darilgach `1011`.

**3) Halqa bufer.** `bosh` (o'qish) va `oxir` (yozish) hisoblagichlari **cheksiz oshadi**; indeks `& (sigim - 1)` bilan olinadi (sig'im 2 ning darajasi bo'lgani uchun `%` o'rniga maska — tez):

```c
size_t kring_band(const struct kring *r)
{
    return r->oxir - r->bosh;                   /* ayirma toshsa ham (ishorasiz) to'g'ri qoladi */
}
```

```c
int kring_yoz(struct kring *r, uint8_t bayt)
{
    if (kring_band(r) == r->sigim)
        return -1;
    r->ma_lumot[r->oxir & (r->sigim - 1)] = bayt;   /* & (sigim-1): 2 ning darajasida % o'rniga */
    r->oxir++;
    return 0;
}
```

`oxir - bosh` — band joy soni; ayirma `size_t` (ishorasiz) bo'lgani uchun hisoblagich toshib o'tgandan keyin ham **to'g'ri** qoladi.

### Sinov: libc bilan solishtirish

`sinov.c` har funksiyani **20 000 marta tasodifiy kirish bilan** libc'dagi etaloni bilan solishtiradi. Muhim joylari: ustma-ust tushadigan siljishlar (`kmemmove` uchun), kichik alifbo (`a b c` — teng satrlar ko'p uchrasin), halqa buferda **FIFO tartibi** tekshiruvi:

```c
#define TEKSHIR(shart, xabar)                                                      \
    do {                                                                           \
        if (!(shart)) {                                                            \
            xatolar++;                                                             \
            if (xatolar <= 5)                                                      \
                printf("  XATO %s:%d: %s\n", __FILE__, __LINE__, xabar);           \
        }                                                                          \
    } while (0)
```

```c
static void halqa_sinovi(void)
{
    uint8_t xotira[8];
    struct kring r;
    kring_boshla(&r, xotira, 8);
    unsigned yozilgan = 0, oqilgan = 0;
    uint8_t kutilgan = 0, yoz = 0;
    for (int t = 0; t < 100000; t++) {
        if (tasodif() % 2) {
            if (kring_yoz(&r, yoz) == 0) {
                yoz++;
                yozilgan++;
            }
        } else {
            uint8_t b;
            if (kring_oqi(&r, &b) == 0) {
                TEKSHIR(b == kutilgan, "halqa: FIFO tartibi buzildi");
                kutilgan++;
                oqilgan++;
            }
        }
        TEKSHIR(kring_band(&r) == yozilgan - oqilgan && kring_band(&r) <= 8, "halqa: band soni");
    }
}
```

Kutubxonani **ikki xil** yig'amiz: (1) **freestanding** — libc'siz, yadroga mos (`-ffreestanding -fno-builtin ...`); (2) sinov uchun **oddiy**, sanitizer bilan:

```console
$ cd katta_loyiha/tizim/18_libk
$ gcc -Wall -Wextra -O2 -ffreestanding -fno-builtin -fno-tree-loop-distribute-patterns -fno-stack-protector -mno-red-zone -fno-pic -c libk.c -o libk.o
$ ar rcs libk.a libk.o
$ echo "libk.o ning tashqi (libc) bog'liqliklari: $(nm -u libk.o | wc -l) ta"
libk.o ning tashqi (libc) bog'liqliklari: 0 ta
$ nm libk.a | awk '$2 == "T" { print $3 }' | sort | tr '\n' ' '; echo
kbit_bor kbit_bosh_top kbit_och kbit_yoq kmemcmp kmemcpy kmemmove kmemset kring_band kring_boshla kring_oqi kring_yoz kstrcmp kstrlcpy kstrlen kutoa 
$ gcc -Wall -Wextra -g -fsanitize=address,undefined sinov.c libk.c -o sinov
$ ./sinov
libk sinovi: hammasi to'g'ri (5 ta guruh, ~200000 tekshiruv)
```

**Nima ko'rdik:**

- `tashqi (libc) bog'liqliklari: 0 ta` — `nm -u` obyekt faylning **hal qilinmagan** (boshqa joydan kerak) belgilarini ko'rsatadi. **0** — demak libk **butunlay mustaqil**: yadroga qo'shilganda libc kerak emas. Agar bu yerda `memcpy` chiqsa, kompilyator sizning siklingizni `memcpy` chaqirig'iga aylantirgan bo'ladi (shuning uchun `-fno-builtin`, `-fno-tree-loop-distribute-patterns` bayroqlari bor!).
- `nm libk.a | ... T` — kutubxonadagi **eksport qilingan** (`T` = kod bo'limi) funksiyalar ro'yxati: 16 ta.
- `libk sinovi: hammasi to'g'ri` — sanitizer (ASan + UBSan) ostida ~200 000 tekshiruv o'tdi: chegaradan chiqish yo'q, aniqlanmagan xatti-harakat yo'q.
- Bayroqlar: `-ffreestanding` — "libc mavjud deb o'ylama"; `-mno-red-zone` — yadroda uzilish steki "qizil zonani" buzishi mumkin; `-fno-stack-protector` — himoya kanareykasi libc'dan funksiya chaqiradi.

> **Eslab qoling:** yadro — libc'siz dunyo. O'z `mem*`/`str*` funksiyalaringizni yozing, **libc etaloni bilan tasodifiy sinang** va `nm -u` bilan hech narsa **tashqaridan** talab qilinmasligini **isbotlang**. `memmove` — ustma-ust holat uchun, `memcpy` — yo'q.

**O'zingiz qo'shing (yechimsiz):**

1. `kstrncmp(a, b, n)` va `kstrchr(s, c)` yozing va `sinov.c` ga ularning libc bilan taqqoslash testini qo'shing.
2. `kring_yoz` dagi `r->sigim - 1` maskasini `% r->sigim` ga almashtiring va sinov o'tadimi? Sig'im 2 ning darajasi **bo'lmasa** (masalan 6) maska nima beradi?
3. `kbit_bosh_top` ni sekin (har bit) o'rniga **bayt-bayt** tezlashtiring: avval `b[i] == 0xFF` bo'lsa butun baytni o'tkazib yuboring. `time` bilan 1 million bitli bitmapda solishtiring.
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. **Hosted** (oddiy dastur: libc, `main`) va **freestanding** (yadro: hech narsa, o'z kirish nuqtasi) — yadro libc'siz yashaydi.
2. Yadro bayroqlari: `-ffreestanding -nostdlib -fno-stack-protector -mno-red-zone -mgeneral-regs-only -mcmodel=kernel`; `memcpy/memset/...` ni yadro **o'zi** yozishi shart.
3. **Linker skripti** — bo'limlarning xotiradagi manzili: `. = manzil;`, `ALIGN(4K)`, `AT(...)` (yuklash manzili), skript belgilari (`__kernel_end`) C'dan o'qiladi.
4. CPU yoqilgach C'gacha: assembly (`boot.asm`) stek, sahifalar va 64-bit rejimni tayyorlaydi → `kmain`.
5. Yadroda xato: page fault → PANIC; ishlovchida xato → triple fault (qayta yuklanish). MyOS'ni v0.1 dan, hujjat + kod birga o'qing.

## Savol-javob

**Savol:** Yadroda `printf` va `malloc` nega yo'q?
**Javob:** Ular libc'da, libc esa OS ustida ishlaydi (fayl, syscall, heap). Yadro o'zi OS — uning ostida hech narsa yo'q. Shuning uchun yadro o'zining `kprintf` va allocator'ini yozadi (18.1, 18.5).

**Savol:** Linker skripti nima uchun kerak?
**Javob:** Oddiy dasturda OS o'zi xotiraga joylashtiradi. Yadroni esa kim joylashtiradi? Linker skripti kod, o'qiladigan va yoziladigan ma'lumot bo'limlari **xotiraning qaysi manzilida** turishini aniq belgilaydi (18.3).

**Savol:** Yadro kodi nega maxsus kompilyatsiya bayroqlari bilan yig'iladi?
**Javob:** Oddiy bayroqlar user dasturi uchun mo'ljallangan: standart kutubxona, SSE registrlari, stek ostidagi "red zone" kabi taxminlar yadroda xavfli (masalan, uzilish kelganda red zone buziladi). Shuning uchun `-ffreestanding` va boshqa bayroqlar kerak (18.2).

## O'zingizni tekshiring

1. Nega yadroda `printf` yo'q, lekin `<stdint.h>` bor?
2. `-ffreestanding` bo'lsa ham nega yadro `memcpy` ni ta'riflashi shart?
3. Linker skriptidagi `.` va `ALIGN(4K)` nima?
4. Yadro kodida page fault bo'lsa nima bo'ladi? Istisno ishlovchisining o'zida-chi?
5. MyOS'ni o'qishni qaysi versiyadan boshlash kerak?

<details><summary>Javoblar</summary>

1. `printf` — kutubxona kodi (OS'ga tayanadi); `<stdint.h>` — faqat tur ta'riflari, kod yo'q.
2. GCC struct nusxalash va nollash uchun `memcpy`/`memset` chaqiruvlarini o'zi qo'shishi mumkin.
3. `.` — joriy manzil hisoblagichi; `ALIGN(4K)` — uni 4096 ga karrali qilib surish.
4. Yadroning page fault ishlovchisi PANIC beradi; ishlovchining o'zida xato → double/triple fault → qayta yuklanish.
5. v0.1 (`git checkout e5906bb`), `docs/01`–`08` bilan.
</details>

## Mashq

- **31–40** (yadro mexanizmlari oddiy dastur sifatida) — [mashqlar/README.md](../mashqlar/README.md).
- MyOS: `kernel/main.c` ni boshidan oxirigacha o'qing va har bir `*_init()` chaqiruvi qaysi faylga olib borishini daftaringizga yozing. Bu — yadroning "mundarijasi".

**Tabriklayman — darslikning I qismi (C tili) tugadi.** Endi siz yadro kodini o'qiy oladigan darajadasiz. II qism (19–31-boblar) — kompyuter tizimlari va operatsion tizimlar nazariyasi: odatda ingliz tilidagi
bir nechta kitobdan o'rganiladigan bilimlar shu yerda.

<!-- loyiha:boshi -->
## Loyiha: `kprintf` — libc'siz formatlash

**Maqsad:** yadroda `printf` **yo'q**. Yadro dasturchisi birinchi bo'lib o'zining `kprintf` ini yozadi — chunki
usiz na xato, na holatni ko'rib bo'ladi (18.5). Bu funksiya bo'lmasa, yadroni debug qilib bo'lmaydi.
**Bobdan ishlatiladi:** libc'siz yashash, o'zgaruvchan sonli argumentlar (`va_list`), sonni matnga o'girish, faqat bitta
"chiqish" funksiyasiga (`kputc`) tayanish.

**Talab:** `kprintf(fmt, ...)` — `%d %u %x %X %c %s %%` ni tushunsin. Barcha chiqish **bitta** `kputc(char)` orqali:
yadroda bu funksiya belgini serial portga yoki ekranga yozadi; bu yerda `write(1, ...)` qiladi.
**Ma'lumotlar:** yo'q (holatsiz). **Qadamlar:** formatni belgima-belgi yurish → `%` topilsa keyingi belgiga qarab
argumentni `va_arg` bilan olish.
**Sonni matnga:** raqamlarni **teskari tartibda** (oxirgisidan) olamiz (`v % asos`), keyin teskari chiqaramiz.

```c
/* kprintf.c - libc'siz printf */
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>

static void kputc(char c)                       /* YAGONA chiqish nuqtasi (yadroda: serial port) */
{
    ssize_t r = write(1, &c, 1);
    (void)r;
}

static void kputs(const char *s)
{
    if (!s)
        s = "(null)";
    while (*s)
        kputc(*s++);
}

static void kson(unsigned long long v, unsigned asos, int katta)
{
    const char *raqam = katta ? "0123456789ABCDEF" : "0123456789abcdef";
    char bufer[24];
    int n = 0;
    do {
        bufer[n++] = raqam[v % asos];           /* eng past raqam */
        v /= asos;
    } while (v != 0);
    while (n > 0)
        kputc(bufer[--n]);                      /* teskari tartibda chiqaramiz */
}

static void kprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            kputc(*fmt);
            continue;
        }
        char c = *++fmt;
        if (c == '\0')
            break;                              /* format '%' bilan tugadi */
        switch (c) {
        case 'd': {
            int v = va_arg(ap, int);
            if (v < 0) {
                kputc('-');
                kson(-(long long)v, 10, 0);     /* INT_MIN uchun ham to'g'ri: long long ga o'tdik */
            } else {
                kson((unsigned)v, 10, 0);
            }
            break;
        }
        case 'u': kson(va_arg(ap, unsigned), 10, 0); break;
        case 'x': kson(va_arg(ap, unsigned), 16, 0); break;
        case 'X': kson(va_arg(ap, unsigned), 16, 1); break;
        case 'c': kputc((char)va_arg(ap, int)); break;
        case 's': kputs(va_arg(ap, const char *)); break;
        case '%': kputc('%'); break;
        default:  kputc('%'); kputc(c); break;  /* noma'lum - o'zgarishsiz chiqaramiz */
        }
    }
    va_end(ap);
}

int main(void)
{
    kprintf("Salom, %s! Son: %d, manfiy: %d, nol: %d\n", "yadro", 42, -17, 0);
    kprintf("O'n oltilik: %x va %X, ishorasiz: %u\n", 255, 255, 4294967295u);
    kprintf("Chegara: %d va %d\n", INT_MAX, INT_MIN);
    kprintf("Belgilar: %c%c%c, foiz: 100%%, noma'lum: %q, bo'sh: %s\n", 'C', 'y', 'a', NULL);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined kprintf.c -o kprintf
$ ./kprintf
Salom, yadro! Son: 42, manfiy: -17, nol: 0
O'n oltilik: ff va FF, ishorasiz: 4294967295
Chegara: 2147483647 va -2147483648
Belgilar: Cya, foiz: 100%, noma'lum: %q, bo'sh: (null)
```

Nega `-(long long)v`? `INT_MIN` ni manfiylab bo'lmaydi: `-INT_MIN` `int` da toshadi (UB!). Kengroq turga o'tish — yechim (13-bob).
Nega raqamlar teskari? `v % 10` **oxirgi** raqamni beradi. `12345` → `5,4,3,2,1`; teskari chiqaramiz.

**Kengaytiring:** `%p` (ko'rsatkich, `0x` bilan o'n oltilik) va `%o` (sakkizlik) qo'shing. Hech qanday `stdio.h` chaqirilmaganini
`nm kprintf | grep " U "` bilan tekshiring (faqat `write` va libc ichki nomlari qoladi).

## Mustaqil loyiha: `kprintf` — kenglik va to'ldirish ★★★

**Vazifa:** yuqoridagi `kprintf` ni **kengaytiring** (yoki noldan yozing): kenglik, nol bilan to'ldirish,
chapga tekislash va ikkilik format. Fayl: `kprintf2.c`. Hamma chiqish — faqat `kputc` orqali.

**Format sintaksisi:** `%[bayroqlar][kenglik]tur`
- **bayroqlar** (ixtiyoriy): `-` — chapga tekislash (o'ngdan probel bilan to'ldiradi); `0` — chapdan `0` bilan to'ldiradi
  (`-` bilan birga kelsa `0` e'tiborga olinmaydi);
- **kenglik**: raqamlar (masalan `5`, `08`) — minimal belgilar soni; matn shundan qisqa bo'lsa to'ldiriladi;
- **tur**: `d u x X o b c s %` (`b` — ikkilik, `o` — sakkizlik).
- Manfiy son `0` bayrog'i bilan: ishora **oldin**, nollar undan **keyin** (`-42`, `%05d` → `-0042`).
- `%s` da `NULL` → `(null)`. `%c` va `%s` uchun `0` bayrog'i ham probel emas, `0` bilan to'ldirmaydi — oddiy probel qo'ying (soddalik uchun).
- Noma'lum tur (`%q`) — o'zgarishsiz chiqarilsin (`%q`).

**Qaytish qiymati:** `int kprintf(...)` — chiqarilgan belgilar **soni**. (Ichki hisoblagichni `kputc` ni o'rab oling.)

**`main` da ushbu chaqiruvlar** (har biri bitta satr chiqarsin):

```c
kprintf("[%d] [%5d] [%-5d] [%05d]\n", 42, 42, 42, 42);
kprintf("[%d] [%05d] [%6d]\n", -42, -42, -42);
kprintf("[%x] [%X] [%08x]\n", 255, 255, 0xBEEF);
kprintf("[%b] [%08b] [%b] [%o]\n", 10, 5, 0, 64);
kprintf("[%s] [%10s] [%-10s] [%s]\n", "salom", "salom", "salom", NULL);
kprintf("belgilar: %c%c%c, foiz: 100%%\n", 'a', 'b', 'c');
kprintf("[%u] [%d]\n", 4294967295u, INT_MIN);
int n = kprintf("hello %d\n", 5);
kprintf("qaytardi: %d\n", n);
kprintf("[%q]\n");
```

**Kutilgan natija** (`darslik/loyihalar/18_kprintf/kutilgan.txt`):

```text
[42] [   42] [42   ] [00042]
[-42] [-0042] [   -42]
[ff] [FF] [0000beef]
[1010] [00000101] [0] [100]
[salom] [     salom] [salom     ] [(null)]
belgilar: abc, foiz: 100%
[4294967295] [-2147483648]
hello 5
qaytardi: 8
[%q]
```

**Maslahat** (yechim emas):
- Avval sonni **bufer**ga (teskari) yozing, uzunligini biling; keyin: `to'ldirish soni = kenglik − uzunlik`.
- Chapga tekislashda: matn, keyin probellar. O'ngga tekislashda: to'ldirish (probel yoki `0`), keyin matn.
- Manfiy son + `0` bayrog'i: avval `-`, keyin nollar, keyin raqamlar. Ishorani alohida qayta ishlang.
- `kenglik` raqamlarini `while (*fmt >= '0' && *fmt <= '9')` bilan o'qing (`0` bayrog'i ham shu belgi, uni **kenglikdan oldin** ajrating).
- Hisoblagich: `static int chiqarildi;` ni `kputc` ichida oshiring; `kprintf` boshida nolga tushiring.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined kprintf2.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/18_kprintf/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [19-bob. Linux terminali va Git](19-terminal-git.md)
