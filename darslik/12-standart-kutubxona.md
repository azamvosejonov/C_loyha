# 12-bob. Standart kutubxona (libc)

> **Bu bobda nima o'rganasiz:** C'ning tayyor funksiyalar to'plami (**libc**) nima ekanini; `printf` ning hamma imkoniyatlarini; fayl bilan ishlashni (`FILE *`)
> va **buferlash** nima ekanini; xato sababini bildiruvchi `errno` ni; `qsort` bilan saralashni; `assert` ni — va ularning **ichida** nima borligini
> (chunki yadroda ularning hech biri yo'q, o'zingiz yozasiz).
> **Oldindan nima kerak:** 5-, 6-, 7-, 9-boblar.   **Vaqt:** 5–6 soat.
> Mashqlar: 11, 12, 19, 24.

> **To'liq ishlaydigan misol:** [misollar/12_stdlib.c](misollar/12_stdlib.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Hozirgacha ishlatgan `printf`, `strlen`, `malloc` — bular C **tilining** qismi emas. Ular **tayyor funksiyalar to'plami**ga — **standart kutubxona**ga (**libc**) tegishli.
Bu bobda eng foydali tayyor qismlarni tartib bilan o'rganamiz: har birining **vazifasi** nima ekanini, keyin qanday ishlatilishini.

**Hayotdan misol: tayyor ehtiyot qismlar do'koni.** Mashina yig'ayotganda har bir boltni o'zingiz yasamaysiz — do'kondan tayyorini olasiz. `printf`, `strlen`, `qsort`, `malloc` —
tayyor qismlar. Ularni minglab odam yillar davomida sinagan. O'zingiz yozgan `strlen` dan ular tezroq va ishonchliroq.

| Do'kondagi bo'lim | libc'da | Vazifasi |
|---|---|---|
| chiqarish bo'limi | `printf`, `puts` | ekranga yozish |
| arxiv | `fopen`, `fgets`, `fclose` | fayllar bilan ishlash |
| diagnostika | `errno`, `strerror` | xato sababini aytish |
| kutubxonachi | `qsort` | saralash |
| nazoratchi | `assert` | "bu hech qachon bo'lmasligi kerak" tekshiruvi |

## 12.1. libc nima

C tilining o'zi juda kichik: kalit so'zlar va operatorlar. `printf`, `malloc`, `strlen`, `fopen` — bular tilning qismi emas, **standart kutubxonaning** (libc) funksiyalari:
oddiy C'da yozilgan kod. Linux'da — glibc yoki musl, MyOS'da — `user/libc/` (siz o'qiy oladigan ~2000 qator).

Kutubxona funksiyalaridan foydalanish uchun tegishli **sarlavha** (`#include`) qo'shiladi:

| Sarlavha | Vazifasi | Misollar |
|---|---|---|
| `<stdio.h>` | kiritish/chiqarish | `printf`, `fopen`, `fgets` |
| `<stdlib.h>` | umumiy yordamchilar | `malloc`, `free`, `exit`, `strtol`, `qsort`, `abs` |
| `<string.h>` | satr va xotira bilan ishlash (6-bob) | `strlen`, `memcpy`, `strcmp` |
| `<ctype.h>` | belgilarni tekshirish | `isdigit`, `toupper` |
| `<stdint.h>`, `<stddef.h>`, `<stdbool.h>`, `<limits.h>` | turlar va chegaralar | `uint32_t`, `size_t`, `INT_MAX` |
| `<errno.h>` | xato sababi | `errno`, xato kodlari |
| `<time.h>` | vaqt | |
| `<assert.h>` | tekshiruv | `assert(shart)` |
| `<unistd.h>`, `<fcntl.h>`, `<sys/wait.h>` | **POSIX** (standart C emas) | `read`, `fork` (14-bob) |

Har bir sarlavhaning batafsil tavsifi — [sarlavhalar.md](sarlavhalar.md).

## 12.2. `printf` — to'liq

**Bu nima?** `printf` — ekranga **formatlangan** matn chiqaruvchi funksiya. "Formatlangan" — matn ichiga qiymatlarni (son, harf, manzil) kerakli ko'rinishda qo'yib chiqarish.
**Asosiy ishi:** qolip matnini olish va undagi `%...` joylarni argumentlar bilan to'ldirib ekranga yozish.

### Qolip tuzilishi

Har bir `%` dan keyin kelgan qism — bitta "bo'sh joy" tavsifi:

```text
%[bayroqlar][kenglik][.aniqlik][uzunlik]tur
```

| Qism | Vazifasi | Misol |
|---|---|---|
| `tur` | **nimani** chiqarish (majburiy) | `d` butun, `s` matn, `x` o'n oltilik, `c` belgi, `f` kasr, `p` manzil |
| `kenglik` | **qancha joy** ajratish | `%5d` — 5 katak |
| `.aniqlik` | kasrda nuqtadan keyin nechta raqam; matnda — nechta belgi | `%.2f`, `%.3s` |
| `bayroqlar` | tekislash va to'ldirish usuli | `-` chapga tekislash, `0` nol bilan to'ldirish, `+` ishora |
| `uzunlik` | argument **turi** | `l` long, `z` size_t |

**Hayotdan misol: blanka katakchalari.** Davlat blankalarida har bir maydon uchun ma'lum sonli katakcha bor. `%5d` — "son uchun 5 ta katakcha, o'ngga tekisla", `%-10s` — "matn uchun 10 ta
katakcha, chapga tekisla", `%05d` — "bo'sh katakchalarga 0 yoz". Jadvallar shuning uchun tekis chiqadi.

```c
/* printf_formatlar.c - printf formatlari bir joyda */
#include <stdio.h>

int main(void)
{
    printf("[%5d]\n", 42);                 /* kenglik 5, o'ngga tekislangan */
    printf("[%-5d]\n", 42);                /* - : chapga */
    printf("[%05d]\n", 42);                /* 0 : nol bilan to'ldirish */
    printf("[%+d]\n", 42);                 /* + : doim ishora */
    printf("[%x %X %#x]\n", 255, 255, 255);        /* o'n oltilik: kichik, katta, 0x bilan */
    printf("[%08lx]\n", 0xBEEFUL);         /* manzillar/registrlar uchun klassik */
    printf("[%.3s]\n", "salom");           /* satr uchun aniqlik = maksimal uzunlik */
    printf("[%.2f]\n", 3.14159);           /* nuqtadan keyin 2 raqam */
    printf("[%*d]\n", 6, 42);              /* kenglik argumentdan olinadi */
    return 0;
}
```

```console
$ gcc -Wall -Wextra printf_formatlar.c -o printf_formatlar
$ ./printf_formatlar
[   42]
[42   ]
[00042]
[+42]
[ff FF 0xff]
[0000beef]
[sal]
[3.14]
[    42]
```

**Bu dastur nima qiladi:** bitta `42` sonini turli qoliplar bilan chiqaradi — shunda har bir qolip farqi ko'rinadi. Kvadrat qavs `[ ]` faqat bo'sh joylar ko'rinsin deb qo'yilgan
(printf uchun oddiy matn).

**Har bir qator (avval vazifasi, keyin natija):**

| Qolip | Vazifasi | Natija |
|---|---|---|
| `%5d` | son uchun 5 katak, o'ngga tekisla | `[   42]` |
| `%-5d` | 5 katak, **chapga** tekisla | `[42   ]` |
| `%05d` | bo'sh kataklarga **nol** yoz | `[00042]` |
| `%+d` | ishorani doim ko'rsat | `[+42]` |
| `%x %X %#x` | o'n oltilik: kichik harf, katta harf, `0x` prefiksi bilan | `[ff FF 0xff]` |
| `%08lx` | 8 katak, nol bilan to'ldirilgan, o'n oltilik, `long` | `[0000beef]` |
| `%.3s` | satrdan **faqat 3 belgi** | `[sal]` |
| `%.2f` | kasrdan nuqtadan keyin 2 raqam | `[3.14]` |
| `%*d` | kenglikni argumentdan ol (`6`), keyin son (`42`) | `[    42]` |

Uzunlik modifikatorlari: `hh` (char), `h` (short), `l` (long), `ll` (long long), `z` (size_t), `t` (ptrdiff_t). `uint64_t` uchun: `%lu` (Linux x86-64) yoki portativ `PRIu64` (`<inttypes.h>`).

### `printf` ning qaytish qiymati

