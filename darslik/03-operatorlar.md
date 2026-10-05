# 3-bob. Operatorlar

> **Bu bobda nima o'rganasiz:** C'dagi har bir **operator belgisi** nima qilishini (`+ - * / %`, `== !=`, `&& || !`,
> `& | ^ ~ << >>`, `++`, `?:`); ularning tuzoqlarini; va eng muhimi — **bitlar bilan ishlashni**: bitta sonning
> ichidagi alohida "kalitlarni" yoqish, o'chirish, tekshirish. Yadroda bu — har kunlik ish.
> **Oldindan nima kerak:** 2-bob (turlar, bit va bayt).   **Vaqt:** 6–8 soat (bitlar bo'limiga ko'proq vaqt bering!).
> Mashqlar: 02, 03, 04.

> **To'liq ishlaydigan misol:** [misollar/03_bitlar.c](misollar/03_bitlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

**Operator** — amal belgisi: `+`, `-`, `==` kabi. Operatorlar qiymatlarni olib, **yangi qiymat** chiqaradi:
`3 + 4` → `7`. Maktabdagi arifmetikadan tashqari C'da yana ikki guruh bor: **mantiqiy** (`&&`, `||`: "va", "yoki")
va **bitli** (`&`, `|`, `<<`: sonning ichki bitlariga ishlaydi). Bitli operatorlar boshlovchilar uchun eng sirli —
shuning uchun bobning yarmi shularga bag'ishlangan va **har bir qadam** ko'rsatilgan.

| Guruh | Belgilar | Savol, javob beradi |
|---|---|---|
| Arifmetika | `+ - * / %` | "Qancha?" |
| Taqqoslash | `== != < > <= >=` | "Teng-mi? Katta-mi?" (javob: 1 yoki 0) |
| Mantiq | `&& \|\| !` | "Ikkala shart ham rostmi?" |
| Bitli | `& \| ^ ~ << >>` | "Sonning ichidagi kalitlar qanday turibdi? Ularni o'zgartir" |
| Berish | `= += -= \|= &= ...` | "Qiymatni qutiga yoz" |

Tez qidirish uchun hamma belgilar [belgilar.md](belgilar.md) da.

## 3.1. Arifmetika

| Operator | Ma'nosi | Misol | Natija |
|---|---|---|---|
| `+ - *` | qo'shish, ayirish, ko'paytirish | `7 * 3` | `21` |
| `/` | bo'lish | `7 / 2` | `3` (**butun** bo'lish!) |
| `%` | qoldiq | `7 % 2` | `1` |
| `-x` | ishorani o'zgartirish | `-5` | |

**Hayotdan misol: olma tarqatish.** 7 ta olmani 2 bolaga bo'lsangiz, har biriga **3** tadan tegadi (`7 / 2 = 3`),
**1** ta ortib qoladi (`7 % 2 = 1`). Olmani kesmaysiz — butun sonlar ham kesilmaydi, kasr tashlanadi.

**Hayotdan misol: soat.** Hozir soat 22:00. 5 soatdan keyin soat nechchi? 27 emas — `(22 + 5) % 24 = 3`. Soat 24 ga
yetganda boshidan boshlanadi. Hafta kunlari (`% 7`), oylar (`% 12`), yadrodagi aylanma bufer — hammasi shu g'oya.

```c
/* hisob.c - arifmetika va butun bo'lish */
#include <stdio.h>

int main(void)
{
    printf("7 bo'lingan 2 = %d,  7 ning 2 ga qoldig'i = %d\n", 7 / 2, 7 % 2);
    printf("-7 bo'lingan 2 = %d, -7 ning 2 ga qoldig'i = %d\n", -7 / 2, -7 % 2);
    printf("7 bo'lingan 2.0 = %.1f\n", 7 / 2.0);
    printf("soat: (22 + 5) ning 24 ga qoldig'i = %d\n", (22 + 5) % 24);
    return 0;
}
```

```console
$ gcc -Wall -Wextra hisob.c -o hisob
$ ./hisob
7 bo'lingan 2 = 3,  7 ning 2 ga qoldig'i = 1
-7 bo'lingan 2 = -3, -7 ning 2 ga qoldig'i = -1
7 bo'lingan 2.0 = 3.5
soat: (22 + 5) ning 24 ga qoldig'i = 3
```

**Bu dastur nima qiladi:** to'rtta hisobni bajaradi va natijasini ekranga yozadi. Asosiy maqsad — `/` (bo'lish) va `%` (qoldiq) amallari
**butun sonlarda** qanday ishlashini ko'rish. Dastur ikki xil ish qiladi, ularni alohida ko'ramiz:

**1) Chiqarish (`printf`).** `printf` — ekranga matn chiqaradigan funksiya; uning **asosiy ishi** shu. Qavs ichida avval qo'shtirnoqdagi matn (**qolip**), keyin vergul bilan
qiymatlar yoziladi. Qolipdagi `%d` — "shu yerga butun son qo'y" degani; qiymatlar qolipdagi `%d` larga **tartib bilan** joylashadi:

```text
printf("7 bo'lingan 2 = %d,  7 ning 2 ga qoldig'i = %d\n",   7 / 2,   7 % 2);
                       ^                                ^       ^       ^
                       1-%d  <-------- 7 / 2 = 3 --------+       |       |
                                                       2-%d <----+-- 7 % 2 = 1
```

**2) Hisoblash.** Bu amallar printf'ning ichida **qiymat sifatida** hisoblanadi:

| Ifoda | Nima qiladi | Natija | Izoh |
|---|---|---|---|
| `7 / 2` | bo'lish; ikkala son `int` bo'lgani uchun **butun** bo'lish | `3` | kasr qismi (0.5) tashlanadi |
| `7 % 2` | **qoldiq**: 7 ni 2 ga bo'lganda ortib qolgani | `1` | 7 = 3·2 + **1** |
| `7 / 2.0` | `2.0` kasr son → natija ham kasr | `3.5` | bittasini kasr qilsak, hisob kasrda bo'ladi |
| `-7 / 2` | manfiy son bo'linganda **nolga tomon** kesadi | `-3` | Python'da `-7 // 2 == -4` (pastga yaxlitlaydi) — farq! |
| `-7 % 2` | qoldiqning ishorasi **bo'linuvchiniki** | `-1` | Python'da `1`. Manfiy sonlar bilan `%` ehtiyot bo'lib ishlating |
| `(22 + 5) % 24` | soat hisobi: 27 soat = 1 sutka + 3 soat | `3` | qoldiq aylanishni beradi |

### Maxsus holat: ekranda `%` belgisini chiqarish

Yuqoridagi dasturda `%` belgisi **ikki xil joyda** uchradi, ularni adashtirmang:

- `printf` **qolipi ichida** (`"...%d..."`): bu yerda `%` — **buyruq belgisi** ("shu yerga qiymat qo'y").
- **hisoblashda** (`7 % 2`): bu yerda `%` — **qoldiq** amali.

Endi savol: agar ekranda oddiy `%` harfini ko'rsatmoqchi bo'lsak-chi (masalan "QQS 12%")? Qolip ichida `%` buyruq hisoblanadi, shuning uchun oddiy `%` ni
**ikkita** yozamiz: `%%`. `printf` buni "ekranga bitta `%` yoz" deb tushunadi.

**Bu dastur nima qiladi (umumiy):** ekranga haqiqiy `%` belgisini chiqarishni ko'rsatadi: `printf` ichida `%%` yoziladi.

```c
/* foiz.c - ekranda % belgisini chiqarish */
#include <stdio.h>

int main(void)
{
    int stavka = 12;
    printf("QQS stavkasi: %d%%\n", stavka);      /* %d - son o'rniga, %% - oddiy % belgisi */
    return 0;
}
```

```console
$ gcc -Wall -Wextra foiz.c -o foiz
$ ./foiz
QQS stavkasi: 12%
```

Qolipni chapdan o'ngga o'qing: `%d` → `12` qo'yildi; keyingi `%%` → ekranda bitta `%`; `\n` → yangi qator. Natija: `QQS stavkasi: 12%`.

> **Eslab qoling:** printf qolipida `%` — buyruq belgisi. Oddiy `%` kerak bo'lsa — `%%`.

