# 12-bob. Standart kutubxona (libc)

> **Bu bobdan keyin:** eng kerakli standart funksiyalarni, `printf` formatlarini to'liq, `FILE *`
> buferlashini, `errno` ni va `qsort` ni bilasiz — va ularning **ichida** nima borligini tushunasiz
> (chunki yadroda ularning hech biri yo'q, o'zingiz yozasiz). Mashqlar: 11, 12, 19, 24.

## 12.1. libc nima

C tilining o'zi juda kichik: kalit so'zlar va operatorlar. `printf`, `malloc`, `strlen`, `fopen` — bular
tilning qismi emas, **standart kutubxonaning** (libc) funksiyalari: oddiy C'da yozilgan kod. Linux'da —
glibc yoki musl, MyOS'da — `user/libc/` (siz o'qiy oladigan ~2000 qator).

| Sarlavha | Nima |
|---|---|
| `<stdio.h>` | kiritish/chiqarish: `printf`, `fopen`, `fgets` |
| `<stdlib.h>` | `malloc`, `free`, `exit`, `strtol`, `qsort`, `abs` |
| `<string.h>` | `strlen`, `memcpy`, `strcmp`... (6-bob) |
| `<ctype.h>` | `isdigit`, `toupper`... |
| `<stdint.h>`, `<stddef.h>`, `<stdbool.h>`, `<limits.h>` | turlar va chegaralar |
| `<errno.h>` | `errno` va xato kodlari |
| `<time.h>` | vaqt |
| `<assert.h>` | `assert(shart)` |
| `<unistd.h>`, `<fcntl.h>`, `<sys/wait.h>` | **POSIX** (standart C emas): `read`, `fork`... (14-bob) |

## 12.2. `printf` — to'liq

Format: `%[bayroqlar][kenglik][.aniqlik][uzunlik]tur`

```c
printf("[%5d]\n", 42);          /* [   42]  kenglik 5, o'ngga tekislangan */
printf("[%-5d]\n", 42);         /* [42   ]  - : chapga */
printf("[%05d]\n", 42);         /* [00042]  0 : nol bilan to'ldirish */
printf("[%+d]\n", 42);          /* [+42]    + : doim ishora */
printf("[%x %X %#x]\n", 255, 255, 255);   /* [ff FF 0xff] */
printf("[%08lx]\n", 0xBEEFUL);  /* [0000beef] - manzillar/registrlar uchun klassik */
printf("[%.3s]\n", "salom");    /* [sal]    satr uchun aniqlik = maksimal uzunlik */
printf("[%.2f]\n", 3.14159);    /* [3.14] */
printf("[%*d]\n", 6, 42);       /* [    42] kenglik argumentdan */
```

Uzunlik modifikatorlari: `hh` (char), `h` (short), `l` (long), `ll` (long long), `z` (size_t),
`t` (ptrdiff_t). `uint64_t` uchun: `%lu` (Linux x86-64) yoki portativ `PRIu64` (`<inttypes.h>`).

**`printf` ning qaytish qiymati** — chiqarilgan belgilar soni. **`snprintf(buf, n, ...)`** — natijani
buferga, ko'pi bilan `n-1` belgi + `'\0'`, qaytish — to'liq natija uchun kerakli uzunlik (6-bob).

**Xavfsizlik:** `printf(foydalanuvchi_satri)` — **hech qachon!** Satrda `%s`/`%n` bo'lsa, `printf`
stekdan yo'q argumentlarni o'qiydi ("format string" hujumi). Doim: `printf("%s", satr)`.

**Ichida nima bor:** format satrini belgima-belgi o'qish, `%` ni ko'rganda `va_arg` bilan navbatdagi
argumentni olish, sonni satrga aylantirish (11-mashq!). MyOS: `user/libc/printf.c`,
`kernel/lib/kprintf.c` — ikkalasi ham ~300 qator. O'zgaruvchan sonli argumentlar:

