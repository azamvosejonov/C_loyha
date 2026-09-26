# 2-bob. O'zgaruvchilar va turlar

> **Bu bobdan keyin:** har bir tur xotirada necha bayt egallashini, nega `int` toshib ketishini,
> `signed`/`unsigned` farqini va yadroda nega `uint32_t` kabi aniq o'lchamli turlar ishlatilishini bilasiz.
> Mashqlar: 01, 02, 03.

## 2.1. O'zgaruvchi nima

Python'da o'zgaruvchi — obyektga yopishtirilgan **nom** (yorliq). C'da o'zgaruvchi — xotiradagi
**aniq o'lchamli quti**, nomi bilan:

```c
int yosh = 20;
```

Bu qator kompilyatorga uchta narsani aytadi:

1. **Tur** — `int`: quti 4 bayt, ichida ishorali butun son.
2. **Nom** — `yosh`: kodda shu nom bilan murojaat qilinadi. Kompilyatsiyadan keyin nom yo'qoladi —
   faqat manzil qoladi.
3. **Boshlang'ich qiymat** — `= 20` (ixtiyoriy, lekin **doim** yozing — pastga qarang).

```text
manzil:   0x7ffd1000  0x7ffd1001  0x7ffd1002  0x7ffd1003
yosh:     [   20    ][    0     ][    0     ][    0     ]   (x86 - "kichik uchi oldin", 16-bob)
```

**Savol: nega turni yozish kerak?**
Kompilyator: 1) qancha xotira ajratishni, 2) qaysi CPU buyrug'ini ishlatishni (butun son qo'shish
va kasr son qo'shish — boshqa-boshqa buyruqlar), 3) xatolarni ushlashni bilishi uchun. Python bu
ma'lumotni har bir obyekt ichida ishga tushirish paytida saqlaydi va tekshiradi — shuning uchun sekin.

**Savol: tur o'zgarishi mumkinmi?**
Yo'q. `int x` doim `int`. Python'dagi `x = 5; x = "salom"` C'da mumkin emas.

## 2.2. Boshlang'ich qiymatsiz o'zgaruvchi — xavfli

```c
int s;              /* qiymati NOMA'LUM - xotirada oldin nima qolgan bo'lsa, o'sha */
s = s + 1;          /* aniqlanmagan xatti-harakat */
```

Funksiya ichidagi (lokal) o'zgaruvchi avtomatik **nollanmaydi**. Unda "axlat" — xotirada oldin
turgan qiymat bo'ladi. Dastur bir kompyuterda ishlaydi, boshqasida — yo'q. Qoida:
**har bir o'zgaruvchiga e'lon qilgan joyda qiymat bering.**

(Global va `static` o'zgaruvchilar esa avtomatik 0 bo'ladi — 8-bob.)

## 2.3. Butun son turlari