**Nega `-7 / 2` C'da −3, Python'da −4?** C "nolga tomon kesadi" (CPU'ning bo'lish buyrug'i shunday ishlaydi — tez).
Python "pastga yaxlitlaydi". Aylanma bufer indeksida `(i - 1) % n` manfiy chiqib qolishi mumkin; to'g'risi `(i + n - 1) % n`.

Nolga bo'lish (`x / 0`, `x % 0`) — UB; amalda dastur `SIGFPE` bilan qulaydi. Yadroda esa bu **#DE istisnosi** — CPU yadroning
uzilish ishlovchisini chaqiradi (MyOS: `kernel/arch/interrupts.c`). `-x` da `-INT_MIN` — toshish (UB).

> **Eslab qoling:** `int / int` = **butun**. Kasr kerak bo'lsa bittasini `double` qiling (`7 / 2.0`). `%` — qoldiq.

## 3.2. Taqqoslash

`==` (teng) `!=` (teng emas) `<` `>` `<=` `>=` — natija `int`: rost = **1**, yolg'on = **0**.

C'da alohida mantiqiy tur tarixan yo'q edi: **0 — yolg'on, noldan farqli har qanday son — rost**. `<stdbool.h>` dagi
`bool`, `true`, `false` — shunchaki 1 va 0.

```c
if (n)          /* n != 0 bilan bir xil */
if (!p)         /* p == NULL bilan bir xil (7-bob) */
```

**Bu dastur nima qiladi (umumiy):** taqqoslash amallari natija sifatida 0 yoki 1 qaytarishini va noldan farqli son shartda "rost" hisoblanishini ko'rsatadi.

```c
/* solishtirish.c - taqqoslash natijasi son */
#include <stdio.h>

int main(void)
{
    int a = 5, b = 3;
    printf("a == b: %d\n", a == b);       /* 0 - yolg'on */
    printf("a != b: %d\n", a != b);       /* 1 - rost */
    printf("a >  b: %d\n", a > b);        /* 1 */
    printf("a <= b: %d\n", a <= b);       /* 0 */

    if (a)                                /* a = 5, noldan farqli -> rost */
        printf("a rost deb hisoblandi\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra solishtirish.c -o solishtirish
$ ./solishtirish
a == b: 0
a != b: 1
a >  b: 1
a <= b: 0
a rost deb hisoblandi
```

### Eng mashhur xato: `=` va `==`

- `=` — **qiymat berish**: "`x` ga 0 yoz". 
- `==` — **taqqoslash**: "`x` nolga tengmi?" 

**Bu dastur nima qiladi (umumiy):** `=` (o'zlashtirish) va `==` (taqqoslash) ni almashtirish xatosini ko'rsatadi: shartning o'zi `x` ga 0 yozib yuboradi.

```c
/* tenglik_xato.c - = va == */
#include <stdio.h>

int main(void)
{
    int x = 5;
    if (x = 0)                           /* XATO: x ga 0 YOZADI, natija 0 -> shart yolg'on */
        printf("x nol\n");
    else
        printf("shart yolg'on, lekin x endi %d (buzildi!)\n", x);
    return 0;
}
```

```console
$ gcc -Wall -Wextra tenglik_xato.c -o tenglik_xato # xato kutiladi
tenglik_xato.c: In function ‘main’:
tenglik_xato.c:7:9: warning: suggest parentheses around assignment used as truth value [-Wparentheses]
    7 |     if (x = 0)                           /* XATO: x ga 0 YOZADI, natija 0 -> shart yolg'on */
      |         ^
$ ./tenglik_xato
shart yolg'on, lekin x endi 0 (buzildi!)
```

`-Wall` ogohlantirdi: "suggest parentheses around assignment used as truth value" ("berishni shart sifatida ishlatdingiz,
ataylab emasmi?"). Ataylab qilinganda ikkita qavs yoziladi: `while ((c = getchar()) != EOF)`.

> **Eslab qoling:** shartda **ikkita** `==`. Bitta `=` — qiymatni **buzadi**.

## 3.3. Mantiqiy operatorlar: `&&`, `||`, `!`

| Belgi | O'qilishi | Python'da | Ma'nosi |
|---|---|---|---|
| `a && b` | a **va** b | `and` | ikkalasi ham rost bo'lsa 1 |
| `a \|\| b` | a **yoki** b | `or` | hech bo'lmasa bittasi rost bo'lsa 1 |
| `!a` | a **emas** | `not` | rost ↔ yolg'on |

**Hayotdan misol: eshik va xona (qisqa tutashuv).** "Eshik ochiqmi **VA** ichkarida kimdir bormi?" Eshik qulflangan bo'lsa,
ichkariga qarab o'tirmaysiz — javob baribir "yo'q". C ham shunday: `&&` da chap tomon yolg'on bo'lsa, o'ng tomon
**umuman tekshirilmaydi**. `||` da chap tomon rost bo'lsa, o'ng tomon tekshirilmaydi. Bunga **qisqa tutashuv** (short-circuit) deyiladi.

```c
/* qisqa.c - qisqa tutashuv: o'ng tomon har doim ham tekshirilmaydi */
#include <stdio.h>

static int tekshir(const char *nom, int natija)
{
    printf("  tekshirildi: %s\n", nom);
    return natija;
}

int main(void)
{
    printf("1) eshik ochiq (yolg'on) VA ichkarida kimdir bor:\n");
    if (tekshir("eshik ochiq", 0) && tekshir("ichkarida kimdir bor", 1))
        printf("  -> kirdik\n");

    printf("2) eshik ochiq (rost) VA ichkarida kimdir bor:\n");
    if (tekshir("eshik ochiq", 1) && tekshir("ichkarida kimdir bor", 1))
        printf("  -> kirdik\n");

    printf("3) lampa yonib turibdi (rost) YOKI quyosh chiqqan:\n");
    if (tekshir("lampa yonib turibdi", 1) || tekshir("quyosh chiqqan", 0))
        printf("  -> yorug'\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra qisqa.c -o qisqa
$ ./qisqa
1) eshik ochiq (yolg'on) VA ichkarida kimdir bor:
  tekshirildi: eshik ochiq
2) eshik ochiq (rost) VA ichkarida kimdir bor:
  tekshirildi: eshik ochiq
  tekshirildi: ichkarida kimdir bor
  -> kirdik
3) lampa yonib turibdi (rost) YOKI quyosh chiqqan:
  tekshirildi: lampa yonib turibdi
  -> yorug'
```

**Kodda nimalar bor:**

- `tekshir(nom, natija)` — yordamchi funksiya: nomi chiqariladi (**"tekshirildi" so'zi chiqsa — shu shart haqiqatan hisoblandi**),
  va berilgan `natija` qaytariladi (0 yoki 1). Funksiyalar 5-bobda; hozir faqat "u o'zi nima qilganini ko'rsatadi" deb qabul qiling.
- 1-holat: chap tomon `0` (yolg'on) → "ikkinchi tekshirildi" **chiqmadi**. Chunki `0 && ...` har doim 0.
- 2-holat: chap tomon rost → ikkinchisi ham tekshirildi.
- 3-holat: `||` da chap tomon rost → o'ng tomon (`quyosh chiqqan`) **tekshirilmadi**.

**Nega bu muhim?** Bu — dasturni **qulashdan saqlaydi**:

```text
if (p != NULL && p->qiymat > 0)     /* p NULL bo'lsa, o'ng tomon HISOBLANMAYDI - xavfsiz */
if (i < n && a[i] == x)              /* chegarani avval tekshirish - massivdan chiqmaslik */
```

Tartibni almashtirsangiz (`a[i] == x && i < n`) — massiv chegarasidan tashqarini o'qiysiz. Yadroda bu uslub hamma joyda:
`if (!page || !page->mapping)`.

> **Eslab qoling:** `&&` va `||` **chapdan o'ngga** tekshiradi va natija aniq bo'lishi bilan to'xtaydi. **Xavfsiz tekshiruvni chapga yozing.**

## 3.4. Bitli operatorlar — yadroning tili

Bu bo'lim bobning eng muhim qismi. Sekin o'qing, har bir jadvalni qog'ozda o'zingiz takrorlang.

### 3.4.1. Bit nima, panel nima

**Hayotdan misol: devordagi chiroq kalitlari paneli.** Uyingizning devorida 8 ta kalit bor. Har biri bitta xonaning chirog'ini
boshqaradi: yoqilgan (**1**) yoki o'chirilgan (**0**). Panelning o'zi — bitta **son**: kalitlarning holati shu sonning bitlari.

```text
kalit raqami:      7    6    5    4    3    2    1    0        <- raqamlash O'NGDAN, 0 dan boshlanadi
xona:            ----  ----  ----  ----  hovli  yotoq  oshx  mehmon
holat (bit):      0    0    0    0    0    1    1    0         <- yotoqxona va oshxona yoniq
```

Bu panelning "qiymati" nechaga teng? Yoqilgan kalitlarning **og'irliklarini** qo'shamiz (kalit `n` ning og'irligi — 2ⁿ):

```text
og'irlik:    128   64   32   16    8    4    2    1
bit:           0    0    0    0    0    1    1    0     ->  4 + 2 = 6
```

**Kompyuterdagi panel** — bu **son o'zgaruvchisi**. Masalan `uint32_t panel = 0;` degani: "**32 ta kalitli panel**, boshida
**hammasi 0 (o'chiq)**". Nega `panel` boshida 0? Chunki 0 = `0000...0000` (32 ta nol) — **hamma kalit o'chiq**. Biror kalitni
yoqish uchun uning bitini `1` qilamiz.

```text
uint32_t panel = 0;       ->   bitlar: 0000 0000 0000 0000 0000 0000 0000 0000    (32 ta kalit, hammasi o'chiq)
                                                                       ^
                                                          eng o'ngdagi bit = 0-kalit
```

> **Eslab qoling:** o'zgaruvchi `uint32_t` = **32 ta kalit**. `uint8_t` = **8 ta kalit**. Bit raqamlari **o'ngdan**, **0 dan** boshlanadi.
> 0 qiymati = hamma kalit o'chiq.

### 3.4.2. Bitlarni ko'rish uchun asbob

Kalitlar holatini ko'z bilan ko'rish uchun kichik yordamchi funksiya yozamiz: `bitlar(nom, son)` sonning **pastki 8 bitini**
chap → o'ngga chiqaradi. (Ichida nima borligini 3.4.9 da to'liq tushuntiramiz; hozircha uni **qora quti** deb qabul qiling —
natijani ko'rish kifoya.) Panelning qolgan 24 ta kaliti ham bor, lekin ular hozir hammasi 0 — ko'rsatish shart emas.

```c
/* bitlar.h - bitlarni ko'rsatuvchi yordamchi asbob */
#pragma once
#include <stdio.h>

/* sonning pastki 8 bitini chiqaradi: nom, 8 ta bit, o'nlik qiymati */
static void bitlar(const char *nom, unsigned x)
{
    printf("%-16s", nom);
    for (int i = 7; i >= 0; i--)
        printf("%u", (x >> i) & 1u);
    printf("   (= %u)\n", x);
}
```

(Bu fayl `bitlar.h` — qolgan dasturlarimiz uni `#include "bitlar.h"` bilan ishlatadi, 1-bobdagi `.h` kabi.)

### 3.4.3. To'rt amal: `&`, `|`, `^`, `~` — haqiqat jadvallari

Bitli amallar **har bir bitga alohida**, o'z o'rnidagi boshqa son biti bilan ishlaydi. Avval **bitta bit** uchun qoidalar.

**`&` (VA) — "ikkalasi ham 1 bo'lsa, natija 1".** Hayotdan: ikki kishi birga bosishi kerak bo'lgan xavfsizlik tugmasi.

| a | b | a & b |
|---|---|---|
| 0 | 0 | 0 |
| 0 | 1 | 0 |
| 1 | 0 | 0 |
| 1 | 1 | **1** |

**`|` (YOKI) — "kamida bittasi 1 bo'lsa, natija 1".** Hayotdan: zalning ikki eshigidan istalgani orqali kirish mumkin.

| a | b | a \| b |
|---|---|---|
| 0 | 0 | 0 |
| 0 | 1 | **1** |
| 1 | 0 | **1** |
| 1 | 1 | **1** |

**`^` (XOR) — "faqat bittasi 1 bo'lsa, natija 1" (farq bo'lsa 1).** Hayotdan: koridordagi chiroq, ikki tomondagi kalitlardan
**har qaysi** bosilganda holat teskariga o'zgaradi.

| a | b | a ^ b |
|---|---|---|
| 0 | 0 | 0 |
| 0 | 1 | **1** |
| 1 | 0 | **1** |
| 1 | 1 | 0 |

**`~` (teskari) — har bitni aylantiradi: 0 → 1, 1 → 0.**

| a | ~a |
|---|---|
| 0 | 1 |
| 1 | 0 |

Endi **ikki butun son** uchun. Qoida: bitlar **ustun-ustun** juftlanadi (har bir ustun alohida hisoblanadi):

```text
        a =  1 1 0 0     (12)
        b =  1 0 1 0     (10)
            ---------
a & b   =    1 0 0 0     (8)    <- faqat 1-ustunda ikkalasi ham 1
a | b   =    1 1 1 0     (14)   <- 0-ustun (o'ngdagi) dan boshqa hammasida kamida bitta 1
a ^ b   =    0 1 1 0     (6)    <- faqat farq qilgan ustunlar 1
~a      =    0 0 1 1     (4 bitli ~12 = 3)
```

Haqiqiy dastur bilan tekshiramiz (8 bit):

**Bu dastur nima qiladi (umumiy):** ikki sonning bitlari ustida `&`, `|`, `^`, `~` amallarini bajarib, har natijani ikkilik ko'rinishda chiqaradi.

```c
/* bit_amallar.c - &, |, ^, ~ amalda */
#include <stdint.h>
#include "bitlar.h"

int main(void)
{
    uint8_t a = 12;                      /* 0000 1100 */
    uint8_t b = 10;                      /* 0000 1010 */

    bitlar("a", a);
    bitlar("b", b);
    bitlar("a & b", a & b);
    bitlar("a | b", a | b);
    bitlar("a ^ b", a ^ b);
    bitlar("~a", (uint8_t)~a);           /* (uint8_t) - natijani 8 bitga qisqartirish */
    return 0;
}
```

```console
$ gcc -Wall -Wextra bit_amallar.c -o bit_amallar
$ ./bit_amallar
a               00001100   (= 12)
b               00001010   (= 10)
a & b           00001000   (= 8)
a | b           00001110   (= 14)
a ^ b           00000110   (= 6)
~a              11110011   (= 243)
```

**Kodda nimalar bor:**

| Nom | Tur | Qiymat | Bitlar (8 ta) |
|---|---|---|---|
| `a` | `uint8_t` — 8 ta kalit | 12 | `00001100` |
| `b` | `uint8_t` | 10 | `00001010` |

`(uint8_t)~a` — `~a` hamma bitni aylantiradi, jumladan tepadagi (ko'rinmas) bitlarni ham: 8 bitdan keyingi bitlar ham 1 bo'lib
qoladi. Biz faqat pastki 8 bitni ko'rsatamiz, shuning uchun natijani `(uint8_t)` bilan 8 bitga qisqartirdik.

> **Eslab qoling:** `&` — "ikkalasida 1", `|` — "kamida birida 1", `^` — "farq qilsa 1", `~` — "hammasini teskari".

**Eng ko'p adashtiriladigan juftlik: `&` va `&&`.** `6 & 1` = 0 (bitlarda umumiy 1 yo'q: `110` va `001`), lekin `6 && 1` = 1
(ikkalasi ham noldan farqli). Birinchisi — **bitlar** bilan, ikkinchisi — "ikkalasi rostmi?" **savoli**. Adashtirish
ko'pincha kompilyatsiya bo'ladi va **jim** noto'g'ri ishlaydi.

### 3.4.4. Surish: `<<` va `>>`

**Hayotdan misol: nol qo'shish.** O'nlik sanoqda songa oxiridan nol qo'shsangiz, 10 marta oshadi (5 → 50). Ikkilikda
bitlarni chapga **bir o'ringa surish** — **2 marta** oshirish: `0011` (3) → `0110` (6). `<<` ning o'zi shu: bitlarni chapga suradi,
o'ngdan nol bilan to'ldiradi.

- `a << n` — bitlarni **chapga** `n` ta suradi. Natija: `a × 2ⁿ`.
- `a >> n` — bitlarni **o'ngga** `n` ta suradi (o'ng chetdagilar tushib ketadi). Natija: `a ÷ 2ⁿ` (butun).

```text
0000 0011 (3)  << 2  ->  0000 1100 (12)      3 * 4 = 12
0000 1100 (12) >> 2  ->  0000 0011 (3)       12 / 4 = 3
```

**Bu dastur nima qiladi (umumiy):** `<<` va `>>` surish amallarini ko'rsatadi: chapga surish ikkiga ko'paytiradi, o'ngga surish ikkiga bo'ladi.

```c
/* surish.c - << va >> */
#include <stdint.h>
#include "bitlar.h"

int main(void)
{
    uint8_t a = 3;
    bitlar("a = 3", a);
    bitlar("a << 1", a << 1);
    bitlar("a << 2", a << 2);
    bitlar("a << 3", a << 3);
    bitlar("12 >> 2", 12 >> 2);
    return 0;
}
```

```console
$ gcc -Wall -Wextra surish.c -o surish
$ ./surish
a = 3           00000011   (= 3)
a << 1          00000110   (= 6)
a << 2          00001100   (= 12)
a << 3          00011000   (= 24)
12 >> 2         00000011   (= 3)
```

Har surishda "1" chapga bir qadam yurdi va qiymat ikki baravar oshdi: 3 → 6 → 12 → 24.

> **Eslab qoling:** `<< n` — ×2ⁿ, `>> n` — ÷2ⁿ. Bitlar chapga/o'ngga yuradi.

### 3.4.5. Niqob (mask): bitta kalitni belgilash

Endi eng muhim g'oya. Panelda **bitta** kalitni (masalan, 3-kalitni) boshqarmoqchimiz. Buning uchun **faqat shu kalit 1**, qolgani
0 bo'lgan son yasaymiz — bunday songa **niqob (mask)** deyiladi.

**Hayotdan misol: trafaret.** Devorni bo'yayotganda faqat kerakli joy ochiq qoladigan trafaret qo'yasiz: bo'yoq faqat ochiq
joyga tushadi. Niqob ham shunday: `1` bo'lgan bit — "ochiq teshik" (shu kalitga tegamiz), `0` bo'lganlar — yopiq (tegmaymiz).

Niqobni qanday yasaymiz? `1` ni kerakli o'ringa suramiz:

```text
1u << 0  =  0000 0001      (0-kalit)
1u << 1  =  0000 0010      (1-kalit)
1u << 2  =  0000 0100      (2-kalit)
1u << 3  =  0000 1000      (3-kalit)
```

Ya'ni **`1u << n` = "faqat `n`-kalit yoqilgan son"**.

**Bu dastur nima qiladi (umumiy):** `1u << n` ifodasi (n = 0…7) bitta bitli niqoblarni (maska) hosil qilishini ikkilik ko'rinishda chiqaradi.

```c
/* maska.c - 1u << n: bitta kalitlik niqoblar */
#include "bitlar.h"

int main(void)
{
    for (int n = 0; n <= 7; n++) {
        char nom[16];
        snprintf(nom, sizeof(nom), "1u << %d", n);
        bitlar(nom, 1u << n);
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra maska.c -o maska
$ ./maska
1u << 0         00000001   (= 1)
1u << 1         00000010   (= 2)
1u << 2         00000100   (= 4)
1u << 3         00001000   (= 8)
1u << 4         00010000   (= 16)
1u << 5         00100000   (= 32)
1u << 6         01000000   (= 64)
1u << 7         10000000   (= 128)
```

(`snprintf` — "`nom` qutisiga matn yoz" degani; bu yerda faqat yorliq yasash uchun, 6-bobda tushuntiriladi.)

**Nega `1` emas, `1u`?** Oddiy `1` — `int` (ishorali). Uni 31-o'ringa surish ishora bitiga tegadi → **UB**. `1u` esa `unsigned`:
ishora biti yo'q, hamma 32 bit toza kalit. 64 bit kerak bo'lsa — `1ull << 40` (`ull` — unsigned long long). **Qoida:** bitlar bilan
**faqat ishorasiz** turlarda ishlang.

**Nega `#define` bilan nom beramiz?** `1u << 3` o'rniga **nom** yozish o'qishni osonlashtiradi:

```c
#define HOVLI (1u << 3)        /* "HOVLI" so'zini ko'rgan joyda (1u << 3) deb o'qi: 0000 1000 */
```

`#define NOM qiymat` — "kodda `NOM` ni ko'rsang, `qiymat` bilan almashtir" (preprotsessor, 10-bob). `HOVLI` — 3-kalitning **nomi**; uning
**qiymati** — `0000 1000` (8). Qavslar `(1u << 3)` ichida bo'lishi shart: aks holda ifodada ustuvorlik buziladi (3.9).

### 3.4.6. To'rt "hunar": yoqish, o'chirish, almashtirish, tekshirish

Endi panel (`x`) va niqob (`M`, bitta kalitga) bor. Kerakli amalni **haqiqat jadvalidan chiqarib olamiz** — yodlash shart emas, mantiq bor.

#### 1) Kalitni YOQISH: `x |= M;`

Kerak: `M` dagi kalit **1** bo'lsin; **qolganlariga tegmaylik**. `|` (YOKI) jadvaliga qarang:

- `biror_bit | 0 = o'sha_bit` → 0 bilan YOKI qilsak, bit **o'zgarmaydi** ✓ ("tegmaslik").
- `biror_bit | 1 = 1` → 1 bilan YOKI qilsak, bit **1 bo'ladi** ✓ ("yoqish").

`M` da kerakli kalit 1, qolganlari 0 → qolganlari o'zgarmaydi, kerakli kalit yonadi. **Shuning uchun yoqish — `|`.**

```text
x          =  0 1 0 0 0 1 1 0
M (3-kalit) =  0 0 0 0 1 0 0 0
x | M      =  0 1 0 0 1 1 1 0      <- faqat 3-kalit o'zgardi (yondi)
```

`x |= M;` — bu `x = x | M;` ning qisqa yozuvi ("`x` ga `x | M` ni yoz").

#### 2) Kalitni O'CHIRISH: `x &= ~M;`

Kerak: `M` dagi kalit **0** bo'lsin; qolganlariga tegmaylik. `&` (VA) jadvalida:

- `biror_bit & 1 = o'sha_bit` → 1 bilan VA qilsak, bit **o'zgarmaydi** ✓.
- `biror_bit & 0 = 0` → 0 bilan VA qilsak, bit **0 bo'ladi** ✓ ("o'chirish").

Demak, kerakli kalitda **0**, qolganlarida **1** bo'lgan niqob kerak. Bu aynan `M` ning **teskarisi**: `~M`! 

```text
M          =  0 0 0 0 1 0 0 0      (3-kalit)
~M         =  1 1 1 1 0 1 1 1      (teskari: 3-kalit 0, qolgan hammasi 1)
x          =  0 1 0 0 1 1 1 0
x & ~M     =  0 1 0 0 0 1 1 0      <- faqat 3-kalit o'chdi
```

**Shuning uchun o'chirish — `& ~`** ("VA teskari-niqob"). `x &= ~M;` = `x = x & (~M);`.

#### 3) Kalitni ALMASHTIRISH: `x ^= M;`

Kerak: kalit yoqiq bo'lsa o'chsin, o'chiq bo'lsa yonsin; qolganlari o'z holicha. `^` (XOR) jadvalida:

- `biror_bit ^ 0 = o'sha_bit` → 0 bilan XOR — **o'zgarmaydi** ✓.
- `biror_bit ^ 1 = teskarisi` (0^1=1, 1^1=0) → 1 bilan XOR — **teskari bo'ladi** ✓.

```text
x          =  0 1 0 0 1 1 1 0
M (3-kalit) =  0 0 0 0 1 0 0 0
x ^ M      =  0 1 0 0 0 1 1 0      <- 3-kalit yoniq edi -> o'chdi
x ^ M ^ M  =  0 1 0 0 1 1 1 0      <- yana bir marta: qaytdi
```

Hayotdan: koridordagi tugma — har bosishda holat teskariga. Ikki marta bossang, asliga qaytadi.

#### 4) Kalitni TEKSHIRISH: `x & M`

Kerak: kalit yoniqmi? `M` bilan VA qilsak, **faqat shu kalit** qoladi, qolganlari 0 bo'ladi:

```text
x          =  0 1 0 0 1 1 1 0
M (3-kalit) =  0 0 0 0 1 0 0 0
x & M      =  0 0 0 0 1 0 0 0      <- 3-kalit yoniq: natija 0 EMAS (rost)

x          =  0 1 0 0 0 1 1 0      <- 3-kalit o'chiq bo'lsa
x & M      =  0 0 0 0 0 0 0 0      <- natija 0 (yolg'on)
```

Natija **0 emas** → kalit yoniq; **0** → o'chiq. Shu uchun `if (x & M)` — "M kaliti yoniqmi?". (C'da 0 — yolg'on, boshqa hamma son — rost.)

> **Eslab qoling (4 hunar):**
> | Nima qilish | Yozuv | Nega |
> |---|---|---|
> | yoqish | `x \|= M;` | YOKI: `0` tegmaydi, `1` yoqadi |
> | o'chirish | `x &= ~M;` | VA: `1` tegmaydi, `0` o'chiradi; shuning uchun niqobni teskari qilamiz |
> | almashtirish | `x ^= M;` | XOR: `1` teskari qiladi |
> | tekshirish | `x & M` | VA: faqat shu kalit qoladi; 0 emas → yoniq |

Panelda to'rt hunarni **bosqichma-bosqich** ko'ramiz:

**Bu dastur nima qiladi (umumiy):** chiroq kalitlari panelida bitlarni yoqish, o'chirish va almashtirishni bosqichma-bosqich bajarib, har qadamda panel bitlarini ko'rsatadi.

```c
/* panel_bosqich.c - to'rt hunar, har qadamda bitlar */
#include <stdint.h>
#include <stdio.h>
#include "bitlar.h"

#define MEHMONXONA (1u << 0)       /* 0-kalit: 0000 0001 */
#define OSHXONA    (1u << 1)       /* 1-kalit: 0000 0010 */
#define YOTOQXONA  (1u << 2)       /* 2-kalit: 0000 0100 */
#define HOVLI      (1u << 3)       /* 3-kalit: 0000 1000 */

int main(void)
{
    uint32_t panel = 0;                         /* 32 ta kalit, HAMMASI O'CHIQ */
    bitlar("boshida", panel);

    panel |= OSHXONA;                           /* oshxonani YOQISH */
    bitlar("oshxona yoqildi", panel);

    panel |= MEHMONXONA | HOVLI;                /* ikkitasini birga yoqish */
    bitlar("+mehmon, hovli", panel);

    panel &= ~OSHXONA;                          /* oshxonani O'CHIRISH */
    bitlar("oshxona o'chdi", panel);

    panel ^= YOTOQXONA;                         /* yotoqxona: ALMASHTIRISH (o'chiq edi -> yondi) */
    bitlar("yotoq almashdi", panel);

    panel ^= YOTOQXONA;                         /* yana almashtirish -> o'chdi */
    bitlar("yotoq yana", panel);

    printf("hovli yoniqmi? %s\n", (panel & HOVLI) ? "ha" : "yo'q");
    printf("oshxona yoniqmi? %s\n", (panel & OSHXONA) ? "ha" : "yo'q");
    return 0;
}
```

```console
$ gcc -Wall -Wextra panel_bosqich.c -o panel_bosqich
$ ./panel_bosqich
boshida         00000000   (= 0)
oshxona yoqildi 00000010   (= 2)
+mehmon, hovli  00001011   (= 11)
oshxona o'chdi  00001001   (= 9)
yotoq almashdi  00001101   (= 13)
yotoq yana      00001001   (= 9)
hovli yoniqmi? ha
oshxona yoniqmi? yo'q
```

**Kodda nimalar bor (tepadan pastga o'qing):**

| Qator | Nima qiladi | Nega kerak |
|---|---|---|
| `#define MEHMONXONA (1u << 0)` ... | 4 ta kalitga **nom** beradi. Har biri — shu kalit yoqilgan son (niqob) | `panel |= 2` o'rniga `panel |= OSHXONA` — kimga o'qish oson |
| `uint32_t panel = 0;` | `panel` — **32 ta kalitli** quti; `0` = **hammasi o'chiq** | Boshlanish holati aniq bo'lsin (2.2: qiymatsiz qoldirmang!) |
| `bitlar("boshida", panel);` | panelni ko'rsatadi (pastki 8 kalit) | Natijani ko'z bilan ko'rish |
| `panel \|= OSHXONA;` | 1-kalitni yoqadi, qolganiga tegmaydi | YOKI qoidasi |
| `panel \|= MEHMONXONA \| HOVLI;` | avval `MEHMONXONA \| HOVLI` = `0000 1001` niqobi **yasaladi** (ikki bit yoniq), keyin panelga YOKI | Bir amalda bir necha kalitni yoqish |
| `panel &= ~OSHXONA;` | `~OSHXONA` = `...1111 1101`; VA bilan 1-kalit o'chadi | VA + teskari niqob |
| `panel ^= YOTOQXONA;` | 2-kalit teskari bo'ladi | XOR |
| `(panel & HOVLI) ? "ha" : "yo'q"` | `panel & HOVLI` nolmi? → matn tanlanadi | tekshirish; `? :` — 3.7 |

**Qiymatlar qanday o'zgaradi (pastki 4 bit; qolgan 28 kalit hammasi 0 bo'lib turadi):**

| Qadam | Amal | bitlar `hovli yotoq oshx mehmon` | Son |
|---|---|---|---|
| boshida | `panel = 0` | `0 0 0 0` | 0 |
| 1 | `\|= OSHXONA` | `0 0 1 0` | 2 |
| 2 | `\|= MEHMONXONA \| HOVLI` | `1 0 1 1` | 11 |
| 3 | `&= ~OSHXONA` | `1 0 0 1` | 9 |
| 4 | `^= YOTOQXONA` | `1 1 0 1` | 13 |
| 5 | `^= YOTOQXONA` | `1 0 0 1` | 9 |

Bu jadvalni dastur chiqarishi bilan solishtiring: bitlar bir xil.

**Tez-tez xato:**
1. `panel |= ~OSHXONA;` — o'chirish uchun `|=` yozish (u hamma kalitni yoqib yuboradi). O'chirish — `&= ~`.
2. `if (panel & OSHXONA == 0)` — ustuvorlik tuzog'i (3.9). To'g'risi `(panel & OSHXONA) == 0`.
3. `panel = OSHXONA;` — **berish**: boshqa kalitlar ham o'chib ketadi. Faqat bittasini yoqish uchun — `|=`.

### 3.4.7. To'liq dastur: aqlli uy

Endi hammasini birlashtiramiz: panel + nomlar + chiroyli chiqarish funksiyasi.

**Bu dastur nima qiladi (umumiy):** bitta son (`panel`) ni uydagi to'rt xonaning chiroq kalitlari sifatida ishlatib, kun davomida ularni yoqib-o'chirishni xona nomlari bilan ko'rsatadi.

```c
/* aqlli_uy.c - bitta son = chiroq kalitlari paneli */
#include <stdint.h>
#include <stdio.h>

#define MEHMONXONA (1u << 0)
#define OSHXONA    (1u << 1)
#define YOTOQXONA  (1u << 2)
#define HOVLI      (1u << 3)

static void korsat(const char *izoh, uint32_t panel)
{
    printf("%-26s", izoh);
    printf(" mehmonxona:%s", (panel & MEHMONXONA) ? "YONIQ" : "-");
    printf(" oshxona:%s", (panel & OSHXONA) ? "YONIQ" : "-");
    printf(" yotoqxona:%s", (panel & YOTOQXONA) ? "YONIQ" : "-");
    printf(" hovli:%s\n", (panel & HOVLI) ? "YONIQ" : "-");
}

int main(void)
{
    uint32_t panel = 0;                              /* hammasi o'chiq */
    korsat("Boshida:", panel);

    panel |= OSHXONA | MEHMONXONA;                   /* ikkitasini yoqish */
    korsat("Kechqurun:", panel);

    panel &= ~OSHXONA;                               /* oshxonani o'chirish */
    panel |= YOTOQXONA;
    korsat("Yotishdan oldin:", panel);

    panel ^= HOVLI;                                  /* hovli: almashtirish */
    korsat("Hovli tugmasi bosildi:", panel);

    int soat = 22;
    printf("\nSoat %d:00 dan 5 soat keyin: %d:00\n", soat, (soat + 5) % 24);
    printf("7 olma, 2 bola: har biriga %d, ortib qoldi %d\n", 7 / 2, 7 % 2);
    printf("Yoniq chiroqlar soni: %d\n", __builtin_popcount(panel));
    return 0;
}
```

```console
$ gcc -Wall -Wextra aqlli_uy.c -o aqlli_uy
$ ./aqlli_uy
Boshida:                   mehmonxona:- oshxona:- yotoqxona:- hovli:-
Kechqurun:                 mehmonxona:YONIQ oshxona:YONIQ yotoqxona:- hovli:-
Yotishdan oldin:           mehmonxona:YONIQ oshxona:- yotoqxona:YONIQ hovli:-
Hovli tugmasi bosildi:     mehmonxona:YONIQ oshxona:- yotoqxona:YONIQ hovli:YONIQ

Soat 22:00 dan 5 soat keyin: 3:00
7 olma, 2 bola: har biriga 3, ortib qoldi 1
Yoniq chiroqlar soni: 3
```

**Kodda nimalar bor:**

- `korsat(izoh, panel)` — funksiya: panelning 4 ta kalitini **o'qib**, har biri uchun "YONIQ" yoki "-" chiqaradi. U panelni **o'zgartirmaydi**,
  faqat ko'rsatadi (nusxa oladi). Har kalit uchun `(panel & NOM) ? "YONIQ" : "-"`: `&` kalitni **tekshiradi**; natija 0 bo'lmasa — `"YONIQ"`.
- `%-26s` — matnni **26 belgi kenglikda, chapga tekislab** chiqar (jadval tekis chiqishi uchun).
- `__builtin_popcount(panel)` — GCC'ning tayyor funksiyasi: panelda **nechta bit 1** ekanini sanaydi (yoniq chiroqlar soni).
- `static` — "bu funksiya faqat shu faylga tegishli" (5-bob); `const char *izoh` — "o'zgartirilmaydigan matn" (6-bob).

Natijaga qarang: har satrda aynan qaysi kalit o'zgarganini jadval bilan solishtiring (`|=` yoqdi, `&= ~` o'chirdi, `^=` almashtirdi).

**Sinab ko'ring:** `#define GARAJ (1u << 4)` qo'shib, garajni yoqing va `korsat` ga garajni qo'shing. `panel ^= HOVLI;` ni ikki marta yozing — nima bo'ladi?

### 3.4.8. Yana uchta foydali bit hunari

**Bu dastur nima qiladi (umumiy):** uchta amaliy bit usulini ko'rsatadi: n-bitni o'qish, eng pastki 1 bitni o'chirish (`x & (x - 1)`) va manzilni sahifa (4096) chegarasiga tekislash.

```c
/* bit_hunar.c - bitni o'qish, hisoblash, tekislash */
#include <stdint.h>
#include <stdio.h>
#include "bitlar.h"

int main(void)
{
    uint32_t x = 0x4E;                                     /* 0100 1110 */
    bitlar("x = 0x4E", x);

    /* 1) n-bitni O'QISH: avval pastga suramiz, keyin faqat 0-bitni qoldiramiz */
    for (int n = 3; n >= 0; n--)
        printf("x ning %d-biti = %u\n", n, (x >> n) & 1u);

    /* 2) eng pastki 1 bitni o'chirish: x & (x - 1) */
    bitlar("x - 1", x - 1);
    bitlar("x & (x - 1)", x & (x - 1));

    /* 3) 4096 ga tekislash: manzilni sahifaga yaxlitlash */
    uint64_t manzil = 0x12345678;
    printf("manzil       = 0x%lx\n", (unsigned long)manzil);
    printf("sahifa boshi = 0x%lx\n", (unsigned long)(manzil & ~0xFFFull));
    printf("sahifa ichida= 0x%lx\n", (unsigned long)(manzil & 0xFFFull));
    return 0;
}
```

```console
$ gcc -Wall -Wextra bit_hunar.c -o bit_hunar
$ ./bit_hunar
x = 0x4E        01001110   (= 78)
x ning 3-biti = 1
x ning 2-biti = 1
x ning 1-biti = 1
x ning 0-biti = 0
x - 1           01001101   (= 77)
x & (x - 1)     01001100   (= 76)
manzil       = 0x12345678
sahifa boshi = 0x12345000
sahifa ichida= 0x678
```

**Qadam-baqadam:**

**1) n-bitni o'qish: `(x >> n) & 1u`.** Avval `x` ni `n` o'ringa **o'ngga suramiz** — kerakli bit **eng o'ng** (0-o'rin)ga keladi. Keyin `& 1u`
(niqob `0000 0001`) — faqat shu bitni qoldiradi: natija 0 yoki 1.

```text
x = 0100 1110, 3-bitni o'qiymiz:
x >> 3     = 0000 1001      (hamma bit 3 qadam o'ngga)
& 0000 0001 =  0000 0001      -> 1   (3-bit yoniq)
```

**2) `x & (x - 1)` — eng pastki 1-bitni o'chiradi.** `x - 1` — eng pastki `1` bit nolga aylanadi va undan pastdagi hamma nollar birga aylanadi:

```text
x      = 0100 1110            (eng pastki 1 - 1-bitda)
x - 1  = 0100 1101            (1-bit -> 0, pastidagi 0-bit -> 1)
x & (x-1) = 0100 1100         (eng pastki 1 yo'qoldi)
```

Bu hunar bitlarni **sanash** uchun ishlatiladi (necha marta takrorlansa, shuncha bit bor).

**3) Tekislash: `manzil & ~0xFFF`.** `0xFFF` = 12 ta bit 1 (`1111 1111 1111`). `~0xFFF` = pastki 12 bit **0**, yuqorisi 1. `&` bilan manzilning
pastki 12 biti **tozalanadi** → natija **4096 ga karrali** (4096 = 2¹²). Bu — **sahifa boshi**. `manzil & 0xFFF` — aksincha: faqat pastki 12 bit
(sahifa ichidagi o'rni). Yadroda xotira shunday sahifalarga bo'linadi (24-bob).

Yadroda qanday ko'rinadi (MyOS, `kernel/mm/vmm.h` — sahifa jadvali yozuvi bitlari):

```c
#define PTE_PRESENT  (1UL << 0)     /* sahifa xotirada bor */
#define PTE_WRITABLE (1UL << 1)     /* yozish mumkin */
#define PTE_USER     (1UL << 2)     /* user rejimi kira oladi */
#define PTE_NX       (1UL << 63)    /* kod sifatida bajarib bo'lmaydi */

uint64_t pte = fizik_manzil | PTE_PRESENT | PTE_WRITABLE;  /* yozuv yasash */
if (!(pte & PTE_PRESENT))  { /* sahifa yo'q -> page fault */ }
pte &= ~PTE_WRITABLE;                                         /* faqat o'qish (fork + COW!) */
uint64_t sahifa_boshi = manzil & ~0xFFFull;                  /* pastki 12 bitni tozalash */
```

Bu — aynan aqlli uy paneli: `pte` — 64 kalitli panel; `PTE_PRESENT` — "sahifa bor" kaliti; `|` yoqadi, `& ~` o'chiradi, `&` tekshiradi.

### 3.4.9. Endi `bitlar` funksiyasining ichi

Boshida "qora quti" degandik. Endi hamma qismini bilamiz:

```c
static void bitlar(const char *nom, unsigned x)
{
    printf("%-16s", nom);                          /* nomni 16 belgi kenglikda chiqar */
    for (int i = 7; i >= 0; i--)                   /* i = 7, 6, 5 ... 0 (chapdan o'ngga) */
        printf("%u", (x >> i) & 1u);               /* i-bitni o'qish (3.4.8 dagi 1-hunar) */
    printf("   (= %u)\n", x);                      /* oxirida o'nlik qiymat */
}
```

U 7-bitdan 0-bitgacha **har birini o'qiydi** (`(x >> i) & 1u`) va ketma-ket chiqaradi. Ko'rdingizmi — yangi hech narsa yo'q: surish + VA.

### 3.4.10. Surish tuzoqlari va qoidalar

- `1 << 31` — `1` bu `int`, 31-bitga surish ishora bitiga tegadi → **UB**. Yozing: `1u << 31`.
- `1u << 32` — surish miqdori tur kengligidan katta yoki teng (32 bitli turda) → **UB**. 64 bit uchun: `1ull << 40`.
- Manfiy sonni `>>` qilish — natija platformaga bog'liq (GCC'da ishora saqlanadi).

**Bu dastur nima qiladi (umumiy):** `int` ning ishora bitiga surish va 32 bitli turni 32 ga surish (aniqlanmagan xatti-harakat) — ataylab xatoli.

```c
/* surish_ub.c - noto'g'ri surish */
#include <stdio.h>

int main(int argc, char **argv)
{
    (void)argv;
    int n = 30 + argc;                  /* n = 31 */
    int a = 1 << n;                     /* XATO: int ning ishora bitiga tegdi */
    printf("1 << 31 = %d\n", a);

    int m = 31 + argc;                  /* m = 32 */
    unsigned b = 1u << m;               /* XATO: 32 bitli turni 32 ga surish */
    printf("1u << 32 = %u\n", b);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=undefined surish_ub.c -o surish_ub
$ ./surish_ub 2>&1
surish_ub.c:8:15: runtime error: left shift of 1 by 31 places cannot be represented in type 'int'
surish_ub.c:12:21: runtime error: shift exponent 32 is too large for 32-bit type 'unsigned int'
1 << 31 = -2147483648
1u << 32 = 1
```

UBSan ikkala xatoni ham topdi. Oddiy yig'ishda "ishlab ketardi", lekin natijaga ishonib bo'lmaydi.

> **Eslab qoling:** bitlar bilan **faqat ishorasiz** turlarda ishlang (`1u`, `uint32_t`). Surish miqdori turning bit sonidan **kichik** bo'lsin. (04-mashq.)

> **Bit amallari bo'yicha qisqa xulosa:**
> - `1u << n` — niqob (`n`-kalit). `x |= M` — yoqish. `x &= ~M` — o'chirish. `x ^= M` — almashtirish. `x & M` — tekshirish.
> - `(x >> n) & 1u` — `n`-bitni o'qish. `x & ~0xFFF` — 4096 ga tekislash.
> - Nega bunday: har amal haqiqat jadvalidan chiqadi — `0` va `1` bilan maxsus amallar "tegmaslik" yoki "majburlash" beradi.

## 3.5. Qiymat berish operatorlari

```c
x = 5;
x += 3;     /* x = x + 3 */
x -= 1;  x *= 2;  x /= 4;  x %= 3;
x |= FLAG;  x &= ~FLAG;  x ^= m;  x <<= 1;  x >>= 2;
```

`x op= y` — **`x = x op y`** ning qisqa yozuvi. `=` ham **ifoda** — uning qiymati berilgan qiymat: `a = b = 0;`
(o'ngdan chapga: `b = 0`, keyin `a = 0`).

## 3.6. `++` va `--`

`i++` — "`i` ga 1 qo'sh". Ikki shakl bor, farqi — **qiymat qachon olinishi**:

**Bu dastur nima qiladi (umumiy):** `i++` va `++i` farqini (qiymat qachon olinishini) va `buf[n++] = x` naqshini ko'rsatadi.

```c
/* inkrement.c - i++ va ++i */
#include <stdio.h>

int main(void)
{
    int i = 5;
    int a = i++;                 /* avval a = eski i (5), KEYIN i = 6 */
    printf("a = %d, i = %d\n", a, i);

    int b = ++i;                 /* AVVAL i = 7, KEYIN b = yangi i (7) */
    printf("b = %d, i = %d\n", b, i);

    int buf[4] = {0, 0, 0, 0};
    int n = 0;
    buf[n++] = 10;               /* 10 ni buf[0] ga yoz, keyin n = 1 */
    buf[n++] = 20;               /* 20 ni buf[1] ga yoz, keyin n = 2 */
    printf("buf: %d %d %d %d, n = %d\n", buf[0], buf[1], buf[2], buf[3], n);
    return 0;
}
```

```console
$ gcc -Wall -Wextra inkrement.c -o inkrement
$ ./inkrement
a = 5, i = 6
b = 7, i = 7
buf: 10 20 0 0, n = 2
```

**Qadamlar:**

| Qator | `i` oldin | `i++` / `++i` | natija | `i` keyin |
|---|---|---|---|---|
| `a = i++` | 5 | **post**-increment: avval qiymat olinadi | `a = 5` | 6 |
| `b = ++i` | 6 | **pre**-increment: avval oshiriladi | `b = 7` | 7 |

`buf[n++] = c;` — eng ko'p ishlatiladigan idioma: "`c` ni `n`-o'ringa yoz, keyin `n` ni oshir" (bufferga ketma-ket yozish).
`*p++ = c;` — "`p` ko'rsatgan joyga yoz, keyin `p` ni sur" (7-bob).

**Taqiqlangan:** bitta ifodada bir o'zgaruvchini ikki marta o'zgartirish — `i = i++;`, `a[i] = i++;`, `f(i++, i++)` — UB (tartib aniqlanmagan).
`-Wall` ogohlantiradi (`operation on 'i' may be undefined`).

## 3.7. Ternar operator `?:`

**Hayotdan misol: svetofor.** "Yashil bo'lsa — yur, aks holda — tur." Bitta savol, ikki javobdan biri.

```c
/* ternar.c - ?: operatori */
#include <stdio.h>

int main(void)
{
    int a = 7, b = 12;
    int max = a > b ? a : b;                         /* shart ? rost_bo'lsa : yolg'on_bo'lsa */
    printf("katta son: %d\n", max);

    for (int n = 1; n <= 3; n += 2)
        printf("%d ta fayl%s\n", n, n == 1 ? "" : "lar");
    return 0;
}
```

```console
$ gcc -Wall -Wextra ternar.c -o ternar
$ ./ternar
katta son: 12
1 ta fayl
3 ta fayllar
```

`shart ? A : B` — shart rost bo'lsa **A**, yolg'on bo'lsa **B** qiymati. Bu qiymat qaytaradigan qisqa `if`. Faqat oddiy hollarda ishlating.

## 3.8. Vergul operatori

```c
for (i = 0, j = n - 1; i < j; i++, j--)    /* ikki o'zgaruvchi birga */
```

Chapdan o'ngga bajariladi, qiymati — eng o'ngdagi. **Faqat `for` ichida ishlating** (boshqa joyda chalkash).

## 3.9. Ustuvorlik (precedence) — tuzoqlar

**Hayotdan misol: matematikadagi amallar tartibi.** `2 + 3 * 4` = 14, 20 emas — maktabda o'rgangansiz. C'da ham shunday, lekin
`&`, `|`, `==` ning tartibi **kutilmagan**: `x & 1 == 0` aslida `x & (1 == 0)`.

Hamma ustuvorlik jadvalini yodlash shart emas. **Shubha bo'lsa — qavs qo'ying.** Lekin quyidagi tuzoqlarni biling:

| Yozuv | Kutganingiz | Aslida | Nega |
|---|---|---|---|
| `x & 1 == 0` | `(x & 1) == 0` | `x & (1 == 0)` → `x & 0` → 0 | `==` bitli `&` dan kuchliroq! |
| `a << 2 + 1` | `(a << 2) + 1` | `a << 3` | `+` surishdan kuchliroq |
| `*p++` | `(*p)++` | `*(p++)` | postfiks `++` `*` dan kuchliroq |

**Bu dastur nima qiladi (umumiy):** qavssiz yozilgan ifodalar kutilmagan natija berishini ko'rsatadi (`x & 1 == 0`, `3 << 2 + 1`) va qavsli to'g'ri variantni.

```c
/* ustuvorlik.c - qavssiz yozish tuzoqlari */
#include <stdio.h>

int main(void)
{
    int x = 6;                                   /* 0110 - juft son */
    printf("x & 1 == 0     -> %d   (kutilgan 1 emas!)\n", x & 1 == 0);
    printf("(x & 1) == 0   -> %d   (to'g'ri: x juft)\n", (x & 1) == 0);

    printf("3 << 2 + 1     -> %d\n", 3 << 2 + 1);
    printf("(3 << 2) + 1   -> %d\n", (3 << 2) + 1);
    return 0;
}
```

```console
$ gcc -Wall -Wextra ustuvorlik.c -o ustuvorlik # xato kutiladi
ustuvorlik.c: In function ‘main’:
ustuvorlik.c:7:61: warning: suggest parentheses around comparison in operand of ‘&’ [-Wparentheses]
    7 |     printf("x & 1 == 0     -> %d   (kutilgan 1 emas!)\n", x & 1 == 0);
      |                                                             ^
ustuvorlik.c:10:40: warning: suggest parentheses around ‘+’ inside ‘<<’ [-Wparentheses]
   10 |     printf("3 << 2 + 1     -> %d\n", 3 << 2 + 1);
      |                                        ^~
$ ./ustuvorlik
x & 1 == 0     -> 0   (kutilgan 1 emas!)
(x & 1) == 0   -> 1   (to'g'ri: x juft)
3 << 2 + 1     -> 24
(3 << 2) + 1   -> 13
```

Kompilyator ikkala joyda ham ogohlantirdi (`suggest parentheses`). Natijalarni solishtiring: qavssiz va qavsli versiyalar **turli** javob beradi.

> **Eslab qoling:** bitli amal bilan taqqoslash aralashsa — **doim qavs**: `if ((flags & MASK) == VALUE)`.

## 3.10. `sizeof` va cast — operatorlar ham

`sizeof(tur)`, `sizeof ifoda` — hajm (kompilyatsiya paytida). `(tur)ifoda` — aylantirish. Ikkalasi ham unary operator va ustuvorligi
yuqori: `(long)a * b` — avval `a` aylantiriladi, keyin ko'paytiriladi (aynan shu kerak edi — 01-mashq).

## Hayotdan misol va to'liq dastur

Chmod (Linux fayl ruxsatlari) ham chiroq paneli: 9 ta kalit (`r w x` × 3 guruh). **Loyiha** bo'limida shuni bitlar bilan qilasiz.
Bu yerda esa — **xavfsizlik signalizatsiyasi**: 4 ta datchik (eshik, oyna, harakat, tutun), har biri — bitta bit.

```c
/* signal.c - datchiklar holati bitta sonda */
#include <stdint.h>
#include <stdio.h>

#define ESHIK   (1u << 0)
#define OYNA    (1u << 1)
#define HARAKAT (1u << 2)
#define TUTUN   (1u << 3)

static void holat(const char *izoh, uint8_t datchiklar)
{
    printf("%-22s eshik:%d oyna:%d harakat:%d tutun:%d\n", izoh,
           (datchiklar & ESHIK) != 0, (datchiklar & OYNA) != 0,
           (datchiklar & HARAKAT) != 0, (datchiklar & TUTUN) != 0);
}

int main(void)
{
    uint8_t d = 0;                               /* hech narsa ishga tushmagan */
    holat("tinch:", d);

    d |= ESHIK | HARAKAT;                        /* eshik ochildi va harakat sezildi */
    holat("o'g'ri kirdi:", d);

    d &= ~ESHIK;                                 /* eshik yopildi */
    holat("eshik yopildi:", d);

    if (d & (HARAKAT | TUTUN))                   /* HARAKAT yoki TUTUN yoniqmi? */
        printf("SIGNAL! Xavfli datchik ishga tushdi.\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra signal.c -o signal
$ ./signal
tinch:                 eshik:0 oyna:0 harakat:0 tutun:0
o'g'ri kirdi:          eshik:1 oyna:0 harakat:1 tutun:0
eshik yopildi:         eshik:0 oyna:0 harakat:1 tutun:0
SIGNAL! Xavfli datchik ishga tushdi.
```

**Kodda nimalar bor:** `d` — `uint8_t`, ya'ni **8 kalit**; boshida `0` (hammasi tinch). `ESHIK`, `OYNA`, `HARAKAT`, `TUTUN` — 0…3-kalitlarning nomlari.
`(datchiklar & ESHIK) != 0` — "kalit yoniqmi?" ni **0 yoki 1** ko'rinishida olish (`%d` ga qo'yish uchun).
`d & (HARAKAT | TUTUN)` — avval `HARAKAT | TUTUN` ikkita bitli niqob yasaydi, keyin `&` ularning **istalgani** yoniqligini tekshiradi.

<!-- katta:boshi -->
## Katta loyiha: Ombor — 3-bosqich: chegirma va mahsulot holati (bit bayroqlari)

**Oldingi bosqichdan:** narxlar va sonlar to'g'ri turlarda. Endi mahsulotga **biznes qoidalari** qo'shamiz: chegirma hisoblash va mahsulot **holati** (yangi? chegirmada? tugayapti?).

### Bu bosqichda nima qilamiz

1. **Chegirma:** `narx − narx × foiz / 100`. Hammasi **butun sonlarda** — yaxlitlash pastga (tiyinlarda xato kichik).
2. **Holat bayroqlari:** har mahsulotning bir nechta "ha/yo'q" xossasi bor (yangi, chegirmada, tugayapti). Uchta alohida o'zgaruvchi o'rniga **bitta `unsigned`** olamiz va har xossa — alohida **bit** (3-bobdagi "chiroq kalitlari paneli"!). Bu yadroda har joyda ishlatiladigan usul: bitta son 32 ta xossani saqlaydi.

| Bayroq | Bit | Qiymat | Ma'nosi |
|---|---|---|---|
| `YANGI` | 0 | `0000 0001` (`0x1`) | yaqinda qo'shilgan |
| `CHEGIRMA` | 1 | `0000 0010` (`0x2`) | chegirmada |
| `TUGAYAPTI` | 2 | `0000 0100` (`0x4`) | zaxira 10 tadan kam |

**Nega bayroqlar?** 3 ta alohida `int` uchun 12 bayt kerak, bitta `unsigned` uchun 4 bayt; va "hammasini nolga qaytarish", "bir nechtasini birga tekshirish" kabi amallar bitta bit amali bilan bajariladi.

**O'zgargan fayl:** faqat `ombor.c` (`ombor_chop.*` o'zgarmagan, 2-bosqichdagi bilan bir xil).

```c
/* ombor.c - Ombor, 3-bosqich: chegirma (arifmetika) va mahsulot holati (bitli bayroqlar) */
#include <stdint.h>
#include <stdio.h>

#include "ombor_chop.h"

/* mahsulot holati: har bir "kalit" - bitta bit (32 kalitli panel, ulardan 3 tasi ishlatilyapti) */
#define YANGI      (1u << 0)                    /* 0000 0001 */
#define CHEGIRMA   (1u << 1)                    /* 0000 0010 */
#define TUGAYAPTI  (1u << 2)                    /* 0000 0100 */

/* chegirmali narx: butun sonlarda, yaxlitlash pastga */
static long chegirmali(long narx, int foiz)
{
    return narx - narx * foiz / 100;
}

/* zaxraga qarab holatni yangilaydi: kam qolsa TUGAYAPTI bayrog'ini yoqamiz, ko'p bo'lsa o'chiramiz */
static unsigned holat_yangila(unsigned holat, uint16_t soni)
{
    if (soni < 10)
        holat |= TUGAYAPTI;                     /* yoqish: boshqa bitlarga tegmaydi */
    else
        holat &= ~TUGAYAPTI;                    /* o'chirish */
    return holat;
}

static void holat_chiqar(const char *nom, unsigned holat)
{
    printf("  %-8s holati: 0x%X (", nom, holat);
    printf("yangi:%s ", (holat & YANGI) ? "ha" : "yo'q");
    printf("chegirma:%s ", (holat & CHEGIRMA) ? "ha" : "yo'q");
    printf("tugayapti:%s)\n", (holat & TUGAYAPTI) ? "ha" : "yo'q");
}

int main(void)
{
    long non_narx = 400000, sut_narx = 1200000, guruch_narx = 1800000;     /* tiyinda */
    uint16_t non_soni = 120, sut_soni = 45, guruch_soni = 8;
    unsigned non_holat = YANGI, sut_holat = 0, guruch_holat = 0;           /* non yangi mahsulot */

    non_holat = holat_yangila(non_holat, non_soni);
    sut_holat = holat_yangila(sut_holat, sut_soni);
    guruch_holat = holat_yangila(guruch_holat, guruch_soni);

    printf("--- Dastlabki holat ---\n");
    holat_chiqar("Non", non_holat);
    holat_chiqar("Sut", sut_holat);
    holat_chiqar("Guruch", guruch_holat);

    printf("\n--- Guruchga 25%% chegirma e'lon qilindi ---\n");
    guruch_holat ^= CHEGIRMA;                   /* almashtirish: o'chiq edi -> yondi */
    if (guruch_holat & CHEGIRMA)
        guruch_narx = chegirmali(guruch_narx, 25);
    holat_chiqar("Guruch", guruch_holat);
    printf("  yangi narx: ");
    chop_pul(guruch_narx);
    printf(" so'm\n");

    printf("\n--- 30 kun o'tdi: non endi yangi emas ---\n");
    non_holat &= ~YANGI;                        /* faqat YANGI bitini o'chirish */
    holat_chiqar("Non", non_holat);

    printf("\n--- Ro'yxat ---\n");
    long jami = non_narx * non_soni + sut_narx * sut_soni + guruch_narx * guruch_soni;
    chop_sarlavha();
    chop_qator("Non", non_narx, non_soni);
    chop_qator("Sut", sut_narx, sut_soni);
    chop_qator("Guruch", guruch_narx, guruch_soni);
    chop_jami(jami, 12);

    printf("\nTiyin ostidagi qoldiq: 1234567 tiyin = %ld so'm va %ld tiyin\n", 1234567L / 100, 1234567L % 100);
    return 0;
}
```

```console
$ cd katta_loyiha/ombor/03_operatorlar
$ gcc -Wall -Wextra ombor.c ombor_chop.c -o ombor
$ ./ombor
--- Dastlabki holat ---
  Non      holati: 0x1 (yangi:ha chegirma:yo'q tugayapti:yo'q)
  Sut      holati: 0x0 (yangi:yo'q chegirma:yo'q tugayapti:yo'q)
  Guruch   holati: 0x4 (yangi:yo'q chegirma:yo'q tugayapti:ha)

--- Guruchga 25% chegirma e'lon qilindi ---
  Guruch   holati: 0x6 (yangi:yo'q chegirma:ha tugayapti:ha)
  yangi narx: 13500.00 so'm

--- 30 kun o'tdi: non endi yangi emas ---
  Non      holati: 0x0 (yangi:yo'q chegirma:yo'q tugayapti:yo'q)

--- Ro'yxat ---
================ OMBOR ================
Mahsulot         Narx   Soni          Summa
---------------------------------------
Non           4000.00    120      480000.00
Sut          12000.00     45      540000.00
Guruch       13500.00      8      108000.00
---------------------------------------
Jami qiymat:                1128000.00
QQS stavkasi:               12%
QQS summasi:                135360.00

Tiyin ostidagi qoldiq: 1234567 tiyin = 12345 so'm va 67 tiyin
```

**Bit amallari — qaysi qatorda nima qilinadi:**

| Kod | Amal | Nima qiladi |
|---|---|---|
| `#define YANGI (1u << 0)` | surish | 0-o'rindagi bitta bitni yoqilgan qiladi → `0x1` |
| `holat \|= TUGAYAPTI;` | **yoqish** (OR) | faqat shu bitni yoqadi, qolganlarga **tegmaydi** |
| `holat &= ~TUGAYAPTI;` | **o'chirish** (AND + NOT) | `~TUGAYAPTI` — shu bitdan tashqari hamma bit 1; `&` bilan faqat shu bit 0 bo'ladi |
| `holat ^= CHEGIRMA;` | **almashtirish** (XOR) | o'chiq edi → yondi (yana qilsak → o'chadi) |
| `holat & YANGI` | **tekshirish** (AND) | natija noldan farqli bo'lsa — bayroq yoniq |

**Trace — guruchning holati (`holat` o'zgaruvchisi):**

| Qadam | Amal | `holat` (bitlar) | Hex |
|---|---|---|---|
| boshida | `guruch_holat = 0` | `0000 0000` | `0x0` |
| `holat_yangila` (zaxira 8 < 10) | `\|= TUGAYAPTI` | `0000 0100` | `0x4` |
| chegirma e'lon qilindi | `^= CHEGIRMA` | `0000 0110` | `0x6` |

**Nima ko'rdik:** `Guruch holati: 0x6` — ikkala bayroq (chegirma + tugayapti) bitta sonda: `0x6 = 0x4 + 0x2`. Chegirmadan keyin narx 18000 → 13500 (25% kamaydi). 30 kundan keyin nonning `YANGI` biti `&= ~YANGI` bilan o'chdi — **boshqa bitlarga tegmasdan**.
Oxirgi qator: `1234567 / 100` = 12345 so'm, `1234567 % 100` = 67 tiyin — `/` va `%` birgalikda butun va qoldiq qismni ajratadi.

> **Eslab qoling:** bayroqlar bitta sonda: **yoqish** `|=`, **o'chirish** `&= ~`, **almashtirish** `^=`, **tekshirish** `&`. Chegirma kabi hisoblarda `*` va `/` tartibi muhim: avval ko'paytirib, keyin bo'lasiz (aks holda butun bo'lish kasrni yo'qotadi).

**O'zingiz qo'shing (yechimsiz):**

1. Yangi bayroq `NOYOB` (`1u << 3`) qo'shing va uni `holat_chiqar` da ko'rsating.
2. `chegirmali` ni tekshiring: `chegirmali(1999, 10)` nima beradi? Yaxlitlash qaysi tomonga ketadi? (Maslahat: butun bo'lish.)
3. Mahsulot "tugayapti"mi va "chegirmada"mi — **ikkalasi birga** bo'lganini bitta `if` bilan tekshiring. (Maslahat: `(holat & (A | B)) == (A | B)`.)
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. `/` butun sonlarda — **butun** bo'lish (kasr tashlanadi); `%` — qoldiq. `=` — berish, `==` — taqqoslash.
2. `&&` / `||` chapdan o'ngga tekshiradi va **qisqa tutashadi** — xavfsiz shartni chapga yozing.
3. Bit — kalit (0/1). `uint32_t` = 32 kalitli panel; `0` = hammasi o'chiq. Raqamlash o'ngdan, 0 dan.
4. Niqob `1u << n`. **Yoqish `|=`**, **o'chirish `&= ~`**, **almashtirish `^=`**, **tekshirish `&`** — har biri haqiqat jadvalidan chiqadi.
5. Bit va taqqoslash aralashsa — **qavs**. Bitlar bilan faqat **ishorasiz** turlarda ishlang (`1u`).

## Savol-javob

**Bitli amallar nega shunchalik muhim?**
Apparat bitlar bilan gaplashadi: qurilma registrining har bir biti alohida sozlama (yoqish, uzilish, xato). Xotira manzillari ham bitlarga
bo'lingan: virtual manzilning 39–47-bitlari — PML4 indeksi, 30–38 — PDPT va hokazo (MyOS: `kernel/mm/vmm.c`). Bitlarni erkin
o'qiy olmasangiz, yadro kodini o'qiy olmaysiz.

**`x * 2` o'rniga `x << 1` yozish kerakmi?**
Yo'q — kompilyator buni o'zi qiladi. Ma'nosi bo'yicha yozing: arifmetika uchun `*`, bitlar uchun `<<`.

**`panel |= OSHXONA` o'rniga `panel = panel + OSHXONA` yozsam bo'ladimi?**
Faqat kalit **o'chiq** bo'lsa. Yoniq bo'lsa, qo'shish yonidagi kalitga o'tib ketadi (`0010 + 0010 = 0100` — boshqa kalit yondi!). `|` esa
yoniq kalitni **yoniq** qoldiradi — shuning uchun xavfsiz.

**Nega o'chirish uchun `& ~M`, shunchaki `- M` emas?**
`-` kalit **yoniq** bo'lsa ishlaydi, o'chiq bo'lsa boshqa kalitlarni buzadi. `& ~M` har ikki holatda to'g'ri.

## O'zingizni tekshiring

1. `5 & 3`, `5 | 3`, `5 ^ 3`, `~0u >> 28` nechaga teng?
2. `if (p->x && p)` da nima xato?
3. `x & 0x80 == 0` qanday bajariladi?
4. `int i = 3; int j = i++ + 1;` — `i` va `j`?
5. 4096 ga tekislangan eng yaqin kichik manzilni qanday topasiz (`addr` dan)?
6. `uint8_t p = 0;` dan boshlab 2-kalitni yoqing, keyin 0-kalitni yoqing, keyin 2-kalitni o'chiring. `p` nechaga teng?

<details><summary>Javoblar</summary>

1. 1, 7, 6, 15 (0xF).
2. `p` NULL bo'lsa, `p->x` oldin bajarilib qulaydi. To'g'risi: `p && p->x`.
3. `x & (0x80 == 0)` → `x & 0` → 0. To'g'risi: `(x & 0x80) == 0`.
4. `i = 4`, `j = 4`.
5. `addr & ~0xFFFull` (yoki `addr & ~(uint64_t)4095`).
6. `p |= 1u<<2;` → `0000 0100` (4). `p |= 1u<<0;` → `0000 0101` (5). `p &= ~(1u<<2);` → `0000 0001` (1). Javob: 1.
</details>

## Mashq

- **02**, **03** — taqqoslash va toshishni oldindan tekshirish.
- **04** (bitlar) — bu bobning asosiy mashqi. Hal qilgach, `kernel/mm/vmm.c` ni oching va bitli amallarni topib, har birini o'qib chiqing.
- Qo'shimcha: `bitlar.h` bilan o'zingiz 8 ta kalitli panel yasang, kalitlarga ism bering, yoqish/o'chirish/almashtirishni mashq qiling.

<!-- loyiha:boshi -->
## Loyiha: Unix fayl ruxsatlari (chmod)

**Maqsad:** `chmod`, `umask` va `ls -l` dagi `rw-r--r--` yozuvi ostida nima borligini bitli amallar bilan ko'rish.
**Bobdan ishlatiladi:** `|`, `&`, `~`, `^`, `?:`, sakkizlik son (`0644`), `%o`.

**Talab:** fayl rejimi (mode) — bitta son. Uni `chmod` buyruqlari kabi o'zgartiring va har safar
`ls -l` ko'rinishida chiqaring.
**Ma'lumotlar:** 9 bit: har uch bit (egasi, guruh, boshqalar) — `r` (4), `w` (2), `x` (1).
Ya'ni `0644` = egasi `rw-` (6), guruh `r--` (4), boshqalar `r--` (4).
**Qadamlar:** o'qish/yozish/bajarish bitlarini `&` bilan tekshirish, `|=` bilan yoqish, `&= ~` bilan o'chirish.

```c
/* ruxsat.c - chmod bitlarda */
#include <stdio.h>

int main(void)
{
    unsigned mode = 0644;                       /* rw-r--r-- */

    printf("boshida:      %04o = %c%c%c%c%c%c%c%c%c\n", mode,
           (mode & 0400) ? 'r' : '-', (mode & 0200) ? 'w' : '-', (mode & 0100) ? 'x' : '-',
           (mode & 0040) ? 'r' : '-', (mode & 0020) ? 'w' : '-', (mode & 0010) ? 'x' : '-',
           (mode & 0004) ? 'r' : '-', (mode & 0002) ? 'w' : '-', (mode & 0001) ? 'x' : '-');

    mode |= 0100;                               /* chmod u+x  : egasi bajara olsin */
    printf("chmod u+x:    %04o = %c%c%c%c%c%c%c%c%c\n", mode,
           (mode & 0400) ? 'r' : '-', (mode & 0200) ? 'w' : '-', (mode & 0100) ? 'x' : '-',
           (mode & 0040) ? 'r' : '-', (mode & 0020) ? 'w' : '-', (mode & 0010) ? 'x' : '-',
           (mode & 0004) ? 'r' : '-', (mode & 0002) ? 'w' : '-', (mode & 0001) ? 'x' : '-');

    mode &= ~0044;                              /* chmod go-r : guruh va boshqalar o'qiy olmasin */
    printf("chmod go-r:   %04o = %c%c%c%c%c%c%c%c%c\n", mode,
           (mode & 0400) ? 'r' : '-', (mode & 0200) ? 'w' : '-', (mode & 0100) ? 'x' : '-',
           (mode & 0040) ? 'r' : '-', (mode & 0020) ? 'w' : '-', (mode & 0010) ? 'x' : '-',
           (mode & 0004) ? 'r' : '-', (mode & 0002) ? 'w' : '-', (mode & 0001) ? 'x' : '-');

    mode ^= 0020;                               /* guruhning yozish bitini almashtirish */
    printf("g+w (xor):    %04o = %c%c%c%c%c%c%c%c%c\n", mode,
           (mode & 0400) ? 'r' : '-', (mode & 0200) ? 'w' : '-', (mode & 0100) ? 'x' : '-',
           (mode & 0040) ? 'r' : '-', (mode & 0020) ? 'w' : '-', (mode & 0010) ? 'x' : '-',
           (mode & 0004) ? 'r' : '-', (mode & 0002) ? 'w' : '-', (mode & 0001) ? 'x' : '-');

    unsigned yangi_fayl = 0666, umask = 022;    /* umask: "olib tashlanadigan" ruxsatlar */
    printf("\numask %03o bilan yangi fayl: 0666 & ~022 = %04o\n", umask, yangi_fayl & ~umask);
    printf("egasi yoza oladimi? %s\n", (mode & 0200) ? "ha" : "yo'q");
    printf("boshqalar bajara oladimi? %s\n", (mode & 0001) ? "ha" : "yo'q");
    return 0;
}
```

```console
$ gcc -Wall -Wextra ruxsat.c -o ruxsat
$ ./ruxsat
boshida:      0644 = rw-r--r--
chmod u+x:    0744 = rwxr--r--
chmod go-r:   0700 = rwx------
g+w (xor):    0720 = rwx-w----

umask 022 bilan yangi fayl: 0666 & ~022 = 0644
egasi yoza oladimi? ha
boshqalar bajara oladimi? yo'q
$ touch sinov.txt && chmod 640 sinov.txt && ls -l sinov.txt | cut -c1-10
-rw-r-----
```

Oxirgi buyruq haqiqiy `ls -l` ni ko'rsatadi: `640` = `rw-r-----`. Bir xil `printf` to'rt marta takrorlandi —
bu 5-bobda **funksiyaga** aylanadigan takror. Hozircha shu ham yaxshi: har bir bit qanday yozilayotgani ko'rinib turibdi.

**Kengaytiring:** `chmod 755` ni bitta qiymat tayinlash bilan bering (`mode = 0755;`). `umask 077` bilan 0666 nima bo'ladi?

## Mustaqil loyiha: IP manzil hisoblagichi ★★☆

**Vazifa:** tarmoq muhandislari har kuni hisoblaydigan narsa — IP manzil va prefiks (`/26`) berilganda
tarmoq manzili, broadcast va host'lar sonini topish. Fayl: `ip.c`. Sikl va massiv **kerak emas** —
faqat bitli amallar (`<<`, `>>`, `&`, `|`, `~`).

**Ma'lumotlar** (kod boshida): `o1 = 192, o2 = 168, o3 = 10, o4 = 77`, prefiks `26`.

**Talab:**
1. To'rt oktetni bitta `uint32_t` ga joylang (`o1` — eng yuqori bayt).
2. Maska — yuqori `prefiks` ta bit 1: 26 uchun `255.255.255.192`.
3. Tarmoq manzili = ip & maska. Broadcast = tarmoq | ~maska.
4. Birinchi host = tarmoq + 1, oxirgi host = broadcast − 1. Hostlar soni = 2^(32−prefiks) − 2.
5. Har bir manzilni `a.b.c.d` ko'rinishida chiqaring: har oktet uchun `(x >> 24) & 255` kabi ifoda.
6. Format: yorliq `%-11s`, keyin qiymat.

**Kutilgan natija** (`darslik/loyihalar/03_ip_hisoblagich/kutilgan.txt`):

```text
IP:        192.168.10.77/26
Maska:     255.255.255.192
Tarmoq:    192.168.10.64
Broadcast: 192.168.10.127
Birinchi:  192.168.10.65
Oxirgi:    192.168.10.126
Hostlar:   62
```

**Qo'shimcha sinovlar.** `10.1.2.3/8`:

```text
IP:        10.1.2.3/8
Maska:     255.0.0.0
Tarmoq:    10.0.0.0
Broadcast: 10.255.255.255
Birinchi:  10.0.0.1
Oxirgi:    10.255.255.254
Hostlar:   16777214
```

`172.16.5.9/30`:

```text
IP:        172.16.5.9/30
Maska:     255.255.255.252
Tarmoq:    172.16.5.8
Broadcast: 172.16.5.11
Birinchi:  172.16.5.9
Oxirgi:    172.16.5.10
Hostlar:   2
```

**Maslahat** (yechim emas):
- `~0u << (32 - prefiks)` — yuqori bitlari 1 bo'lgan maska. Prefiks 0 bo'lsa nima bo'ladi? (32 ga siljitish — UB, 13-bob;
  bu mashqda prefiks 1..30.)
- Ipni yig'ish: `(uint32_t)o1 << 24 | o2 << 16 | ...` — qavslar va tur haqida o'ylang: `o1 << 24` `int` da hisoblansa-chi?
- Hostlar sonini `(1u << (32 - prefiks)) - 2` bilan toping.
- Bir xil `printf` ni besh marta yozasiz — charchatadimi? 5-bobda buni funksiya bilan qisqartirasiz. Hozir shunday qiling.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g ip.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/03_ip_hisoblagich/kutilgan.txt && echo "TO'G'RI"
```

Qo'shimcha sinovlar uchun `o1..o4` va prefiksni o'zgartirib, `kutilgan_2.txt` va `kutilgan_3.txt` bilan solishtiring.
<!-- loyiha:oxiri -->

Keyingi bob: [4-bob. Boshqaruv oqimi](04-boshqaruv.md)