```c
#include <stdarg.h>
int yigindi(int n, ...)
{
    va_list ap;
    va_start(ap, n);            /* n dan keyingi argumentlar */
    int s = 0;
    for (int i = 0; i < n; i++)
        s += va_arg(ap, int);   /* navbatdagisi, turi int deb */
    va_end(ap);
    return s;
}
yigindi(3, 10, 20, 30);         /* 60 */
```

## 12.3. `FILE *` va buferlash

```c
FILE *f = fopen("ma'lumot.txt", "r");     /* "r" o'qish, "w" yozish (tozalab), "a" qo'shish, "rb" binar */
if (!f) {
    perror("fopen");                      /* "fopen: No such file or directory" */
    return 1;
}
char qator[256];
while (fgets(qator, sizeof(qator), f))    /* qatorma-qator */
    printf("%s", qator);
fclose(f);
```

`FILE` — opaque struktura (9-bob): ichida fayl deskriptori va **bufer**. `fputc` har bir belgi uchun
syscall qilmaydi — buferga yig'adi va to'lganda (yoki `\n` da, yoki `fflush` da) bitta `write` qiladi.
Syscall qimmat (~100 ns+), shuning uchun buferlash dasturni o'nlab marta tezlashtiradi.

**Oqibatlar:**
- `printf` chiqishi darhol ko'rinmasligi mumkin (terminalda `\n` gacha, pipe'da — bufer to'lguncha).
- Dastur qulasa, buferdagi matn **yo'qoladi**. Debug xabarlari uchun `stderr` (buferlanmaydi) yoki
  `fflush(stdout)`. Mashqlardagi `TEST_BOSHLA()` aynan shuning uchun `setvbuf(stdout, NULL, _IONBF, 0)` qiladi.
- `fork` dan oldin `fflush` qilinmasa, buferdagi matn **ikki marta** chiqadi (bola nusxasi ham chiqaradi) —
  27-mashqdagi `_exit` maslahati shu sababdan.

Uch standart oqim: `stdin` (0), `stdout` (1), `stderr` (2).

MyOS: `user/libc/stdio.c` — `FILE`, buferlash va `fflush` ni o'qib chiqing.

## 12.4. `errno` — xato sababi

```c
#include <errno.h>
#include <string.h>

int fd = open("yoq.txt", O_RDONLY);
if (fd < 0) {
    printf("xato %d: %s\n", errno, strerror(errno));   /* xato 2: No such file or directory */
    perror("open");                                    /* qisqasi */
}
```

