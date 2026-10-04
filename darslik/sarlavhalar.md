# Sarlavha fayllari: `#include` nima qiladi va qaysi biri nima beradi

> **Bu sahifada:** kodning boshida yozadigan `#include <...>` qatorlari nimaligini, ularni nega yozish
> kerakligini va **qaysi ish uchun qaysi birini** yozishni bilib olasiz. Alohida bo'lim `<stdint.h>` ga bag'ishlangan.
> **Oldindan nima kerak:** hech narsa. 0-bobni o'qigan bo'lsangiz kifoya.
> **Qachon qaytasiz:** kodda notanish `<something.h>` uchrasa yoki kompilyator
> `implicit declaration of function` desa.

## 1. `#include` nima qiladi?

**Oddiy qilib aytganda:** `#include <fayl.h>` — "shu faylning ichidagi hamma narsani **mana shu joyga
ko'chirib qo'y**" degan buyruq.

**Hayotdan misol.** Siz oshxonada ovqat pishiryapsiz va retseptda yozilgan: "Sho'rva tayyorlash usulini
5-sahifadan ko'ring". Siz o'sha sahifani retseptingizga ko'chirib qo'ymaysiz, shunchaki ko'rsatma bor.
`#include` ham shunday ko'rsatma, faqat **kompilyator o'zi ko'chirib qo'yadi**.

**Qadam-baqadam.** Mana uch qatorli "sarlavha fayl" va uni ishlatuvchi dastur:

```c
/* salom_xabar.h - o'zim yasagan eng oddiy sarlavha fayl */
#define KIM "Dilnoza"
#define NECHCHI 3
```

```c
/* ilova.c - sarlavhani ichiga ko'chirib qo'yish */
#include <stdio.h>
#include "salom_xabar.h"

int main(void)
{
    printf("Salom, %s! Sizda %d ta xabar bor.\n", KIM, NECHCHI);
    return 0;
}
```

```console
$ gcc -Wall -Wextra ilova.c -o ilova
$ ./ilova
Salom, Dilnoza! Sizda 3 ta xabar bor.
```

Endi kompilyatorning ko'zi bilan qaraymiz: `#include` lar bajarilgandan **keyin** fayl qanday ko'rinadi?
`-E` bayrog'i faqat shu birinchi bosqichni qiladi (1-bob):

```console
$ gcc -E ilova.c | tail -4
{
    printf("Salom, %s! Sizda %d ta xabar bor.\n", "Dilnoza", 3);
    return 0;
}
```

Ko'rdingizmi? `#include "salom_xabar.h"` qatori yo'q bo'lib, uning o'rniga fayl mazmuni keldi,
`KIM` va `NECHCHI` esa o'z qiymatlari bilan almashtirildi. `#include <stdio.h>` ham xuddi shunday ishlaydi —
faqat uning ichida 700 dan ortiq qator bor:

```console
$ gcc -E ilova.c | wc -l
823
```

> **Eslab qoling:** `#include` — kod yozmaydi, **ko'chiradi**. Sarlavha fayl ichida ko'pincha **tayyor ish**
> emas, faqat **"bunday narsa bor" degan e'lonlar** turadi.

### `<fayl.h>` va `"fayl.h"` farqi

| Yozilishi | Qayerdan qidiriladi | Qachon ishlatiladi |
|---|---|---|
| `#include <stdio.h>` | tizim papkalaridan (`/usr/include`...) | **tayyor** kutubxonalar uchun |
| `#include "mening.h"` | avval **sizning papkangizdan**, topilmasa tizimdan | **o'zingiz** yozgan fayllar uchun |

Eslab qolish oson: burchakli qavs `< >` — "tizimdan ol", qo'shtirnoq `" "` — "yonimdagidan ol".

## 2. Nega kerak? Yozmasam nima bo'ladi?

**Oddiy qilib aytganda:** kompilyator dastur matnini **tepadan pastga** o'qiydi. U `printf` degan so'zni
birinchi marta ko'rganda uning nima ekanini **bilishi kerak**: nima oladi, nima qaytaradi. Buni
sarlavha fayl aytib beradi.

**Hayotdan misol.** Do'kon menyusi. Mijoz "Osh bering" desa, oshpaz menyuda osh bor-yo'qligini bilishi kerak.
Menyuda bo'lmasa — "bunday taom yo'q". `#include <stdio.h>` — kompilyatorga `printf` menyusini berish.

Ataylab sinab ko'ramiz. Mana `#include` siz dastur:

```c
/* include_yoq.c - sarlavhasiz printf */
int main(void)
{
    printf("Salom\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra include_yoq.c -o include_yoq # xato kutiladi
include_yoq.c: In function ‘main’:
include_yoq.c:4:5: warning: implicit declaration of function ‘printf’ [-Wimplicit-function-declaration]
    4 |     printf("Salom\n");
      |     ^~~~~~
include_yoq.c:1:1: note: include ‘<stdio.h>’ or provide a declaration of ‘printf’
  +++ |+#include <stdio.h>
    1 | /* include_yoq.c - sarlavhasiz printf */
include_yoq.c:4:5: warning: incompatible implicit declaration of built-in function ‘printf’ [-Wbuiltin-declaration-mismatch]
    4 |     printf("Salom\n");
      |     ^~~~~~
include_yoq.c:4:5: note: include ‘<stdio.h>’ or provide a declaration of ‘printf’
```

Kompilyator `implicit declaration of function 'printf'` ("`printf` degan funksiya e'lon qilinmagan") deydi va
yana yordam beradi: `include '<stdio.h>' or provide a declaration`. Ya'ni "`<stdio.h>` ni qo'shing".
Yangi kompilyatorlarda (GCC 14 va undan keyingi) bu endi ogohlantirish emas, **xato**.

> **Eslab qoling:** `implicit declaration of function 'X'` ko'rsangiz — `X` uchun kerakli
> `#include` ni unutgansiz. Qaysi biri kerakligini pastdagi jadvaldan toping.

## 3. Qaysi ish uchun qaysi sarlavha?

Python'da `print()` ni to'g'ridan-to'g'ri yozasiz. C'da esa funksiya **qaysi sarlavhada** turishini
bilish kerak. Eng ko'p uchraydiganlari:

| Nima qilmoqchisiz | Funksiya / nom | Yozing |
|---|---|---|
| Ekranga chiqarish, fayl bilan ishlash | `printf`, `scanf`, `fopen`, `fgets`, `snprintf` | `#include <stdio.h>` |
| Xotira ajratish, tartiblash, songa aylantirish | `malloc`, `free`, `qsort`, `atoi`, `exit`, `rand` | `#include <stdlib.h>` |
| Satr va xotira bilan ishlash | `strlen`, `strcpy`, `strcmp`, `memcpy`, `memset` | `#include <string.h>` |
| **Aniq o'lchamli sonlar** | `uint8_t`, `int32_t`, `uint64_t` | `#include <stdint.h>` |
| `size_t`, `NULL`, `offsetof` | | `#include <stddef.h>` |
| `true` / `false` | `bool` | `#include <stdbool.h>` |
| Turlarning chegaralari | `INT_MAX`, `CHAR_BIT`, `LONG_MIN` | `#include <limits.h>` |
| Belgini tekshirish | `isalpha`, `isdigit`, `toupper` | `#include <ctype.h>` |
| Xato kodi | `errno`, `ENOENT` | `#include <errno.h>` |
| Matematika | `sqrt`, `pow`, `sin` | `#include <math.h>` (+ `-lm`) |
| O'zgaruvchan argumentlar | `va_list`, `va_arg` | `#include <stdarg.h>` |
| Tekshiruv | `assert` | `#include <assert.h>` |
| Vaqt | `time`, `clock_gettime` | `#include <time.h>` |
| Tizim chaqiruvlari (Linux) | `read`, `write`, `fork`, `getpid` | `#include <unistd.h>` |
| Fayl ochish | `open`, `O_RDONLY` | `#include <fcntl.h>` |
| Oqimlar (threads) | `pthread_create`, `pthread_mutex_lock` | `#include <pthread.h>` (+ `-pthread`) |

> **Qanday bilaman?** Terminalda `man 3 funksiya_nomi` yozing (masalan `man 3 strlen`). Eng tepada
> `SYNOPSIS` bo'limida `#include <...>` qatori yozilgan bo'ladi. Bu eng ishonchli usul.

## 4. `<stdint.h>` — aniq o'lchamli sonlar

### Muammo: `int` necha bayt?

**Oddiy qilib aytganda:** C da `int` ning o'lchami **qat'iy emas**. Bir kompyuterda 4 bayt, boshqasida 2 bayt
bo'lishi mumkin. Python'da bunday muammo yo'q (son o'zi kattalashadi). C'da esa har bir son xotirada
**aniq** necha bayt joy olishini **siz** bilishingiz kerak.

