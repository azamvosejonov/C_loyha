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