`printf` **chiqarilgan belgilar sonini** qaytaradi. `snprintf(buf, n, ...)` — natijani **buferga** yozadi (ko'pi bilan `n-1` belgi + `'\0'`) va to'liq natija uchun kerakli uzunlikni qaytaradi (6-bob).

### Xavfsizlik: `printf(foydalanuvchi_satri)` — hech qachon!

**Nega xavfli?** Agar foydalanuvchi satrida `%x` yoki `%s` bo'lsa, `printf` uni **qolip** deb o'qiydi va **yo'q argumentlarni** stekdan o'qiy boshlaydi (ma'lumot oqib chiqadi,
`%n` bilan esa xotiraga yozish ham mumkin: "format string" hujumi). To'g'risi: `printf("%s", satr)`.

```c
/* format_xavfsizlik.c - satrni qolip sifatida ishlatish */
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;
    printf(argv[1]);                    /* XATO: foydalanuvchi satri qolip bo'ldi */
    printf("\n");
    printf("%s\n", argv[1]);            /* TO'G'RI: satr oddiy ma'lumot sifatida */
    return 0;
}
```

```console
$ gcc -Wall -Wextra -Wformat-security format_xavfsizlik.c -o format_xavfsizlik # xato kutiladi
format_xavfsizlik.c: In function ‘main’:
format_xavfsizlik.c:8:5: warning: format not a string literal and no format arguments [-Wformat-security]
    8 |     printf(argv[1]);                    /* XATO: foydalanuvchi satri qolip bo'ldi */
      |     ^~~~~~
$ ./format_xavfsizlik "salom"
salom
salom
```

Kompilyator ogohlantirdi (`format not a string literal`). Oddiy matn bilan ikkala usul bir xil ishladi; xavf `%` belgilar bilan paydo bo'ladi — shuning uchun doim `printf("%s", satr)`.

### O'zgaruvchan sonli argumentlar: `va_list`

**Bu nima?** `printf` har safar turlicha argument oladi. Shunday funksiyani o'zingiz ham yozish mumkin — buning uchun `<stdarg.h>`.
**Asosiy ishi:** argumentlar ro'yxatini (`...`) birma-bir olish (`va_arg`).

```c
/* yigindi.c - o'zgaruvchan sonli argumentlar */
#include <stdarg.h>
#include <stdio.h>

static int yigindi(int n, ...)          /* n - keyin nechta son kelishini aytadi */
{
    va_list ap;
    va_start(ap, n);                    /* n dan keyingi argumentlardan boshlash */
    int s = 0;
    for (int i = 0; i < n; i++)
        s += va_arg(ap, int);           /* navbatdagisini int deb ol */
    va_end(ap);
    return s;
}

int main(void)
{
    printf("yigindi(3, 10, 20, 30) = %d\n", yigindi(3, 10, 20, 30));
    return 0;
}
```

```console
$ gcc -Wall -Wextra yigindi.c -o yigindi
$ ./yigindi
yigindi(3, 10, 20, 30) = 60
```

**Qismlar:** `...` — "bu yerga istalgancha argument kelishi mumkin". `va_start` — ro'yxatni boshlash, `va_arg(ap, int)` — "navbatdagi argumentni `int` deb ol", `va_end` — tugatish.
Funksiya argumentlar **sonini** o'zi bilmaydi, shuning uchun `n` ni birinchi argument qilib beramiz (`printf` esa buni qolipdan biladi: nechta `%`, shuncha argument).

**Ichida nima bor:** format satrini belgima-belgi o'qish, `%` ni ko'rganda `va_arg` bilan navbatdagi argumentni olish, sonni satrga aylantirish (11-mashq!).
MyOS: `user/libc/printf.c`, `kernel/lib/kprintf.c` — ikkalasi ham ~300 qator.

## 12.3. `FILE *` va buferlash

**Bu nima?** `FILE *` — ochilgan faylni ifodalovchi "tutqich". **Asosiy ishi:** faylni ochish (`fopen`), o'qish/yozish (`fgets`, `fprintf`) va yopish (`fclose`).

```c
/* fayl_oqish.c - fayl yozish, o'qish va xatoni ushlash */
#include <stdio.h>

int main(void)
{
    FILE *f = fopen("royxat.txt", "w");         /* "w" - yozish uchun (tozalab) ochish */
    if (!f) {
        perror("fopen");
        return 1;
    }
    fprintf(f, "birinchi qator\n");
    fprintf(f, "ikkinchi qator\n");
    fclose(f);                                   /* yozishni tugatish va yopish */

    f = fopen("royxat.txt", "r");                /* "r" - o'qish uchun */
    if (!f) {
        perror("fopen");
        return 1;
    }
    char qator[256];
    while (fgets(qator, sizeof(qator), f))       /* qatorma-qator */
        printf("o'qildi: %s", qator);
    fclose(f);

    f = fopen("yoq_fayl.txt", "r");              /* mavjud bo'lmagan fayl */
    if (!f)
        perror("fopen");                          /* sababini chop etadi */
    return 0;
}
```

```console
$ gcc -Wall -Wextra fayl_oqish.c -o fayl_oqish
$ ./fayl_oqish
o'qildi: birinchi qator
o'qildi: ikkinchi qator
fopen: No such file or directory
```

**Bu dastur nima qiladi:** (1) `royxat.txt` faylini yaratib, ichiga ikki qator yozadi; (2) uni qayta ochib, qatorma-qator o'qiydi; (3) mavjud bo'lmagan faylni ochishga urinib, xato sababini ko'rsatadi.

**Qismlar:**

| Qism | Vazifasi | Tafsilot |
|---|---|---|
| `fopen(nom, rejim)` | faylni ochish | Rejim: `"r"` o'qish, `"w"` yozish (tozalab), `"a"` oxiriga qo'shish, `"rb"` binar. Muvaffaqiyatsiz bo'lsa `NULL` qaytaradi — **doim tekshiring** |
| `fprintf(f, ...)` | faylga formatlab yozish | `printf` ning fayl varianti: birinchi argument — fayl |
| `fgets(qator, hajm, f)` | bitta qatorni o'qish | `hajm` — bufer hajmi (xavfsiz, 6.9). Fayl tugasa `NULL` |
| `fclose(f)` | faylni yopish | bufer diskka yoziladi va tutqich bo'shatiladi |
| `perror("fopen")` | xato xabarini chiqarish | "fopen: No such file or directory" — nima va **nega** (12.4) |

`FILE` — opaque struktura (9-bob): ichida fayl deskriptori va **bufer**.

### Buferlash nima

**Hayotdan misol: pochta qutisi.** Pochtachi har bir xat uchun alohida kelmaydi — xatlar qutiga yig'iladi va quti to'lganda (yoki belgilangan vaqtda) bir yo'la olib ketiladi. `fprintf` ham yozuvni
avval xotiradagi **buferga** qo'yadi, keyin bir yo'la diskka yozadi — bu ming marta tezroq (syscall qimmat: ~100 ns+). `fflush` — "pochtachini hozir chaqir". `fclose` — oxirgi yig'ilganini ham
jo'natib, qutini yopish. **Dastur qulasa, qutidagi xatlar yo'qoladi.**

Buni ko'ramiz: `printf("salom")` dan keyin dastur qulasa, "salom" ko'rinadimi?

```c
/* bufer_yoqoladi.c - qulagan dasturda bufer yo'qoladi */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    (void)argv;
    printf("salom (buferda)");           /* bufer: ekranga/faylga hali yozilmadi */
    if (argc > 1)
        fflush(stdout);                  /* argument berilsa - buferni darhol yuborish */
    abort();                             /* dastur "qulaydi" */
}
```

```console
$ gcc -Wall -Wextra bufer_yoqoladi.c -o bufer_yoqoladi
$ bash -c '(./bufer_yoqoladi > chiqish1.txt) 2>/dev/null; echo "fflush siz: $(wc -c < chiqish1.txt) bayt"' 2>&1 | grep -v Aborted
fflush siz: 0 bayt
$ bash -c '(./bufer_yoqoladi x > chiqish2.txt) 2>/dev/null; echo "fflush bilan: $(wc -c < chiqish2.txt) bayt"' 2>&1 | grep -v Aborted
fflush bilan: 15 bayt
```

**Nima ko'rdik:** chiqish faylga yo'naltirilganda (`>`) `stdout` **to'liq buferlanadi**. `fflush` siz — hech narsa yozilmadi (0 bayt); `fflush` bilan — matn saqlandi. Terminalda esa
`\n` gacha buferlanadi.

**Oqibatlar:**
- `printf` chiqishi darhol ko'rinmasligi mumkin (terminalda `\n` gacha, pipe'da — bufer to'lguncha).
- Dastur qulasa, buferdagi matn **yo'qoladi**. Debug xabarlari uchun `stderr` (buferlanmaydi) yoki `fflush(stdout)`. Mashqlardagi `TEST_BOSHLA()` aynan shuning uchun `setvbuf(stdout, NULL, _IONBF, 0)` qiladi.
- `fork` dan oldin `fflush` qilinmasa, buferdagi matn **ikki marta** chiqadi (bola nusxasi ham chiqaradi) — 27-mashqdagi `_exit` maslahati shu sababdan.

Uch standart oqim: `stdin` (0), `stdout` (1), `stderr` (2). MyOS: `user/libc/stdio.c` — `FILE`, buferlash va `fflush` ni o'qib chiqing.

## 12.4. `errno` — xato sababi

**Bu nima?** `errno` — "oxirgi xato sababi" raqami. **Asosiy ishi:** funksiya `-1` yoki `NULL` qaytarib "xato bo'ldi" desa, **nima uchun** bo'lganini aytish.

**Hayotdan misol: mashina panelidagi xato kodi.** Mashina "Check engine" chirog'ini yoqadi — muammo bor. Ammo **qanday** muammo — diagnostika kodi aytadi: P0301. `fopen` ham `NULL` qaytaradi
(chiroq), sababini esa `errno` aytadi: `ENOENT` (fayl yo'q), `EACCES` (ruxsat yo'q). `strerror` — kodni odam tiliga tarjima qiladi.

```c
/* errno_misol.c - xato kodini o'qish */
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    FILE *f = fopen("yoq_papka/fayl.txt", "r");
    if (f == NULL) {                                        /* birinchi: xato bo'ldimi? */
        int kod = errno;                                    /* keyin: sababi nima? */
        printf("xato %d: %s\n", kod, strerror(kod));
        printf("ENOENT = %d\n", ENOENT);
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra errno_misol.c -o errno_misol
$ ./errno_misol
xato 2: No such file or directory
ENOENT = 2
```

**Qismlar:**

- `f == NULL` — **avval** xato bo'lganini aniqlaymiz (funksiyaning qaytish qiymatidan).
- `errno` — **keyin** sababini o'qiymiz. Raqam: `2` = `ENOENT` ("fayl/papka yo'q").
- `strerror(kod)` — raqamni odam tilidagi matnga aylantiradi. `perror("matn")` — qisqasi: "matn: sabab" ni `stderr` ga yozadi.

Boshqa kodlar: `EACCES` (ruxsat yo'q), `ENOMEM` (xotira yo'q), `EINTR` (signal uzdi), `EAGAIN`...

> **Eslab qoling:** `errno` ni faqat funksiya **xato qaytarganda** tekshiring — muvaffaqiyatda u o'zgarmaydi (eski qiymat qoladi).

**Yadro bilan bog'liqlik:** yadro syscall'dan `-ENOENT` (manfiy son) qaytaradi; libc uni ko'rib, `errno = ENOENT` qiladi va `-1` qaytaradi. MyOS: `user/libc/syscall.h` → `__sysret`.
Kodlar ikkala tomonda bir xil — bitta fayldan: `include/myos/abi.h` (yadro ham, `user/include/errno.h` ham uni ishlatadi).

## 12.5. `<stdlib.h>` — asosiylari

| Funksiya | Vazifasi |
|---|---|
| `malloc/calloc/realloc/free` | dinamik xotira (8-bob) |
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

**Bu nima?** `qsort` — massivni tartiblaydigan tayyor funksiya. **Asosiy ishi:** saralash algoritmini o'zi bajarish; **qoidani** (nimani nimadan oldin qo'yish) siz berasiz.

**Hayotdan misol: kutubxonachi.** Kutubxonachiga aytasiz: "Kitoblarni tartibla". U so'raydi: "Qanday qoida bilan? Muallif bo'yichami, yil bo'yichami?" Siz **solishtirish qoidasini** berasiz
(funksiya), tartiblash ishini u o'zi qiladi. Qoidani o'zgartirsangiz — tartib o'zgaradi, kutubxonachi o'sha.

```c
/* qsort_misol.c - solishtirish funksiyasi bilan saralash */
#include <stdio.h>
#include <stdlib.h>

static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a;            /* void * -> int * -> qiymat */
    int y = *(const int *)b;
    return (x > y) - (x < y);           /* -1, 0 yoki 1 */
}

int main(void)
{
    int a[] = { 5, 2, 9, 1 };
    qsort(a, 4, sizeof(a[0]), cmp_int);
    for (int i = 0; i < 4; i++)
        printf("%d ", a[i]);
    printf("\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra qsort_misol.c -o qsort_misol
$ ./qsort_misol
1 2 5 9 
```