**Hayotdan misol.** Choy qoshiq va oshxona qoshig'ini olaylik. Retseptda "1 qoshiq tuz" desa, qaysi
qoshiq? Har oshxonada o'lchami har xil. Retseptda "5 gramm tuz" deyilsa — hamma joyda bir xil.
`int` — "qoshiq", `int32_t` — "5 gramm".

Mana ko'ramiz, bu kompyuterda tabiiy turlar qancha joy oladi:

```c
/* turlar_olchami.c - tabiiy turlar necha bayt */
#include <stdio.h>

int main(void)
{
    printf("char  : %zu bayt\n", sizeof(char));
    printf("short : %zu bayt\n", sizeof(short));
    printf("int   : %zu bayt\n", sizeof(int));
    printf("long  : %zu bayt\n", sizeof(long));
    return 0;
}
```

```console
$ gcc -Wall -Wextra turlar_olchami.c -o turlar_olchami
$ ./turlar_olchami
char  : 1 bayt
short : 2 bayt
int   : 4 bayt
long  : 8 bayt
```

Linux'da (64 bitli) `long` — 8 bayt, Windows'da esa 4 bayt. Bir xil kod boshqa kompyuterda
**boshqacha** ishlashi mumkin. Dasturchi buni istamaydi.

### Yechim: `<stdint.h>`

`<stdint.h>` (standard integers — "standart butun sonlar") shunday turlar beradi, ularning nomi
o'lchamini **aytib turadi**:

| Tur | Bit | Bayt | Qiymatlar | Qo'shimcha |
|---|---|---|---|---|
| `int8_t` | 8 | 1 | −128 … 127 | ishorali (manfiy ham bo'ladi) |
| `uint8_t` | 8 | 1 | 0 … 255 | ishorasiz (`u` = unsigned = ishorasiz) |
| `int16_t` | 16 | 2 | −32 768 … 32 767 | |
| `uint16_t` | 16 | 2 | 0 … 65 535 | |
| `int32_t` | 32 | 4 | ≈ −2.1 mlrd … 2.1 mlrd | |
| `uint32_t` | 32 | 4 | 0 … ≈ 4.29 mlrd | |
| `int64_t` | 64 | 8 | ≈ ±9.2 × 10¹⁸ | |
| `uint64_t` | 64 | 8 | 0 … ≈ 1.8 × 10¹⁹ | |

**Nom qanday o'qiladi:** `u?int` + **bit soni** + `_t`. `uint8_t` = **u**nsigned **int** **8** bit.
`_t` — "type" (tur) ning qisqartmasi.

### Qanday ishlatiladi

```c
/* stdint_misol.c - aniq o'lchamli sonlar */
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    uint8_t  bayt = 200;                /* aniq 1 bayt, 0..255 */
    int32_t  harorat = -5;              /* aniq 4 bayt, manfiy ham mumkin */
    uint64_t katta = 1ull << 40;        /* aniq 8 bayt */

    printf("hajmlari: %zu, %zu, %zu bayt\n", sizeof(bayt), sizeof(harorat), sizeof(katta));
    printf("uint8_t chegarasi: 0 dan %u gacha\n", UINT8_MAX);
    printf("int32_t chegarasi: %d dan %d gacha\n", INT32_MIN, INT32_MAX);

    bayt = bayt + 100;                  /* 300 sig'maydi: 300 - 256 = 44 */
    printf("200 + 100 (uint8_t ichida) = %u\n", bayt);
    return 0;
}
```

```console
$ gcc -Wall -Wextra stdint_misol.c -o stdint_misol
$ ./stdint_misol
hajmlari: 1, 4, 8 bayt
uint8_t chegarasi: 0 dan 255 gacha
int32_t chegarasi: -2147483648 dan 2147483647 gacha
200 + 100 (uint8_t ichida) = 44
```

Nimalar ko'rdingiz:
1. `uint8_t` har qanday kompyuterda **aniq 1 bayt** — natija hamma joyda bir xil.
2. `<stdint.h>` yana **chegara nomlarini** beradi: `UINT8_MAX`, `INT32_MIN`, `INT32_MAX`... Ularni yodlash shart emas.
3. Tur to'lib ketganda (200 + 100) sig'maydi va "aylanib" 44 bo'ladi (2-bobdagi "toshish").

### Nega aynan shu kerak? (kimga foydasi)

| Holat | Nega `int` yetmaydi, `int32_t` kerak |
|---|---|
| **Qurilma registri** (16-bob) | Datasheet "32 bitli registr" deydi. `int` ga ishonib bo'lmaydi |
| **Fayl formati** (ELF, MBR, 22-bob) | Faylda maydon aniq 4 bayt. Boshqa o'lchamda fayl buziladi |
| **Tarmoq paketi** | Sarlavhadagi maydonlar aniq o'lchamda |
| **Bit amallari** (3-bob) | `1 << 31` ning natijasi tur o'lchamiga bog'liq |
| **Yadro** | Hamma joyda aniq o'lcham kerak. Linux'da `u8`, `u16`, `u32`, `u64` — xuddi shu g'oya |

> **Eslab qoling:** *"Son necha bit bo'lishi muhim bo'lsa — `<stdint.h>` va `uint32_t`; oddiy sanagich (0, 1, 2...)
> uchun — `int`."*

### `<stdint.h>` da yana nimalar bor

| Nom | Nima | Qachon |
|---|---|---|
| `uintptr_t`, `intptr_t` | **manzilni** son sifatida saqlaydigan tur (ko'rsatkich bilan bir xil o'lcham) | manzilni bit amallari bilan o'zgartirish (7, 16-boblar) |
| `UINT8_MAX`, `INT64_MIN`, ... | chegara nomlari | toshishni tekshirish (13-bob) |
| `UINT32_C(5)` | "5" sonini `uint32_t` turida yozish | doimiy son turi muhim bo'lganda |
| `SIZE_MAX` | `size_t` ning eng katta qiymati | |

`size_t` bu ro'yxatda **yo'q**: u `<stddef.h>` da (va ko'p boshqa sarlavhalar uni o'zi qo'shadi).
`printf` da `uint64_t` ni chiqarish uchun `%llu` va `(unsigned long long)` o'tkazish ishlatiladi, yoki
`<inttypes.h>` dagi `PRIu64`.

### Chuqurroq (xohlasangiz)

`<stdint.h>` ichida aslida **`typedef`** lar bor (9-bob). Kompilyator buni ko'rsatadi:

```console
$ echo '#include <stdint.h>' | gcc -E -x c - | grep -E "typedef .* (uint8_t|int32_t|uint64_t);"
typedef __int32_t int32_t;
typedef __uint8_t uint8_t;
typedef __uint64_t uint64_t;
```

Ko'rdingizmi: `uint8_t` — shunchaki boshqa nomga (`__uint8_t`, oxir-oqibat `unsigned char`) berilgan **yangi nom**. `int32_t` esa
hozirgi kompyuterda `int` ga bog'langan. Sehr yo'q. Boshqa kompyuterda (masalan, `int` 2 bayt bo'lgan) `int32_t` o'sha yerdagi
**4 baytli** turga bog'lanadi. Shuning uchun nom doim to'g'ri.

## 5. Sarlavha fayllar qayerda turadi?

```console
$ echo '#include <stdint.h>' | gcc -E -H -x c - 2>&1 >/dev/null | head -3
. /usr/lib/gcc/x86_64-linux-gnu/13/include/stdint.h
.. /usr/include/stdint.h
... /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
```

Ko'rinib turibdiki, `<stdint.h>` tizimda oddiy matn fayli (`/usr/include/stdint.h`). Uni ochib o'qishingiz mumkin:
`less /usr/include/stdint.h`. Tashqaridan o'rnatilgan kutubxona bo'lsa, boshqa papkada turadi; `gcc -I/yo'l` bilan
qo'shimcha qidiruv papkasini bildirasiz.

## 6. O'zingizning sarlavha faylingiz

O'z sarlavha faylingizga **`#pragma once`** yozing — shunda u ikki marta ko'chirilib ketmaydi (1.3-bo'lim).
Ichiga **e'lonlar** yoziladi (funksiya nomi, `struct`, `#define`), funksiyaning **tanasi** emas.
Tafsilot — 1-bobning 1.3 bo'limi va 11-bob.

## Xulosa (yodlash uchun)

1. `#include` — faylni shu joyga **ko'chiradi**; o'zi hech narsa hisoblamaydi.
2. Sarlavha faylda ko'pincha **e'lonlar** turadi: kompilyator funksiyani "taniydi".
3. `<...>` — tizimdan, `"..."` — o'zingizning papkangizdan.
4. `implicit declaration of function 'X'` — `X` ning sarlavhasi yozilmagan.
5. Son o'lchami muhim bo'lsa — `<stdint.h>`: `uint8_t`, `int32_t`, `uint64_t`. Qanday sarlavha kerakligini `man 3 nom` aytadi.
