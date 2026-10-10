# 20-bob. Sonlar kompyuterda: butun sonlar va kasr sonlar (IEEE 754)

> **Bu bobda nima o'rganasiz:** istalgan sonni ikkilik va o'n oltilik tizimda boshingizda aylantirishni; ikkiga to'ldirish (two's complement) nega ishlashini **isbotlashni**; ishora kengayishi va qirqishni;
> `float` ning bitlarini va `0.1 + 0.2 != 0.3` sababini. (Mavzu odatda "Computer Systems: A Programmer's Perspective" kitobining 2-bobidan o'rganiladi.)
> **Oldindan nima kerak:** 2-, 3-boblar (turlar, bit amallar).   **Vaqt:** 6–8 soat.
> Mashqlar: 03, 04, 41.

> **To'liq ishlaydigan misol:** [misollar/20_sonlar.c](misollar/20_sonlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

2-bobda `int` toshishini va `float` aniq emasligini ko'rdingiz. Endi **nega** shunday ekanini ochamiz: kompyuter sonni **bitlarda** qanday saqlaydi? Manfiy son qanday? Kasr son qanday?
Bu bilim bo'lmasa, "tushunarsiz" xatolar (`-INT_MIN`, `0.1 + 0.2`) siz uchun sirligicha qoladi.

**Hayotdan misol: pozitsion sanoq tizimi — kupyuralar.** 3 745 so'm — bu 3 ta mingtalik, 7 ta yuztalik, 4 ta o'ntalik va 5 ta birtalik. Raqamning **joyi** uning qiymatini belgilaydi. Ikkilik tizimda ham xuddi shunday,
faqat "kupyuralar" 1, 2, 4, 8, 16... va har biridan ko'pi bilan bitta: `1101` = 8 + 4 + 1 = 13. O'n oltilik tizim — ikkilikning qisqa yozuvi: har bir o'n oltilik raqam — to'rtta bit.

## 20.1. Pozitsion sanoq tizimlari

O'nlik: `347 = 3·10² + 4·10¹ + 7·10⁰`. Har bir o'rin — asosning darajasi.

Ikkilik (asos 2): `1011₂ = 1·8 + 0·4 + 1·2 + 1·1 = 11`.

O'n oltilik (asos 16, raqamlar `0–9, a–f`): `0x2F = 2·16 + 15 = 47`.

**Nega kompyuterda ikkilik:** tranzistor ikki holatda ishonchli: "tok bor" / "tok yo'q".

**Nega dasturchilar o'n oltilikni yaxshi ko'radi:** har bir o'n oltilik raqam aynan **4 bit**:

```text
0x  F    F    8    0    1    0    0    0
  1111 1111 1000 0000 0001 0000 0000 0000
```

Shuning uchun `0xFFFFFFFF80100000` ni ko'rib, bitlarini darhol "ko'rish" mumkin. Ikkilikda yozilsa 64 ta raqam, o'n oltilikda — 16 ta.

### O'nlikdan ikkilikka: ketma-ket bo'lish

**Qoida:** sonni 2 ga bo'lamiz, **qoldiq**ni yozamiz, bo'linmani yana 2 ga bo'lamiz — 0 ga yetguncha. Qoldiqlar **pastdan yuqoriga** o'qiladi.

```c
/* ondan_ikkiga.c - 13 ni ikkilikka aylantirish: qadamma-qadam */
#include <stdio.h>

int main(void)
{
    unsigned son = 13;
    unsigned x = son;
    char bit[33];
    int n = 0;

    printf("%u ni 2 ga ketma-ket bo'lamiz:\n", son);
    while (x > 0) {
        printf("  %2u / 2 = %2u, qoldiq %u\n", x, x / 2, x % 2);
        bit[n++] = (char)('0' + x % 2);         /* qoldiq - keyingi bit (pastdan yuqoriga) */
        x /= 2;
    }

    printf("qoldiqlarni PASTDAN yuqoriga o'qiymiz: ");
    for (int i = n - 1; i >= 0; i--)
        putchar(bit[i]);
    printf("  (= %u)\n", son);
    return 0;
}
```

```console
$ gcc -Wall -Wextra ondan_ikkiga.c -o ondan_ikkiga
$ ./ondan_ikkiga
13 ni 2 ga ketma-ket bo'lamiz:
  13 / 2 =  6, qoldiq 1
   6 / 2 =  3, qoldiq 0
   3 / 2 =  1, qoldiq 1
   1 / 2 =  0, qoldiq 1
qoldiqlarni PASTDAN yuqoriga o'qiymiz: 1101  (= 13)
```

**Bu dastur nima qiladi:** 13 ni ikkilikka aylantirishni qadamlarini ko'rsatib bajaradi. Har bo'lishda **qoldiq** (0 yoki 1) — navbatdagi bit. Birinchi qoldiq — **eng pastki** bit (0-kalit),
oxirgisi — eng yuqorisi; shuning uchun teskari tartibda o'qiymiz → `1101`.

### Boshda aylantirish

Yodlash kerak bo'lgan jadval (bir marta o'rganing — butun umr ishlatasiz):

| hex | ikkilik | o'nlik | | hex | ikkilik | o'nlik |
|---|---|---|---|---|---|---|
| 0 | 0000 | 0 | | 8 | 1000 | 8 |
| 1 | 0001 | 1 | | 9 | 1001 | 9 |
| 2 | 0010 | 2 | | a | 1010 | 10 |
| 3 | 0011 | 3 | | b | 1011 | 11 |
| 4 | 0100 | 4 | | c | 1100 | 12 |
| 5 | 0101 | 5 | | d | 1101 | 13 |
| 6 | 0110 | 6 | | e | 1110 | 14 |
| 7 | 0111 | 7 | | f | 1111 | 15 |

Ikkining darajalari: 2¹⁰ = 1024 (1 KiB), 2¹² = 4096 (sahifa), 2²⁰ ≈ million (1 MiB), 2³⁰ ≈ milliard (1 GiB), 2³² ≈ 4.29 milliard, 2⁴⁸ = 256 TiB (x86-64 virtual manzil maydoni), 2⁶⁴ ≈ 1.8·10¹⁹.

`2ⁿ` ni tez hisoblash: `2^(10a + b) = 2^b · 1024^a`. Masalan 2³⁹ = 2⁹ · 2³⁰ = 512 GiB — bitta PML4 yozuvi qamraydigan hudud (31-mashq).

> **Eslab qoling:** hex raqam = 4 bit. `0xF` = `1111`, `0x8` = `1000`. Jadvalni yodlang — keyin istalgan hex sonni "ko'rasiz".

## 20.2. Ishorasiz butun sonlar

w bitli ishorasiz son: `B = b[w-1]·2^(w-1) + ... + b[1]·2 + b[0]`. Oraliq: `0 .. 2^w − 1`. Arifmetika **mod 2^w**: natijaning ortiqcha bitlari tashlanadi.

```text
8 bit:  255 + 1 = 1 0000 0000  ->  0000 0000 = 0
        0 - 1   = 1111 1111 = 255
```

C'da bu **aniqlangan** xatti-harakat (2-bob). Shuning uchun xesh funksiyalar, tasodifiy son generatorlari, bitli hisoblar ishorasiz turda yoziladi.

## 20.3. Ikkiga to'ldirish (two's complement) — nega aynan shunday

**Hayotdan misol: orqaga aylanadigan hisoblagich.** Uch xonali hisoblagich 000 dan bitta orqaga aylantirilsa — 999 ko'rsatadi. Demak "−1" ni 999 deb kelishish mumkin: 999 + 1 = 000 — to'g'ri!
8 bitli sonlarda ham: −1 = `11111111`. Shu kelishuv tufayli protsessorga ayirish uchun alohida sxema kerak emas — qo'shish sxemasi ishorali sonlar uchun ham ishlaydi.

w bitli ishorali son: eng yuqori bitning "og'irligi" **manfiy**:

```text
B = -b[w-1]·2^(w-1) + b[w-2]·2^(w-2) + ... + b[0]
```

8 bit uchun: `1000 0000 = -128`, `1111 1111 = -128 + 127 = -1`, `0111 1111 = 127`. Oraliq: `-2^(w-1) .. 2^(w-1) - 1` (masalan int: −2147483648 .. 2147483647). Manfiylar bittaga ko'p.

**Ishorani o'zgartirish qoidasi:** `-x = ~x + 1` (hamma bitlarni teskari qilib, 1 qo'shish):

```text
 5 = 0000 0101
~5 = 1111 1010
+1 = 1111 1011 = -5     tekshirish: -128 + 64+32+16+8+2+1 = -128 + 123 = -5 ✓
```

```c
/* ikkiga_toldirish.c - -x = ~x + 1 */
#include <stdint.h>
#include <stdio.h>

static void bitlar8(const char *nom, uint8_t x)
{
    printf("%-14s ", nom);
    for (int i = 7; i >= 0; i--)
        putchar((x >> i) & 1 ? '1' : '0');
    printf("  ishorasiz = %3u, ishorali = %4d\n", x, (int8_t)x);
}

int main(void)
{
    uint8_t x = 5;
    bitlar8("x = 5", x);
    bitlar8("~x", (uint8_t)~x);
    bitlar8("~x + 1", (uint8_t)(~x + 1));
    bitlar8("-5 (to'g'ri)", (uint8_t)-5);

    printf("\nbir xil 8 bit, ikki talqin:\n");
    bitlar8("0xFB", 0xFB);
    bitlar8("0x80", 0x80);
    bitlar8("0x7F", 0x7F);

    int8_t kichik = -128;                   /* eng kichik: -(-128) sig'maydi */
    printf("\n-(-128) int8_t da: %d (yana -128!)\n", (int8_t)(-kichik));
    return 0;
}
```

```console
$ gcc -Wall -Wextra ikkiga_toldirish.c -o ikkiga_toldirish
$ ./ikkiga_toldirish
x = 5          00000101  ishorasiz =   5, ishorali =    5
~x             11111010  ishorasiz = 250, ishorali =   -6
~x + 1         11111011  ishorasiz = 251, ishorali =   -5
-5 (to'g'ri)   11111011  ishorasiz = 251, ishorali =   -5

bir xil 8 bit, ikki talqin:
0xFB           11111011  ishorasiz = 251, ishorali =   -5
0x80           10000000  ishorasiz = 128, ishorali = -128
0x7F           01111111  ishorasiz = 127, ishorali =  127

-(-128) int8_t da: -128 (yana -128!)
```

**Bu dastur nima qiladi:** bir xil 8 bitni ikkita usulda — **ishorasiz** va **ishorali** — o'qib ko'rsatadi; `-5` ni `~5 + 1` qoidasi bilan hosil qiladi; eng kichik sonning ishorasini o'zgartirishda nima bo'lishini ko'rsatadi.

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `bitlar8` | 8 bitni chap→o'ngga chiqaradi (3.4.9 dagi usul) va ikki talqinni ko'rsatadi: `x` (ishorasiz) va `(int8_t)x` (ishorali) |
| `~x`, `~x + 1` | `5` → `11111010` (250) → `11111011` (251 = −5 ishorali) — ishorani o'zgartirish qoidasi |
| `0xFB`, `0x80`, `0x7F` | `-5`, `-128` (eng kichik), `127` (eng katta) |
| `-(-128)` | `~10000000 + 1 = 01111111 + 1 = 10000000` — **yana −128**: musbati sig'maydi |

**Nega kompyuterlar shuni tanladi:** qo'shish/ayirish/ko'paytirish **ishorali va ishorasiz uchun bir xil sxema** bilan bajariladi — faqat natijani talqin qilish boshqa. `1111 1111 + 0000 0001`:
ishorasiz talqinda 255 + 1 = 0 (mod 256), ishorali talqinda −1 + 1 = 0. CPU'da bitta qo'shuvchi — ikki xil ma'no. Shu sababli CPU'da `add` bitta, lekin taqqoslashdan keyingi sakrashlar ikki xil:
`jl/jg` (ishorali) va `jb/ja` (ishorasiz) — 17-bob.

**Muhim nosimmetriklik:** `-INT_MIN` — sig'maydi (`~1000...0 + 1 = 1000...0` — yana o'zi!). Shuning uchun `abs(INT_MIN)`, `INT_MIN / -1` — UB (03, 20-mashqlar).