**Qismlar:**

- `qsort(a, 4, sizeof(a[0]), cmp_int)` — to'rt argument: **qaysi massiv**, **nechta element**, **bitta element hajmi**, **solishtirish funksiyasi** (uning manzili — 5.10).
- `cmp_int(a, b)` — `qsort` har gal ikki elementning **manzilini** beradi (`const void *` — turi noma'lum, chunki `qsort` massiv turini bilmaydi). Biz uni `(const int *)` ga aylantirib, qiymatni olamiz.
- Natija: `a` birinchisi kichik bo'lsa **manfiy**, teng bo'lsa **0**, katta bo'lsa **musbat**.

**Nega `return x - y;` emas?** `x - y` katta qarama-qarshi ishorali sonlarda **toshadi** (UB): masalan `2147483647 - (-5)` `int` ga sig'maydi, ishora noto'g'ri chiqadi.
`(x > y) - (x < y)` hech qachon toshmaydi. 19-mashq.

> **Eslab qoling:** `qsort` massiv turini bilmaydi; faqat baytlar (`void *`) va element hajmini. Taqqoslashni **sizning funksiyangiz** hal qiladi.

## 12.6. `assert`

**Bu nima?** `assert(shart)` — "shu shart **rost bo'lishi shart**" degan tekshiruv. **Asosiy ishi:** shart yolg'on bo'lsa, dasturni darhol to'xtatib, **qaysi qatorda** ekanini aytish — xato uzoqqa ketmaydi.

**Hayotdan misol: uchishdan oldingi tekshiruv ro'yxati.** Uchuvchi har parvozdan oldin ro'yxat bo'yicha tekshiradi: "yoqilg'i bor, g'ildiraklar joyida". Bittasi bajarilmasa — samolyot uchmaydi.

```c
/* assert_misol.c - assert */
#include <assert.h>
#include <stdio.h>

static int bol(int a, int b)
{
    assert(b != 0);                     /* b nol bo'lmasligi KERAK */
    return a / b;
}

int main(void)
{
    printf("10 / 2 = %d\n", bol(10, 2));
    printf("10 / 0 = %d\n", bol(10, 0));    /* shart buziladi */
    return 0;
}
```

```console
$ gcc -Wall -Wextra assert_misol.c -o assert_misol
$ bash -c './assert_misol 2>&1; echo "chiqish kodi: $?"' 2>&1 | grep -v Aborted
assert_misol: assert_misol.c:7: bol: Assertion `b != 0' failed.
chiqish kodi: 134
```

**Nima ko'rdik:** birinchi chaqiruv (`10 / 2`) o'tdi; ikkinchisida `assert` fayl, qator, funksiya va **shart matnini** chiqarib, `abort()` qildi (chiqish kodi 134 = 128 + 6, signal `SIGABRT`).

Diqqat: `10 / 2 = 5` qatori ekranda **ko'rinmadi**! Chiqish `|` orqali yo'naltirilgani uchun `stdout` buferlangan va `abort()` buferni yubormadi — bu 12.3 dagi dars: qulagan dasturda bufer **yo'qoladi**. Xato xabari (`stderr`) esa buferlanmaydi, shuning uchun ko'rindi.

`-DNDEBUG` bilan yig'sangiz, `assert` butunlay **o'chadi** — shuning uchun ichiga yon ta'sirli kod (`assert(x++ > 0)`) yozmang. Yadroda analogi — `BUG_ON(shart)` / `panic()` (MyOS: `kernel/lib/panic.c`).

## 12.7. Yadroda libc yo'q — nima qilinadi

Yadro `-ffreestanding -nostdlib` bilan yig'iladi: `printf`, `malloc`, `strlen`, hatto `memcpy` ham **yo'q**. Hammasi yadroning o'zida qayta yoziladi:

| libc | MyOS yadrosida |
|---|---|
| `printf` | `kprintf` — `kernel/lib/kprintf.c` (ekran + serial + dmesg) |
| `malloc/free` | `kmalloc/kfree` — `kernel/mm/slab.c` |
| `memcpy/strlen/...` | `kernel/lib/string.c` |
| `assert/abort` | `panic()` — `kernel/lib/panic.c` |
| `errno` | yo'q — xato kodi to'g'ridan-to'g'ri qaytariladi (`-ENOMEM`) |
| mutex | `kernel/lib/spinlock.c`, `mutex.c` |

Faqat bir nechta sarlavha freestanding'da ham bor, chunki ular faqat tur va makrolar: `<stdint.h>`, `<stddef.h>`, `<stdbool.h>`, `<stdarg.h>`, `<limits.h>`. 18-bobda batafsil.

## Hayotdan misol va to'liq dastur

**Imtihon natijalari.** Talabalarni ball bo'yicha tartiblaymiz (`qsort`), faylga yozamiz (`fprintf`), qayta o'qib jadval chiqaramiz (`fscanf`, `printf` formatlari) va xatoni ushlaymiz (`errno`).

```c
/* imtihon.c - qsort, faylga yozish/o'qish, printf formatlari, errno */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct talaba {
    char ism[20];
    int ball;
};

/* Solishtirish qoidasi: ball bo'yicha kamayish tartibida, teng bo'lsa - ism bo'yicha */
static int ball_boyicha(const void *a, const void *b)
{
    const struct talaba *x = a, *y = b;
    if (x->ball != y->ball)
        return y->ball - x->ball;
    return strcmp(x->ism, y->ism);
}

int main(void)
{
    struct talaba guruh[] = {
        { "Jasur", 78 }, { "Madina", 95 }, { "Otabek", 64 }, { "Zarina", 95 }, { "Kamola", 88 },
    };
    int n = sizeof(guruh) / sizeof(guruh[0]);
    qsort(guruh, n, sizeof(guruh[0]), ball_boyicha);

    FILE *f = fopen("natijalar.txt", "w");
    if (!f) {
        printf("Faylni ochib bo'lmadi: %s\n", strerror(errno));
        return 1;
    }
    for (int i = 0; i < n; i++)
        fprintf(f, "%s %d\n", guruh[i].ism, guruh[i].ball);
    fclose(f);                                  /* bufer diskka yoziladi */

    /* Fayldan qayta o'qib, jadval chiqarish */
    f = fopen("natijalar.txt", "r");
    if (!f)
        return 1;
    char ism[20];
    int ball, orin = 0;
    printf("%-4s %-10s %5s  %s\n", "O'rin", "Ism", "Ball", "Baho");
    while (fscanf(f, "%19s %d", ism, &ball) == 2) {
        const char *baho = ball >= 86 ? "a'lo" : ball >= 71 ? "yaxshi" : "qoniqarli";
        printf("%-5d %-10s %5d  %s\n", ++orin, ism, ball, baho);
    }
    fclose(f);

    if (fopen("yoq_papka/fayl.txt", "r") == NULL)
        printf("\nyoq_papka/fayl.txt: errno = %d (%s)\n", errno, strerror(errno));
    return 0;
}
```

```console
$ gcc -Wall -Wextra imtihon.c -o imtihon
$ ./imtihon
O'rin Ism         Ball  Baho
1     Madina        95  a'lo
2     Zarina        95  a'lo
3     Kamola        88  a'lo
4     Jasur         78  yaxshi
5     Otabek        64  qoniqarli

yoq_papka/fayl.txt: errno = 2 (No such file or directory)
$ cat natijalar.txt
Madina 95
Zarina 95
Kamola 88
Jasur 78
Otabek 64
```

**Bu dastur nima qiladi (umumiy):** 5 talabaning ballini kamayish tartibida saralaydi, `natijalar.txt` ga yozadi, fayldan qayta o'qib baholar bilan jadval chiqaradi va oxirida mavjud bo'lmagan faylni ochib xato kodini ko'rsatadi.

**Qismlar (har birining vazifasi):**

| Qism | Vazifasi |
|---|---|
| `struct talaba` | bitta talaba: ism + ball |
| `ball_boyicha` | `qsort` uchun **qoida**: ball katta bo'lsa oldin; teng bo'lsa — ism alifbo tartibida (`strcmp`) |
| `qsort(guruh, n, sizeof(...), ball_boyicha)` | saralashni bajaradi |
| `fopen("natijalar.txt", "w")` + `fprintf` + `fclose` | tartiblangan ro'yxatni faylga yozish |
| `fscanf(f, "%19s %d", ism, &ball)` | fayldan "ism son" juftini o'qish; `%19s` — ko'pi bilan 19 belgi (bufer to'lmasin); `== 2` — ikkala qiymat ham o'qildi |
| `ball >= 86 ? "a'lo" : ball >= 71 ? ...` | ball bo'yicha baho matnini tanlash (3.7) |
| `%-5d %-10s %5d` | jadval ustunlarini tekis chiqarish (12.2) |
| `errno`, `strerror` | oxirgi xato sababi (12.4) |

**Sinab ko'ring:** `ball_boyicha` ni o'zgartirib, ism bo'yicha alifbo tartibida saralang. `fclose(f);` (birinchisini) o'chirib, o'rniga `abort();` qo'ying — `natijalar.txt` da nima qoladi?

<!-- katta:boshi -->
## Katta loyiha: Ombor — 12-bosqich: faylga saqlash, `qsort`, `errno`, `assert`

**Oldingi bosqichdan:** modullarga bo'lingan, toza dastur. Lekin ikki muhim kamchilik: (1) dastur tugasa **hamma ma'lumot yo'qoladi** — ombor faqat xotirada; (2) saralash funksiyasi (7-bosqichdagi pufakcha) — sekin va faqat narx bo'yicha.

### Bu bosqichda nima qilamiz

Standart kutubxonaning (12-bob) uchta asosiy qismini ishlatamiz:

| Yangi imkoniyat | Vosita | Nega |
|---|---|---|
| **Faylga saqlash va yuklash** | `fopen`, `fprintf`, `fgets`, `sscanf`, `fclose` | ma'lumot dastur tugagandan keyin ham qoladi |
| **Saralash** (nom yoki narx bo'yicha) | `qsort` + **taqqoslagich funksiya** | tayyor, tez (O(n log n)) va xatosiz; o'zimiz yozmaymiz |
| **Xatoni aniq bildirish** | `errno`, `strerror` | "fayl topilmadi" yoki "format noto'g'ri" — foydalanuvchiga tushunarli |
| **Ichki to'g'rilikni tekshirish** | `assert` | "bu hech qachon bo'lmasligi kerak" shartini tekshiradi: buzilsa dastur darhol to'xtaydi (xatoni qidirishni osonlashtiradi) |

**Fayl formati — matn, har qatorda bitta mahsulot:** `nom;narx;soni;toifa;holat`. Matn format — odam ham o'qiy oladi va xatoni topish oson.

**Muhim dizayn qarori — `ombor_yukla` xavfsiz:** faylni **vaqtinchalik** omborga o'qiydi; faqat **hammasi muvaffaqiyatli** o'qilgandan keyin asl ombor almashtiriladi. Fayl buzuq bo'lsa, asl ombor **o'zgarmay** qoladi (*atomik* almashtirish g'oyasi — 27-bob: `tmp` + `rename`).

**Faqat o'zgargan fayllar:** `ombor.h` (yangi funksiyalar), `ombor.c` (yangi qismlar), `main.c` (3 ta menyu bandi). `log.h`, `pul.h`, `pul.c` o'zgarmagan (11-bosqichdagi bilan bir xil).

```c
/* ombor.h - ombor kutubxonasining OCHIQ interfeysi (12-bosqich: saralash, faylga saqlash) */
#ifndef OMBOR_H
#define OMBOR_H

#include <stdint.h>

/* toifalar ro'yxati bir joyda: enum ham, nomlar jadvali ham undan hosil bo'ladi */
#define TOIFALAR(X) \
    X(OZIQ_OVQAT, "oziq-ovqat") \
    X(ICHIMLIK, "ichimlik") \
    X(UY_RUZGOR, "uy-ro'zg'or")

#define X_ENUM(id, nom) id,
enum toifa { TOIFALAR(X_ENUM) TOIFA_SONI };
#undef X_ENUM

/* funksiyalar natija kodlari: 0 - OK, manfiy - xato */
enum { OK = 0, XOTIRA_YOQ = -1, NOM_BAND = -2, TOPILMADI = -3, NOTOGRI_MIQDOR = -4, YETARLI_EMAS = -5, NOTOGRI_TOIFA = -6 };

typedef struct ombor Ombor;                     /* faqat nom: ichi ombor.c da */

enum saralash { SARALASH_NOM = 1, SARALASH_NARX = 2 };

Ombor *ombor_yarat(void);
void ombor_yoq(Ombor *o);
int ombor_qosh(Ombor *o, const char *nom, long narx, uint16_t soni, enum toifa toifa);
int ombor_sot(Ombor *o, const char *nom, int miqdor, uint16_t *qoldi);
int ombor_ochir(Ombor *o, const char *nom);
int ombor_soni(const Ombor *o);
void ombor_royxat(const Ombor *o);
void ombor_hisobot(const Ombor *o);
void ombor_saralash(Ombor *o, enum saralash mezon);
int ombor_saqla(const Ombor *o, const char *fayl);      /* 0 yoki -1 (errno qo'yiladi) */
int ombor_yukla(Ombor *o, const char *fayl);            /* nechta mahsulot yuklandi yoki -1 (errno) */
const char *toifa_nomi(enum toifa t);
const char *ombor_xato(int kod);

#endif
```

`ombor.c` ning to'liq matni — o'zgargan va yangi qismlari izohlangan:

```c
/* ombor.c - ombor kutubxonasining ichki ishi (12-bosqich: qsort, fayl, errno, assert) */
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "ombor.h"
#include "pul.h"

#define YANGI      BIT(0)
#define TUGAYAPTI  BIT(2)

#define X_NOM(id, nom) nom,
static const char *const nomlar[] = { TOIFALAR(X_NOM) };
#undef X_NOM

struct mahsulot {
    char *nom;
    long narx;
    uint16_t soni;
    enum toifa toifa;
    unsigned holat;
};

struct ombor {                                  /* TA'RIF faqat shu faylda: tashqaridan ko'rinmaydi */
    struct mahsulot *m;
    int n;
    int sig;
};

const char *toifa_nomi(enum toifa t)
{
    return (unsigned)t < TOIFA_SONI ? nomlar[t] : "?";
}

Ombor *ombor_yarat(void)
{
    return calloc(1, sizeof(Ombor));            /* hammasi nolga: m = NULL, n = 0, sig = 0 */
}

void ombor_yoq(Ombor *o)
{
    if (!o)
        return;
    for (int i = 0; i < o->n; i++)
        free(o->m[i].nom);
    free(o->m);
    free(o);
}

static struct mahsulot *topish(const Ombor *o, const char *nom)
{
    for (int i = 0; i < o->n; i++)
        if (strcmp(o->m[i].nom, nom) == 0)
            return &o->m[i];
    return NULL;
}

static void holat_yangila(struct mahsulot *p)
{
    if (p->soni < 10)
        p->holat |= TUGAYAPTI;
    else
        p->holat &= ~TUGAYAPTI;
}

int ombor_qosh(Ombor *o, const char *nom, long narx, uint16_t soni, enum toifa toifa)
{
    if ((unsigned)toifa >= TOIFA_SONI)
        return NOTOGRI_TOIFA;
    if (topish(o, nom))
        return NOM_BAND;
    if (o->n == o->sig) {
        int yangi = o->sig ? o->sig * 2 : 2;
        struct mahsulot *y = realloc(o->m, (size_t)yangi * sizeof(*y));
        if (!y)
            return XOTIRA_YOQ;
        LOG("sig'im %d -> %d", o->sig, yangi);
        o->m = y;
        o->sig = yangi;
    }
    char *nusxa = strdup(nom);
    if (!nusxa)
        return XOTIRA_YOQ;
    assert(o->n < o->sig);                      /* joy borligiga ishonch: yo'q bo'lsa dastur to'xtaydi */
    struct mahsulot *p = &o->m[o->n++];
    p->nom = nusxa;
    p->narx = narx;
    p->soni = soni;
    p->toifa = toifa;
    p->holat = YANGI;
    holat_yangila(p);
    LOG("qo'shildi: %s (%d ta)", nom, o->n);
    return OK;
}

int ombor_sot(Ombor *o, const char *nom, int miqdor, uint16_t *qoldi)
{
    struct mahsulot *p = topish(o, nom);
    if (!p)
        return TOPILMADI;
    if (miqdor <= 0)
        return NOTOGRI_MIQDOR;
    if (miqdor > p->soni)
        return YETARLI_EMAS;
    p->soni -= miqdor;
    holat_yangila(p);
    if (qoldi)
        *qoldi = p->soni;
    LOG("sotildi: %s, %d dona", nom, miqdor);
    return OK;
}

int ombor_ochir(Ombor *o, const char *nom)
{
    struct mahsulot *p = topish(o, nom);
    if (!p)
        return TOPILMADI;
    int i = (int)(p - o->m);
    free(p->nom);
    memmove(&o->m[i], &o->m[i + 1], (size_t)(o->n - i - 1) * sizeof(*o->m));
    o->n--;
    return OK;
}

int ombor_soni(const Ombor *o)
{
    return o->n;
}

void ombor_royxat(const Ombor *o)
{
    printf("%-10s %-12s %10s %6s %s\n", "Mahsulot", "Toifa", "Narx", "Soni", "Holat");
    printf("----------------------------------------------------\n");
    long jami = 0;
    for (int i = 0; i < o->n; i++) {
        const struct mahsulot *p = &o->m[i];
        char narx_matn[32];
        pul_matn(narx_matn, sizeof(narx_matn), p->narx);
        printf("%-10s %-12s %10s %6u %s%s\n", p->nom, toifa_nomi(p->toifa), narx_matn, p->soni,
               (p->holat & YANGI) ? "[yangi] " : "", (p->holat & TUGAYAPTI) ? "[TUGAYAPTI]" : "");
        jami += p->narx * p->soni;
    }
    printf("----------------------------------------------------\n");
    printf("Jami qiymat: ");
    pul_chiqar(jami);
    printf(" so'm\n");
}

void ombor_hisobot(const Ombor *o)
{
    long toifa_jami[TOIFA_SONI] = { 0 };
    for (int i = 0; i < o->n; i++)
        toifa_jami[o->m[i].toifa] += o->m[i].narx * o->m[i].soni;
    for (int t = 0; t < TOIFA_SONI; t++) {
        printf("  %-12s ", toifa_nomi((enum toifa)t));
        pul_chiqar(toifa_jami[t]);
        printf(" so'm\n");
    }
}

/* qsort uchun taqqoslagichlar: ikki mahsulotni solishtiradi (manfiy / 0 / musbat) */
static int solishtir_nom(const void *a, const void *b)
{
    const struct mahsulot *x = a, *y = b;
    return strcmp(x->nom, y->nom);
}

static int solishtir_narx(const void *a, const void *b)
{
    const struct mahsulot *x = a, *y = b;
    return (y->narx > x->narx) - (y->narx < x->narx);   /* qimmati birinchi; ayirmasiz: toshmaydi */
}

void ombor_saralash(Ombor *o, enum saralash mezon)
{
    if (o->n > 1)
        qsort(o->m, (size_t)o->n, sizeof(*o->m), mezon == SARALASH_NARX ? solishtir_narx : solishtir_nom);
}

/* matn fayl: har qatorda "nom;narx;soni;toifa;holat" */
int ombor_saqla(const Ombor *o, const char *fayl)
{
    FILE *f = fopen(fayl, "w");
    if (!f)
        return -1;                              /* errno ni fopen o'zi qo'ydi */
    for (int i = 0; i < o->n; i++) {
        const struct mahsulot *p = &o->m[i];
        if (fprintf(f, "%s;%ld;%u;%d;%u\n", p->nom, p->narx, p->soni, (int)p->toifa, p->holat) < 0) {
            fclose(f);
            return -1;
        }
    }
    return fclose(f) == 0 ? 0 : -1;             /* fclose ham xato berishi mumkin (disk to'lgan) */
}

int ombor_yukla(Ombor *o, const char *fayl)
{
    FILE *f = fopen(fayl, "r");
    if (!f)
        return -1;
    Ombor *t = ombor_yarat();                   /* vaqtincha ombor: xato bo'lsa asl ombor buzilmaydi */
    if (!t) {
        fclose(f);
        errno = ENOMEM;
        return -1;
    }
    char qator[128];
    int son = 0;
    while (fgets(qator, sizeof(qator), f)) {
        char nom[24];
        long narx;
        unsigned short soni;
        int toifa;
        unsigned holat;
        if (sscanf(qator, "%23[^;];%ld;%hu;%d;%u", nom, &narx, &soni, &toifa, &holat) != 5 ||
            ombor_qosh(t, nom, narx, soni, (enum toifa)toifa) != OK) {
            ombor_yoq(t);
            fclose(f);
            errno = EINVAL;                     /* "Invalid argument": fayl formati noto'g'ri */
            return -1;
        }
        t->m[t->n - 1].holat = holat;
        son++;
    }
    fclose(f);
    for (int i = 0; i < o->n; i++)              /* hammasi yaxshi: eskisini almashtiramiz */
        free(o->m[i].nom);
    free(o->m);
    *o = *t;
    free(t);
    return son;
}

const char *ombor_xato(int kod)
{
    switch (kod) {
    case XOTIRA_YOQ: return "xotira yetmadi";
    case NOM_BAND: return "bunday nomli mahsulot allaqachon bor";
    case TOPILMADI: return "bunday mahsulot topilmadi";
    case NOTOGRI_MIQDOR: return "miqdor musbat bo'lishi kerak";
    case YETARLI_EMAS: return "omborda yetarli emas";
    case NOTOGRI_TOIFA: return "toifa 0, 1 yoki 2 bo'lishi kerak";
    default: return "noma'lum xato";
    }
}
```

```c
/* main.c - menyu (12-bosqich: saralash, saqlash, yuklash) */
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "ombor.h"

int main(void)
{
    Ombor *ombor = ombor_yarat();
    if (!ombor)
        return 1;
    ombor_qosh(ombor, "non", 400000, 120, OZIQ_OVQAT);
    ombor_qosh(ombor, "sut", 1200000, 45, ICHIMLIK);
    ombor_qosh(ombor, "guruch", 1800000, 8, OZIQ_OVQAT);

    int tanlov;
    while (printf("\n1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish\nTanlov:\n"),
           scanf("%d", &tanlov) == 1 && tanlov != 0) {
        char s[24];
        long so_m;
        int miqdor, toifa, r;
        uint16_t qoldi;
        switch (tanlov) {
        case 1:
            ombor_royxat(ombor);
            break;
        case 2:
            printf("Mahsulot nomi va necha dona?\n");
            if (scanf("%23s %d", s, &miqdor) != 2)
                break;
            r = ombor_sot(ombor, s, miqdor, &qoldi);
            if (r == OK)
                printf("  Sotildi: %d dona %s. Qoldi: %u dona\n", miqdor, s, qoldi);
            else
                printf("  XATO: %s\n", ombor_xato(r));
            break;
        case 3:
            printf("Nom, narx (so'mda), soni va toifa (0-oziq-ovqat, 1-ichimlik, 2-uy-ro'zg'or)?\n");
            if (scanf("%23s %ld %d %d", s, &so_m, &miqdor, &toifa) != 4)
                break;
            r = ombor_qosh(ombor, s, so_m * 100, (uint16_t)miqdor, (enum toifa)toifa);
            if (r == OK)
                printf("  Qo'shildi: %s (%s)\n", s, toifa_nomi((enum toifa)toifa));
            else
                printf("  XATO: %s\n", ombor_xato(r));
            break;
        case 4:
            printf("Qaysi mahsulot o'chirilsin?\n");
            if (scanf("%23s", s) != 1)
                break;
            r = ombor_ochir(ombor, s);
            if (r == OK)
                printf("  O'chirildi: %s (qoldi %d ta)\n", s, ombor_soni(ombor));
            else
                printf("  XATO: %s\n", ombor_xato(r));
            break;
        case 5:
            ombor_hisobot(ombor);
            break;
        case 6:
            printf("Saralash: 1-nom, 2-narx (qimmati birinchi)?\n");
            if (scanf("%d", &miqdor) != 1)
                break;
            ombor_saralash(ombor, miqdor == 2 ? SARALASH_NARX : SARALASH_NOM);
            printf("  Saralandi\n");
            break;
        case 7:
            printf("Qaysi faylga saqlansin?\n");
            if (scanf("%23s", s) != 1)
                break;
            if (ombor_saqla(ombor, s) == 0)
                printf("  Saqlandi: %s (%d ta mahsulot)\n", s, ombor_soni(ombor));
            else
                printf("  XATO: %s: %s\n", s, strerror(errno));
            break;
        case 8:
            printf("Qaysi fayldan yuklansin?\n");
            if (scanf("%23s", s) != 1)
                break;
            r = ombor_yukla(ombor, s);
            if (r >= 0)
                printf("  Yuklandi: %s (%d ta mahsulot)\n", s, r);
            else
                printf("  XATO: %s: %s\n", s, strerror(errno));
            break;
        default:
            printf("  XATO: menyuda bunday band yo'q\n");
        }
    }
    ombor_yoq(ombor);
    printf("\nXayr!\n");
    return 0;
}
```

`Makefile` — 11-bosqichdagi bilan bir xil (faqat `clean` yangi `ombor.txt` ni ham o'chiradi).

### Ishga tushirish

Buzuq fayl tayyorlaymiz va ssenariyni ishga tushiramiz:

```console
$ cd katta_loyiha/ombor/12_stdlib
$ printf 'sarlavha qatori\nbu fayl formati noto'"'"'g'"'"'ri\n' > buzuq.txt
$ printf '3\nsovun 3000 5 2\n3\nlimonad 6000 30 1\n6\n2\n1\n6\n1\n1\n7\nombor.txt\n8\nyoq_fayl.txt\n8\nbuzuq.txt\n2\nsovun 3\n8\nombor.txt\n1\n0\n' > kirish.txt
$ make
gcc -Wall -Wextra -g -MMD -c main.c -o main.o
gcc -Wall -Wextra -g -MMD -c ombor.c -o ombor.o
gcc -Wall -Wextra -g -MMD -c pul.c -o pul.o
gcc -Wall -Wextra -g main.o ombor.o pul.o -o ombor
$ ./ombor < kirish.txt

1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish
Tanlov:
Nom, narx (so'mda), soni va toifa (0-oziq-ovqat, 1-ichimlik, 2-uy-ro'zg'or)?
  Qo'shildi: sovun (uy-ro'zg'or)

1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish
Tanlov:
Nom, narx (so'mda), soni va toifa (0-oziq-ovqat, 1-ichimlik, 2-uy-ro'zg'or)?
  Qo'shildi: limonad (ichimlik)

1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish
Tanlov:
Saralash: 1-nom, 2-narx (qimmati birinchi)?
  Saralandi

1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish
Tanlov:
Mahsulot   Toifa              Narx   Soni Holat
----------------------------------------------------
guruch     oziq-ovqat     18000.00      8 [yangi] [TUGAYAPTI]
sut        ichimlik       12000.00     45 [yangi] 
limonad    ichimlik        6000.00     30 [yangi] 
non        oziq-ovqat      4000.00    120 [yangi] 
sovun      uy-ro'zg'or     3000.00      5 [yangi] [TUGAYAPTI]
----------------------------------------------------
Jami qiymat: 1359000.00 so'm

1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish
Tanlov:
Saralash: 1-nom, 2-narx (qimmati birinchi)?
  Saralandi

1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish
Tanlov:
Mahsulot   Toifa              Narx   Soni Holat
----------------------------------------------------
guruch     oziq-ovqat     18000.00      8 [yangi] [TUGAYAPTI]
limonad    ichimlik        6000.00     30 [yangi] 
non        oziq-ovqat      4000.00    120 [yangi] 
sovun      uy-ro'zg'or     3000.00      5 [yangi] [TUGAYAPTI]
sut        ichimlik       12000.00     45 [yangi] 
----------------------------------------------------
Jami qiymat: 1359000.00 so'm

1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish
Tanlov:
Qaysi faylga saqlansin?
  Saqlandi: ombor.txt (5 ta mahsulot)

1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish
Tanlov:
Qaysi fayldan yuklansin?
  XATO: yoq_fayl.txt: No such file or directory

1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish
Tanlov:
Qaysi fayldan yuklansin?
  XATO: buzuq.txt: Invalid argument

1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish
Tanlov:
Mahsulot nomi va necha dona?
  Sotildi: 3 dona sovun. Qoldi: 2 dona

1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish
Tanlov:
Qaysi fayldan yuklansin?
  Yuklandi: ombor.txt (5 ta mahsulot)

1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish
Tanlov:
Mahsulot   Toifa              Narx   Soni Holat
----------------------------------------------------
guruch     oziq-ovqat     18000.00      8 [yangi] [TUGAYAPTI]
limonad    ichimlik        6000.00     30 [yangi] 
non        oziq-ovqat      4000.00    120 [yangi] 
sovun      uy-ro'zg'or     3000.00      5 [yangi] [TUGAYAPTI]
sut        ichimlik       12000.00     45 [yangi] 
----------------------------------------------------
Jami qiymat: 1359000.00 so'm

1) ro'yxat 2) sotish 3) qo'shish 4) o'chirish 5) hisobot 6) saralash 7) saqlash 8) yuklash 0) chiqish
Tanlov:

Xayr!
$ cat ombor.txt
guruch;1800000;8;0;5
limonad;600000;30;1;1
non;400000;120;0;1
sovun;300000;5;2;5
sut;1200000;45;1;1
```

**Ssenariy (kirish tahlili):**

| Kirish | Natija |
|---|---|
| `3` sovun, `3` limonad | mahsulotlar qo'shildi (jami 5) |
| `6`, `2` va `1` | **narx bo'yicha** saralash → ro'yxat: guruch, sut, limonad, non, sovun (qimmatdan arzonga) |
| `6`, `1` va `1` | **nom bo'yicha** saralash → alifbo tartibi: guruch, limonad, non, sovun, sut |
| `7 ombor.txt` | **saqlandi** (5 ta) |
| `8 yoq_fayl.txt` | **XATO: No such file or directory** (`errno = ENOENT`, `fopen` qo'ygan) |
| `8 buzuq.txt` | **XATO: Invalid argument** (`errno = EINVAL`: format noto'g'ri; dastur o'zi qo'ydi) |
| `2 sovun 3` | 3 dona sotildi → qoldi 2 |
| `8 ombor.txt` | **yuklandi**: asl holat tiklandi — sovun yana 5 dona (saqlangandan keyingi sotish **bekor** bo'ldi) |
| `1` | yakuniy ro'yxat |

`cat ombor.txt` — saqlangan fayl: har qatorda `nom;narx(tiyin);soni;toifa;holat`.

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `qsort(o->m, n, sizeof(*o->m), solishtir)` | massivni saralaydi: **boshlanishi**, **elementlar soni**, **bir element hajmi**, **taqqoslagich** |
| `solishtir_nom(const void *a, const void *b)` | ikki elementni solishtiradi: `<0` (a oldin), `0` (teng), `>0` (b oldin); `void *` ni **aniq turga** aylantiramiz |
| `(y->narx > x->narx) - (y->narx < x->narx)` | solishtirishning toshmaydigan shakli: `y->narx - x->narx` ayirmasi katta sonlarda **toshib ketishi** mumkin |
| `fopen(fayl, "w")` | yozish uchun ochadi; muvaffaqiyatsiz bo'lsa `NULL`, `errno` o'rnatiladi |
| `fprintf(f, "%s;%ld;...\n", ...)` | `printf` ning fayl varianti; **manfiy** natija — yozish xatosi |
| `fclose(f) == 0` | `fclose` ham xato berishi mumkin (disk to'lgan bo'lsa — buferdagi ma'lumot shu paytda yoziladi!) |
| `fgets(qator, sizeof(qator), f)` | fayldan bir qator o'qiydi, **chegarali** |
| `sscanf(qator, "%23[^;];%ld;%hu;%d;%u", ...)` | qatorni qismlarga ajratadi: `%23[^;]` — "`;` gacha, ko'pi bilan 23 belgi"; **5** qiymat o'qilishi shart |
| `errno = EINVAL;` | o'z xatomizni ham `errno` orqali bildiramiz; `main` `strerror(errno)` bilan matn qiladi |
| `assert(o->n < o->sig)` | "joy bor" deb **ishonamiz**; buzilsa dastur `Assertion failed` bilan to'xtaydi — mantiq xatosini darhol topamiz |
| `*o = *t; free(t);` | vaqtinchalik omborning ichini asl omborga **ko'chirib**, o'zini bo'shatamiz (nomlar `t` dan `o` ga o'tdi, qayta `free` qilinmaydi) |

**Xatolar uch toifada:** (1) **foydalanuvchi xatosi** (noto'g'ri raqam, yo'q fayl) — natija kodi + xabar; (2) **muhit xatosi** (disk to'lgan, xotira yo'q) — `errno` va tekshiruv; (3) **dasturchi xatosi** (mantiq buzildi) — `assert`. Ularni **aralashtirmang**: `assert` foydalanuvchi kiritishini tekshirmaydi!

> **Eslab qoling:** `qsort` + taqqoslagich — tayyor saralash; **hamma** fayl amali (`fopen`, `fprintf`, `fclose`) natijasini tekshiring; `errno` + `strerror` — sababni aytadi; yuklashni **vaqtinchalik** obyektda bajaring — xato bo'lsa asl ma'lumot buzilmaydi; `assert` — dasturchi xatosi uchun, foydalanuvchi xatosi uchun emas.

### Ombor tayyor — endi nima?

12-bosqichda sizda ~400 qatorli, 5 faylli, sanitizer'dan o'tgan dastur bor. Keyingi qadamlar (hammasi imkoniyatga qarab):

1. Savol-javob uchun **o'z funksiyangizni** qo'shing: "eng ko'p zaxirali 3 mahsulot", "toifa bo'yicha filtr", "CSV'ga eksport".
2. 13-bobdan keyin shu dasturni **xavfsizlik** nuqtai nazaridan qayta tekshiramiz (sanitizer, chegaralar). 14–15-boblardan keyin esa **tizim dasturlash** loyihalariga (shell, parallel dastur) o'tamiz.

**O'zingiz qo'shing (yechimsiz):**

1. `ombor_saqla` ni "atomik" qiling: avval `ombor.txt.tmp` ga yozing, `fclose` dan keyin `rename("ombor.txt.tmp", "ombor.txt")` (27-bob). Nega bu xavfsizroq? (Disk to'lib qolsa eski fayl saqlanib qoladi.)
2. CSV eksport: `ombor_eksport_csv(o, fayl)` yozing (`nom,narx,soni`); narxni `pul_matn` bilan so'm.tiyin ko'rinishida chiqaring.
3. `solishtir_nom` ni katta-kichik harfni farqlamaydigan qiling (`strcasecmp`, `<strings.h>`). `qsort` ning ikkala taqqoslagichini bir funksiyaga birlashtirib bo'ladimi? (Maslahat: global o'zgaruvchi kerak bo'ladi — bu yaxshimi? `qsort_r` ni qidiring.)
<!-- katta:oxiri -->

<!-- kadrlar:boshi -->
## Katta loyiha: Kadrlar tizimi — 12-bosqich: fayllar, `qsort`, `errno`

**Oldingi bosqichdan:** xodimlar ma'lumoti **kod ichida** edi (`boshlangich[]` massivi). Yangi xodim qo'shish uchun dasturni **qayta kompilyatsiya** qilish kerak. Haqiqiy dastur ma'lumotni **fayldan** oladi — va fayl **noto'g'ri** bo'lishi mumkinligini bilib ishlaydi.

### Bu bosqichda nima qilamiz

| Yangi | Nima |
|---|---|
| `data/xodimlar.txt`, `data/davomat.txt` | ma'lumot **matn fayllarida** (ataylab **noto'g'ri qatorlar** bilan!) |
| `src/yukla.c` | fayldan o'qish: `fopen`, `fgets`, `strtoll`, `errno`; noto'g'ri qator — **sababi va qator raqami** bilan rad etiladi |
| `src/vaqt.c` | kirish/chiqish vaqti (`0930` ko'rinishida) → ish daqiqalari |
| `ombor_saralash` | `qsort` bilan saralash (ism yoki maosh bo'yicha) |
| `pul_matn` | `1 234 567.89` ko'rinishidagi pul (`snprintf` bilan) |
| `hisobot_saqla` | hisobotni **faylga** yozish va yozish xatolarini tekshirish |

**Ma'lumot formati** (har qator — bitta yozuv; `#` bilan boshlangan qatorlar — izoh):

```text
# id ism toifa(1..4) tarif(tiyin/soat)
1042 Aziza 2 2500050
2087 Bobur 1 3150000
3150 Dilnoza 3 1875050
9999 Sardor 4 1250000075
# quyidagilar ataylab noto'g'ri: dastur ularni sababi bilan rad etadi
12 Qisqa 1 1000
1042 Takror 1 1000
4000 Nom 7 1000
5000 Tarifsiz 2 0
6000 Kam
7000 Harf2 x 500
```

```text
# id kun kirish chiqish (HHMM)
1042 1 0900 1800
1042 2 0830 1900
1042 3 0900 1800
2087 1 0800 1700
2087 2 0800 1600
3150 1 0900 2000
3150 2 1000 1800
9999 1 0900 1800
9999 2 0900 2100
5555 1 0900 1800
1042 4 1800 0900
1042 5 0975 1800
```

`xodimlar.txt`: `id ism toifa(1..4) tarif(tiyin/soat)`. `davomat.txt`: `id kun kirish chiqish` (vaqtlar `HHMM`). Quyidagi qatorlar **ataylab buzilgan** — tizim ularni **to'xtamasdan** o'tkazib yuborib, **sababini** aytishi kerak.

### Yangi tushunchalar

**1) Fayl bilan ishlash (12-bob).** `fopen(yol, "r")` → `NULL` bo'lsa **xato** (sababi `errno` da, matni `strerror(errno)`) → `fgets` bilan qator-qator o'qish → `fclose`. **Har bosqichning natijasini tekshiring.**