| Tur | Hajm (x86-64 Linux) | Oraliq | Qachon |
|---|---|---|---|
| `char` | 1 bayt | −128..127 (x86'da) | Belgi, bayt |
| `short` | 2 | −32 768..32 767 | Kam ishlatiladi |
| `int` | 4 | −2 147 483 648..2 147 483 647 | Oddiy hisoblagichlar |
| `long` | 8 | ±9.2·10¹⁸ | Katta sonlar (Windows'da esa 4 bayt!) |
| `long long` | 8 | ±9.2·10¹⁸ | Har doim kamida 64 bit |
| `unsigned int` | 4 | 0..4 294 967 295 | Manfiy bo'lmaydigan qiymatlar |
| `size_t` | 8 | 0..1.8·10¹⁹ | **Hajm va indekslar** (`sizeof`, `strlen`, `malloc` shuni ishlatadi) |

**Savol: nega `int` ning hajmi "taxminan"?**
C standarti faqat **minimal** hajmni kafolatlaydi (`int` ≥ 16 bit, `long` ≥ 32 bit). Haqiqiy
o'lcham platformaga bog'liq: `long` Linux'da 8 bayt, Windows'da 4 bayt. Yadroda bu qabul qilib
bo'lmaydi — registr aniq 32 bit bo'lsa, tur ham aniq 32 bit bo'lishi kerak. Yechim — pastdagi
`<stdint.h>`.

### `<stdint.h>` — aniq o'lchamli turlar

```c
#include <stdint.h>
uint8_t  bayt = 0xFF;          /* aniq 8 bit, ishorasiz */
uint16_t port = 0x3F8;
uint32_t registr = 0;
uint64_t manzil = 0xFFFFFFFF80000000;
int32_t  farq = -5;            /* aniq 32 bit, ishorali */
uintptr_t p;                   /* ko'rsatkich sig'adigan butun son */
```

`u` — unsigned, raqam — bitlar soni, `_t` — "type". **Yadroda deyarli faqat shular ishlatiladi**,
chunki apparat tuzilmalari (sahifa jadvali yozuvi — aniq 64 bit, ELF sarlavhasi, ext2 superbloki)
bayt-baybayt aniq bo'lishi kerak. MyOS'dagi har qanday `struct` ni oching — `uint32_t`, `uint64_t` ni ko'rasiz.

### `sizeof` — hajmni bilish

```c
printf("%zu %zu %zu\n", sizeof(int), sizeof(long), sizeof(uint16_t));   /* 4 8 2 */
int a[10];
printf("%zu\n", sizeof(a));           /* 40 - butun massiv */
```

`sizeof` — funksiya emas, **operator**: natija kompilyatsiya paytida hisoblanadi (ish vaqtida hech
narsa bajarilmaydi). Natija turi — `size_t`, `printf` da `%zu`.

## 2.4. Signed va unsigned — ishora biti

Ishorali (`signed`) sonlar **ikkiga to'ldirish** (two's complement) usulida saqlanadi. 8 bit uchun:

```text
 0000 0000 =    0          1000 0000 = -128
 0000 0001 =    1          1111 1110 =   -2
 0111 1111 =  127          1111 1111 =   -1
```

Eng yuqori bit — "manfiy" belgisi. `-1` ning hamma bitlari 1. Nega shunday? Chunki qo'shish uchun
alohida "manfiy" sxema kerak bo'lmaydi: `1111 1111 + 0000 0001 = 1 0000 0000` → ortiqcha bit tashlanadi
→ `0000 0000` = 0. Ya'ni −1 + 1 = 0 oddiy ikkilik qo'shish bilan chiqadi.

Xuddi shu 8 bit `unsigned` sifatida: `1111 1111` = 255. **Bitlar bir xil, talqin boshqa.**

```c
unsigned char u = 255;
signed char s = (signed char)u;     /* -1 : bitlar o'sha, faqat boshqacha "o'qildi" */
```

## 2.5. Toshish (overflow) — Python'da yo'q, C'da bor

```c
unsigned int u = 4294967295u;  /* eng katta */
u = u + 1;                     /* 0 - ishorasizda "aylanib" qoladi (bu ANIQLANGAN xatti-harakat) */

int i = 2147483647;            /* INT_MAX */
i = i + 1;                     /* ANIQLANMAGAN XATTI-HARAKAT (UB)! */
```

- **Ishorasiz** toshish — standart bo'yicha "modulli arifmetika": 2³² ga bo'lingan qoldiq. Xavfsiz,
  lekin ko'pincha mantiqiy xato (masalan, `size_t n = 0; n - 1` → 18446744073709551615).
- **Ishorali** toshish — **UB**. Kompilyator "bu hech qachon bo'lmaydi" deb faraz qilib kodni
  o'zgartirishi mumkin. Natija oldindan aytib bo'lmaydi. 13-bobda batafsil, 03-mashqda amalda.

```c
long s = 0;
for (int i = 1; i <= 100000; i++)
    s += i * i;             /* XATO: i * i int'da hisoblanadi va 46341 dan boshlab toshadi */

for (int i = 1; i <= 100000; i++)
    s += (long)i * i;       /* TO'G'RI: avval long ga o'tkazib, keyin ko'paytirish */
```

**Muhim tushuncha:** `s` ning turi `long` bo'lishi `i * i` ni qutqarmaydi. Ifoda **o'z operandlari
turida** hisoblanadi (`int * int` → `int`), natija keyin `long` ga aylantiriladi — lekin u allaqachon
toshib bo'lgan. 01-mashq aynan shu tuzoq.

## 2.6. Chegaralar: `<limits.h>`

```c
#include <limits.h>
INT_MAX   INT_MIN   UINT_MAX   LONG_MAX   LONG_MIN   CHAR_BIT (= 8)
```

`<stdint.h>` da: `UINT32_MAX`, `INT64_MIN`, `SIZE_MAX` va hokazo.

## 2.7. Kasr sonlar: `float`, `double`

```c
double pi = 3.14159;
float f = 1.5f;             /* f - float literali */
printf("%.3f\n", pi);       /* 3.142 */
```

- `double` — 8 bayt, ~15 xona aniqlik. `float` — 4 bayt, ~7 xona.
- `0.1 + 0.2 != 0.3` — Python'dagi kabi (ikkilik kasr).
- **Yadroda kasr sonlar deyarli ishlatilmaydi.** Nega: FPU/SSE registrlarini har bir kontekst
  almashishda saqlash qimmat, shuning uchun yadro ularni umuman ishlatmaydi (MyOS: `-mgeneral-regs-only`).
  Vaqt, foizlar — hammasi butun sonlarda (millisekund, "mingdan bir").

## 2.8. `char` — belgi ham, son ham

```c
char c = 'A';               /* aslida 65 (ASCII kodi) */
printf("%c %d\n", c, c);    /* A 65 */
char d = c + 1;             /* 'B' */
int raqam = '7' - '0';      /* 7 - raqam belgisini songa aylantirishning klassik usuli */
```

- `'A'` (bitta tirnoq) — **bitta belgi**, ya'ni son.
- `"A"` (qo'sh tirnoq) — **satr**: 2 baytli massiv `{'A', '\0'}`. Adashtirish — klassik xato.
- `char` ning ishorali yoki ishorasizligi platformaga bog'liq (x86 — ishorali, ARM — ishorasiz).
  Baytlar bilan ishlaganda doim `unsigned char` yoki `uint8_t` yozing (08-mashqdagi tuzoq).

## 2.9. Literallar

| Yozuv | Ma'nosi |
|---|---|
| `42` | `int` |
| `42u` | `unsigned int` |
| `42L`, `42UL` | `long`, `unsigned long` |
| `42LL`, `42ULL` | `long long` |
| `0x2A` | o'n oltilik (= 42) — **yadroda eng ko'p** |
| `052` | **sakkizlik** (= 42)! Oldidagi `0` sakkizlik degani — `010` bu 8, 10 emas |
| `0b101010` | ikkilik (GCC kengaytmasi, C23 da standart) |
| `'\n'`, `'\0'`, `'\x1b'` | maxsus belgilar |
| `1e6` | `double` (1000000.0) |

## 2.10. `const` — o'zgarmas

```c
const int MAX = 100;
MAX = 5;                    /* xato: assignment of read-only variable */
```

`const` — "bu qiymatni o'zgartirmayman" degan va'da, kompilyator uni tekshiradi. Ko'rsatkichlarda
eng foydali: `size_t strlen(const char *s)` — "men sizning satringizni o'zgartirmayman" (7-bob).

## 2.11. Turlarni aylantirish (conversion va cast)

```c
int a = 7, b = 2;
double d1 = a / b;              /* 3.0 ! - int / int = int (3), keyin double ga */
double d2 = (double)a / b;      /* 3.5 - avval a ni double ga (cast) */
```

- `(tur)ifoda` — **cast**: aniq aylantirish.
- Aralash ifodada kichik tur kattasiga ko'tariladi (`int + long` → `long`, `int + double` → `double`).
- **Xavfli joy — signed va unsigned aralashganda:**

```c
int x = -1;
unsigned int u = 1;
if (x < u) printf("kichik\n"); else printf("katta!\n");     /* "katta!" chiqadi */
```

`x` `unsigned` ga aylantiriladi → 4294967295 → u 1 dan katta. `-Wextra` bu haqda ogohlantiradi
(`comparison of integer expressions of different signedness`). Shuning uchun ogohlantirishlarni
yoqish shart.

## 2.12. Ko'rinish sohasi (scope)

```c
int global = 1;                 /* butun faylda (va boshqa fayllarda ham, extern bilan) */

int main(void)
{
    int x = 2;                  /* main ichida */
    {
        int y = 3;              /* faqat shu { } ichida */
        int x = 4;              /* ICHKI x tashqisini "yashiradi" (shadowing) - chalkash, yozmang */
    }
    /* y bu yerda yo'q */
    return 0;
}
```

`{ }` — faqat blok emas, **yangi ko'rinish sohasi**. Python'da `if` ichida yaratilgan o'zgaruvchi
tashqarida ham ko'rinadi; C'da — yo'q.

## 2.13. Savol-javob

**Qachon `int`, qachon `long`, qachon `size_t`?**
Hisoblagich va kichik sonlar — `int`. Hajm, uzunlik, massiv indeksi — `size_t`. Katta hisob-kitob —
`long`/`int64_t`. Apparat bilan ishlash (registrlar, disk tuzilmalari) — `uint8_t..uint64_t`.

**Nega `size_t` ishorasiz?**
Hajm manfiy bo'lmaydi va ishorasiz tur ikki baravar katta qiymatni sig'diradi. Lekin tuzog'i bor:
`for (size_t i = n - 1; i >= 0; i--)` — **cheksiz sikl**, chunki `i >= 0` doim rost (05-mashq).

**`0x` sonlarni qanday tez o'qiyman?**
Har bir o'n oltilik raqam — 4 bit. `0xFF` = 8 bit hammasi 1. `0x1000` = 4096 (sahifa hajmi).
`0xFFFFFFFF80000000` — MyOS yadrosining boshlanish manzili. Yadroda 16-lik tizimda o'ylashga o'rganing.

## 2.14. O'zingizni tekshiring

1. `int` qancha bayt? `long` Linux'da va Windows'da?
2. `unsigned char c = 200; c = c + 100;` — `c` nechaga teng?
3. `7 / 2` va `7.0 / 2` natijasi?
4. `'5' - '0'` nechaga teng va nega?
5. `010` nechaga teng?
6. `long s = 50000 * 50000;` — nima xato?

<details><summary>Javoblar</summary>

1. 4; Linux'da 8, Windows'da 4.
2. 44 (300 − 256): ishorasiz toshish modulli.
3. 3 va 3.5.
4. 5: raqam belgilari ASCII'da ketma-ket ('0'=48, '5'=53).
5. 8 (sakkizlik).
6. `50000 * 50000` `int` da hisoblanadi va toshadi (UB); `50000L * 50000` yozish kerak.
</details>

## 2.15. Mashqlar

- **01** (kvadratlar yig'indisi) — tur va toshish.
- **02** (tub sonlar) — `unsigned` va toshmaydigan shart.
- **03** (toshishsiz arifmetika) — chegaralar va UB.
- Qo'shimcha: `sizeof` bilan barcha turlarning hajmini chiqaradigan dastur yozing.

Keyingi bob: [3-bob. Operatorlar](03-operatorlar.md)