> **Eslab qoling:** manfiy son = `~x + 1`. Eng yuqori bit — "minus" (og'irligi manfiy). Bitlar bir xil — talqin (turi) boshqa.

## 20.4. Ishora kengayishi va qirqish

**Hayotdan misol: narxni kattaroq blankaga ko'chirish.** Kichik blankada "−5" yozilgan. Katta blankaga ko'chirganda bo'sh katakchalarni to'ldirish kerak: musbat son uchun 0 lar bilan, manfiy son uchun —
1 lar bilan (ikkiga to'ldirishda manfiy sonning boshi 1 lardan iborat). Aks holda −5 katta musbat songa aylanib qoladi.

Kichik turdan kattasiga o'tkazish:

- **ishorasiz** → yuqoriga **nollar** qo'shiladi (zero extension);
- **ishorali** → yuqoriga **ishora biti** takrorlanadi (sign extension).

Kattadan kichikka — **qirqish**: pastki bitlar qoladi. **Hayotdan misol:** 4 xonali displeyga 12 345 ni yozsangiz, faqat oxirgi 4 raqami qoladi: 2345.

```c
/* kengaytirish.c - ishora kengayishi, nol kengayishi va qirqish */
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    int8_t a = -5;                          /* 1111 1011 */
    int32_t b = a;                          /* ishora kengayishi: qiymat saqlanadi */
    printf("int8_t -5 -> int32_t: %d (0x%08X)\n", b, (unsigned)b);

    uint8_t c = 0xFB;                       /* 251 */
    uint32_t d = c;                         /* nol kengayishi */
    printf("uint8_t 251 -> uint32_t: %u (0x%08X)\n", d, d);

    int32_t x = 300;                        /* 0001 0010 1100 */
    int8_t y = (int8_t)x;                   /* qirqish: pastki 8 bit */
    printf("300 ni int8_t ga qirqish: %d (300 %% 256 = %d)\n", y, 300 % 256);

    printf("12345 ni uint8_t ga qirqish: %u (12345 %% 256 = %d)\n", (uint8_t)12345, 12345 % 256);

    int8_t m1 = -1;
    uint32_t katta = (uint32_t)m1;          /* avval ishora kengayishi, keyin ishorasiz */
    printf("int8_t -1 -> uint32_t: %u (0x%08X)\n", katta, katta);
    return 0;
}
```

```console
$ gcc -Wall -Wextra kengaytirish.c -o kengaytirish
$ ./kengaytirish
int8_t -5 -> int32_t: -5 (0xFFFFFFFB)
uint8_t 251 -> uint32_t: 251 (0x000000FB)
300 ni int8_t ga qirqish: 44 (300 % 256 = 44)
12345 ni uint8_t ga qirqish: 57 (12345 % 256 = 57)
int8_t -1 -> uint32_t: 4294967295 (0xFFFFFFFF)
```

**Qadamlar:**

- `-5` (`1111 1011`) ni 32 bitga kengaytirganda **1 lar** bilan to'ldiriladi → `0xFFFFFFFB` = −5 (qiymat saqlandi).
- `0xFB` (ishorasiz, 251) → **0 lar** bilan → 251.
- `300` = `0001 0010 1100`: pastki 8 bit `0010 1100` = 44 (= 300 mod 256).
- `-1` ishorasiz katta turga o'tkazilganda avval ishora kengayadi (`0xFFFFFFFF`), keyin ishorasiz talqin → 4294967295.

**Klassik tuzoq — `char` va EOF:**

```c
char c;                          /* x86 da ishorali */
while ((c = getchar()) != EOF)   /* 0xFF bayt -> c = -1 -> EOF bilan teng -> sikl oldin tugaydi! */
```

`getchar` `int` qaytaradi (0..255 yoki −1). Natijani `int` da saqlang. Buni ko'ramiz:

```c
/* getchar_tuzoq.c - char va EOF */
#include <stdio.h>

int main(void)
{
    int soni = 0;
    char c;                                  /* XATO: int bo'lishi kerak edi */
    while ((c = (char)getchar()) != EOF)
        soni++;
    printf("char bilan o'qildi: %d bayt (3 bo'lishi kerak edi)\n", soni);
    return 0;
}
```

```console
$ gcc -Wall -Wextra getchar_tuzoq.c -o getchar_tuzoq
$ printf 'a\377b' | ./getchar_tuzoq
char bilan o'qildi: 1 bayt (3 bo'lishi kerak edi)
```

Kirishda 3 bayt bor: `a`, `0xFF`, `b`. Lekin `0xFF` `char` (ishorali) ga sig'ganda `-1` bo'ldi, u `EOF` (−1) bilan teng chiqdi va sikl **erta** tugadi (1 bayt o'qildi). Yechim: `int c = getchar();`.

**Yadrodagi misol — kanonik manzillar:** x86-64 da virtual manzil 48 bitli, lekin registr 64 bit. 63..48-bitlar 47-bitning **ishora kengayishi** bo'lishi shart: shuning uchun yadro manzillari `0xFFFF8000...`
dan boshlanadi (47-bit = 1 → yuqori bitlar ham 1). 31-mashqdagi kanonik tekshiruv — shu.

## 20.5. Butun son arifmetikasining xossalari

- Qo'shish, ko'paytirish **mod 2^w** — halqa: kommutativ, assotsiativ, hatto toshish bo'lganda ham (ishorasiz uchun; ishorali uchun C'da UB, lekin apparat darajasida xuddi shu).
- Ikkining darajasiga ko'paytirish = chapga surish: `x * 8 == x << 3`.
- Ikkining darajasiga bo'lish = o'ngga surish, **lekin** manfiy son uchun yaxlitlash farq qiladi: `-7 / 2 == -3` (C, nolga tomon), `-7 >> 1 == -4` (pastga). Kompilyator buni to'g'rilash uchun
  qo'shimcha buyruq qo'yadi — shuning uchun `unsigned` bo'lish tezroq.
- `x % 2^k == x & (2^k - 1)` — faqat ishorasiz (yoki manfiy bo'lmagan) x uchun.

## 20.6. Kasr sonlar: IEEE 754 standarti

**Hayotdan misol: kalkulyator ekrani va ilmiy yozuv.** Kalkulyator ekranida faqat 8–10 raqam sig'adi. Juda katta son `6.02e23` ko'rinishida yoziladi: raqamlar (**mantissa**) va **daraja**.
`float` da ~7 ta aniq raqam bor. Shuning uchun 16 777 217 ni `float` da aniq saqlab bo'lmaydi, 0.1 ni esa umuman aniq saqlab bo'lmaydi (ikkilikda u cheksiz kasr — o'nlikdagi 1/3 = 0.3333... kabi).

Kompyuterlarning deyarli hammasi `float` (32 bit) va `double` (64 bit) ni bir xil standart bo'yicha saqlaydi:

```text
float (32 bit):   [ S | E E E E E E E E | M M M M ... M (23 bit) ]
                   1       8 bit              23 bit
double (64 bit):  [ S | 11 bit eksponenta | 52 bit mantissa ]
```

**Normal son qiymati:**

```text
qiymat = (-1)^S × 1.M × 2^(E - 127)          (double uchun 127 o'rniga 1023)
```

- `S` — ishora (0 musbat, 1 manfiy).
- `E` — **siljitilgan** (biased) eksponenta: haqiqiy daraja + 127. Nega siljitilgan: shunda musbat sonlarning bitlari butun son sifatida solishtirilsa ham tartib to'g'ri chiqadi.
- `1.M` — "yashirin bir": normal sonda mantissa doim `1.xxx` ko'rinishida, shuning uchun 1 saqlanmaydi — bitta bit tejaladi.

**Misol: 6.5 ni float'ga**

```text
6.5 = 110.1₂ = 1.101₂ × 2²
S = 0,  E = 2 + 127 = 129 = 1000 0001,  M = 101 0000 ... (1. dan keyingi qism)
bitlar: 0 10000001 10100000000000000000000 = 0x40D00000
```

Buni dastur bilan tekshiramiz:

```c
/* float_bitlar.c - float ning ichki tuzilishi */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void ajrat(const char *nom, float f)
{
    uint32_t u;
    memcpy(&u, &f, sizeof(u));                      /* float baytlarini songa nusxalash */
    unsigned s = u >> 31;
    unsigned e = (u >> 23) & 0xFF;
    unsigned m = u & 0x7FFFFF;
    printf("%-10s 0x%08X  S=%u  E=%3u  M=0x%06X\n", nom, u, s, e, m);
}

int main(void)
{
    volatile float nol = 0.0f;                      /* volatile: kompilyator oldindan hisoblamasin */
    ajrat("6.5", 6.5f);
    ajrat("-1.0", -1.0f);
    ajrat("0.1", 0.1f);
    ajrat("0.0", 0.0f);
    ajrat("+inf", 1.0f / nol);
    ajrat("NaN", nol / nol);
    ajrat("denormal", 1e-45f);
    return 0;
}
```

```console
$ gcc -Wall -Wextra float_bitlar.c -o float_bitlar
$ ./float_bitlar
6.5        0x40D00000  S=0  E=129  M=0x500000
-1.0       0xBF800000  S=1  E=127  M=0x000000
0.1        0x3DCCCCCD  S=0  E=123  M=0x4CCCCD
0.0        0x00000000  S=0  E=  0  M=0x000000
+inf       0x7F800000  S=0  E=255  M=0x000000
NaN        0xFFC00000  S=1  E=255  M=0x400000
denormal   0x00000001  S=0  E=  0  M=0x000001
```

**Bu dastur nima qiladi:** `float` ning 32 bitini `S` (ishora), `E` (siljitilgan daraja), `M` (mantissa) maydonlariga ajratib ko'rsatadi. (`memcpy` — baytlarni nusxalash, 13.3 dagi xavfsiz usul.)

**Tekshiruv:** `6.5` → `0x40D00000`, `S=0`, `E=129` (= 2 + 127), `M=0x500000` (= `101` va 20 ta nol) — qo'lda hisoblaganimiz bilan mos. `-1.0` → `S=1`, `E=127`, `M=0`. `NaN` da ishora biti ahamiyatsiz (x86 da `0.0/0.0` natijasi `S=1`); muhimi — `E=255` va `M≠0`. `0.1` ning `M=0x4CCCCD` — `CCCC...` davriy kasr, oxiri `D` ga yaxlitlangan.

**Maxsus qiymatlar:**

| E | M | Nima |
|---|---|---|
| 0 | 0 | ±0 (ha, −0 ham bor!) |
| 0 | ≠0 | **denormal**: `0.M × 2^(-126)` — nolga juda yaqin sonlar, asta-sekin yo'qolish |
| 1..254 | istalgan | normal son |
| 255 | 0 | ±∞ (`1.0/0.0`) |
| 255 | ≠0 | NaN ("son emas": `0.0/0.0`, `sqrt(-1)`). `NaN != NaN` — hatto o'zi bilan ham! |

### Nega `0.1 + 0.2 != 0.3`

0.1 ikkilikda cheksiz davriy kasr: `0.0001100110011...₂` (xuddi 1/3 o'nlikda 0.333... kabi). 52 bitda qirqiladi → kichik xato. Ikki xatoli son yig'indisi 0.3 ning eng yaqin tasviriga
teng chiqmaydi.

```c
/* nol_bir.c - 0.1 + 0.2 va float yig'ilishi */
#include <math.h>
#include <stdio.h>

int main(void)
{
    double a = 0.1, b = 0.2;
    printf("0.1 + 0.2 = %.17g\n", a + b);
    printf("0.3       = %.17g\n", 0.3);
    printf("(a + b) == 0.3 ? %d\n", (a + b) == 0.3);
    printf("fabs(a + b - 0.3) < 1e-9 ? %d   (to'g'ri taqqoslash)\n", fabs(a + b - 0.3) < 1e-9);

    float x = 0.0f;
    for (int i = 0; i < 10; i++)
        x += 0.1f;                          /* 10 marta 0.1 */
    printf("float: 0.1 ni 10 marta qo'shsak = %.9f  (x == 1.0 ? %d)\n", x, x == 1.0f);
    return 0;
}
```

```console
$ gcc -Wall -Wextra nol_bir.c -o nol_bir -lm
$ ./nol_bir
0.1 + 0.2 = 0.30000000000000004
0.3       = 0.29999999999999999
(a + b) == 0.3 ? 0
fabs(a + b - 0.3) < 1e-9 ? 1   (to'g'ri taqqoslash)
float: 0.1 ni 10 marta qo'shsak = 1.000000119  (x == 1.0 ? 0)
```

**Qoida:** kasr sonlarni `==` bilan solishtirmang, `fabs(a - b) < eps` ishlating; pul hisobida — butun sonlar (tiyinlar). (`-lm` — matematika kutubxonasi, 1-bob.)
Shu sababli `for (float x = 0; x != 1.0; x += 0.1)` cheksiz sikl bo'lishi mumkin: yig'indi hech qachon aynan 1.0 bo'lmasligi mumkin.

**Yaxlitlash:** standart bo'yicha — "eng yaqiniga, teng bo'lsa juftiga" (round-to-nearest-even): 2.5 → 2, 3.5 → 4. Nega juftiga: ko'p yaxlitlashda xatolar bir tomonga to'planmaydi.

**Aniqlik:** `float` ~7 o'nlik raqam, `double` ~15–16. 2²⁴ + 1 = 16777217 ni `float` aniq saqlay olmaydi: `(float)16777217 == 16777216`. Shuning uchun `int` → `float` aylantirish ma'lumot yo'qotishi mumkin.

```c
/* aniqlik.c - float chegarasi va yaxlitlash */
#include <math.h>
#include <stdio.h>

int main(void)
{
    printf("(float)16777216 = %.0f\n", (double)(float)16777216);
    printf("(float)16777217 = %.0f  (2^24 + 1 sig'maydi)\n", (double)(float)16777217);
    printf("rint(2.5) = %.0f, rint(3.5) = %.0f  (juftiga yaxlitlash)\n", rint(2.5), rint(3.5));
    return 0;
}
```

```console
$ gcc -Wall -Wextra aniqlik.c -o aniqlik -lm
$ ./aniqlik
(float)16777216 = 16777216
(float)16777217 = 16777216  (2^24 + 1 sig'maydi)
rint(2.5) = 2, rint(3.5) = 4  (juftiga yaxlitlash)
```

> **Eslab qoling:** `float` = ishora + daraja + mantissa; 0.1 aniq emas; kasrni `==` bilan solishtirmang; pul — butun sonda; 2²⁴ dan katta butun sonlar `float` da aniq emas.

## 20.7. Yadroda kasr sonlar o'rniga — qat'iy nuqtali (fixed-point) arifmetika

**Hayotdan misol: pulni tiyinda sanash.** Bankda hisob "12 650.55 so'm" deb emas, "1 265 055 tiyin" deb saqlanadi — butun son, hech qanday yaxlitlash xatosi yo'q. Yadroda `float` ishlatilmaydi,
shuning uchun foizlar, vaqt va boshqalar aynan shunday butun sonlar bilan hisoblanadi.

Yadro FPU registrlarini ishlatmaydi (18-bob). Kasr kerak bo'lsa — butun sonni "masshtab" bilan:

```c
/* qat_nuqta.c - qat'iy nuqtali hisob: foiz va vaqt */
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    /* CPU yuklamasi foizda, 2 xona aniqlik: 12.34% -> 1234 */
    uint64_t band_tiklar = 1234, jami_tiklar = 10000;
    uint32_t yuklama_x100 = (uint32_t)(band_tiklar * 10000 / jami_tiklar);
    printf("yuklama: %u.%02u%%\n", yuklama_x100 / 100, yuklama_x100 % 100);

    /* TSC chastotasi kHz da saqlanadi - Hz da float emas */
    uint64_t tsc_khz = 2400000;              /* 2.4 GHz = 2 400 000 kHz */
    uint64_t tsc_farq = 4800000;             /* shuncha takt o'tdi */
    uint64_t mikrosekund = tsc_farq * 1000 / tsc_khz;
    printf("%llu takt = %llu mikrosekund\n", (unsigned long long)tsc_farq, (unsigned long long)mikrosekund);
    return 0;
}
```

```console
$ gcc -Wall -Wextra qat_nuqta.c -o qat_nuqta
$ ./qat_nuqta
yuklama: 12.34%
4800000 takt = 2000 mikrosekund
```

**Qadamlar:** foiz uchun avval **ko'paytiramiz** (`× 10000`), keyin **bo'lamiz** — butun sonlarda tartib shunday bo'lishi kerak (aks holda `1234 / 10000 = 0` — aniqlik yo'qoladi). `1234 × 10000 / 10000 = 1234` → `12.34%`.
Vaqt: 4 800 000 takt ÷ 2 400 000 kHz = 2 ms = 2000 mikrosekund.

Linux ham shunday qiladi: yuklama o'rtachalari (load average), scheduler og'irliklari — hammasi butun sonlarda. Tartib muhim: avval ko'paytirish, keyin bo'lish (aks holda aniqlik yo'qoladi), lekin toshishga e'tibor (64 bit).

## Hayotdan misol va to'liq dastur

**Valyuta ayirboshlash shoxobchasi.** `float` xatosi yig'ilishini va tiyin/sentlarda **aniq** hisobni ko'ramiz; shuningdek, ikkilik/o'n oltilik ko'rinish, ikkiga to'ldirish va qirqish.

```c
/* valyuta.c - float xatosi yig'ilishi va qat'iy nuqtali (tiyinli) hisob */
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    /* Bir kunda 100 000 ta mijoz, har biridan 0.01 dollar komissiya */
    float komissiya_f = 0.0f;
    long komissiya_sent = 0;                    /* sentlarda - butun son */
    for (int i = 0; i < 100000; i++) {
        komissiya_f += 0.01f;
        komissiya_sent += 1;
    }
    printf("float bilan:  %.4f dollar\n", komissiya_f);
    printf("sent bilan:   %ld.%02ld dollar (aniq 1000.00 bo'lishi kerak)\n",
           komissiya_sent / 100, komissiya_sent % 100);

    /* Kurs: 1 dollar = 12 650.55 so'm -> tiyinda 1 265 055 */
    const int64_t kurs_tiyin = 1265055;
    int64_t dollar = 350;
    int64_t natija = dollar * kurs_tiyin;       /* tiyinda */
    printf("\n%lld dollar = %lld.%02lld so'm\n", (long long)dollar,
           (long long)(natija / 100), (long long)(natija % 100));

    /* Ikkilik va o'n oltilik ko'rinish */
    unsigned summa = 13;
    printf("\n13 ikkilikda: ");
    for (int b = 7; b >= 0; b--)
        putchar((summa >> b) & 1 ? '1' : '0');
    printf(", o'n oltilikda: 0x%X\n", summa);

    /* Ikkiga to'ldirish va ishora kengayishi */
    int8_t qarz = -5;
    printf("-5 baytda: 0x%02X, int ga kengaytirilganda: %d (0x%08X)\n",
           (uint8_t)qarz, (int)qarz, (unsigned)(int)qarz);
    printf("12345 ni 8 bitga qirqish: %u\n", (uint8_t)12345);
    printf("(float)16777217 = %.0f\n", (double)(float)16777217);
    return 0;
}
```

```console
$ gcc -Wall -Wextra valyuta.c -o valyuta
$ ./valyuta
float bilan:  1000.6650 dollar
sent bilan:   1000.00 dollar (aniq 1000.00 bo'lishi kerak)

350 dollar = 4427692.50 so'm

13 ikkilikda: 00001101, o'n oltilikda: 0xD
-5 baytda: 0xFB, int ga kengaytirilganda: -5 (0xFFFFFFFB)
12345 ni 8 bitga qirqish: 57
(float)16777217 = 16777216
```

**Bu dastur nima qiladi (umumiy):** (1) 100 000 marta 0.01 dollar qo'shishni `float` da va **sentlarda** (butun son) bajarib, ikkalasini solishtiradi; (2) so'm/dollar kursini tiyinlarda hisoblaydi; (3) 13 ning ikkilik/o'n oltilik
ko'rinishini, `-5` ning ikkiga to'ldirilgan va kengaytirilgan ko'rinishini, qirqishni va `float` aniqlik chegarasini ko'rsatadi.

`float` bilan 100 000 ta 0.01 ni qo'shish 1000 dan sezilarli farq qildi — har bir qo'shishdagi kichik yaxlitlash xatosi yig'ilib boradi. Sentlar esa doim aniq.

**Sinab ko'ring:** `float` ni `double` ga almashtiring — xato kamayadimi, yo'qoladimi? 12345 ni 8 bitga qirqish natijasini qo'lda hisoblang: 12345 % 256.

<!-- katta:boshi -->
## Katta loyiha: IEEE 754 laboratoriyasi (kasr sonlar ichidan)

**Umumiy fikr.** `0.1 + 0.2 == 0.3` — **yolg'on**! Bu xato emas: kompyuter kasr sonlarni **ikkilikda** saqlaydi, `0.1` esa ikkilikda **cheksiz kasr** (xuddi 1/3 o'nlikda `0.333...` bo'lgani kabi). 20-bob shu haqda; bu bosqichda `float` ning **ichki bitlarini** ochib ko'ramiz.

**Hayotiy o'xshatish:** ilmiy yozuv. `0.000123` ni `1.23 × 10⁻⁴` deb yozamiz: **ishora**, **mantissa** (1.23), **daraja** (−4). `float` ham xuddi shunday, faqat **ikkilikda**: `±1.mantissa × 2^daraja`.

### Float ning 32 biti

```text
 ishora | daraja (8 bit) | mantissa (23 bit)
   1    |   10000000     | 01000000000000000000000      -> -2.5
```

| Qism | Bitlar | Ma'nosi |
|---|---|---|
| ishora | 1 | 0 = musbat, 1 = manfiy |
| daraja | 8 | 2 ning darajasi, **127 qo'shilgan** (daraja = saqlangan − 127) |
| mantissa | 23 | `1.` dan keyingi kasr qismi (boshidagi `1.` **saqlanmaydi** — u har doim bor) |

Masalan `-2.5 = -1.25 × 2¹`: ishora 1, daraja `1 + 127 = 128 = 10000000`, mantissa `.25 = 01000...`. **Maxsus qiymatlar:** daraja hammasi 1 (`0xFF`) → cheksizlik yoki NaN; daraja 0 → nol yoki **denormal** (juda kichik sonlar).

### Dastur

```c
/* float_lab.c - IEEE 754 laboratoriyasi: bitlarni ajratish, yaxlitlash xatosi, ULP, yig'ish aniqligi */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* float ning 32 bitini butun songa "o'qiymiz" (turni o'zgartirmasdan: memcpy - Asos bob, A.7) */
static uint32_t bitlar(float f)
{
    uint32_t b;
    memcpy(&b, &f, sizeof(b));
    return b;
}

static void ikkilik(uint32_t v, int bit_soni)
{
    for (int i = bit_soni - 1; i >= 0; i--)
        putchar((v >> i) & 1 ? '1' : '0');
}

/* float ni ishora (1 bit) | daraja (8 bit) | mantissa (23 bit) ga ajratib tushuntiradi */
static void ajrat(float f)
{
    uint32_t b = bitlar(f);
    uint32_t ishora = b >> 31;
    uint32_t daraja = (b >> 23) & 0xFF;
    uint32_t mantissa = b & 0x7FFFFF;

    printf("%-14g 0x%08X  ", f, b);
    printf("%u ", ishora);
    ikkilik(daraja, 8);
    putchar(' ');
    ikkilik(mantissa, 23);

    const char *tur;
    if (daraja == 0xFF)
        tur = mantissa ? "NaN (son emas)" : (ishora ? "-cheksizlik" : "+cheksizlik");
    else if (daraja == 0)
        tur = mantissa ? "denormal (juda kichik)" : "nol";
    else
        tur = "oddiy";
    printf("  %s", tur);
    if (daraja != 0 && daraja != 0xFF)
        printf(", 2^%d * 1.%06X(hex)", (int)daraja - 127, mantissa << 1);
    printf("\n");
}

/* ikki float orasida nechta "qadam" (ULP) bor: bitlarni tartibli sonlar kabi ayiramiz */
static long ulp_farq(float a, float b)
{
    int32_t x = (int32_t)bitlar(a), y = (int32_t)bitlar(b);
    return (long)y - (long)x;
}

/* oddiy yig'ish va Kahan (xatoni eslab qoluvchi) yig'ish */
static float oddiy_yigindi(float qiymat, long n)
{
    float s = 0;
    for (long i = 0; i < n; i++)
        s += qiymat;
    return s;
}

static float kahan_yigindi(float qiymat, long n)
{
    float s = 0, tuzatish = 0;                  /* tuzatish - oldingi qadamlarda yo'qotilgan qism */
    for (long i = 0; i < n; i++) {
        float y = qiymat - tuzatish;
        float t = s + y;
        tuzatish = (t - s) - y;                 /* (t - s) - y: qo'shishda yo'qotilgan xato */
        s = t;
    }
    return s;
}

int main(void)
{
    printf("1) float bitlari: ishora | daraja(8) | mantissa(23)\n");
    float sinov[] = { 1.0f, 0.5f, -2.5f, 0.1f, 16777216.0f, 1e-40f, INFINITY, NAN };
    for (int i = 0; i < 8; i++) {
        printf("  ");
        ajrat(sinov[i]);
    }

    printf("\n2) 0.1 aniq saqlanmaydi:\n");
    printf("  0.1f           = %.20f\n", (double)0.1f);
    printf("  0.1  (double)  = %.20f\n", 0.1);
    printf("  0.1 + 0.2      = %.20f\n", 0.1 + 0.2);
    printf("  0.3            = %.20f\n", 0.3);
    printf("  0.1 + 0.2 == 0.3 ? %s\n", 0.1 + 0.2 == 0.3 ? "ha" : "YO'Q");
    printf("  |farq| < 1e-9 ?    %s   (to'g'ri taqqoslash: epsilon bilan)\n", fabs(0.1 + 0.2 - 0.3) < 1e-9 ? "ha" : "yo'q");

    printf("\n3) katta sonlarda butun sonlar ham yo'qoladi (float 24 bit mantissa):\n");
    float f = 16777216.0f;                      /* 2^24 */
    printf("  16777216 + 1 = %.0f (float'da 1 qo'shilmadi!)\n", f + 1.0f);
    printf("  16777216 + 2 = %.0f\n", f + 2.0f);

    printf("\n4) ULP: qo'shni float'lar orasidagi masofa\n");
    printf("  1.0f dan keyingi float: %.10f (nextafterf), ULP farqi: %ld\n", nextafterf(1.0f, 2.0f),
           ulp_farq(1.0f, nextafterf(1.0f, 2.0f)));
    printf("  1.0f va 1.000001f orasida %ld ta float bor\n", ulp_farq(1.0f, 1.000001f) - 1);
    printf("  float epsilon (1.0 dan keyingi qadam): %g\n", (double)(nextafterf(1.0f, 2.0f) - 1.0f));

    printf("\n5) 0.1 ni 10 million marta qo'shish (to'g'ri javob: 1000000):\n");
    long n = 10000000;
    printf("  oddiy yig'ish: %.1f   (xato: %.1f)\n", (double)oddiy_yigindi(0.1f, n), 1000000.0 - (double)oddiy_yigindi(0.1f, n));
    printf("  Kahan yig'ish: %.1f   (xato: %.1f)\n", (double)kahan_yigindi(0.1f, n), 1000000.0 - (double)kahan_yigindi(0.1f, n));
    return 0;
}
```

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `bitlar(f)` | `memcpy` bilan `float` bitlarini `uint32_t` ga **ko'chiradi** (turni o'zgartirmasdan; `*(uint32_t*)&f` — qat'iy aliasing qoidasini buzadi, UB) |
| `ajrat(f)` | `>> 31`, `>> 23 & 0xFF`, `& 0x7FFFFF` — uch qismni **siljitish va maska** bilan ajratadi (3-bob!) |
| `ulp_farq(a, b)` | musbat floatlar uchun bitlar **tartibli** (kattaroq son = kattaroq bit-qiymat), shuning uchun bitlarni ayirish — orada nechta float borligini beradi |
| `kahan_yigindi()` | **Kahan yig'indisi**: har qo'shishda yo'qotilgan xatoni `tuzatish` da saqlaydi va keyingi qadamda qaytaradi |

Yig'amiz va ishga tushiramiz (`-ffp-contract=off` — kompilyator `a*b+c` ni birlashtirib natijani o'zgartirmasligi uchun):

```console
$ cd katta_loyiha/tizim/20_float_lab
$ gcc -Wall -Wextra -O2 -ffp-contract=off float_lab.c -o float_lab -lm
$ ./float_lab
1) float bitlari: ishora | daraja(8) | mantissa(23)
  1              0x3F800000  0 01111111 00000000000000000000000  oddiy, 2^0 * 1.000000(hex)
  0.5            0x3F000000  0 01111110 00000000000000000000000  oddiy, 2^-1 * 1.000000(hex)
  -2.5           0xC0200000  1 10000000 01000000000000000000000  oddiy, 2^1 * 1.400000(hex)
  0.1            0x3DCCCCCD  0 01111011 10011001100110011001101  oddiy, 2^-4 * 1.99999A(hex)
  1.67772e+07    0x4B800000  0 10010111 00000000000000000000000  oddiy, 2^24 * 1.000000(hex)
  9.99995e-41    0x000116C2  0 00000000 00000010001011011000010  denormal (juda kichik)
  inf            0x7F800000  0 11111111 00000000000000000000000  +cheksizlik
  nan            0x7FC00000  0 11111111 10000000000000000000000  NaN (son emas)

2) 0.1 aniq saqlanmaydi:
  0.1f           = 0.10000000149011611938
  0.1  (double)  = 0.10000000000000000555
  0.1 + 0.2      = 0.30000000000000004441
  0.3            = 0.29999999999999998890
  0.1 + 0.2 == 0.3 ? YO'Q
  |farq| < 1e-9 ?    ha   (to'g'ri taqqoslash: epsilon bilan)

3) katta sonlarda butun sonlar ham yo'qoladi (float 24 bit mantissa):
  16777216 + 1 = 16777216 (float'da 1 qo'shilmadi!)
  16777216 + 2 = 16777218

4) ULP: qo'shni float'lar orasidagi masofa
  1.0f dan keyingi float: 1.0000001192 (nextafterf), ULP farqi: 1
  1.0f va 1.000001f orasida 7 ta float bor
  float epsilon (1.0 dan keyingi qadam): 1.19209e-07

5) 0.1 ni 10 million marta qo'shish (to'g'ri javob: 1000000):
  oddiy yig'ish: 1087937.0   (xato: -87937.0)
  Kahan yig'ish: 1000000.0   (xato: 0.0)
```

**Nima ko'rdik:**

1. **Bitlar.** `1.0` → `0x3F800000`: ishora 0, daraja `01111111` (=127 → 2⁰), mantissa nol. `0.1` → mantissa `10011001100110011001101` — `1001` takrorlanadi, oxirida **yaxlitlangan** (`...1100` → `...1101`): ana **cheksiz kasrning kesilishi**. `inf` va `nan` — daraja hammasi 1, `NaN` ning mantissasi nolmas.
2. **0.1 aniq emas.** `0.1f = 0.10000000149...` — haqiqiy 0.1 dan biroz **katta**. `double` da yaxshiroq, lekin baribir aniq emas. `0.1 + 0.2 == 0.3` → **YO'Q**. To'g'ri taqqoslash: `fabs(a - b) < epsilon`.
3. **Katta sonlar.** `16777216 + 1 = 16777216` — `float` ning mantissasi 24 bit (shu jumladan yashirin `1`), 2²⁴ dan boshlab **qo'shni sonlar orasi 2 ga teng**: `+1` yaxlitlanib yo'qoladi, `+2` esa ishlaydi. Pulni `float` da **hech qachon** saqlamang (2-bob).
4. **ULP.** `1.0f` dan keyingi son `1.0000001192`: **ULP** (units in the last place) — qo'shni floatlar orasidagi masofa, `1.0` atrofida ≈ `1.19e-7`. `ulp_farq` = 1 — bitlar **ketma-ket** qo'shni.
5. **10 million qo'shish.** `0.1f` ni 10 000 000 marta qo'shsak — `1087937` chiqadi (to'g'ri javob `1000000`, xato **−87937**). Sabab: yig'indi katta bo'lgach, `0.1` qo'shish natijasi yaxlitlanadi va xato **to'planadi**. **Kahan** esa aniq `1000000.0` beradi: yo'qotilgan qismni `tuzatish` da eslab qoladi.

> **Eslab qoling:** `float` = `±1.mantissa × 2^daraja`. Ko'p kasrlar (0.1) aniq saqlanmaydi → `==` bilan **solishtirmang**, `epsilon` ishlating. Katta sonlarda **kichik qo'shilmalar yo'qoladi**. Uzun yig'indilarda xato **to'planadi** — Kahan yoki katta o'lchamli tur (`double`/`long double`).

**O'zingiz qo'shing (yechimsiz):**

1. `ajrat()` ga `0.15625f`, `-0.0f`, `3.4e38f * 10` ni bering. `-0.0f` ning bitlari qanday? `0.0f == -0.0f` nima beradi?
2. `double` uchun ham shu ishni qiling: daraja **11 bit**, mantissa **52 bit**, siljish **1023**. `uint64_t` va `memcpy` dan foydalaning.
3. `float` da qaysi **eng kichik musbat butun** `n` uchun `n + 1 == n` bo'ladi? Dasturda **topib** isbotlang (maslahat: sikl, `float` o'zgaruvchi).
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. Pozitsion sanoq: ikkilik (1, 2, 4, 8...), hex raqam = **4 bit**; o'nlikdan ikkilikka — ketma-ket 2 ga bo'lish (qoldiqlar pastdan yuqoriga).
2. **Ikkiga to'ldirish:** `-x = ~x + 1`; eng yuqori bit — "minus"; qo'shish ishorali va ishorasiz uchun **bir xil**; `-INT_MIN` sig'maydi.
3. Kengaytirish: ishorasiz → **nollar**, ishorali → **ishora biti** takrorlanadi; qirqish — pastki bitlar qoladi (`mod 2^w`). `getchar()` natijasini **`int`** da saqlang.
4. **IEEE 754:** `(-1)^S × 1.M × 2^(E−127)`; `0.1` aniq emas; `==` bilan solishtirmang; ±∞, NaN, denormal maxsus qiymatlar; 2²⁴ dan katta butun sonlar `float` da aniq emas.
5. Yadroda kasr yo'q: **qat'iy nuqta** (butun son × masshtab); avval ko'paytiring, keyin bo'ling.

## Savol-javob

**Savol:** Nega `0.1 + 0.2 == 0.3` rost emas?
**Javob:** 0.1 va 0.2 ikkilik tizimda cheksiz kasr bo'lib chiqadi (xuddi 1/3 o'nlikda 0.333…), kompyuter ularni qisqartirib saqlaydi. Yig'indi 0.3 ga **juda yaqin**, lekin aynan teng emas. Shuning uchun kasrlarni `==` bilan emas, kichik farq (epsilon) bilan taqqoslang (20.6).

**Savol:** Nega pulni `float`/`double` bilan emas, butun tiyin bilan saqlaymiz?
**Javob:** Kasr xatolari yig'iladi (2-bobdagi 10 × 0.10 misoli). Butun sonlar esa aniq: 1000 tiyin = 10 so'm, hech qanday yaxlitlash xatosi yo'q.

**Savol:** Ikkiga to'ldirish (two's complement) nega qulay?
**Javob:** Qo'shish va ayirish ishorali ham, ishorasiz ham **bir xil** sxema (bir xil apparat) bilan bajariladi; nolning bitta yozilishi bor. Shuning uchun deyarli barcha protsessorlar shuni ishlatadi (20.3).

## O'zingizni tekshiring

1. `0xC0` ni ikkilik va o'nlikda yozing.
2. 8 bitli ikkiga to'ldirishda `1000 0001` nechaga teng?
3. `int8_t` −1 ni `uint32_t` ga aylantirsak nima chiqadi?
4. −1.0 float'ning bitlari (o'n oltilikda)?
5. Nega `for (float x = 0; x != 1.0; x += 0.1)` cheksiz sikl bo'lishi mumkin?