**2) `strtoll` — `atoi` dan xavfsiz.** `atoi("12abc")` jimgina `12` beradi; `atoi("abc")` — `0`; katta sonda — noma'lum. `strtoll` esa **qayerda to'xtaganini** (`oxir`) va **toshganini** (`errno == ERANGE`) bildiradi:

```c
static int son_oqi(const char *s, long long *natija)
{
    char *oxir;
    errno = 0;
    long long v = strtoll(s, &oxir, 10);
    if (oxir == s || *oxir != '\0' || errno == ERANGE)
        return -1;
    *natija = v;
    return 0;
}
```

Shart: `oxir == s` (hech narsa o'qilmadi) yoki `*oxir != '\0'` (son **keyin** yana belgi bor: `12abc`) yoki `ERANGE` (toshdi) — **xato**. Faqat butun matn son bo'lsa — qabul.

**3) Noto'g'ri qator — dasturni to'xtatmaydi.** Haqiqiy ma'lumot **doim** kirlangan bo'ladi. Qoida: noto'g'ri qatorni **o'tkazib yubor**, **sababini** `fayl:qator: sabab` ko'rinishida `stderr` ga yoz, **hisobla** (`struct yuklash` — `qabul` va `rad`). `stderr` — xatolar uchun alohida oqim: `stdout` ni boshqa dasturga ulaganda xato xabarlari natijaga **aralashib ketmaydi**.

