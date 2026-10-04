# 2-bob. O'zgaruvchilar va turlar

> **Bu bobda nima o'rganasiz:** o'zgaruvchi nima va u xotirada qanday turishini; nega har bir o'zgaruvchining **turi**
> bo'lishini; har bir tur necha bayt joy olishini va qanday sonlarni sig'dirishini; nega son **toshib ketishi**
> mumkinligini; yadroda nega `uint32_t` kabi turlar ishlatilishini.
> **Oldindan nima kerak:** 0- va 1-boblar.   **Vaqt:** 4–5 soat.
> Mashqlar: 01, 02, 03.

> **To'liq ishlaydigan misol:** [misollar/02_turlar.c](misollar/02_turlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Dastur ma'lumot bilan ishlaydi: yosh, narx, harorat, ism. Bu ma'lumotni **qayerdadir saqlash** kerak. Saqlash joyi —
**o'zgaruvchi**. C'da har bir o'zgaruvchi uchun ikki narsani **oldindan** aytishingiz shart: **nomi** va **turi**
(qancha joy olishi va ichida nima turishi).

**Hayotdan misol: oshxonadagi bankalar.** Har bir bankada **yorliq** ("shakar", "tuz") va ichida nimadir bor. Bankaning
**o'lchami** oldindan aniq: choy qoshig'iga (`char`, 1 bayt) chelak suv sig'maydi. Python'da banka o'zi kattalashadi;
C'da — yo'q, o'lchamni siz tanlaysiz va u o'zgarmaydi.

| Oshxonada | C'da |
|---|---|
| banka | o'zgaruvchi (xotiradagi quti) |
| yorliq | o'zgaruvchining nomi |
| banka hajmi | tur (`char`, `int`, `long` ...) |
| ichidagi narsa | qiymat |

## 2.1. O'zgaruvchi nima

Python'da o'zgaruvchi — obyektga yopishtirilgan **nom** (yorliq). C'da o'zgaruvchi — xotiradagi **aniq o'lchamli
quti**, o'z nomi bilan. Bitta qator yozamiz:

```c
int yosh = 20;
```

Bu qator kompilyatorga **uchta** narsani aytadi:

1. **Tur** — `int`: "quti **4 bayt** (32 bit), ichida **ishorali butun son**" (ishora — plyus/minus belgisi).
2. **Nom** — `yosh`: kodda shu nom bilan murojaat qilamiz. Dastur kompilyatsiyadan keyin nom yo'qoladi — faqat manzil qoladi.
3. **Boshlang'ich qiymat** — `= 20`. Ixtiyoriy, lekin **doim yozing** (2.2 ga qarang).

Xotirada bu shunday ko'rinadi (4 ta bayt, har birining manzili bor):

```text
manzil:   1000        1001        1002        1003
yosh:     [   20    ] [    0    ] [    0    ] [    0    ]      <- 20 = 0x00000014, 4 baytga yoyilgan
```

(Nega `20` birinchi baytda, qolganlari `0`? x86 protsessori kichik baytni **oldin** yozadi — 16-bobda.
Hozircha: "`yosh` 4 ta qo'shni bayt joy egallaydi".)

**Nega turni yozish kerak? Python'da kerak emas-ku.** Kompilyator uch narsani bilishi kerak:

1. **Qancha joy** ajratish (1, 2, 4 yoki 8 bayt).
2. **Qaysi CPU buyrug'ini** ishlatish (butun sonlarni qo'shish va kasr sonlarni qo'shish — **turli** buyruqlar).
3. **Xatolarni ushlash** ("matnni songa qo'shdingiz!").

Python bu ma'lumotni **har bir obyektning ichida** saqlaydi va har amalda tekshiradi — shuning uchun sekin. C'da tur
**kompilyatsiya paytida** hal bo'ladi, ish vaqtida **hech qanday tekshiruv yo'q**: tez, lekin javobgarlik sizda.

**Tur o'zgarishi mumkinmi?** Yo'q. `int x` butun umr `int`. Python'dagi `x = 5; x = "salom"` C'da mumkin emas.

### Birinchi tajriba: turlarning hajmi

```c
/* hajm.c - har bir turning xotiradagi o'lchami */
#include <stdio.h>

int main(void)
{
    int yosh = 20;

    printf("char  = %zu bayt\n", sizeof(char));
    printf("short = %zu bayt\n", sizeof(short));
    printf("int   = %zu bayt\n", sizeof(int));
    printf("long  = %zu bayt\n", sizeof(long));
    printf("double= %zu bayt\n", sizeof(double));
    printf("yosh  = %zu bayt\n", sizeof(yosh));
    return 0;
}
```

```console
$ gcc -Wall -Wextra hajm.c -o hajm
$ ./hajm
char  = 1 bayt
short = 2 bayt
int   = 4 bayt
long  = 8 bayt
double= 8 bayt
yosh  = 4 bayt
```

**Kodda nimalar bor:**

- `sizeof(char)` — "`char` turi **necha bayt** joy oladi?" `sizeof` — **operator** (funksiya emas): javob **kompilyatsiya paytida**
  hisoblanadi, ish vaqtida hech narsa bajarilmaydi. Turni ham (`sizeof(int)`), o'zgaruvchini ham (`sizeof(yosh)`) berish mumkin.
- `%zu` — `sizeof` natijasi uchun format. Natija turi — `size_t` ("hajm uchun ishorasiz son", pastda), uning formati `%zu`.
- `int yosh = 20;` — bu yerda faqat `sizeof(yosh)` ni ko'rsatish uchun yaratdik: `int` bilan bir xil (4).

