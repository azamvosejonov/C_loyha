# 4-bob. Boshqaruv oqimi: `if`, sikllar, `switch`, `goto`

> **Bu bobda nima o'rganasiz:** dasturni **qaror qabul qilishga** (`if`, `switch`) va **takrorlashga** (`while`, `for`)
> o'rgatishni. `{ }` qo'yilmasa nima bo'lishini, `switch` dagi "tushib ketish"ni va yadroda nega `goto` ishlatilishini bilib olasiz.
> **Oldindan nima kerak:** 2–3-boblar (turlar, taqqoslash).   **Vaqt:** 4–5 soat.
> Mashqlar: 01, 02, 05.

> **To'liq ishlaydigan misol:** [misollar/04_boshqaruv.c](misollar/04_boshqaruv.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Hozirgacha dasturlarimiz **tepadan pastga** bitta yo'ldan bordi. Haqiqiy dasturlar esa **tanlaydi** ("pin to'g'rimi?") va **takrorlaydi**
("1000 ta faylni o'qi"). Buning uchun ikki guruh buyruq bor:

| Maqsad | Buyruqlar | Hayotdan misol |
|---|---|---|
| **Tanlash** | `if`, `else`, `switch` | yo'l chorrahasi: "chiptangiz bormi?" |
| **Takrorlash** | `while`, `do-while`, `for` | "suv qaynamaguncha kut" |
| **Yo'lni o'zgartirish** | `break`, `continue`, `goto` | "topdim — to'xta", "buni tashla — keyingisi" |

Python bilasiz, shuning uchun farqlarni boshidan ko'rib oling:

| Python | C |
|---|---|
| `if x > 0:` (qavs yo'q, `:` bor) | `if (x > 0)` (qavs **shart**, `:` yo'q) |
| blok — **chekinish** bilan | blok — **`{ }`** bilan |
| `elif` | `else if` |
| `for i in range(10):` | `for (int i = 0; i < 10; i++)` |
| `while True:` | `while (1)` yoki `for (;;)` |
| `match` / `if-elif` | `switch` (faqat butun sonlar) |

## 4.1. `if` / `else`

**Hayotdan misol: eshikdagi qo'riqchi.** "Chiptangiz bormi? Bo'lsa — kiring, bo'lmasa — kassaga boring." Bitta shart, ikki yo'l.
`else if` zanjiri — ko'p eshikli yo'lak: **birinchi mos kelgan** eshikdan kirasiz, qolganlariga qaramaysiz.

```c
/* baho.c - if / else if / else zanjiri */
#include <stdio.h>

int main(void)
{
    int ball = 78;

    if (ball >= 86) {
        printf("a'lo\n");
    } else if (ball >= 71) {
        printf("yaxshi\n");
    } else {
        printf("qoniqarli emas\n");
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra baho.c -o baho
$ ./baho
yaxshi
```

**Kodda nimalar bor:**