```c
int xodimlar_yukla(struct ombor *o, const char *fayl, struct yuklash *h)
{
    FILE *f = och(fayl);
    if (!f)
        return -1;
    char qator[QATOR_UZ];
    for (int raqam = 1; fgets(qator, sizeof(qator), f); raqam++) {
        if (qator[0] == '#' || qator[0] == '\n')
            continue;
        char *soz[8];
        if (bol(qator, soz, 8) != 4) {
            rad_et(fayl, raqam, "maydonlar soni 4 emas", h);
            continue;
        }
        long long id, toifa, tarif;
        if (son_oqi(soz[0], &id) || id < 1000 || id > 9999) {
            rad_et(fayl, raqam, "ID 1000..9999 oralig'ida son emas", h);
            continue;
        }
        if (strlen(soz[1]) >= ISM_UZ) {
            rad_et(fayl, raqam, "ism juda uzun", h);
            continue;
        }
        if (son_oqi(soz[2], &toifa) || toifa < 1 || toifa > T_SONI) {
            rad_et(fayl, raqam, "toifa 1..4 oralig'ida son emas", h);
            continue;
        }
        if (son_oqi(soz[3], &tarif) || tarif <= 0) {
            rad_et(fayl, raqam, "tarif musbat son emas", h);
            continue;
        }
        struct xodim x = { .id = (int)id, .toifa = (enum toifa)(toifa - 1), .tarif = tarif };
        snprintf(x.ism, sizeof(x.ism), "%s", soz[1]);
        int r = ombor_qosh(o, &x);
        if (r == -2) {
            rad_et(fayl, raqam, "bu ID allaqachon bor", h);
        } else if (r != 0) {
            rad_et(fayl, raqam, "xotira yetmadi", h);
        } else {
            h->qabul++;
        }
    }
    fclose(f);
    return 0;
}
```