<details><summary>Javoblar</summary>

1. `1100 0000` = 192.
2. −128 + 1 = −127.
3. Avval `int` ga ishora kengayishi (−1), keyin ishorasizga: 4294967295 (0xFFFFFFFF).
4. S=1, E=127 (0111 1111), M=0 → `1 01111111 000...` = 0xBF800000.
5. 0.1 aniq tasvirlanmaydi — yig'indi hech qachon aynan 1.0 ga teng bo'lmasligi mumkin.
</details>

## Mashq

### Isitish: bitlar ichida sonlar ★☆☆ — eng osoni, avval shuni qiling

Faqat 0–20-boblar kerak (ikkiga to'ldirish, ishora kengayishi, IEEE 754, qat'iy nuqta).
Skeletni `isitish.c` ga **qo'lda** yozing (ko'chirmang), izohlarni o'qing va `TODO` joylarini to'ldiring.
"Namuna" qismlar tayyor — qolganini qanday yozishni ko'rsatadi. Skelet hozir ham ogohlantirishsiz yig'iladi:
har `TODO` dan keyin yig'ib, ishga tushirib boring.

```c
/* isitish.c - 20-bob, isitish: ikkiga to'ldirish, ishora kengayishi, qirqish, float bitlari, qat'iy nuqta. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    /* 1) -1 ning bitlari: 1111 1111. Ishorasiz o'qilsa - 255 (20.3). (Namuna - tayyor.) */
    int8_t m = -1;
    uint8_t u = (uint8_t)m;
    printf("-1 -> uint8_t: %u (0x%02x)\n", u, u);

    /* 2) TODO: int8_t kichik = (int8_t)0x80; int32_t katta = kichik;  - ishora kengayishi (20.4):
     *    yangi yuqori bitlar ishora biti (1) bilan to'ldiriladi -> 0xffffff80 = -128.
     *    printf("int8_t 0x80 -> int32_t: %d (0x%08x)\n", katta, (uint32_t)katta);
     *    Natija: int8_t 0x80 -> int32_t: -128 (0xffffff80) */

    /* 3) TODO: (uint8_t)300 - qirqish: faqat pastki 8 bit qoladi: 300 = 1 0010 1100 -> 0010 1100 = 44.
     *    Natija: 300 -> uint8_t: 44 */

    /* 4) float ning bitlari: memcpy bilan (union yoki ko'rsatkich cast emas - 13.3). 1.0 = 1.0 * 2^0:
     *    ishora 0, eksponenta 127 (0x7f), mantissa 0 -> 0x3f800000 (20.6). (Namuna - tayyor.) */
    float f = 1.0f;
    uint32_t bitlar;
    memcpy(&bitlar, &f, sizeof(bitlar));
    printf("1.0f bitlari: 0x%08x\n", bitlar);

    /* 5) TODO: 0.1 + 0.2 == 0.3 natijasini %d bilan chiqaring. 0.1 ikkilikda cheksiz kasr - aniq saqlanmaydi.
     *    Kasr sonlarni == bilan solishtirmang: fabs(a - b) < 1e-9.
     *    Natija: 0.1 + 0.2 == 0.3 ? 0 */

    /* 6) Q16.16 qat'iy nuqta (20.7): son * 65536 butun son sifatida. Ko'paytmada 32 bit kasr hosil bo'ladi -
     *    64 bitda ko'paytirib, 16 ga suramiz. Yadroda float yo'q - shuning uchun shunday hisoblanadi.
     *    TODO: int32_t r = (int32_t)(((int64_t)a * b) >> 16); keyin butun qism r >> 16,
     *          kasr qism ((r & 0xFFFF) * 100) >> 16 ni "%d.%02d" bilan chiqaring.
     *    Natija: Q16.16: 2.5 * 1.5 = 3.75 */
    int32_t a = (int32_t)(2.5 * 65536), b = (int32_t)(1.5 * 65536);
    (void)a;                            /* 6-qadamni yozgach, bu ikki qatorni o'chiring */
    (void)b;
    return 0;
}
```