| Qator | Nima qiladi |
|---|---|
| `int ball = 78;` | `ball` — butun son qutisi (32 bit), boshlang'ich qiymati **78** |
| `if (ball >= 86)` | 78 >= 86? **Yolg'on (0)** → `{ }` ichini **o'tkazib yuboradi** |
| `else if (ball >= 71)` | 78 >= 71? **Rost (1)** → `"yaxshi"` chiqadi |
| `else { ... }` | **qolgan hamma holat** (hech biri rost bo'lmasa). Bu yerga **kelmadi** |

Tartib muhim: C shartlarni **tepadan pastga** tekshiradi va **birinchi rost** bo'lganda to'xtaydi (qolganlarini umuman ko'rmaydi).
`ball = 90` qilib ko'ring — `"a'lo"` chiqadi (`>= 71` tekshirilmaydi ham).

- Shart **qavs ichida** bo'lishi shart: `if (x > 0)`. Python'dagi `if x > 0:` C'da xato.
- Shart — istalgan son: 0 — yolg'on, qolgani — rost (3-bob).
- `elif` yo'q — `else if` yoziladi. (Aslida bu `else` + ichida yangi `if`: `else` dan keyin istalgan **bitta buyruq** kelishi mumkin, `if` ham buyruq.)

### `{ }` qo'yilmasa — eng xavfli tuzoq

`if` dan keyin `{ }` bo'lmasa, **faqat bitta keyingi buyruq** shartga tegishli:

**Bu dastur nima qiladi (umumiy):** `{ }` siz `if` faqat bitta buyruqqa tegishli ekanini va chekinish aldashini ko'rsatadi.

```c
/* skobka.c - { } siz if: faqat BITTA buyruq tegishli */
#include <stdio.h>

int main(void)
{
    int xato = 0;                       /* xato yo'q */

    if (xato)
        printf("xato topildi\n");
        printf("bu qator if ga TEGISHLI EMAS\n");   /* chekinish aldaydi! */
    return 0;
}
```

```console
$ gcc -Wall -Wextra skobka.c -o skobka # xato kutiladi
skobka.c: In function ‘main’:
skobka.c:8:5: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
    8 |     if (xato)
      |     ^~
skobka.c:10:9: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
   10 |         printf("bu qator if ga TEGISHLI EMAS\n");   /* chekinish aldaydi! */
      |         ^~~~~~
$ ./skobka
bu qator if ga TEGISHLI EMAS
```

`xato = 0` (yolg'on) bo'lsa ham, ikkinchi qator chiqdi! Chekinish (bo'sh joy) faqat odam uchun; kompilyator uchun u **yo'q**. U buni quyidagicha o'qidi:

```text
if (xato)
    printf("xato topildi\n");          <- faqat shu if ga tegishli
printf("bu qator ...\n");               <- bu alohida, doim bajariladi
```

2014-yilda Apple'ning SSL kodidagi mashhur "goto fail" xatosi aynan shunday bo'lgan: bitta ortiqcha qator butun sertifikat tekshiruvini
o'chirib qo'ygan. GCC `-Wall` (`-Wmisleading-indentation`) buni ogohlantiradi — yuqorida ko'rdingiz.

> **Eslab qoling:** ko'p qatorli blokda **doim `{ }`**. Linux uslubida bitta qatorli `if` uchun `{ }` qo'yilmaydi — lekin ikkinchi qator
> qo'shganda albatta qo'ying.

### `;` tuzog'i

**Bu dastur nima qiladi (umumiy):** `if (...)` dan keyin `;` qo'yilsa, shart bo'sh buyruqqa tegishli bo'lib qolishini ko'rsatadi.

```c
/* nuqtali_vergul.c - if dan keyin ; */
#include <stdio.h>

int main(void)
{
    int x = -5;
    if (x > 0);                         /* ; - BO'SH BUYRUQ. if shu bilan tugadi */
    {
        printf("musbat\n");             /* bu blok if ga tegishli emas */
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra nuqtali_vergul.c -o nuqtali_vergul # xato kutiladi
nuqtali_vergul.c: In function ‘main’:
nuqtali_vergul.c:7:15: warning: suggest braces around empty body in an ‘if’ statement [-Wempty-body]
    7 |     if (x > 0);                         /* ; - BO'SH BUYRUQ. if shu bilan tugadi */
      |               ^
nuqtali_vergul.c:7:5: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
    7 |     if (x > 0);                         /* ; - BO'SH BUYRUQ. if shu bilan tugadi */
      |     ^~
nuqtali_vergul.c:8:5: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
    8 |     {
      |     ^
$ ./nuqtali_vergul
musbat
```

`x` manfiy, lekin `"musbat"` chiqdi: `;` bo'sh buyruq bo'lib, `if` shu bilan tugagan; `{ }` esa alohida blok sifatida doim bajarildi.
Kompilyator `-Wextra` bilan ogohlantiradi (`suggest braces around empty body`).

## 4.2. `while`

**Hayotdan misol: choynak qaynashini kutish.** "Suv qaynamaguncha — kut." Avval **tekshirasiz**, keyin kutasiz. Suv allaqachon qaynagan bo'lsa,
umuman kutmaysiz — tana **bir marta ham bajarilmasligi mumkin**.

```c
/* while_misol.c - teskari sanash */
#include <stdio.h>

int main(void)
{
    int n = 3;
    while (n > 0) {
        printf("n = %d\n", n);
        n--;                            /* n = n - 1 */
    }
    printf("tugadi, n = %d\n", n);
    return 0;
}
```

```console
$ gcc -Wall -Wextra while_misol.c -o while_misol
$ ./while_misol
n = 3
n = 2
n = 1
tugadi, n = 0
```

**Qiymatlar qanday o'zgaradi:**

| Aylanish | Shart `n > 0` | Tana | `n` keyin |
|---|---|---|---|
| 1 | `3 > 0` rost | `n = 3` chiqdi; `n--` | 2 |
| 2 | `2 > 0` rost | `n = 2` chiqdi | 1 |
| 3 | `1 > 0` rost | `n = 1` chiqdi | 0 |
| — | `0 > 0` yolg'on | sikl tugadi | 0 |

Shart **avval** tekshiriladi. `n` boshidan 0 bo'lsa — tana bir marta ham bajarilmaydi.

**Tez-tez xato:** `n--` ni unutish — shart hech qachon yolg'on bo'lmaydi, **cheksiz sikl** (to'xtatish: `Ctrl+C`).

## 4.3. `do ... while`

**Hayotdan misol: ovqatni tatib ko'rish.** Tuz yetarlimi bilish uchun **avval tatib ko'rasiz**, keyin qaror qilasiz: "yetmasa — tuz qo'shib, yana tatib ko'r".
Tana **kamida bir marta** bajariladi. PIN kod so'rash ham shunday: kamida bir marta so'raladi.

```c
/* dowhile_misol.c - kamida bir marta bajariladi */
#include <stdio.h>

int main(void)
{
    int n = 0;                          /* shart boshidanoq yolg'on */

    while (n > 0)
        printf("while: bajarilmaydi\n");

    do {
        printf("do-while: bir marta bajarildi (n = %d)\n", n);
    } while (n > 0);                    /* ; SHART - do-while oxirida */
    return 0;
}
```

```console
$ gcc -Wall -Wextra dowhile_misol.c -o dowhile_misol
$ ./dowhile_misol
do-while: bir marta bajarildi (n = 0)
```

Farq: `while` — **avval tekshiradi**, keyin bajaradi; `do-while` — **avval bajaradi**, keyin tekshiradi. Oxiridagi `;` — ko'p unutiladi.

## 4.4. `for`

**Hayotdan misol: zinapoyadan chiqish.** "1-qavatdan boshla; 9-qavatgacha; har safar bitta yuqoriga." Qayerdan boshlash, qachon to'xtash va
qanday qadam — hammasi **bitta qatorda**.

```text
for ( BOSHLASH ; SHART ; QADAM )  TANA
      bir marta   har aylanishdan   har aylanishdan
                  OLDIN             KEYIN
```

```c
/* for_misol.c - for va unga teng while */
#include <stdio.h>

int main(void)
{
    for (int i = 0; i < 3; i++)
        printf("for: i = %d\n", i);

    {
        int i = 0;                      /* BOSHLASH */
        while (i < 3) {                 /* SHART */
            printf("while: i = %d\n", i);
            i++;                        /* QADAM */
        }
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra for_misol.c -o for_misol
$ ./for_misol
for: i = 0
for: i = 1
for: i = 2
while: i = 0
while: i = 1
while: i = 2
```

Ikkala sikl **bir xil** natija berdi: `for` — shunchaki `while` ning qisqa yozuvi. `for` tartibi:

1. `int i = 0` — **bir marta**, sikl boshida.
2. `i < 3` tekshiriladi. Rost bo'lsa — tana bajariladi; yolg'on bo'lsa — sikl tugaydi.
3. Tana tugagach — `i++` (qadam).
4. 2-qadamga qaytish.

**Python bilan taqqos:**

| Python | C |
|---|---|
| `for i in range(10):` | `for (int i = 0; i < 10; i++)` |
| `for i in range(a, b, qadam):` | `for (int i = a; i < b; i += qadam)` |
| `for i in range(n-1, -1, -1):` | `for (int i = n - 1; i >= 0; i--)` |

**Teskari sikl va `size_t` tuzog'i (2-bob).** `size_t` ishorasiz: `i >= 0` doim rost — **cheksiz sikl**!

**Bu dastur nima qiladi (umumiy):** ishorasiz son bilan teskari sikl: xato variant (`i >= 0` doim rost) va to'g'ri `i-- > 0` naqshi.

```c
/* teskari.c - ishorasiz teskari sikl */
#include <stdio.h>

int main(void)
{
    size_t n = 3;

    /* XATO versiya (ishga tushirmaymiz!): for (size_t i = n - 1; i >= 0; i--)
       i ishorasiz -> 0 dan keyin 18446744073709551615 ga aylanadi -> cheksiz */

    /* TO'G'RI versiya: shart tekshirilganda i kamayadi */
    for (size_t i = n; i-- > 0; )
        printf("i = %zu\n", i);
    return 0;
}
```

```console
$ gcc -Wall -Wextra teskari.c -o teskari
$ ./teskari
i = 2
i = 1
i = 0
```

**Nega `i-- > 0` ishlaydi?** Har tekshirishda **avval** `i > 0` solishtiriladi, **keyin** `i` kamayadi (post-decrement, 3.6). `i = 3`: `3 > 0` rost, `i` 2 bo'ldi, tana `i = 2` ni ko'radi;
… `i = 1`: rost, `i` 0 bo'ldi, tana `i = 0` ni ko'radi; `i = 0`: `0 > 0` yolg'on — tugadi. Tana `n-1 … 0` ni ko'radi.

Istalgan qismini tashlab ketish mumkin: `for (;;)` — **cheksiz sikl** (yadrodagi scheduler va `init` jarayoni shunday: `kernel/proc/process.c`
dagi `scheduler_loop`, `user/bin/init.c`). `while (1)` va `for (;;)` bir xil; Linux uslubi — `for (;;)`.

## 4.5. `break` va `continue`

**Hayotdan misol: kalitni topdingiz.** Kalitni cho'ntaklardan qidiryapsiz. Ikkinchi cho'ntakda topdingiz — qolganlarini tekshirmaysiz: **`break`**.
Savatdagi olmalarni saralayapsiz: chirigani chiqsa — uni tashlab, **keyingisiga** o'tasiz: **`continue`**.

```c
/* break_continue.c - qidirish va tashlab ketish */
#include <stdio.h>

int main(void)
{
    int a[] = { 4, -1, 7, 9, -3, 7 };
    int n = 6, x = 9;

    for (int i = 0; i < n; i++) {
        if (a[i] < 0)
            continue;                   /* manfiyni o'tkazib, keyingisiga */
        if (a[i] == x) {
            printf("%d topildi, indeks %d\n", x, i);
            break;                      /* sikldan butunlay chiqish */
        }
        printf("a[%d] = %d - bu emas\n", i, a[i]);
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra break_continue.c -o break_continue
$ ./break_continue
a[0] = 4 - bu emas
a[2] = 7 - bu emas
9 topildi, indeks 3
```

**Kodda nimalar bor:** `int a[] = {...}` — 6 ta `int` qutisi qatori (massivlar — 8-bob, hozircha: `a[0]`, `a[1]`, … — raqamlash 0 dan). `x` — izlanayotgan qiymat.

| `i` | `a[i]` | nima bo'ladi |
|---|---|---|
| 0 | 4 | manfiy emas, `x` emas → `"a[0] = 4 - bu emas"` |
| 1 | -1 | manfiy → **`continue`**: chiqarmasdan keyingi `i` ga |
| 2 | 7 | `"a[2] = 7 - bu emas"` |
| 3 | 9 | `== x` → topildi, **`break`**: sikl tugadi (`a[4]`, `a[5]` ko'rilmadi) |

`break` faqat **eng ichki** sikldan (yoki `switch` dan) chiqaradi. Ichma-ich sikllardan birdaniga chiqish uchun: flag o'zgaruvchi, funksiyaga ajratib `return`, yoki `goto`.

> **Eslab qoling:** `break` — **sikldan chiq**. `continue` — **shu aylanishni tashla, keyingisiga o't**.

## 4.6. `switch`

**Hayotdan misol: liftning tugmalari.** Qaysi tugma bosilsa, lift o'sha qavatga boradi. `case` — tugmalar, `default` — "bunday qavat yo'q".

```c
/* kalkulyator.c - switch bilan amal tanlash */
#include <stdio.h>

int main(void)
{
    int a = 12, b = 4;
    char amal = '/';
    int r = 0;

    switch (amal) {
    case '+':
        r = a + b;
        break;
    case '-':
        r = a - b;
        break;
    case '/':
        r = a / b;
        break;
    case 'q':
    case 'Q':                           /* ikki case bitta kodga - ataylab "tushib ketish" */
        printf("chiqish\n");
        return 0;
    default:                            /* hech biri mos kelmasa */
        printf("noma'lum amal: %c\n", amal);
        return 1;
    }
    printf("%d %c %d = %d\n", a, amal, b, r);
    return 0;
}
```

```console
$ gcc -Wall -Wextra kalkulyator.c -o kalkulyator
$ ./kalkulyator
12 / 4 = 3
```

**Qanday ishlaydi:** `switch (amal)` — `amal` qiymatini oladi (`'/'`) va mos `case` ga **sakraydi** (bu yerda `case '/':`), shu yerdan boshlab kodni bajaradi.
`break` — "`switch` dan chiq". `default` — hech biri mos kelmasa.

- Faqat **butun son** qiymatlar (`int`, `char`, `enum`) bo'yicha ishlaydi — satrlar bo'yicha emas.
- `case` qiymatlari — **o'zgarmas** (literal yoki `#define`/`enum`).
- Kompilyator ko'p `case` li `switch` ni **sakrash jadvaliga** aylantirishi mumkin — `if` zanjiridan tezroq.

### `break` ni unutish — "tushib ketish" (fallthrough)

**`break` bo'lmasa — keyingi `case` ga ham "tushib ketadi"!** Lift kerakli qavatda to'xtamay, keyingisiga ham chiqib ketgandek.

**Bu dastur nima qiladi (umumiy):** `switch` da `break` unutilsa keyingi `case` ham bajarilishini ("tushib ketish") ko'rsatadi.

```c
/* tushish.c - break unutildi */
#include <stdio.h>

int main(void)
{
    int tugma = 2;

    switch (tugma) {
    case 1:
        printf("1-qavat\n");
        break;
    case 2:
        printf("2-qavat\n");            /* break YO'Q! */
    case 3:
        printf("3-qavat\n");
        break;
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra tushish.c -o tushish # xato kutiladi
tushish.c: In function ‘main’:
tushish.c:13:9: warning: this statement may fall through [-Wimplicit-fallthrough=]
   13 |         printf("2-qavat\n");            /* break YO'Q! */
      |         ^~~~~~~~~~~~~~~~~~~
tushish.c:14:5: note: here
   14 |     case 3:
      |     ^~~~
$ ./tushish
2-qavat
3-qavat
```

`tugma = 2` edi, lekin `"3-qavat"` ham chiqdi. Ba'zan bu **ataylab** (yuqoridagi `'q'`/`'Q'`), ko'pincha — **xato**. Ataylab qilinganda izoh yozing:
`/* fallthrough */` (GCC `-Wimplicit-fallthrough` ogohlantirishini ham o'chiradi).

Yadroda qayerda: syscall raqami bo'yicha tarqatish (`kernel/sys/syscall.c`: `switch (nr)`), klaviatura skankodlari, terminal escape-ketma-ketliklari (`kernel/drivers/vt.c`).

## 4.7. `goto` — yadroda nega ishlatiladi

**Hayotdan misol: uydan chiqishdagi tartib.** Avval gazni o'chirasiz, keyin chiroqni, oxirida eshikni qulflaysiz. Gazni o'chirayotganda muammo chiqsa ham,
chiroq va eshikni baribir yopish kerak. `goto` aynan shu: qayerda xato bo'lmasin, olingan resurslar **teskari tartibda** qaytariladi.

"`goto` yomon" degan gapni eshitgan bo'lsangiz — umuman olganda to'g'ri: tartibsiz sakrashlar kodni tushunib bo'lmas qiladi. Lekin C'da bitta joyda
u eng toza yechim — **xatodan keyin tozalash**. Quyida resurslarni **o'ynab** ko'rsatamiz (haqiqiy fayl o'rniga matn chiqaramiz):

```c
/* goto_tozalash.c - xato bo'lsa, olinganlarni teskari tartibda qaytarish */
#include <stdio.h>

/* qadam: nechanchi bosqichda xato bo'ladi (0 = xato yo'q) */
static int ish(int qadam)
{
    int rc = -1;                        /* natija: -1 = xato, 0 = muvaffaqiyat */

    if (qadam == 1) {
        printf("1) birinchi resurs OLINMADI (xato)\n");
        goto chiq;
    }
    printf("1) birinchi resurs olindi\n");

    if (qadam == 2) {
        printf("2) ikkinchi resurs OLINMADI (xato)\n");
        goto yop_1;
    }
    printf("2) ikkinchi resurs olindi\n");

    if (qadam == 3) {
        printf("3) uchinchi resurs OLINMADI (xato)\n");
        goto yop_2;
    }
    printf("3) uchinchi resurs olindi\n");

    printf("   asosiy ish bajarildi\n");
    rc = 0;

    printf("   3-resurs qaytarildi\n");
yop_2:
    printf("   2-resurs qaytarildi\n");
yop_1:
    printf("   1-resurs qaytarildi\n");
chiq:
    return rc;
}

int main(void)
{
    printf("--- xato yo'q ---\n");
    printf("natija: %d\n\n", ish(0));
    printf("--- 3-resursda xato ---\n");
    printf("natija: %d\n", ish(3));
    return 0;
}
```

```console
$ gcc -Wall -Wextra goto_tozalash.c -o goto_tozalash
$ ./goto_tozalash
--- xato yo'q ---
1) birinchi resurs olindi
2) ikkinchi resurs olindi
3) uchinchi resurs olindi
   asosiy ish bajarildi
   3-resurs qaytarildi
   2-resurs qaytarildi
   1-resurs qaytarildi
natija: 0

--- 3-resursda xato ---
1) birinchi resurs olindi
2) ikkinchi resurs olindi
3) uchinchi resurs OLINMADI (xato)
   2-resurs qaytarildi
   1-resurs qaytarildi
natija: -1
```

**Nima ko'rdik:** xato yo'q bo'lsa — hamma resurs olinadi va hammasi **teskari tartibda** qaytariladi. 3-resursni olishda xato bo'lsa — faqat **olingan** (1 va 2) resurslar
qaytariladi; 3-resurs olinmagan edi, uni qaytarish kerak emas. Har bir xato o'z "yorlig'iga" sakraydi va o'shandan pastdagi hamma tozalash bajariladi.

Haqiqiy kodda shunday ko'rinadi (fragment — `open`, `malloc` 14- va 8-boblarda):

```text
int nusxala(const char *a, const char *b)
{
    int rc = -1;
    int in = open(a, O_RDONLY);
    if (in < 0)
        goto chiq;
    int out = open(b, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out < 0)
        goto yop_in;
    char *buf = malloc(4096);
    if (!buf)
        goto yop_out;

    /* ... asosiy ish ... */
    rc = 0;

    free(buf);
yop_out:
    close(out);
yop_in:
    close(in);
chiq:
    return rc;
}
```

`goto` siz buni qilish uchun ichma-ich `if` lar yoki takrorlangan `close` lar kerak bo'lardi — ular xatoga ko'proq joy beradi. Linux yadrosida bu uslub minglab joyda
uchraydi. (C++ va Rust'da buning o'rniga destruktorlar bor; C'da — `goto`.)

> **Eslab qoling:** `goto` faqat **pastga** va faqat **tozalash** uchun.

## 4.8. Bloklar va ko'rinish sohasi

```c
for (int i = 0; i < 3; i++) {
    int kvadrat = i * i;    /* har aylanishda yangi */
}
/* i va kvadrat bu yerda YO'Q */
```

`for (int i = ...)` dagi `i` faqat sikl ichida yashaydi (C99'dan). Bu yaxshi: sikldan keyin tasodifan eski `i` ni ishlatib qo'ymaysiz.

## Hayotdan misol va to'liq dastur

**Bankomat.** Foydalanuvchi o'rniga buyruqlar oldindan massivda yozilgan — dastur har safar bir xil ishlaydi. Bu dasturda **hamma** bob buyruqlari ishlatilgan.

```c
/* bankomat.c - if, while, do-while, for, switch, break, continue */
#include <stdio.h>

int main(void)
{
    const int togri_pin = 1234;
    int urinishlar[] = { 1111, 4321, 1234 };      /* foydalanuvchi kiritgan PIN'lar */
    int urinish = 0, pin;

    do {                                          /* kamida bir marta so'raladi */
        pin = urinishlar[urinish++];
        if (pin != togri_pin)
            printf("PIN %d noto'g'ri\n", pin);
    } while (pin != togri_pin && urinish < 3);

    if (pin != togri_pin) {
        printf("Karta bloklandi\n");
        return 1;
    }
    printf("PIN to'g'ri (%d-urinishda)\n\n", urinish);

    long balans = 1000000;
    char buyruqlar[] = { 'b', 'y', 'y', 'x', 'b', 'q' };   /* b-balans, y-yechish, x-noma'lum, q-chiqish */

    for (int i = 0; i < 6; i++) {
        char b = buyruqlar[i];
        if (b == 'x') {
            printf("'%c': noma'lum tugma, o'tkazib yuborildi\n", b);
            continue;                             /* keyingi buyruqqa */
        }
        switch (b) {
        case 'b':
            printf("Balans: %ld so'm\n", balans);
            break;
        case 'y':
            if (balans >= 700000) {
                balans -= 700000;
                printf("700 000 so'm berildi\n");
            } else {
                printf("Mablag' yetarli emas\n");
            }
            break;
        case 'q':
            printf("Kartangizni oling. Xayr!\n");
            break;
        }
        if (b == 'q')
            break;                                /* sikldan butunlay chiqish */
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra bankomat.c -o bankomat
$ ./bankomat
PIN 1111 noto'g'ri
PIN 4321 noto'g'ri
PIN to'g'ri (3-urinishda)

Balans: 1000000 so'm
700 000 so'm berildi
Mablag' yetarli emas
'x': noma'lum tugma, o'tkazib yuborildi
Balans: 300000 so'm
Kartangizni oling. Xayr!
```

**Kodda nimalar bor:**

| Nom | Tur | Boshlang'ich | Nima uchun |
|---|---|---|---|
| `togri_pin` | `const int` | 1234 | to'g'ri PIN (`const` — o'zgarmaydi) |
| `urinishlar[]` | 3 ta `int` | `{1111, 4321, 1234}` | foydalanuvchi "kiritgan" PIN'lar |
| `urinish` | `int` | 0 | nechanchi urinish (massiv indeksi ham) |
| `pin` | `int` | (qiymatsiz, `do` ichida to'ldiriladi) | hozirgi urinishdagi PIN |
| `balans` | `long` | 1 000 000 | hisobdagi pul |
| `buyruqlar[]` | 6 ta `char` | `b y y x b q` | tugmalar ketma-ketligi |

**PIN so'rash (do-while) qadamlari:**

| Aylanish | `pin = urinishlar[urinish++]` | `pin != togri_pin`? | `urinish` keyin | Davom? |
|---|---|---|---|---|
| 1 | 1111 (`urinish` 0 → 1) | ha → `"noto'g'ri"` | 1 | `pin != 1234 && 1 < 3` rost |
| 2 | 4321 (1 → 2) | ha → `"noto'g'ri"` | 2 | rost |
| 3 | 1234 (2 → 3) | yo'q | 3 | `pin != 1234` yolg'on → chiqish |

**Buyruqlar (for + switch):** `b` — balans; `y` — yechish (700 000 yetarli bo'lsa); ikkinchi `y` — endi 300 000 qoldi, **yetmaydi**; `x` — `continue` bilan o'tkazib yuboriladi;
`b` — yangi balans 300 000; `q` — xayr va `break` bilan sikldan chiqish.

**Sinab ko'ring:** `urinishlar` massivini `{ 1, 2, 3 }` qiling — karta bloklanadimi? `case 'b':` dagi `break;` ni o'chiring — balansni so'raganda nima bo'ladi va nega?

<!-- katta:boshi -->
## Katta loyiha: Ombor — 4-bosqich: menyu va tanlov (sikl, `switch`)

**Oldingi bosqichdan:** dastur bir marta ishlab, jadval chiqarib tugardi. Haqiqiy ombor esa **foydalanuvchi bilan gaplashishi** kerak: "ro'yxatni ko'rsat", "shuncha sot", "chiqish". Buning uchun dastur **takrorlanishi** (sikl) va foydalanuvchi tanloviga qarab **tarmoqlanishi** (`switch`) lozim.

### Bu bosqichda nima qilamiz

1. **Menyu sikli:** `while (1)` — foydalanuvchi `0` (chiqish) tanlamaguncha takrorlanadi.
2. **Tanlov:** `switch (tanlov)` — 1 → ro'yxat, 2 → sotish, 0 → chiqish, boshqasi → xato xabari.
3. **Sotish tekshiruvlari:** mahsulot raqami to'g'rimi? miqdor musbatmi? omborda yetarlimi? Har biri uchun alohida `if` va tushunarli xato xabari.
4. **Foydalanuvchi kiritishi:** `scanf` — klaviaturadan son o'qiydi. `&` belgisi (`&tanlov`) — "o'zgaruvchining **manzilini** ber, o'qilgan qiymatni o'sha yerga yoz". Manzil va ko'rsatkichlarni 7-bobda to'liq o'rganasiz; hozircha shunday yozilishini qabul qiling.

**Nega `scanf` ning natijasini tekshiramiz?** `scanf` nechta qiymatni muvaffaqiyatli o'qiganini qaytaradi. Fayl tugasa yoki harf kiritilsa — kutilgan sondan kam qaytadi. Tekshirmasak, dastur cheksiz siklga tushib qolishi mumkin.

**Muammo, ataylab qoldirilgan:** mahsulot raqami (1/2/3) bo'yicha zaxirani o'qish va kamaytirish uchun **ikki marta takrorlanadigan `switch`** yozishga to'g'ri keldi. Bu takror — ogohlik: 4-mahsulot qo'shsak yana hamma joyni tahrirlashimiz kerak. 5-bobda funksiyalar, 6-bobda massivlar buni hal qiladi.

```c
/* ombor.c - Ombor, 4-bosqich: menyu (sikl) va tanlov (switch), sotish tekshiruvlari bilan */
#include <stdint.h>
#include <stdio.h>

#include "ombor_chop.h"

int main(void)
{
    long non_narx = 400000, sut_narx = 1200000, guruch_narx = 1800000;
    uint16_t non_soni = 120, sut_soni = 45, guruch_soni = 8;

    int tanlov;
    while (1) {                                         /* menyu: 0 tanlanguncha takrorlanadi */
        printf("\n1) ro'yxat   2) sotish   0) chiqish\nTanlov:\n");
        if (scanf("%d", &tanlov) != 1)                  /* son o'qilmasa (fayl tugadi) - chiqamiz */
            break;

        if (tanlov == 0)
            break;

        switch (tanlov) {
        case 1:
            chop_sarlavha();
            chop_qator("Non", non_narx, non_soni);
            chop_qator("Sut", sut_narx, sut_soni);
            chop_qator("Guruch", guruch_narx, guruch_soni);
            chop_jami(non_narx * non_soni + sut_narx * sut_soni + guruch_narx * guruch_soni, 12);
            break;

        case 2: {
            int id, miqdor;
            printf("Qaysi mahsulot (1-non, 2-sut, 3-guruch) va necha dona?\n");
            if (scanf("%d %d", &id, &miqdor) != 2)
                return 1;

            if (id < 1 || id > 3) {
                printf("  XATO: bunday mahsulot yo'q\n");
                break;
            }
            if (miqdor <= 0) {
                printf("  XATO: miqdor musbat bo'lishi kerak\n");
                break;
            }

            uint16_t mavjud;                            /* tanlangan mahsulotning zaxirasi */
            switch (id) {
            case 1: mavjud = non_soni; break;
            case 2: mavjud = sut_soni; break;
            default: mavjud = guruch_soni; break;
            }
            if (miqdor > mavjud) {
                printf("  XATO: omborda faqat %u dona bor\n", mavjud);
                break;
            }
            switch (id) {                               /* zaxirani kamaytiramiz */
            case 1: non_soni -= miqdor; break;
            case 2: sut_soni -= miqdor; break;
            default: guruch_soni -= miqdor; break;
            }
            printf("  Sotildi: %d dona. Qoldi: %u dona\n", miqdor, mavjud - miqdor);
            break;
        }

        default:
            printf("  XATO: menyuda bunday band yo'q\n");
        }
    }
    printf("\nXayr!\n");
    return 0;
}
```

Dasturni sinash uchun javoblar faylini tayyorlaymiz (foydalanuvchi klaviaturada yozadigan narsalar o'rniga):

```console
$ cd katta_loyiha/ombor/04_boshqaruv
$ printf '1\n2\n1 20\n2\n3 100\n2\n9 1\n2\n2 -5\n7\n1\n0\n' > kirish.txt
$ gcc -Wall -Wextra ombor.c ombor_chop.c -o ombor
$ ./ombor < kirish.txt

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
================ OMBOR ================
Mahsulot         Narx   Soni          Summa
---------------------------------------
Non           4000.00    120      480000.00
Sut          12000.00     45      540000.00
Guruch       18000.00      8      144000.00
---------------------------------------
Jami qiymat:                1164000.00
QQS stavkasi:               12%
QQS summasi:                139680.00

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
Qaysi mahsulot (1-non, 2-sut, 3-guruch) va necha dona?
  Sotildi: 20 dona. Qoldi: 100 dona

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
Qaysi mahsulot (1-non, 2-sut, 3-guruch) va necha dona?
  XATO: omborda faqat 8 dona bor

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
Qaysi mahsulot (1-non, 2-sut, 3-guruch) va necha dona?
  XATO: bunday mahsulot yo'q

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
Qaysi mahsulot (1-non, 2-sut, 3-guruch) va necha dona?
  XATO: miqdor musbat bo'lishi kerak

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
  XATO: menyuda bunday band yo'q

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
================ OMBOR ================
Mahsulot         Narx   Soni          Summa
---------------------------------------
Non           4000.00    100      400000.00
Sut          12000.00     45      540000.00
Guruch       18000.00      8      144000.00
---------------------------------------
Jami qiymat:                1084000.00
QQS stavkasi:               12%
QQS summasi:                130080.00

1) ro'yxat   2) sotish   0) chiqish
Tanlov:

Xayr!
```

(`< kirish.txt` — klaviatura o'rniga fayldan o'qi. Siz `./ombor` ni o'zingiz ishga tushirib, klaviaturada yozsangiz ham bo'ladi. Fayl bo'lgani uchun javoblar ekranda ko'rinmaydi — shuning uchun natijani tushunish uchun kirish ketma-ketligini quyida tahlil qilamiz.)

**Kiritilgan ketma-ketlik va natija:**

| Kirish | Natijasi |
|---|---|
| `1` | ro'yxat chiqdi |
| `2`, `1 20` | 1-mahsulot (non) dan 20 dona sotildi → qoldi 100 |
| `2`, `3 100` | guruchda faqat 8 dona → **XATO: omborda faqat 8 dona bor** |
| `2`, `9 1` | 9-raqamli mahsulot yo'q → **XATO: bunday mahsulot yo'q** |
| `2`, `2 -5` | manfiy miqdor → **XATO: miqdor musbat bo'lishi kerak** |
| `7` | menyuda 7 yo'q → **XATO: menyuda bunday band yo'q** (`default`) |
| `1` | yangilangan ro'yxat: non 100 dona |
| `0` | chiqish |

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `while (1) { ... }` | cheksiz sikl; ichidan `break` bilan chiqamiz |
| `if (scanf("%d", &tanlov) != 1) break;` | son o'qilmasa (fayl tugadi / harf) — sikldan chiqish |
| `switch (tanlov) { case 1: ... break; ... default: ... }` | tanlovga qarab bir tarmoqni bajaradi; **`break` unutilsa keyingi `case` ham bajariladi** (4-bobdagi "tushib ketish") |
| `case 2: { ... }` | figurali qavs — `case` ichida **o'z o'zgaruvchilarini** (`id`, `miqdor`) e'lon qilish uchun |
| ichki `switch (id)` | id bo'yicha mahsulotni tanlash: 3 marta takrorlanadi — kelajakda yo'qotamiz |
| `miqdor > mavjud` | zaxiradan ko'p sotib bo'lmaydi |

> **Eslab qoling:** menyu = `while` + `switch`. `scanf` natijasini **doim** tekshiring. Har `case` oxirida `break`. Kiritishni **tekshirmasdan** ishlatmang — foydalanuvchi har narsa yozishi mumkin.

**O'zingiz qo'shing (yechimsiz):**

1. Menyuga `3) jami qiymat` bandini qo'shing (zaxira × narx yig'indisi).
2. `case 1:` oxiridagi `break` ni o'chirib sinab ko'ring: nima bo'ladi? Nega?
3. `scanf("%d", &tanlov)` ga "salom" kiriting (`printf 'salom\n' | ./ombor`): dastur nima qiladi? Nega cheksiz sikl bo'lmadi?
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. `if (shart)` — qavs **shart**; `elif` yo'q, `else if` bor; shartlar **tepadan pastga**, birinchi rost to'xtaydi.
2. Ko'p qatorli blokda doim **`{ }`**; `if (x);` — bo'sh buyruq tuzog'i.
3. `while` — avval tekshir; `do-while` — avval bajar (kamida 1 marta); `for (boshlash; shart; qadam)`.
4. `break` — sikl/`switch` dan chiq; `continue` — keyingi aylanishga. `switch` da `break` unutilsa — **tushib ketadi**.
5. `goto` — faqat pastga, faqat xato bo'lganda **teskari tozalash** uchun.

## Savol-javob

**`while (1)` va `for (;;)` farqi bormi?**
Yo'q, ikkalasi ham cheksiz sikl. Linux uslubi — `for (;;)`.

**Nega `else if` alohida kalit so'z emas?**
`else` dan keyin istalgan **bitta** buyruq kelishi mumkin — `if` ham buyruq. Shuning uchun `else if (...)` aslida `else { if (...) ... }`.

**Qaysi biri tezroq: `switch` yoki `if/else`?**
Ko'p `case` bo'lsa — odatda `switch` (sakrash jadvali). Lekin avval to'g'ri va tushunarli yozing; tezlikni o'lchab ko'rmasdan optimallashtirmang.

**Nega cheksiz sikl kerak?**
Yadro hech qachon "tugamaydi": u uzilishlarni kutadi va jarayonlarni almashtiradi. `for (;;)` — shunday ishning odatiy shakli.

## O'zingizni tekshiring

1. `for (int i = 0; i < 3; i++);` `printf("%d", i);` — nima bo'ladi?
2. `switch` da `break` unutilsa nima bo'ladi?
3. `do { } while (0)` necha marta bajariladi? (10-bobda bu nimaga kerakligini ko'rasiz.)
4. `size_t` bilan teskari sikl qanday yoziladi?
5. `goto` yadroda nima uchun ishlatiladi?

<details><summary>Javoblar</summary>

1. Kompilyatsiya xatosi: `;` sikl tanasini bo'sh qildi, `i` esa sikldan tashqarida mavjud emas.
2. Keyingi `case` ning kodi ham bajariladi (fallthrough).
3. Bir marta.
4. `for (size_t i = n; i-- > 0; )` — shart tekshirilganda `i` kamayadi, tana `n-1 .. 0` ni ko'radi.
5. Xatodan keyin resurslarni teskari tartibda tozalash uchun.
</details>

## Mashq

- **01**, **02** — sikllar.
- **05** (massivlar) — teskari sikl va `size_t` tuzog'i.
- Qo'shimcha: 1 dan 100 gacha FizzBuzz; ko'paytirish jadvali (ichma-ich `for`); kiritilgan sonning raqamlari yig'indisi (`while` + `% 10` va `/ 10`).

<!-- loyiha:boshi -->
## Loyiha: taxmin o'yini (ikkilik qidiruv)

**Maqsad:** `while`, `if/else if/else`, `break` va ichma-ich `for` bilan kichik "aql" yaratish: kompyuter
1 dan 100 gacha maxfiy sonni eng kam urinishda topadi.
**Bobdan ishlatiladi:** `while`, `for`, `if`, `else if`, `break`.

**Talab:** maxfiy son berilgan. Kompyuter har safar oraliqning o'rtasini aytadi. "Kattaroq" javobi
kelsa — pastki yarmini tashlaydi, "kichikroq" bo'lsa — yuqori yarmini. Har urinishda variantlar **ikki
barobar** kamayadi.
**Ma'lumotlar:** `maxfiy`, oraliq chegaralari `past`, `yuqori`, `qadam` hisoblagichi.
**Qadamlar:** o'rta = (past + yuqori) / 2 → solishtirish → chegarani siljitish → takrorlash.

```c
/* taxmin.c - ikkiga bo'lib qidirish */
#include <stdio.h>

int main(void)
{
    int maxfiy = 73;
    int past = 1, yuqori = 100, qadam = 0;

    printf("Maxfiy son 1..100 orasida. Kompyuter oraliqni ikkiga bo'lib qidiradi:\n");
    while (past <= yuqori) {
        int orta = (past + yuqori) / 2;
        qadam++;
        if (orta == maxfiy) {
            printf("  %d-qadam: %d -> TOPILDI!\n", qadam, orta);
            break;
        } else if (orta < maxfiy) {
            printf("  %d-qadam: %d -> maxfiy kattaroq\n", qadam, orta);
            past = orta + 1;
        } else {
            printf("  %d-qadam: %d -> maxfiy kichikroq\n", qadam, orta);
            yuqori = orta - 1;
        }
    }

    /* Hamma maxfiy sonlar uchun eng yomon holatni topamiz */
    int eng_kop = 0, eng_kop_son = 0;
    for (int s = 1; s <= 100; s++) {
        int p = 1, y = 100, q = 0;
        while (p <= y) {
            int o = (p + y) / 2;
            q++;
            if (o == s)
                break;
            if (o < s)
                p = o + 1;
            else
                y = o - 1;
        }
        if (q > eng_kop) {
            eng_kop = q;
            eng_kop_son = s;
        }
    }
    printf("1..100 ichida eng ko'p qadam: %d (birinchi marta maxfiy = %d da)\n", eng_kop, eng_kop_son);
    return 0;
}
```

```console
$ gcc -Wall -Wextra taxmin.c -o taxmin
$ ./taxmin
Maxfiy son 1..100 orasida. Kompyuter oraliqni ikkiga bo'lib qidiradi:
  1-qadam: 50 -> maxfiy kattaroq
  2-qadam: 75 -> maxfiy kichikroq
  3-qadam: 62 -> maxfiy kattaroq
  4-qadam: 68 -> maxfiy kattaroq
  5-qadam: 71 -> maxfiy kattaroq
  6-qadam: 73 -> TOPILDI!
1..100 ichida eng ko'p qadam: 7 (birinchi marta maxfiy = 2 da)
```

100 ta variantni 7 qadamda topdi: 2⁷ = 128 > 100. Oddiy ketma-ket sanash (1, 2, 3...) esa 100 gacha qadam talab qilardi.
Bir milliard variant bo'lsa ham 30 qadam yetadi — algoritm shu bilan kuchli (28-bob).

**Kengaytiring:** `maxfiy` ni 1, 50, 100 qilib ko'ring. Sikl sharti `past <= yuqori` ni `<` ga almashtirsangiz, qaysi
sonlarda dastur xato qiladi?

## Mustaqil loyiha: oy kalendari ★★☆

**Vazifa:** `cal` buyrug'i kabi oy kalendarini chiqaring. Massiv va funksiya **kerak emas** — faqat sikl,
`if` va `%`. Fayl: `kalendar.c`.

**Ma'lumotlar** (kod boshida):
- `birinchi` — oyning 1-kuni haftaning qaysi kuni (0 = Dushanba, 1 = Seshanba, ..., 6 = Yakshanba);
- `kunlar` — oyda nechta kun.

**Talab:**
- Birinchi qator: ` Du Se Ch Pa Ju Sh Ya` (har ustun 3 belgi kenglikda).
- Har kun `%3d` bilan chiqadi. 1-kundan oldingi bo'sh kataklar — 3 tadan probel.
- Har Yakshanbadan keyin yangi qator. Oxirgi qatordan keyin **bitta** yangi qator (bo'sh qator bo'lmasin;
  agar oy aynan Yakshanba bilan tugasa, ikkita yangi qator chiqmasin!).
- Qatorlar oxirida ortiqcha probel bo'lmasin.

**Kutilgan natija** — `birinchi = 2` (Chorshanba), `kunlar = 30` (`darslik/loyihalar/04_kalendar/kutilgan.txt`):

```text
 Du Se Ch Pa Ju Sh Ya
        1  2  3  4  5
  6  7  8  9 10 11 12
 13 14 15 16 17 18 19
 20 21 22 23 24 25 26
 27 28 29 30
```

**Qo'shimcha sinovlar.** `birinchi = 6`, `kunlar = 31` (Yakshanbadan boshlanadigan):

```text
 Du Se Ch Pa Ju Sh Ya
                    1
  2  3  4  5  6  7  8
  9 10 11 12 13 14 15
 16 17 18 19 20 21 22
 23 24 25 26 27 28 29
 30 31
```

`birinchi = 0`, `kunlar = 28` (mukammal to'rt hafta):

```text
 Du Se Ch Pa Ju Sh Ya
  1  2  3  4  5  6  7
  8  9 10 11 12 13 14
 15 16 17 18 19 20 21
 22 23 24 25 26 27 28
```

**Maslahat** (yechim emas):
- Avval qog'ozda 30 kunlik oyni chizing va "hafta kuni" hisoblagichini yuritish qoidasini toping:
  har kundan keyin u 1 ga oshadi, 7 ga yetsa 0 ga qaytadi (`%` yoki `if`).
- Birinchi qatordagi bo'sh kataklar soni = `birinchi`.
- Oxirgi qator to'liq bo'lmasa, uni yangi qator bilan yopish kerak. To'liq bo'lsa — allaqachon yopilgan.
  Bu ikki holatni bitta shart bilan ajrating.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g kalendar.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/04_kalendar/kutilgan.txt && echo "TO'G'RI"
```

Ikkinchi va uchinchi sinov uchun `birinchi`, `kunlar` ni o'zgartirib, `kutilgan_2.txt`, `kutilgan_3.txt` bilan solishtiring.
Uchala sinovdan o'tsangiz — chegaraviy holatlar (oy Yakshanba bilan tugash, hafta Dushanbadan boshlanish) to'g'ri.
<!-- loyiha:oxiri -->

Keyingi bob: [5-bob. Funksiyalar](05-funksiyalar.md)