**4) `qsort` — standart saralash.** Siz faqat **taqqoslagich** funksiyasini yozasiz: ikki elementga `void *` ko'rsatkichlar oladi va **manfiy / nol / musbat** qaytaradi. Maosh bo'yicha **kamayish** uchun:

```c
static int maosh_boyicha(const void *a, const void *b)
{
    int64_t p = sof_maosh(a), q = sof_maosh(b);
    return (p < q) - (p > q);                   /* kamayish tartibi; ayirish EMAS: int64_t ayirmasi int ga sig'masligi mumkin */
}
```

`(p < q) - (p > q)` — `-1`, `0` yoki `1`. **`p - q` yozmaymiz:** ikki `int64_t` ning ayirmasi `int` ga sig'masligi (va natija ishorasi buzilishi) mumkin — 2-bobdagi toshish tuzog'i.

**5) `snprintf` — xavfsiz formatlash.** `pul_matn` pulni `1 234 567.89` ga aylantiradi. `snprintf(bufer, hajm, ...)` bufer hajmidan **chiqmaydi** (`sprintf` chiqib ketishi mumkin) va **kerak bo'lgan uzunlikni** qaytaradi:

```c
int pul_matn(char *bufer, size_t hajm, int64_t tiyin)
{
    char raqam[32];
    int64_t som = tiyin / 100;
    int n = snprintf(raqam, sizeof(raqam), "%lld", (long long)som);        /* "1234567" */
    char chiq[48];
    int k = 0;
    for (int i = 0; i < n; i++) {
        if (i > 0 && (n - i) % 3 == 0)
            chiq[k++] = ' ';                    /* har uch raqamdan oldin bo'sh joy (oxiridan sanaganda) */
        chiq[k++] = raqam[i];
    }
    chiq[k] = '\0';
    return snprintf(bufer, hajm, "%s.%02lld", chiq, (long long)(tiyin % 100));
}
```