**Kutilgan natija** (`darslik/loyihalar/20_qat_nuqta/isitish.txt`):

```text
-1 -> uint8_t: 255 (0xff)
int8_t 0x80 -> int32_t: -128 (0xffffff80)
300 -> uint8_t: 44
1.0f bitlari: 0x3f800000
0.1 + 0.2 == 0.3 ? 0
Q16.16: 2.5 * 1.5 = 3.75
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined isitish.c -o isitish
$ ./isitish | diff - ~/C_loyha/darslik/loyihalar/20_qat_nuqta/isitish.txt && echo "TO'G'RI"
TO'G'RI
```

### Keyingi mashqlar

- **41** (float bitlari) — float'ni sign/eksponenta/mantissaga ajratish va butun sonni qo'lda float'ga aylantirish (yaxlitlash bilan).
- **04** (bitlar) va **03** (toshish) — bu bob bilan qayta ko'ring.
- Qo'shimcha: `union { float f; uint32_t u; }` bilan 1.0, −2.5, 0.1 ning bitlarini chiqarib, qo'lda hisoblaganingiz bilan solishtiring.

<!-- loyiha:boshi -->
## Loyiha: sonlar konvertori

**Maqsad:** bir xil bitlar turli tur bilan qanday "o'qilishini" ko'rish: ishorali/ishorasiz, ikkiga to'ldirish, ishora
kengayishi, qirqish, va `float` ning ichki tuzilishi.
**Bobdan ishlatiladi:** ikkilik/o'n oltilik yozuv, ikkiga to'ldirish, ishora kengayishi, IEEE 754.