> **Eslab qoling:** `char`=1, `short`=2, `int`=4, `long`=8 (Linux'da), `double`=8 bayt. Natija **sizning kompyuteringizda**
> shunday chiqdi; boshqa tizimda (masalan Windows'da `long` = 4) farq qilishi mumkin — shu uchun yadroda `<stdint.h>` ishlatiladi (2.3).

## 2.2. Boshlang'ich qiymatsiz o'zgaruvchi — xavfli

**Hayotdan misol: ijaraga olingan kvartira.** Yangi kvartirangizning tortmalarida oldingi egasidan qolgan narsalar
bo'lishi mumkin. `int x;` ham shunday: xotirada shu joyda **oldin nima turgan bo'lsa**, o'sha qolgan.

```c
/* axlat.c - boshlang'ich qiymatsiz o'zgaruvchi */
#include <stdio.h>

int main(void)
{
    int s;                  /* qiymat BERILMAGAN: ichida tasodifiy "axlat" */
    printf("%d\n", s + 1);  /* nima chiqishi noma'lum! */
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O1 axlat.c -o axlat # xato kutiladi
axlat.c: In function ‘main’:
axlat.c:7:5: warning: ‘s’ is used uninitialized [-Wuninitialized]
    7 |     printf("%d\n", s + 1);  /* nima chiqishi noma'lum! */
      |     ^~~~~~~~~~~~~~~~~~~~~
axlat.c:6:9: note: ‘s’ was declared here
    6 |     int s;                  /* qiymat BERILMAGAN: ichida tasodifiy "axlat" */
      |         ^
```

Kompilyator ogohlantirdi (`'s' is used uninitialized`). Biz buni hozir **ishga tushirmaymiz**: natija oldindan
ma'lum emas — bir kompyuterda `1`, boshqasida `32768`, uchinchisida dastur qulaydi.

**Nega shunday?** Kompilyator `int s;` uchun xotiradan joy ajratadi, lekin uni **tozalamaydi**: tozalash vaqt oladi,
C esa tezlikka qurilgan. Qoida:

> **Eslab qoling:** **har bir o'zgaruvchiga e'lon qilgan joyda qiymat bering:** `int s = 0;`.

(Global va `static` o'zgaruvchilar esa avtomatik 0 bo'ladi — 8-bob.)

## 2.3. Butun son turlari

**Hayotdan misol: har xil o'lchamdagi idishlar.** `char` — stakan, `short` — chelak, `int` — bochka, `long` — sisterna.
Idishni ichiga qo'yadigan narsangizning **eng katta** bo'lishi mumkin bo'lgan hajmiga qarab tanlaysiz: kishining
yoshi uchun stakan ham yetadi; dunyo aholisi (8 mlrd) uchun `int` bochkasi (~2 mlrd) yetmaydi.

Idish nima uchun cheklangan? Chunki quti **bitlardan** iborat. Bit — kalit (0/1). `n` ta bit `2ⁿ` xil holatni ifodalaydi:

| Bitlar | Holatlar soni | Ishorasiz oraliq |
|---|---|---|
| 8 bit (1 bayt) | 2⁸ = 256 | 0 … 255 |
| 16 bit (2 bayt) | 2¹⁶ = 65 536 | 0 … 65 535 |
| 32 bit (4 bayt) | 2³² = 4 294 967 296 | 0 … 4 294 967 295 |
| 64 bit (8 bayt) | 2⁶⁴ ≈ 1.8·10¹⁹ | 0 … 18 446 744 073 709 551 615 |

Ishorali turlarda holatlarning yarmi manfiy sonlarga ketadi (2.4 da ko'rasiz).

| Tur | Hajm (Linux, 64 bit) | Oraliq | Qachon ishlatiladi |
|---|---|---|---|
| `char` | 1 bayt | −128 … 127 (x86'da) | Belgi, bayt |
| `short` | 2 | −32 768 … 32 767 | Kam ishlatiladi |
| `int` | 4 | −2 147 483 648 … 2 147 483 647 | Oddiy hisoblagichlar |
| `long` | 8 | ±9.2·10¹⁸ | Katta sonlar (Windows'da esa **4** bayt!) |
| `long long` | 8 | ±9.2·10¹⁸ | Har doim kamida 64 bit |
| `unsigned int` | 4 | 0 … 4 294 967 295 | Manfiy bo'lmaydigan qiymatlar |
| `size_t` | 8 | 0 … 1.8·10¹⁹ | **Hajm va indekslar** (`sizeof`, `strlen`, `malloc` shuni ishlatadi) |

**Nega `int` ning hajmi "taxminan"?** C standarti faqat **minimal** hajmni kafolatlaydi (`int` ≥ 16 bit, `long` ≥ 32 bit).
Haqiqiy hajm platformaga bog'liq. Yadroda bu qabul qilib bo'lmaydi: apparat registri **aniq** 32 bit bo'lsa, tur ham aniq 32 bit
bo'lishi kerak. Yechim — `<stdint.h>`.

### `<stdint.h>` — aniq o'lchamli turlar

**Oddiy qilib aytganda:** `<stdint.h>` — "**bu tur aniq shuncha bit**" deb va'da beradigan turlar to'plami. `int` — "taxminan 32 bit",
`int32_t` — "**aniq** 32 bit, ishorali". Nomi tuzilishi: `u` — unsigned (ishorasiz), raqam — bitlar soni, `_t` — "type".

| Nom | Ma'nosi |
|---|---|
| `uint8_t` | ishorasiz, aniq 8 bit (0…255) — "bitta bayt" |
| `uint16_t` | ishorasiz, 16 bit (0…65 535) — masalan port raqami |
| `uint32_t` | ishorasiz, 32 bit — apparat **registri** |
| `uint64_t` | ishorasiz, 64 bit — xotira **manzili** |
| `int32_t` | **ishorali**, aniq 32 bit |
| `uintptr_t` | ko'rsatkich (manzil) sig'adigan ishorasiz butun |

```c
/* stdint_misol.c - aniq o'lchamli turlar */
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    uint8_t  bayt    = 0xFF;                /* aniq 8 bit, hammasi 1 -> 255 */
    uint16_t port    = 0x3F8;               /* tarmoq / seriya port raqami */
    uint32_t registr = 0;                   /* 32 ta kalitli panel, hammasi o'chiq */
    uint64_t manzil  = 0xFFFFFFFF80000000;  /* yadro manzili, 64 bit */
    int32_t  farq    = -5;                  /* aniq 32 bit, manfiy bo'lishi mumkin */

    printf("bayt=%u port=%u registr=%u\n", bayt, port, registr);
    printf("manzil=%lu farq=%d\n", (unsigned long)manzil, farq);
    printf("hajmlar: %zu %zu %zu %zu %zu\n", sizeof(bayt), sizeof(port), sizeof(registr),
           sizeof(manzil), sizeof(farq));
    return 0;
}
```

```console
$ gcc -Wall -Wextra stdint_misol.c -o stdint_misol
$ ./stdint_misol
bayt=255 port=1016 registr=0
manzil=18446744071562067968 farq=-5
hajmlar: 1 2 4 8 4
```

**Kodda nimalar bor:**

| Qator | Nima qiladi | Nega shu tur |
|---|---|---|
| `#include <stdint.h>` | `uint8_t` va boshqalarni "tanitadi" | Bularni `stdint.h` e'lon qiladi (typedef — 9-bob). Qo'shmasangiz kompilyator bu nomni bilmaydi |
| `uint8_t bayt = 0xFF;` | 8 kalit, hammasi yoqilgan → 255 | `0xFF` — o'n oltilik yozuv: `F` = `1111` (4 bit), ikkita `F` = 8 bit hammasi 1 |
| `uint16_t port = 0x3F8;` | 16 bitli quti | Portlar 0…65535 oralig'ida — aynan 16 bit |
| `uint32_t registr = 0;` | **32 ta kalitli panel**, boshida **hammasi 0** (o'chiq) | Apparat registrlari 32 bit; 3-bobda shu panelni kalitlar bilan boshqaramiz |
| `uint64_t manzil = 0xFFFF...;` | 64 bitli manzil | 64 bitli CPU'da manzil 64 bit |
| `int32_t farq = -5;` | ishorali, 32 bit | Manfiy son kerak bo'lgani uchun `u` yo'q |

**Nega yadroda deyarli faqat shu turlar?** Apparat tuzilmalari — sahifa jadvali yozuvi (aniq 64 bit), ELF sarlavhasi,
ext2 superbloki — **bayt-baybayt aniq** bo'lishi kerak. `int` bir kompyuterda 16 bit, boshqasida 32 bit bo'lsa, tuzilma
buziladi. MyOS'dagi har qanday `struct` ni oching — `uint32_t`, `uint64_t` ni ko'rasiz.

> **Eslab qoling:** apparatga yoki fayl formatiga yozayotgan bo'lsangiz — **`uintN_t`**. Oddiy hisoblagich — `int`.
> Hajm va indeks — `size_t`. Hamma `#include` larning ma'nosi — [sarlavhalar.md](sarlavhalar.md).

### `sizeof` va `size_t`

`sizeof(a)` massiv uchun **butun massiv** hajmini beradi (`int a[10]` → 40). Natijasining turi — `size_t`: hajm va
indekslar uchun maxsus **ishorasiz** tur (hajm manfiy bo'lmaydi va ishorasiz tur ikki baravar katta qiymatni sig'diradi).
Chop etish formati — `%zu`.

## 2.4. Signed va unsigned — ishora biti

**Hayotdan misol: termometr va spidometr.** Termometrda manfiy son bor (−15 °C) — bu **signed** (ishorali). Spidometr yoki
yosh manfiy bo'lmaydi — bu **unsigned** (ishorasiz). Ishorasiz idishda manfiy uchun joy ajratilmagani sababli, musbat
tomonda **ikki barobar** ko'p joy bor.

### Avval: ikkilik sanoq (tez eslatma)

Bitlar o'ngdan chapga **og'irlik** oladi: 1, 2, 4, 8, 16, 32, 64, 128 (har biri oldingisidan ikki baravar). Son — **yoqilgan**
kalitlarning og'irliklari yig'indisi:

```text
og'irlik:   128   64   32   16    8    4    2    1
bit:          0    1    0    0    1    0    0    0      -> 64 + 8 = 72
bit:          1    1    1    1    1    1    1    1      -> hammasi yoqiq = 255
```

(Bu mavzu 3-bobda batafsil. Bu yerda faqat ishorali sonlarni tushunish uchun kerak.)

### Manfiy sonlar qanday saqlanadi: "ikkiga to'ldirish"

Kompyuterda "minus belgisi" yo'q — faqat bitlar. Manfiy sonlar **ikkiga to'ldirish** (two's complement) usulida saqlanadi.
**Qoida (manfiy son yasash):** 1) sonning bitlarini **teskari** qiling (0↔1), 2) natijaga **1 qo'shing**.

```text
-5 ni 8 bitda yasaymiz:
  5          =  0000 0101
  teskari    =  1111 1010
  + 1        =  1111 1011      <- bu "-5"

Tekshiramiz: 5 + (-5):
    0000 0101
  + 1111 1011
  -----------
  1 0000 0000      <- 9 ta bit chiqdi, eng chap "ortiqcha" bit tashlanadi (quti 8 bit!) -> 0000 0000 = 0  ✓
```

**Nega aynan shunday?** Chunki shunda **qo'shish** uchun alohida "manfiy son" sxemasi kerak bo'lmaydi: protsessor
oddiy ikkilik qo'shish bilan `5 + (−5) = 0` ni to'g'ri chiqaradi. Bu juda arzon va tez.

8 bitli ishorali sonlar jadvali:

```text
 0000 0000 =    0          1000 0000 = -128
 0000 0001 =    1          1111 1110 =   -2
 0111 1111 =  127          1111 1111 =   -1
```

Eng chap (yuqori) bit — "manfiy" belgisi: u `1` bo'lsa son manfiy. `-1` ning **hamma** bitlari 1.

**Muhim:** xuddi shu 8 bit `unsigned` sifatida o'qilsa — `1111 1111` = 255. **Bitlar bir xil, talqin boshqa.**

```c
/* ishora.c - bir xil bitlar, ikki xil talqin */
#include <stdio.h>

int main(void)
{
    unsigned char u = 255;                 /* 1111 1111 */
    signed char s = (signed char)u;        /* o'sha bitlar, lekin ishorali deb o'qiladi */
    printf("unsigned: %d, signed: %d\n", u, s);

    signed char h = -15;
    unsigned char hu = (unsigned char)h;   /* -15 ning bitlarini ishorasiz o'qish */
    printf("harorat -15 ni unsigned o'qisak: %d\n", hu);
    return 0;
}
```

```console
$ gcc -Wall -Wextra ishora.c -o ishora
$ ./ishora
unsigned: 255, signed: -1
harorat -15 ni unsigned o'qisak: 241
```

**Qatorma-qator:**

| Qator | Nima qiladi | Qiymat / bitlar |
|---|---|---|
| `unsigned char u = 255;` | 1 baytli ishorasiz quti | `1111 1111`, ya'ni 255 |
| `(signed char)u` | **cast**: "shu bitlarni ishorali deb o'qi" | bitlar o'zgarmadi; yuqori bit 1 → manfiy: −1 |
| `signed char h = -15;` | ishorali quti | −15 = `1111 0001` |
| `(unsigned char)h` | xuddi shu bitlarni ishorasiz o'qish | `1111 0001` = 241 |

> **Eslab qoling:** signed va unsigned — bitlar emas, **bitlarni o'qish usuli**. Eng yuqori bit signed'da "minus" belgisi.

**Tez-tez xato:** manfiy bo'lishi mumkin bo'lgan qiymatni `unsigned` ga solish (masalan harorat). `-15` → `241` bo'ladi.

## 2.5. Toshish (overflow) — Python'da yo'q, C'da bor

**Hayotdan misol: eski mashinaning kilometr hisoblagichi.** 6 xonali hisoblagich 999 999 dan keyin `000 000` ni
ko'rsatadi: mashina yangi bo'lib qolmadi, shunchaki xonalar **tugadi**. Quti ham shunday: 16 bitli quti 65535 dan oshmaydi.

```c
/* toshish.c - ishorasiz toshish */
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    uint16_t km = 65530;                /* 16 bitli hisoblagich, eng kattasi 65535 */
    km = km + 10;                       /* 65540 sig'maydi */
    printf("65530 + 10 = %u\n", km);

    uint8_t bayt = 255;
    bayt = bayt + 1;
    printf("255 + 1 (8 bit) = %u\n", bayt);

    uint32_t nol = 0;
    printf("0 - 1 (32 bit) = %u\n", nol - 1);
    return 0;
}
```

```console
$ gcc -Wall -Wextra toshish.c -o toshish
$ ./toshish
65530 + 10 = 4
255 + 1 (8 bit) = 0
0 - 1 (32 bit) = 4294967295
```

**Qadam-baqadam (`km`):** 65530 + 10 = 65540. Quti 16 bit — maksimum 65535. Ortiqcha: 65540 − 65536 = **4**. Ya'ni
natija = (yig'indi) **bo'lingan qoldiq** 2¹⁶ ga. Xuddi soat kabi: 11 dan 3 soat keyin 14 emas, **2** (12 ga bo'lingan qoldiq).

```text
                 65535 ──+1──> 0 ──> 1 ──> 2 ──> 3 ──> 4
65530 ──+1──> ... ──> 65535        (aylanib, boshiga qaytdi)
```

- **Ishorasiz** toshish — standart bo'yicha **aniqlangan**: 2ⁿ ga bo'lingan qoldiq (aylanadi). Xavfsiz, lekin ko'pincha
  **mantiqiy xato**: `0 - 1` → `4294967295` (32 bit) yoki `18446744073709551615` (`size_t`, 64 bit).
- **Ishorali** toshish (`int`) — **UB** (aniqlanmagan xatti-harakat, 13-bob): standart "nima bo'lishini" kafolatlamaydi.
  Kompilyator "bu hech qachon bo'lmaydi" deb faraz qilib kodni o'zgartirishi mumkin. Natija oldindan aytib bo'lmaydi.

Ishorali toshishni ko'rish uchun **UBSan** (aniqlanmagan xatti-harakatni ushlovchi vosita, 13-bob) bilan yig'amiz:

```c
/* ub_toshish.c - int toshishi */
#include <limits.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    (void)argv;
    int i = INT_MAX;                    /* eng katta int: 2147483647 */
    i = i + argc;                       /* argc = 1: 2147483647 + 1 sig'maydi -> UB */
    printf("i = %d\n", i);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=undefined ub_toshish.c -o ub_toshish
$ ./ub_toshish 2>&1
ub_toshish.c:9:7: runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'
i = -2147483648
```

UBSan aniq ko'rsatdi: `signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'`. Oddiy yig'ishda
dastur "shunchaki ishlab ketardi" va `-2147483648` chiqarardi — **bu tasodif**, kafolat emas.

### Klassik tuzoq: `s += i * i`

```c
/* kvadrat_yigindi.c - toshish tuzog'i */
#include <stdio.h>

int main(void)
{
    long s = 0;
    for (int i = 1; i <= 100000; i++)
        s += i * i;                     /* XATO: i * i  int da hisoblanadi */
    printf("noto'g'ri: %ld\n", s);

    long t = 0;
    for (int i = 1; i <= 100000; i++)
        t += (long)i * i;               /* TO'G'RI: avval long ga o'tkazib ko'paytirish */
    printf("to'g'ri:   %ld\n", t);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=undefined kvadrat_yigindi.c -o kvadrat_yigindi
$ ./kvadrat_yigindi 2>&1 | head -4
kvadrat_yigindi.c:8:16: runtime error: signed integer overflow: 46341 * 46341 cannot be represented in type 'int'
noto'g'ri: 18104913692784
to'g'ri:   333338333350000
```

UBSan birinchi xatoni ko'rsatdi (qolgani xuddi shunday takrorlanadi, biz `head` bilan qisqartirdik). "noto'g'ri" qator — **axlat son**, "to'g'ri" qator — haqiqiy yig'indi. Xato: `46341 * 46341` (= 2 147 488 281) `int` ga (maks. 2 147 483 647) **sig'maydi**.

**Muhim tushuncha:** `s` ning `long` bo'lishi `i * i` ni qutqarmaydi. Ifoda **o'z operandlari turida** hisoblanadi
(`int * int` → `int`) va natija keyin `long` ga aylantiriladi — lekin u **allaqachon toshib bo'lgan**. Yechim: ko'paytirishdan
**oldin** bittasini `long` ga aylantirish: `(long)i * i`. 01-mashq aynan shu tuzoq.

> **Eslab qoling:** ifoda **operandlari** turida hisoblanadi, natija turida emas. Katta natija kutsangiz — **ko'paytirishdan oldin** kattaroq turga o'tkazing.

## 2.6. Chegaralar: `<limits.h>`

Har safar "int ning eng kattasi nechta?" deb yodlash shart emas — kompilyator nomlar beradi:

| Nom | Ma'nosi | Qayerdan |
|---|---|---|
| `INT_MAX`, `INT_MIN` | `int` ning eng katta va eng kichik qiymati | `<limits.h>` |
| `UINT_MAX` | `unsigned int` ning eng kattasi | `<limits.h>` |
| `LONG_MAX`, `LONG_MIN` | `long` uchun | `<limits.h>` |
| `CHAR_BIT` | bir baytda nechta bit (= 8) | `<limits.h>` |
| `UINT32_MAX`, `INT64_MIN`, `SIZE_MAX` ... | aniq o'lchamli turlar uchun | `<stdint.h>` |

## 2.7. Kasr sonlar: `float`, `double`

**Hayotdan misol: chizg'ich.** Chizg'ichda millimetrdan kichigini ko'rolmaysiz. `float` ham cheklangan aniqlikda "o'lchaydi":
`0.1` ni aniq saqlay olmaydi, faqat unga **juda yaqin** sonni (chunki kompyuter ikkilikda sanaydi va `0.1` ikkilikda
cheksiz kasr: 1/3 = 0.3333… o'nlikda cheksiz bo'lgani kabi).

- `double` — 8 bayt, ~15 xona aniqlik. `float` — 4 bayt, ~7 xona. `f` bilan tugasa (`1.5f`) — `float`, tugamasa — `double`.
- `0.1 + 0.2 != 0.3` — Python'dagi kabi.
- **Yadroda kasr sonlar deyarli ishlatilmaydi.** Nega: FPU/SSE registrlarini har bir kontekst almashishda saqlash qimmat,
  shuning uchun yadro ularni umuman ishlatmaydi (MyOS: `-mgeneral-regs-only`). Vaqt, foizlar — hammasi butun sonlarda
  (millisekund, "mingdan bir").

**Pul haqida oltin qoida:** pulni **hech qachon** `float` da saqlamang — tiyinda, **butun son** bilan saqlang.

```c
/* pul.c - float va butun son bilan pul */
#include <stdio.h>

int main(void)
{
    float balans_f = 0.0f;
    for (int i = 0; i < 10; i++)
        balans_f += 0.10f;                      /* 10 marta 10 tiyin */
    printf("float bilan:  10 x 0.10 = %.10f\n", balans_f);

    long balans_tiyin = 0;
    for (int i = 0; i < 10; i++)
        balans_tiyin += 10;                     /* 0.10 so'm = 10 tiyin */
    printf("tiyin bilan:  %ld.%02ld so'm\n", balans_tiyin / 100, balans_tiyin % 100);
    return 0;
}
```

```console
$ gcc -Wall -Wextra pul.c -o pul
$ ./pul
float bilan:  10 x 0.10 = 1.0000001192
tiyin bilan:  1.00 so'm
```

**Qatorma-qator:**

| Qator | Nima qiladi | Nega |
|---|---|---|
| `float balans_f = 0.0f;` | kasr quti, boshida 0 | `f` — "bu `float`, `double` emas" |
| `balans_f += 0.10f;` | `balans_f = balans_f + 0.10f` (10 marta) | 0.10 aniq saqlanmaydi: har qo'shishda mayda xato yig'iladi |
| `%.10f` | nuqtadan keyin 10 xona chiqar | xatoni ko'rish uchun |
| `long balans_tiyin` | **butun** son, tiyinda | butun sonlar **aniq** |
| `balans_tiyin / 100` | so'm qismi (butun bo'lish) | 100 / 100 = 1 |
| `balans_tiyin % 100` | tiyin qismi (qoldiq) | 100 % 100 = 0 |
| `%02ld` | ikki xonali, oldiga nol qo'yiladi | `05` ko'rinishida |

`float` ning yig'indisi `1.0000001192` chiqdi (aniq 1 emas); tiyinlarda esa **aniq** `1.00`.

## 2.8. `char` — belgi ham, son ham

Kompyuter uchun harf ham — son. Har bir belgining **kodi** bor (ASCII jadvali): `'A'` = 65, `'B'` = 66, `'0'` = 48, `'7'` = 55.

```c
/* belgi.c - char: harf ham, son ham */
#include <stdio.h>

int main(void)
{
    char c = 'A';                       /* aslida 65 */
    printf("%c %d\n", c, c);            /* bir xil qiymat: belgi sifatida va son sifatida */

    char d = c + 1;                     /* 65 + 1 = 66 */
    printf("%c\n", d);

    int raqam = '7' - '0';              /* 55 - 48 = 7 */
    printf("'7' - '0' = %d\n", raqam);

    printf("char c = %zu bayt, \"A\" satri = %zu bayt\n", sizeof(c), sizeof("A"));
    return 0;
}
```

```console
$ gcc -Wall -Wextra belgi.c -o belgi
$ ./belgi
A 65
B
'7' - '0' = 7
char c = 1 bayt, "A" satri = 2 bayt
```

**Qatorma-qator:**

| Qator | Nima qiladi | Natija |
|---|---|---|
| `char c = 'A';` | 1 baytli quti, ichida `A` kodi | 65 |
| `printf("%c %d\n", c, c)` | bir xil qiymatni **ikki xil** chiqaradi: `%c` — belgi, `%d` — son | `A 65` |
| `char d = c + 1;` | 66 | `'B'` |
| `'7' - '0'` | raqam belgisini songa aylantiradi: 55 − 48 | 7 |
| `sizeof(c)` va `sizeof("A")` | `char` — **1 bayt**; `"A"` — satr: **2 bayt** (`'A'` va oxirida `'\0'`) | 1 va 2 |

**Nega `'7' - '0'` ishlaydi?** ASCII'da raqam belgilari ketma-ket: `'0'`=48, `'1'`=49, … `'9'`=57. Demak `'7' - '0'` = 7.
Bu raqam belgisini songa aylantirishning **klassik usuli**.

- `'A'` (bitta tirnoq) — **bitta belgi**, ya'ni son. `"A"` (qo'sh tirnoq) — **satr**: 2 baytli massiv `{'A', '\0'}`.
  Adashtirish — klassik xato.
- `char` ning ishorali yoki ishorasizligi platformaga bog'liq (x86 — ishorali, ARM — ishorasiz).
  Baytlar bilan ishlaganda doim `unsigned char` yoki `uint8_t` yozing (08-mashqdagi tuzoq).

## 2.9. Literallar — kodga to'g'ridan-to'g'ri yozilgan sonlar

**Literal** — kodga **to'g'ridan-to'g'ri yozilgan qiymat** (`42`, `'A'`, `"salom"`). Yozilishi uning **turini** belgilaydi:

| Yozuv | Ma'nosi |
|---|---|
| `42` | `int` |
| `42u` | `unsigned int` (`u` — unsigned) |
| `42L`, `42UL` | `long`, `unsigned long` |
| `42LL`, `42ULL` | `long long` |
| `0x2A` | **o'n oltilik** (hex) = 42 — **yadroda eng ko'p** |
| `052` | **sakkizlik** (octal) = 42! Oldidagi `0` — sakkizlik degani |
| `0b101010` | ikkilik (GCC kengaytmasi, C23 da standart) |
| `'\n'`, `'\0'`, `'\x1b'` | maxsus belgilar |
| `1e6` | `double` (1000000.0) |

```c
/* literal.c - bir son, to'rt yozuv */
#include <stdio.h>

int main(void)
{
    printf("%d %d %d %d\n", 42, 0x2A, 052, 0b101010);   /* hammasi 42 */
    printf("%d\n", 010);                                /* 8, 10 EMAS! */
    return 0;
}
```

```console
$ gcc -Wall -Wextra literal.c -o literal
$ ./literal
42 42 42 42
8
```

**Nega `0x2A` = 42?** O'n oltilik tizimda 16 ta raqam: `0 1 2 ... 9 A B C D E F` (A=10, … F=15). `0x2A` = 2·16 + 10 = **42**.
Har bir hex raqam — **4 bit**: `0xF` = `1111`, `0xFF` = 8 bit hammasi 1 (255), `0x1000` = 4096 (sahifa hajmi).

**Tez-tez xato:** `010` ni o'nta deb o'ylash. Oldidagi `0` — sakkizlik: `010` = 8. Sonlarning oldiga **nol qo'ymang**.

## 2.10. `const` — o'zgarmas

**Hayotdan misol: toshga o'yilgan yozuv.** Qog'ozdagi yozuvni o'chirib qayta yozish mumkin; toshga o'yilganini — yo'q.

```c
/* const_xato.c - const ni o'zgartirishga urinish */
#include <stdio.h>

int main(void)
{
    const int MAX = 100;                /* "bu qiymat o'zgarmaydi" */
    MAX = 5;                            /* xato! */
    printf("%d\n", MAX);
    return 0;
}
```

```console
$ gcc -Wall -Wextra const_xato.c -o const_xato # xato kutiladi
const_xato.c: In function ‘main’:
const_xato.c:7:9: error: assignment of read-only variable ‘MAX’
    7 |     MAX = 5;                            /* xato! */
      |         ^
```

`const` — "bu qiymatni o'zgartirmayman" degan va'da; kompilyator uni **tekshiradi**. Ko'rsatkichlarda eng foydali:
`size_t strlen(const char *s)` — "men sizning satringizni o'zgartirmayman" (7-bob).

> **Eslab qoling:** o'zgarmasligi kerak bo'lgan hamma narsaga `const` yozing — xato sizdan oldin kompilyatorga ko'rinadi.

## 2.11. Turlarni aylantirish (conversion va cast)

```c
/* aylantirish.c - turlar aralashganda */
#include <stdio.h>

int main(void)
{
    int a = 7, b = 2;
    double d1 = a / b;                  /* int / int = int (3), keyin double ga: 3.0 */
    double d2 = (double)a / b;          /* avval a double ga (cast): 7.0 / 2 = 3.5 */
    printf("d1 = %.1f, d2 = %.1f\n", d1, d2);

    int x = -1;
    unsigned int u = 1;
    if (x < u)
        printf("kichik\n");
    else
        printf("katta! (kutilmagan)\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra aylantirish.c -o aylantirish # xato kutiladi
aylantirish.c: In function ‘main’:
aylantirish.c:13:11: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
   13 |     if (x < u)
      |           ^
$ ./aylantirish
d1 = 3.0, d2 = 3.5
katta! (kutilmagan)
```

**Nima bo'ldi:**

- `a / b` — ikkala operand `int`, shuning uchun **butun** bo'lish: 7 / 2 = 3 (kasr tashlandi). `double` ga solish **keyin** bo'ldi — kech.
- `(double)a` — **cast**: `(tur)ifoda` — "shu qiymatni boshqa turga aylantir". Endi `7.0 / 2` = 3.5.
- Aralash ifodada kichik tur kattasiga ko'tariladi (`int + long` → `long`, `int + double` → `double`).
- **Xavfli joy — signed va unsigned aralashganda:** `x < u` da `x` (−1) `unsigned` ga aylantiriladi → `4294967295` → u (1) dan katta!
  Kompilyator ogohlantirdi (`comparison of integer expressions of different signedness`). Shuning uchun `-Wall -Wextra` shart.

> **Eslab qoling:** `int / int` → `int`. Kasr kerak bo'lsa — **bo'lishdan oldin** `(double)` ga o'tkazing. `signed` bilan `unsigned` ni solishtirmang.

## 2.12. Ko'rinish sohasi (scope)

**Hayotdan misol: xonadagi narsalar.** Oshxonadagi pichoqni yotoqxonadan turib olib bo'lmaydi. `{ }` ichida e'lon qilingan
o'zgaruvchi faqat shu `{ }` ichida **mavjud**.

```c
/* soha.c - o'zgaruvchi qayerda yashaydi */
#include <stdio.h>

int global = 1;                         /* butun faylda ko'rinadi */

int main(void)
{
    int x = 2;                          /* main ichida */
    {
        int y = 3;                      /* faqat shu { } ichida */
        int x = 4;                      /* ICHKI x tashqisini "yashiradi" (shadowing) */
        printf("ichkarida: global=%d x=%d y=%d\n", global, x, y);
    }
    printf("tashqarida: global=%d x=%d\n", global, x);   /* x yana 2, y yo'q */
    return 0;
}
```

```console
$ gcc -Wall -Wextra -Wshadow soha.c -o soha # xato kutiladi
soha.c: In function ‘main’:
soha.c:11:13: warning: declaration of ‘x’ shadows a previous local [-Wshadow]
   11 |         int x = 4;                      /* ICHKI x tashqisini "yashiradi" (shadowing) */
      |             ^
soha.c:8:9: note: shadowed declaration is here
    8 |     int x = 2;                          /* main ichida */
      |         ^
$ ./soha
ichkarida: global=1 x=4 y=3
tashqarida: global=1 x=2
```

**Nima ko'rdik:** ichkarida `x` = 4 (yangi, boshqa quti), tashqarida esa asl `x` = 2 o'zgarmagan. `y` ni `}` dan keyin
ishlatsangiz — "y undeclared" xatosi. Ichki `x` tashqisini **yashirishi** (shadowing) chalkash — shunday yozmang;
`-Wshadow` bayrog'i buni ogohlantiradi.

`{ }` — faqat blok emas, **yangi ko'rinish sohasi**. Python'da `if` ichida yaratilgan o'zgaruvchi tashqarida ham ko'rinadi; C'da — yo'q.

## Hayotdan misol va to'liq dastur

**Qaysi turni tanlayman?** Savollar bilan:

| Nimani saqlayman? | Tur |
|---|---|
| yosh, hisoblagich, oddiy son | `int` |
| juda katta son (milliardlab) | `int64_t` (yoki `long`) |
| hajm, uzunlik, massiv indeksi | `size_t` |
| apparat registri, fayl formati, manzil | `uint8_t`…`uint64_t` |
| harorat (manfiy bo'lishi mumkin) | `int` (ishorali!) |
| pul | butun son (tiyinda) |
| kasr (o'lchov, ilmiy hisob) | `double` |

Hammasini bir dasturda ko'ramiz: toshish, ishora, pul, hajmlar.

```c
/* olchovlar.c - toshish, ishora va pulni to'g'ri saqlash */
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    /* Kilometr hisoblagichi: 16 bitli, eng katta qiymati 65535 */
    uint16_t km = 65530;
    km = km + 10;                               /* 65540 sig'maydi -> aylanadi */
    printf("Hisoblagich: 65530 + 10 = %u (aylanib ketdi)\n", km);

    /* Termometr: ishorali, manfiy bor */
    int8_t harorat = -15;
    printf("Harorat: %d gradus\n", harorat);

    /* Pul: float bilan 0.10 ni 10 marta qo'shish */
    float balans_f = 0.0f;
    for (int i = 0; i < 10; i++)
        balans_f += 0.10f;
    printf("float bilan: 10 x 0.10 = %.10f (1.0 emas!)\n", balans_f);

    /* Pul: tiyinda, butun son bilan - har doim aniq */
    long balans_tiyin = 0;
    for (int i = 0; i < 10; i++)
        balans_tiyin += 10;                     /* 0.10 so'm = 10 tiyin */
    printf("tiyin bilan: %ld.%02ld so'm (aniq)\n", balans_tiyin / 100, balans_tiyin % 100);

    printf("Idishlar o'lchami: char %zu, short %zu, int %zu, long %zu bayt\n",
           sizeof(char), sizeof(short), sizeof(int), sizeof(long));
    return 0;
}
```

```console
$ gcc -Wall -Wextra olchovlar.c -o olchovlar
$ ./olchovlar
Hisoblagich: 65530 + 10 = 4 (aylanib ketdi)
Harorat: -15 gradus
float bilan: 10 x 0.10 = 1.0000001192 (1.0 emas!)
tiyin bilan: 1.00 so'm (aniq)
Idishlar o'lchami: char 1, short 2, int 4, long 8 bayt
```

**Kodda nimalar bor:**

| Nom | Tur | Boshlang'ich | Nima uchun shu tur |
|---|---|---|---|
| `km` | `uint16_t` (16 bit) | 65530 | hisoblagich: manfiy bo'lmaydi, chegarasi 65535 |
| `harorat` | `int8_t` (8 bit, ishorali) | −15 | manfiy bo'lishi mumkin |
| `balans_f` | `float` | 0.0 | **yomon** misol: pulni kasrda saqlash |
| `balans_tiyin` | `long` | 0 | **to'g'ri**: pulni tiyinda, butun sonda |

**Sinab ko'ring:** `uint16_t km` ni `uint32_t` qiling — endi aylanadimi? `int8_t harorat = -15;` ni `uint8_t` qiling — nima
chiqadi va nega (2.4)?

## Bob xulosasi (yodlash uchun)

1. O'zgaruvchi — xotiradagi **aniq o'lchamli quti**: tur (hajm) + nom + qiymat. Qiymatni **doim** boshida bering.
2. `char`=1, `short`=2, `int`=4, `long`=8 bayt; apparat uchun — `uint8_t … uint64_t` (`<stdint.h>`).
3. Quti `n` bit bo'lsa, `2ⁿ` xil qiymat sig'adi; ishorali sonda yarmi manfiy (**ikkiga to'ldirish**: teskari + 1).
4. **Toshish:** ishorasiz — aylanadi (aniqlangan), ishorali — **UB**. Ifoda **operandlari** turida hisoblanadi.
5. `int / int` = `int`; pul — butun son; `signed` bilan `unsigned` ni aralashtirmang; `{ }` — alohida ko'rinish sohasi.

## Savol-javob

**Qachon `int`, qachon `long`, qachon `size_t`?**
Hisoblagich va kichik sonlar — `int`. Hajm, uzunlik, massiv indeksi — `size_t`. Katta hisob-kitob — `long`/`int64_t`.
Apparat bilan ishlash (registrlar, disk tuzilmalari) — `uint8_t..uint64_t`.

**Nega `size_t` ishorasiz?**
Hajm manfiy bo'lmaydi va ishorasiz tur ikki baravar katta qiymatni sig'diradi. Lekin tuzog'i bor:
`for (size_t i = n - 1; i >= 0; i--)` — **cheksiz sikl**, chunki `i >= 0` ishorasizda doim rost (05-mashq).

**`0x` sonlarni qanday tez o'qiyman?**
Har bir o'n oltilik raqam — 4 bit. `0xFF` = 8 bit hammasi 1. `0x1000` = 4096 (sahifa hajmi).
`0xFFFFFFFF80000000` — MyOS yadrosining boshlanish manzili. Yadroda 16-lik tizimda o'ylashga o'rganing.

**Nega `-Wall` kerak, agar dastur baribir ishlayapti?**
Chunki C xatoni **jim** qoldiradi (toshish, aralash tur, boshlang'ich qiymatsiz). Ogohlantirish — yagona erta signal.

## O'zingizni tekshiring

1. `int` qancha bayt? `long` Linux'da va Windows'da?
2. `unsigned char c = 200; c = c + 100;` — `c` nechaga teng?
3. `7 / 2` va `7.0 / 2` natijasi?
4. `'5' - '0'` nechaga teng va nega?
5. `010` nechaga teng?
6. `long s = 50000 * 50000;` — nima xato?
7. `-3` ni 8 bitda ikkiga to'ldirish bilan yozing.

<details><summary>Javoblar</summary>

1. 4; Linux'da 8, Windows'da 4.
2. 44 (300 − 256): ishorasiz toshish modulli.
3. 3 va 3.5.
4. 5: raqam belgilari ASCII'da ketma-ket ('0'=48, '5'=53).
5. 8 (sakkizlik).
6. `50000 * 50000` `int` da hisoblanadi va toshadi (UB); `50000L * 50000` yozish kerak.
7. 3 = `0000 0011` → teskari `1111 1100` → +1 = `1111 1101`.
</details>

## Mashq

- **01** (kvadratlar yig'indisi) — tur va toshish.
- **02** (tub sonlar) — `unsigned` va toshmaydigan shart.
- **03** (toshishsiz arifmetika) — chegaralar va UB.
- Qo'shimcha: `sizeof` bilan barcha turlarning hajmini chiqaradigan dastur yozing.

<!-- loyiha:boshi -->
## Loyiha: turlar jadvali va toshish

**Maqsad:** har bir butun tur xotirada necha bayt olishini, chegaralarini va toshganda nima bo'lishini
o'z ko'zingiz bilan ko'rish.
**Bobdan ishlatiladi:** `<stdint.h>` turlari, `sizeof`, `<limits.h>`, unsigned toshishi, butun bo'lish.

**Talab:** (1) turlar jadvali: nomi, hajmi, eng kichik va eng katta qiymati; (2) uchta "tuzoq" natijasi.
**Ma'lumotlar:** yo'q — hammasi doimiylar (`INT8_MIN`, `sizeof(...)`).
**Qadamlar:** sarlavha chiqarish → har tur uchun bitta `printf` → tuzoqlar.

```c
/* turlar.c - turlar jadvali */
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    printf("%-9s %4s  %-20s %s\n", "tur", "bayt", "eng kichik", "eng katta");
    printf("%-9s %4zu  %-20d %d\n", "int8_t", sizeof(int8_t), INT8_MIN, INT8_MAX);
    printf("%-9s %4zu  %-20u %u\n", "uint8_t", sizeof(uint8_t), 0u, UINT8_MAX);
    printf("%-9s %4zu  %-20d %d\n", "int16_t", sizeof(int16_t), INT16_MIN, INT16_MAX);
    printf("%-9s %4zu  %-20u %u\n", "uint16_t", sizeof(uint16_t), 0u, UINT16_MAX);
    printf("%-9s %4zu  %-20d %d\n", "int32_t", sizeof(int32_t), INT32_MIN, INT32_MAX);
    printf("%-9s %4zu  %-20u %u\n", "uint32_t", sizeof(uint32_t), 0u, UINT32_MAX);
    printf("%-9s %4zu  %-20lld %lld\n", "int64_t", sizeof(int64_t), (long long)INT64_MIN,
           (long long)INT64_MAX);
    printf("%-9s %4zu  %-20u %llu\n", "uint64_t", sizeof(uint64_t), 0u,
           (unsigned long long)UINT64_MAX);

    printf("\nTuzoqlar:\n");
    uint8_t bayt = 255;
    bayt = bayt + 1;                    /* 256 sig'maydi -> 0 ga aylanadi */
    printf("  uint8_t: 255 + 1 = %u\n", bayt);

    uint32_t nol = 0;
    printf("  uint32_t: 0 - 1 = %u\n", nol - 1);

    printf("  butun bo'lish: 7 / 2 = %d, lekin 7 / 2.0 = %.1f\n", 7 / 2, 7 / 2.0);
    return 0;
}
```

```console
$ gcc -Wall -Wextra turlar.c -o turlar
$ ./turlar
tur       bayt  eng kichik           eng katta
int8_t       1  -128                 127
uint8_t      1  0                    255
int16_t      2  -32768               32767
uint16_t     2  0                    65535
int32_t      4  -2147483648          2147483647
uint32_t     4  0                    4294967295
int64_t      8  -9223372036854775808 9223372036854775807
uint64_t     8  0                    18446744073709551615

Tuzoqlar:
  uint8_t: 255 + 1 = 0
  uint32_t: 0 - 1 = 4294967295
  butun bo'lish: 7 / 2 = 3, lekin 7 / 2.0 = 3.5
```

**Kengaytiring:** `char` va `long` uchun ham qator qo'shing (`CHAR_MIN` — `<limits.h>`). `long` hajmi
sizda 8 bayt bo'lishi mumkin, Windows'da esa 4 — shuning uchun yadroda `long` emas, `int64_t` yoziladi.

## Mustaqil loyiha: video hajmi hisoblagichi ★☆☆

**Vazifa:** siqilmagan video qancha joy oladi? Bu hisobda `int` **toshadi** — sizning ishingiz shuni
sezish va to'g'ri turni tanlash. Fayl: `video.c`.

**Ma'lumotlar** (o'zgaruvchi qilib yozing):
- kenglik 1920 piksel, balandlik 1080 piksel
- har piksel 3 bayt (R, G, B)
- soniyasiga 60 kadr
- davomiyligi 600 soniya (10 daqiqa)

**Talab.** Quyidagilarni hisoblab chiqaring:
- bitta kadr hajmi (bayt), bir soniya hajmi (bayt), butun video hajmi (bayt);
- butun video hajmi **GiB** da (1 GiB = 1024×1024×1024 bayt) — kasr bilan, 2 xona aniqlik;
- `int` ning eng katta qiymati (`INT_MAX`, `<limits.h>` dan);
- butun video hajmi shu chegaradan **necha barobar** katta (butun bo'lish bilan) — shu bilan `int` nega
  yetmasligini ko'rsatasiz.

Yorliq `%-22s`, qiymat `%lld`/`%.2f` (aniq shaklini kutilgan natijadan ko'ring).

**Kutilgan natija** (`darslik/loyihalar/02_tur_jadvali/kutilgan.txt`):

```text
Bitta kadr:           6220800 bayt
Bir soniya:           373248000 bayt
Butun video:          223948800000 bayt
Hajmi:                208.57 GiB
int eng kattasi:      2147483647
int ga sig'maydi:     104 barobar katta
```

**Qo'shimcha sinov.** 1280×720, 3 bayt, 30 kadr, 120 soniya:

```text
Bitta kadr:           2764800 bayt
Bir soniya:           82944000 bayt
Butun video:          9953280000 bayt
Hajmi:                9.27 GiB
int eng kattasi:      2147483647
int ga sig'maydi:     4 barobar katta
```

**Maslahat** (yechim emas):
- Avval qog'ozda kadr hajmini hisoblang: 1920 × 1080 × 3 = 6 220 800 — bu `int` ga sig'adi. 60 ga ko'paytirsangiz-chi?
- Qaysi tur 20 milliarddan katta sonni saqlaydi? `int64_t`. Ko'paytmada **bitta** operand `int64_t` bo'lsa,
  butun ifoda shu turda hisoblanadi — lekin qaysi joydan boshlab?
- `(int64_t)a * b * c` va `a * b * (int64_t)c` farqi nima? (Ikkinchisida `a * b` allaqachon `int` da hisoblanadi!)
- GiB uchun `double` kerak: `(double)hajm / (1024.0 * 1024 * 1024)`.
- `int64_t` ni `printf` da `%lld` bilan chiqaring va `(long long)` bilan cast qiling.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g video.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/02_tur_jadvali/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [3-bob. Operatorlar](03-operatorlar.md)