**6) `assert` — ichki qoida.** `ombor_qosh` da `assert(o->soni < o->sigim)`: "bu yerda joy **doim** bor" degan **dastur xatosiga qarshi** tekshiruv (foydalanuvchi xatosi emas!). `-DNDEBUG` bilan o'chiriladi.

**7) Faylga yozishni to'g'ri tekshirish.** Yozish xatosi (disk to'lgan) ba'zan **`fclose` da** bilinadi, chunki ma'lumot avval **buferda** turadi:

```c
int hisobot_saqla(struct ombor *o, const char *fayl)
{
    FILE *f = fopen(fayl, "w");
    if (!f) {
        fprintf(stderr, "%s: yozish uchun ochilmadi: %s\n", fayl, strerror(errno));
        return -1;
    }
    hisobot_royxat(o, TARTIB_MAOSH, f);
    hisobot_jami(o, f);
    if (ferror(f) || fclose(f) != 0) {          /* disk to'lgan bo'lsa, xato aynan shu yerda bilinadi */
        fprintf(stderr, "%s: yozishda xato: %s\n", fayl, strerror(errno));
        return -1;
    }
    return 0;
}
```

### Ishga tushirish

Avval yig'amiz va **maosh bo'yicha** tartiblangan jadvalni olamiz (`stderr` ham ko'rinsin: `2>&1`):

```console
$ cd katta_loyiha/kadrlar/12_stdlib
$ make -s
$ ./bin/kadrlar royxat maosh 2>&1
data/xodimlar.txt:7: ID 1000..9999 oralig'ida son emas
data/xodimlar.txt:8: bu ID allaqachon bor
data/xodimlar.txt:9: toifa 1..4 oralig'ida son emas
data/xodimlar.txt:10: tarif musbat son emas
data/xodimlar.txt:11: maydonlar soni 4 emas
data/xodimlar.txt:12: toifa 1..4 oralig'ida son emas
xodimlar: 4 ta qabul, 6 ta rad
data/davomat.txt:11: bunday ID li xodim yo'q
data/davomat.txt:12: vaqt noto'g'ri (HHMM, chiqish kirishdan keyin bo'lishi kerak)
data/davomat.txt:13: vaqt noto'g'ri (HHMM, chiqish kirishdan keyin bo'lishi kerak)
davomat: 9 ta qabul, 3 ta rad
ID    Ism       Toifa               Brutto          Soliq               Qo'lga
9999  Sardor    Rahbar        296250017.77    58760003.55       234 527 514.04
1042  Aziza     Mutaxassis       686263.72       82351.65           597 049.43
2087  Bobur     Boshlovchi       472500.00       56700.00           411 075.00
3150  Dilnoza   Yetakchi         365634.75       43876.17           318 102.23
Jami (4 xodim): brutto 297774416.24, soliq 58942931.37, qo'lga tegadi 235 853 740.70 so'm
```

Ism bo'yicha tartib (xabarlarsiz), hisobotni faylga saqlash va xato holati:

```console
$ cd katta_loyiha/kadrlar/12_stdlib
$ ./bin/kadrlar royxat ism 2>/dev/null | head -3
ID    Ism       Toifa               Brutto          Soliq               Qo'lga
1042  Aziza     Mutaxassis       686263.72       82351.65           597 049.43
2087  Bobur     Boshlovchi       472500.00       56700.00           411 075.00
$ ./bin/kadrlar saqla hisobot.txt 2>/dev/null
hisobot hisobot.txt fayliga saqlandi
$ head -3 hisobot.txt
ID    Ism       Toifa               Brutto          Soliq               Qo'lga
9999  Sardor    Rahbar        296250017.77    58760003.55       234 527 514.04
1042  Aziza     Mutaxassis       686263.72       82351.65           597 049.43
$ ./bin/kadrlar saqla /yoq_papka/h.txt 2>&1 | tail -1
/yoq_papka/h.txt: yozish uchun ochilmadi: No such file or directory
$ ./bin/kadrlar saqla /yoq_papka/h.txt 2>/dev/null; echo "chiqish kodi: $?"
chiqish kodi: 1
$ KADRLAR_DATA=/yoq_papka ./bin/kadrlar royxat 2>&1; echo "chiqish kodi: $?"
/yoq_papka/xodimlar.txt: ochilmadi: No such file or directory
chiqish kodi: 2
```

**Nima ko'rdik:**

- `xodimlar.txt` da **10 ta ma'lumot qatori**: 4 tasi to'g'ri, **6 tasi rad etildi** — har biri **o'z sababi** bilan: `ID 1000..9999 oralig'ida son emas` (`12`), `bu ID allaqachon bor` (takror), `toifa ...` (`7` va `x`), `tarif musbat son emas` (`0`), `maydonlar soni 4 emas` (yarim qator). `davomat.txt` da 12 ta yozuvdan 9 tasi qabul, **3 tasi rad**: noma'lum ID, chiqish kirishdan oldin, vaqt `0975` (75 daqiqa bo'lmaydi).
- Jadval **maosh bo'yicha** tartiblandi (Sardor birinchi). `Qo'lga` ustuni **`234 527 514.04`** ko'rinishida — `pul_matn` ishladi.
- Ism bo'yicha tartib (`strcmp`): `Aziza`, `Bobur`, `Dilnoza`, `Sardor`.
- `hisobot.txt` da jadval **faylga yozilgan**. Mavjud bo'lmagan papkaga yozish urinishi: aniq xabar (`No such file or directory`) va **chiqish kodi 1** — skriptlar shu kod orqali xatoni bilib oladi.
- Ma'lumot papkasi yo'q bo'lsa: dastur **qulamaydi**, sababini aytib `2` kodi bilan chiqadi.

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `bol(qator, soz, maks)` | `strtok_r` bilan qatorni so'zlarga bo'ladi (`_r` — qayta kirishga xavfsiz variant) |
| `rad_et(...)` | xabar chiqaradi va `h->rad` ni oshiradi — **bitta joyda** |
| `davomat_yukla` | har yozuv uchun `ish_daqiqalari`: kunlik norma (8 soat) gacha — **oddiy**, ortig'i — **qo'shimcha** |
| `hisobot_royxat(o, tartib, FILE *)` | chiqish — **`FILE *`**: `stdout` ham, fayl ham bo'lishi mumkin (shuning uchun `saqla` bir xil kodni ishlatadi) |
| `ombor_top_yoz` | `ombor_top` ning o'zgartirish mumkin varianti (davomat xodimning daqiqalarini oshiradi) |

> **Eslab qoling:** fayldan kelgan ma'lumotga **ishonmang**. `atoi` o'rniga `strtoll` + `errno`; noto'g'ri qatorni **sabab bilan** rad eting va **hisoblang**; xatolar `stderr` ga; `fopen`, `fclose` natijasini **tekshiring**. `qsort` taqqoslagichida **ayirma** emas, `(a > b) - (a < b)`. Chiqish `FILE *` bo'lsa, bitta funksiya ham ekranga, ham faylga yozadi.

**O'zingiz qo'shing (yechimsiz):**

1. `data/xodimlar.txt` ga **uzun ism** (30 harf) qo'shing: qaysi xabar chiqadi? Bu tekshiruv kodning **qayerida** (`strlen(soz[1]) >= ISM_UZ`)?
2. `royxat` ga **uchinchi tartib** qo'shing: **tarif** bo'yicha. Yangi taqqoslagich va `enum tartib` ga yangi qiymat — qaysi **uch joy**?
3. `data/davomat.txt` ga **bir xodim uchun ko'p kunlik** yozuvlar qo'shing, qo'shimcha daqiqalar qanday yig'ilishini `varaqa` bilan tekshiring. 31 kundan ortiq yozsangiz nima bo'ladi? (13-bosqichda javobi bor.)
<!-- kadrlar:oxiri -->

## Bob xulosasi (yodlash uchun)

1. libc — tilning qismi emas, **tayyor funksiyalar to'plami**; har biri uchun tegishli sarlavha (`<stdio.h>`...) qo'shiladi.
2. `printf` — qolip + argumentlar; qolipdagi `%tur` joylarni to'ldiradi; kenglik, aniqlik, bayroqlar jadvalni tekis qiladi; foydalanuvchi satrini **hech qachon qolip qilmang**.
3. `FILE *`: `fopen` (NULL ni tekshiring) → `fprintf`/`fgets` → `fclose`; chiqish **buferlanadi** (`fflush`, qulasa yo'qoladi).
4. `errno` — xato sababi; avval qaytish qiymatini tekshiring, keyin `errno`/`strerror`/`perror`.
5. `qsort(massiv, n, hajm, cmp)` — qoidani siz berasiz; `cmp` da `x - y` emas, `(x > y) - (x < y)`. `assert(shart)` — "bu hech qachon bo'lmasligi kerak".

## Savol-javob

**Nega Python'da `import` kerak, C'da `#include`?**
Ikkalasi ham tayyor funksiyalarni "ulash" uchun. C'da `#include` e'lonlarni ko'chiradi, ta'riflarni esa linker kutubxonadan oladi (1-bob).

**`puts` bilan `printf` farqi?**
`puts("matn")` — oddiy matnni va oxiriga `\n` ni chiqaradi, qolipsiz (tezroq). Kompilyator `printf("matn\n")` ni ko'pincha `puts` ga o'zi almashtiradi (1-bob).

**Nega `fgets`, `gets` emas?**
`gets` bufer hajmini bilmaydi (to'lishi mumkin), shuning uchun standartdan olib tashlangan. `fgets` hajmni qabul qiladi (6.9).

## O'zingizni tekshiring

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

## Mashq

### Isitish: qsort, strtol, errno ★☆☆ — eng osoni, avval shuni qiling

Faqat 0–12-boblar kerak (`qsort`, `strtol`, `errno`, `printf` formatlari).
Skeletni `isitish.c` ga **qo'lda** yozing (ko'chirmang), izohlarni o'qing va `TODO` joylarini to'ldiring.
"Namuna" qismlar tayyor — qolganini qanday yozishni ko'rsatadi. Skelet hozir ham ogohlantirishsiz yig'iladi:
har `TODO` dan keyin yig'ib, ishga tushirib boring.

```c
/* isitish.c - 12-bob, isitish: standart kutubxona - qsort, strtol, errno, printf formatlari. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* qsort taqqoslash funksiyasi: manfiy - a oldin, 0 - teng, musbat - b oldin (12.5).
 * TODO: x va y ni olib, (x > y) - (x < y) ni qaytaring.
 *       `return x - y;` DEMANG: katta sonlarda toshadi (masalan INT_MIN - 1, 13-bob). */
static int taqqosla(const void *a, const void *b)
{
    (void)a;
    (void)b;
    return 0;
}

int main(void)
{
    int sonlar[] = {13, 2, 21, 8, 5};
    size_t n = sizeof(sonlar) / sizeof(sonlar[0]);

    /* 1) qsort - har qanday turni saralaydi: massiv, nechta, bitta elementning hajmi, taqqoslash. (Namuna - tayyor.) */
    qsort(sonlar, n, sizeof(sonlar[0]), taqqosla);
    printf("saralangan:");
    for (size_t i = 0; i < n; i++)
        printf(" %d", sonlar[i]);
    printf("\n");

    /* 2) TODO: long v = strtol("42abc", &oxiri, 10); - oxiri raqam BO'LMAGAN birinchi belgini ko'rsatadi.
     *    atoi dan farqi: qayerda to'xtaganini bilasiz, shuning uchun xatoni tekshira olasiz (12.5).
     *    Natija: strtol("42abc"): 42, qolgan: "abc" */
    char *oxiri = NULL;
    (void)oxiri;                        /* 2-qadamni yozgach, bu qatorni o'chiring */

    /* 3) TODO: FILE *f = fopen("bunday_fayl_yoq.txt", "r"); NULL bo'lsa - errno da sabab raqami,
     *    strerror(errno) - uning matni (12.4). Aks holda fclose(f).
     *    Natija: fopen: xato 2 (No such file or directory) */

    /* 4) printf: kenglik 6, 2 xona; '-' - chapga tekislash; '0' - oldiga nol (12.2). (Namuna - tayyor.) */
    printf("[%6.2f] [%-6.2f] [%03d]\n", 3.14159, 3.14159, 7);
    return 0;
}
```

**Kutilgan natija** (`darslik/loyihalar/12_csv_hisobot/isitish.txt`):

```text
saralangan: 2 5 8 13 21
strtol("42abc"): 42, qolgan: "abc"
fopen: xato 2 (No such file or directory)
[  3.14] [3.14  ] [007]
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined isitish.c -o isitish
$ ./isitish | diff - ~/C_loyha/darslik/loyihalar/12_csv_hisobot/isitish.txt && echo "TO'G'RI"
TO'G'RI
```

### Keyingi mashqlar

- **11** (son → satr — `printf` yuragi), **12** (`strtol` o'xshashi), **19** (`qsort`), **24** (parsing).
- Qo'shimcha: `va_list` bilan o'z mini-`printf`ingizni yozing: `%d`, `%s`, `%x`, `%c`, `%%` — chiqarish uchun faqat `putchar` ishlating. Keyin MyOS `kernel/lib/kprintf.c` bilan solishtiring.

<!-- loyiha:boshi -->
## Loyiha: server log tahlilchisi

**Maqsad:** matnli faylni satrma-satr o'qib, undan ma'lumot ajratish va hisobot chiqarish. Real hayotda log tahlili —
har kuni bajariladigan ish.
**Bobdan ishlatiladi:** `fopen`/`fgets`/`fprintf`/`fclose`, `sscanf` bilan bo'lish, `strcmp`, `errno`/`strerror`, `remove`.

**Talab:** `server.log` faylida har qator: `SANA VAQT DARAJA METOD YO'L NNms`. Dastur:
1. namunaviy logni **o'zi yaratadi** (dastur mustaqil bo'lsin),
2. o'qib, INFO/WARN/ERROR sonini, o'rtacha va eng sekin javob vaqtini topadi,
3. noto'g'ri formatdagi qatorni topib xabar beradi,
4. faylni o'chiradi.

**Asosiy usul:** `sscanf` qiymatlar sonini qaytaradi — 6 ta kutilsa, `!= 6` bo'lsa qator buzuq.

```c
/* logtahlil.c - server log tahlilchisi */
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *fayl = "server.log";
    FILE *f = fopen(fayl, "w");
    if (!f) {
        perror(fayl);
        return 1;
    }
    fprintf(f, "2026-09-01 10:00:01 INFO GET /index 12ms\n");
    fprintf(f, "2026-09-01 10:00:02 INFO GET /rasm.png 45ms\n");
    fprintf(f, "2026-09-01 10:00:03 WARN POST /kirish 230ms\n");
    fprintf(f, "buzuq qator\n");
    fprintf(f, "2026-09-01 10:00:05 ERROR GET /hisobot 1500ms\n");
    fprintf(f, "2026-09-01 10:00:06 INFO GET /index 9ms\n");
    fclose(f);                                  /* bufer diskka yoziladi */

    f = fopen(fayl, "r");
    if (!f) {
        perror(fayl);
        return 1;
    }
    char qator[128];
    int info = 0, ogoh = 0, xato = 0, tartib = 0, jami_ms = 0, n = 0;
    int eng_sekin = 0;
    char eng_sekin_yol[64] = "";
    while (fgets(qator, sizeof(qator), f)) {
        tartib++;
        char sana[16], vaqt[16], daraja[16], metod[8], yol[64];
        int ms;
        if (sscanf(qator, "%15s %15s %15s %7s %63s %dms", sana, vaqt, daraja, metod, yol, &ms) != 6) {
            qator[strcspn(qator, "\n")] = '\0';
            printf("  %d-qator buzuq: \"%s\"\n", tartib, qator);
            continue;
        }
        n++;
        jami_ms += ms;
        if (strcmp(daraja, "INFO") == 0)
            info++;
        else if (strcmp(daraja, "WARN") == 0)
            ogoh++;
        else if (strcmp(daraja, "ERROR") == 0)
            xato++;
        if (ms > eng_sekin) {
            eng_sekin = ms;
            snprintf(eng_sekin_yol, sizeof(eng_sekin_yol), "%s", yol);
        }
    }
    fclose(f);
    remove(fayl);

    printf("Tahlil qilingan qatorlar: %d\n", n);
    printf("INFO: %d, WARN: %d, ERROR: %d\n", info, ogoh, xato);
    printf("O'rtacha javob: %d ms\n", jami_ms / n);
    printf("Eng sekin: %s (%d ms)\n", eng_sekin_yol, eng_sekin);

    if (!fopen("yoq.log", "r"))
        printf("yoq.log: %s (errno = %d)\n", strerror(errno), errno);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g logtahlil.c -o logtahlil
$ ./logtahlil
  4-qator buzuq: "buzuq qator"
Tahlil qilingan qatorlar: 5
INFO: 3, WARN: 1, ERROR: 1
O'rtacha javob: 359 ms
Eng sekin: /hisobot (1500 ms)
yoq.log: No such file or directory (errno = 2)
```

**Kengaytiring:** `fgets` uzunroq qatorni ikkiga bo'lib beradi (bufer 128 bayt) — 6.9-bo'limdagi tuzoq. Fayl oxirida
`\n` bo'lmasa ham to'g'ri ishlashiga ishonch hosil qiling. `n == 0` bo'lsa `jami_ms / n` nima bo'ladi? Himoya yozing.

## Mustaqil loyiha: CSV baholar hisoboti ★★★

**Vazifa:** talabalar baholari fayli (`kirish.txt`) standart kirishdan (`stdin`) o'qiladi. Hisobot tuzing.
Fayl: `hisobot.c`. Har qator: `ism,ball1,ball2,ball3`.

**Kirish** (`darslik/loyihalar/12_csv_hisobot/kirish.txt`):

```text
# ism,ball1,ball2,ball3
Jasur,78,85,90
Madina,95,92,97
Salim,abc,80,90

Otabek,64,70,58
Zarina,95,92,97
Nodir,101,50,60
Kamola,88,91,79
Bobur,70,65,80
```

**Qoidalar:**
1. `#` bilan boshlangan va bo'sh qatorlar **jimgina** o'tkazib yuboriladi (lekin qator raqamiga kiradi).
2. Buzuq qator — 4 ta qiymatga bo'linmaydigan yoki ball son bo'lmagan: `<k>-qator: noto'g'ri format`.
3. Ball 0..100 oralig'ida bo'lmasa: `<k>-qator: ball oralig'ida emas`. Bunday qator hisobga kirmaydi.
4. To'g'ri qatorlarning o'rtachasi = `(ball1+ball2+ball3) / 3.0`.
5. Talabalar **o'rtachasi kamayish** tartibida chiqadi; teng bo'lsa — **ism alifbo tartibida** (`qsort` va `strcmp`).
6. Ism uzunligi 31 belgidan oshmaydi (bufer o'lchamiga ehtiyot bo'ling!).

**Chiqish tartibi:** avval jadval (`%d. %-10s %6.2f`), keyin umumiy qator, keyin xatolar ro'yxati.

**Kutilgan natija** (`./dastur < kirish.txt`) (`darslik/loyihalar/12_csv_hisobot/kutilgan.txt`):

```text
1. Madina      94.67
2. Zarina      94.67
3. Kamola      86.00
4. Jasur       84.33
5. Bobur       71.67
6. Otabek      64.00
Talabalar: 6, o'rtacha: 82.56, eng yaxshi: Madina, eng past: Otabek
Xato qatorlar: 2
  4-qator: noto'g'ri format
  8-qator: ball oralig'ida emas
```

**Maslahat** (yechim emas):
- `fgets(qator, sizeof qator, stdin)` — qator raqamini o'zingiz sanaysiz.
- `sscanf(qator, "%31[^,],%d,%d,%d", ism, &a, &b, &c) == 4` — `%[^,]` "vergulgacha hamma belgi" degani.
  Buzuq qator (`abc`) 2 ta qiymat o'qib to'xtaydi — natija 4 emas.
- Talaba tuzilmasi: `struct talaba { char ism[32]; double ortacha; };` va `taqqosla` funksiyasi (12.5).
- `double` larni `==` bilan solishtirmang (20-bob) — bu yerda ikkita 94.67 bir xil formuladan chiqqani uchun teng keladi, lekin `>` orqali tartiblash xavfsizroq.
- `qsort` ning solishtirish funksiyasi `const void *` oladi.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined hisobot.c -o dastur && ./dastur < ~/C_loyha/darslik/loyihalar/12_csv_hisobot/kirish.txt | diff - ~/C_loyha/darslik/loyihalar/12_csv_hisobot/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [13-bob. Aniqlanmagan xatti-harakat va xavfsizlik](13-ub-xavfsizlik.md)