**Talab:** (1) `int8_t` qiymatlari jadvali: o'nlik, o'n oltilik, ikkilik, ishorasiz talqin; (2) kengayish va qirqish; (3) `float` ni
ishora/daraja/mantissaga ajratish.
**Asosiy fikr:** bir xil 8 bit — `11111011` — `int8_t` da `−5`, `uint8_t` da `251`. Ma'no — turda, bitlarda emas.

```c
/* sonlar.c - sonlar konvertori */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void ikkilik(uint32_t v, int bitlar)     /* har 4 bitdan keyin '_' */
{
    for (int b = bitlar - 1; b >= 0; b--) {
        putchar((v >> b) & 1 ? '1' : '0');
        if (b % 4 == 0 && b > 0)
            putchar('_');
    }
}

static void float_tahlil(float f)
{
    uint32_t u;
    memcpy(&u, &f, sizeof(u));                  /* bitlarni butun son sifatida olamiz (13-bob: type punning) */
    unsigned ishora = u >> 31, daraja = (u >> 23) & 0xFF, mantissa = u & 0x7FFFFF;
    printf("%6.2f = 0x%08X: ishora %u, daraja %3u (2^%d), mantissa 0x%06X\n", (double)f, u, ishora, daraja,
           (int)daraja - 127, mantissa);
}

int main(void)
{
    printf("%-6s %-5s %-10s %s\n", "int8_t", "hex", "ikkilik", "uint8_t deb o'qilsa");
    int8_t qiymatlar[] = { 0, 5, 127, -128, -5, -1 };
    for (int i = 0; i < 6; i++) {
        uint8_t u = (uint8_t)qiymatlar[i];
        printf("%6d 0x%02X  ", qiymatlar[i], u);
        ikkilik(u, 8);
        printf("  %u\n", u);
    }

    printf("\nKengayish va qirqish:\n");
    int8_t manfiy = -5;
    uint8_t katta = 251;
    printf("  int8_t -5    -> int32_t : %d (0x%08X)   [ishora kengayadi]\n", (int32_t)manfiy, (uint32_t)(int32_t)manfiy);
    printf("  uint8_t 251  -> uint32_t: %u (0x%08X)   [nol bilan to'ldiriladi]\n", (uint32_t)katta, (uint32_t)katta);
    printf("  int 300      -> uint8_t : %u    [yuqori bitlar tashlandi: 300 mod 256]\n", (uint8_t)300);
    printf("  int 200      -> int8_t  : %d   [bitlar o'sha, talqin boshqa]\n", (int8_t)200);

    printf("\nfloat ning ichida:\n");
    float_tahlil(6.5f);
    float_tahlil(-0.75f);
    float_tahlil(0.1f);
    float_tahlil(1.0f);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g sonlar.c -o sonlar
$ ./sonlar
int8_t hex   ikkilik    uint8_t deb o'qilsa
     0 0x00  0000_0000  0
     5 0x05  0000_0101  5
   127 0x7F  0111_1111  127
  -128 0x80  1000_0000  128
    -5 0xFB  1111_1011  251
    -1 0xFF  1111_1111  255

Kengayish va qirqish:
  int8_t -5    -> int32_t : -5 (0xFFFFFFFB)   [ishora kengayadi]
  uint8_t 251  -> uint32_t: 251 (0x000000FB)   [nol bilan to'ldiriladi]
  int 300      -> uint8_t : 44    [yuqori bitlar tashlandi: 300 mod 256]
  int 200      -> int8_t  : -56   [bitlar o'sha, talqin boshqa]

float ning ichida:
  6.50 = 0x40D00000: ishora 0, daraja 129 (2^2), mantissa 0x500000
 -0.75 = 0xBF400000: ishora 1, daraja 126 (2^-1), mantissa 0x400000
  0.10 = 0x3DCCCCCD: ishora 0, daraja 123 (2^-4), mantissa 0x4CCCCD
  1.00 = 0x3F800000: ishora 0, daraja 127 (2^0), mantissa 0x000000
```