- Funksiya xato qilganda (-1 yoki NULL qaytarib) sababni global `errno` ga yozadi: `ENOENT` (fayl yo'q),
  `EACCES` (ruxsat yo'q), `ENOMEM`, `EINTR` (signal uzdi), `EAGAIN`...
- `errno` ni faqat funksiya **xato qaytarganda** tekshiring — muvaffaqiyatda u o'zgarmaydi (eski qiymat).
- **Yadro bilan bog'liqlik:** yadro syscall'dan `-ENOENT` (manfiy son) qaytaradi; libc uni ko'rib,
  `errno = ENOENT` qiladi va `-1` qaytaradi. MyOS: `user/libc/syscall.h` → `__sysret`. Kodlar ikkala
  tomonda bir xil — bitta fayldan: `include/myos/abi.h` (yadro ham, `user/include/errno.h` ham uni ishlatadi).

## 12.5. `<stdlib.h>` — asosiylari

| Funksiya | Nima |
|---|---|
| `malloc/calloc/realloc/free` | 8-bob |
| `exit(kod)` | dasturni tugatish (buferlarni `fflush` qilib, `atexit` funksiyalarini chaqirib) |
| `_exit(kod)` (`<unistd.h>`) | darhol tugatish, buferlarsiz — `fork` qilingan bolada |
| `strtol(s, &end, baza)` | satr → son, xatoni aniqlash mumkin (12-mashq) |
| `atoi(s)` | oddiy, lekin xatoni bildirmaydi |
| `qsort(massiv, n, hajm, cmp)` | saralash |
| `bsearch(...)` | saralangan massivda ikkilik qidiruv |
| `abs`, `labs` | modul |
| `rand`, `srand` | psevdotasodifiy sonlar (kriptografiya uchun emas!) |
| `getenv("PATH")` | muhit o'zgaruvchisi |

### `qsort` — funksiya ko'rsatkichi amalda

```c
static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a;        /* void * -> int * -> qiymat */
    int y = *(const int *)b;
    return (x > y) - (x < y);       /* -1, 0, 1. "x - y" TOSHISHI mumkin! */
}

int a[] = { 5, 2, 9, 1 };
qsort(a, 4, sizeof(a[0]), cmp_int);
```

`qsort` massiv turini bilmaydi — faqat baytlar (`void *`) va element hajmini. Taqqoslashni sizning
funksiyangizga topshiradi (19-mashq).

## 12.6. `assert`

```c
#include <assert.h>
assert(n > 0);          /* yolg'on bo'lsa: "Assertion `n > 0' failed" va abort() */
```

"Bu hech qachon bo'lmasligi kerak" degan invariantlar uchun. `-DNDEBUG` bilan o'chadi — shuning uchun
ichiga yon ta'sirli kod (`assert(x++ > 0)`) yozmang. Yadroda analogi — `BUG_ON(shart)` / `panic()`
(MyOS: `kernel/lib/panic.c`).

## 12.7. Yadroda libc yo'q — nima qilinadi

Yadro `-ffreestanding -nostdlib` bilan yig'iladi: `printf`, `malloc`, `strlen`, hatto `memcpy` ham
**yo'q**. Hammasi yadroning o'zida qayta yoziladi:

| libc | MyOS yadrosida |
|---|---|
| `printf` | `kprintf` — `kernel/lib/kprintf.c` (ekran + serial + dmesg) |
| `malloc/free` | `kmalloc/kfree` — `kernel/mm/slab.c` |
| `memcpy/strlen/...` | `kernel/lib/string.c` |
| `assert/abort` | `panic()` — `kernel/lib/panic.c` |
| `errno` | yo'q — xato kodi to'g'ridan-to'g'ri qaytariladi (`-ENOMEM`) |
| mutex | `kernel/lib/spinlock.c`, `mutex.c` |

Faqat bir nechta sarlavha freestanding'da ham bor, chunki ular faqat tur va makrolar: `<stdint.h>`,
`<stddef.h>`, `<stdbool.h>`, `<stdarg.h>`, `<limits.h>`. 18-bobda batafsil.

## 12.8. O'zingizni tekshiring

1. `printf("%08x", 255)` nima chiqaradi?
2. Nega `printf(s)` xavfli?
3. Dastur `printf("salom")` dan keyin qulasa, "salom" nega ko'rinmasligi mumkin?
4. `errno` ni qachon tekshirish kerak?
5. `qsort` taqqoslash funksiyasida nega `return x - y;` yozmaslik kerak?

<details><summary>Javoblar</summary>

1. `000000ff`.
2. `s` ichidagi `%` belgilarini `printf` format deb talqin qiladi (format string hujumi).
3. Matn stdout buferida qolib ketgan, `write` bo'lmagan.
4. Funksiya xato qaytargandan keyingina.
5. Katta qarama-qarshi ishorali sonlarda ayirish toshadi (UB) va ishora noto'g'ri chiqadi.
</details>

## 12.9. Mashqlar

- **11** (son → satr — `printf` yuragi), **12** (`strtol` o'xshashi), **19** (`qsort`), **24** (parsing).
- Qo'shimcha: `va_list` bilan o'z mini-`printf`ingizni yozing: `%d`, `%s`, `%x`, `%c`, `%%` —
  chiqarish uchun faqat `putchar` ishlating. Keyin MyOS `kernel/lib/kprintf.c` bilan solishtiring.

Keyingi bob: [13-bob. Aniqlanmagan xatti-harakat va xavfsizlik](13-ub-xavfsizlik.md)