`6.5 = 1.625 × 2²` — daraja `129 − 127 = 2`, mantissa `0.625` ning bitlari. `0.1` ning mantissasi `0x4CCCCD` — cheksiz `0.0001100110011…`
kasrning qirqilgani. Shuning uchun `0.1` **aniq** saqlanmaydi (20.6).

**Kengaytiring:** `double` uchun ham shunday yozing (11 bit daraja, 52 bit mantissa). `0.0f`, `-0.0f` va `1.0f/0.0f` ning bitlariga qarang.

## Mustaqil loyiha: Q16.16 qat'iy nuqtali kalkulyator ★★★

**Vazifa:** yadroda `float` **ishlatilmaydi** (20.7). Kasr sonlar butun sonda saqlanadi: `int32_t` da yuqori 16 bit — butun qism,
pastki 16 bit — kasr qism. `1.0 = 65536 (0x10000)`. **Hisob-kitobning o'zida `float`/`double` taqiqlangan.** Fayl: `qat.c`.

**Turi:** `typedef int32_t q16;`

**Funksiyalar** (qoidalar aniq — natija shularga bog'liq):
- `q16 q_kasr(int surat, int maxraj)` — `surat/maxraj`. Musbat uchun **yaxlitlab**: `(surat·65536 + maxraj/2) / maxraj`
  (`int64_t` da). Manfiy surat uchun: `-q_kasr(-surat, maxraj)`.
- `q16 q_qosh(q16 a, q16 b)`, `q16 q_ayir(q16 a, q16 b)` — oddiy `+`, `−`.
- `q16 q_kop(q16 a, q16 b)` — `((int64_t)a * b) >> 16` (arifmetik siljitish: pastga yaxlitlanadi).
- `q16 q_bol(q16 a, q16 b)` — `((int64_t)a << 16) / b` (C bo'lishi: nolga qarab yaxlitlanadi).
- `q16 q_sqrt(q16 x)` — Nyuton: `y = x` dan boshlab **aynan 12 marta** `y = (y + q_bol(x, y)) >> 1`. (`x > 0` deb oling.)
- `void q_chiqar(q16 x)` — **4 xona** kasr bilan, faqat butun arifmetika: kasr qismi `f` (0..65535) uchun
  raqamlar = `(f · 10000 + 32768) / 65536`; agar `10000` chiqsa — butun qismga `1` qo'shib kasr `0000`. Manfiy sonda ishora
  alohida (`-` chiqarib, moduli bilan ishlang). Format: `-4.1250`.

**Sinovlar** (`main` da, har biri bitta satr; kirish qiymatlari `q_kasr` bilan yasaladi):

| Chiqishi | Hisob |
|---|---|
| `3.5 * 2.25 =` | `q_kop(7/2, 9/4)` |
| `10 / 3 =` | `q_bol(10/1, 3/1)` |
| `sqrt(2) =` | `q_sqrt(2/1)` |
| `-2.75 * 1.5 =` | `q_kop(-11/4, 3/2)` |
| `0.1 + 0.2 =` | `q_qosh(1/10, 2/10)` |
| `1/3 + 1/3 + 1/3 =` | uch marta `q_kasr(1,3)` yig'indisi |
| `sqrt(0.25) =` | `q_sqrt(1/4)` |

**Kutilgan natija** (`darslik/loyihalar/20_qat_nuqta/kutilgan.txt`):

```text
3.5 * 2.25 = 7.8750
10 / 3 = 3.3333
sqrt(2) = 1.4142
-2.75 * 1.5 = -4.1250
0.1 + 0.2 = 0.3000
1/3 + 1/3 + 1/3 = 1.0000
sqrt(0.25) = 0.5000
```

**Maslahat** (yechim emas):
- `int64_t` oraliq natija — 32 bitli ko'paytma toshib ketadi. Nega `>> 16`?
- `1/3+1/3+1/3` `1.0000` chiqishi kerak, ammo ichida `65535` (1.0 dan bitta bit kam) — bu **yaxlitlash kaskadi** va `q_chiqar` dagi `10000` holati.
  Uni ushlamasangiz `0.9999` yoki `0.10000` chiqadi.
- Manfiy `q16` ning kasr qismini olishda `x & 0xFFFF` xato beradi. Avval `-x` ga o'tib, ishorani alohida saqlang.
- `q_sqrt` da bo'lish `q_bol` orqali — `x / y` emas!
- `-fsanitize=undefined` bilan yig'ing: chap siljitishda manfiy son UB. `(int64_t)a << 16` — `a` allaqachon 64 bitga o'tgan.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined qat.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/20_qat_nuqta/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [21-bob. Xotira ierarxiyasi va kesh](21-kesh.md)
