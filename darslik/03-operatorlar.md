# 3-bob. Operatorlar

> **Bu bobdan keyin:** C'dagi har bir operator belgisini (`%`, `&&`, `&`, `<<`, `?:`, `++` ...) bilasiz,
> ularning ustuvorligidagi tuzoqlarni taniysiz va bitlar bilan ishlashni boshlaysiz.
> Mashqlar: 02, 03, 04.

## 3.1. Arifmetika

| Operator | Ma'nosi | E'tibor |
|---|---|---|
| `+ - *` | qo'shish, ayirish, ko'paytirish | Toshish mumkin (2-bob) |
| `/` | bo'lish | **Ikkala operand butun bo'lsa — butun bo'lish**: `7 / 2 == 3`. Nolga tomon kesiladi: `-7 / 2 == -3` (Python'da `-7 // 2 == -4`!) |
| `%` | qoldiq | Ishorasi bo'linuvchiniki: `-7 % 2 == -1` (Python'da `1`) |
| `-x` | ishorani o'zgartirish | `-INT_MIN` — toshish (UB) |

Nolga bo'lish (`x / 0`, `x % 0`) — UB, amalda dastur `SIGFPE` bilan qulaydi. Yadroda esa bu
**#DE istisnosi** — CPU yadroning uzilish ishlovchisini chaqiradi (MyOS: `kernel/arch/interrupts.c`).

## 3.2. Taqqoslash

`==` `!=` `<` `>` `<=` `>=` — natija `int`: rost = **1**, yolg'on = **0**.

C'da alohida mantiqiy tur tarixan yo'q edi: **0 — yolg'on, noldan farqli har qanday son — rost**.
`<stdbool.h>` dagi `bool`, `true`, `false` — shunchaki 1 va 0.

```c
if (n)          /* n != 0 bilan bir xil */
if (!p)         /* p == NULL bilan bir xil */
```

### Eng mashhur xato: `=` va `==`

```c
if (x = 0) { ... }      /* x ga 0 YOZADI, shart doim yolg'on */
if (x == 0) { ... }     /* taqqoslaydi */
```

`-Wall` ogohlantiradi: `suggest parentheses around assignment used as truth value`. Ataylab
qilinganda ikkita qavs yoziladi: `while ((c = getchar()) != EOF)`.

## 3.3. Mantiqiy operatorlar: `&&`, `||`, `!`

| | Ma'nosi |
|---|---|
| `a && b` | VA — ikkalasi ham rost bo'lsa 1 |
| `a \|\| b` | YOKI — hech bo'lmasa bittasi rost bo'lsa 1 |
| `!a` | EMAS |

**Qisqa tutashuv (short-circuit) — juda muhim:**

```c
if (p != NULL && p->qiymat > 0)     /* p NULL bo'lsa, o'ng tomon HISOBLANMAYDI - xavfsiz */
if (i < n && a[i] == x)              /* chegarani avval tekshirish - massivdan chiqmaslik */
```

`&&` da chap tomon yolg'on bo'lsa, o'ng tomon umuman bajarilmaydi. `||` da chap tomon rost bo'lsa —
o'ng tomon bajarilmaydi. Tartibni almashtirsangiz (`a[i] == x && i < n`) — massiv chegarasidan
tashqarini o'qiysiz. Yadroda bu uslub hamma joyda: `if (!page || !page->mapping)`.

## 3.4. Bitli operatorlar — yadroning tili

Bitli operatorlar sonning **har bir bitiga alohida** ishlaydi:

| Operator | Nomi | Misol (8 bit) |
|---|---|---|
| `a & b` | VA | `1100 & 1010 = 1000` |
| `a \| b` | YOKI | `1100 \| 1010 = 1110` |
| `a ^ b` | XOR (faqat biri) | `1100 ^ 1010 = 0110` |
| `~a` | teskari | `~1100 0000 = 0011 1111` |
| `a << n` | chapga surish (×2ⁿ) | `0000 0011 << 2 = 0000 1100` |
| `a >> n` | o'ngga surish (÷2ⁿ) | `0000 1100 >> 2 = 0000 0011` |

**`&` va `&&` farqi:** `6 & 1` = 0 (bitlarda umumiy 1 yo'q), `6 && 1` = 1 (ikkalasi ham noldan farqli).
Adashtirish ko'pincha kompilyatsiya bo'ladi va jim noto'g'ri ishlaydi.

### Bitlar bilan 5 ta asosiy "hunar"

```c
x |=  (1u << n);          /* n-bitni YOQISH  (1 qilish)        */
x &= ~(1u << n);          /* n-bitni O'CHIRISH (0 qilish)      */
x ^=  (1u << n);          /* n-bitni ALMASHTIRISH              */
(x >> n) & 1u             /* n-bitni O'QISH (0 yoki 1)          */
x & (x - 1)               /* eng pastki 1-bitni o'chirish       */
```

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

### Surish tuzoqlari

- `1 << 31` — `1` bu `int`, 31-bitga surish ishora bitini buzadi → **UB**. Yozing: `1u << 31`.
- `1u << 32` — surish miqdori tur kengligidan katta yoki teng → **UB**. 64 bit uchun: `1ull << 40`.
- Manfiy sonni `>>` qilish — natija platformaga bog'liq (GCC'da ishora saqlanadi).

**Qoida: bitlar bilan faqat ishorasiz turlarda ishlang.** (04-mashq.)

## 3.5. Qiymat berish operatorlari

```c
x = 5;
x += 3;     /* x = x + 3 */
x -= 1;  x *= 2;  x /= 4;  x %= 3;
x |= FLAG;  x &= ~FLAG;  x ^= m;  x <<= 1;  x >>= 2;
```

`=` ham **ifoda** — uning qiymati berilgan qiymat: `a = b = 0;` (o'ngdan chapga: `b = 0`, keyin `a = 0`).

## 3.6. `++` va `--`

```c
int i = 5;
int a = i++;    /* a = 5, keyin i = 6   (post-increment: avval eski qiymat) */
int b = ++i;    /* i = 7, keyin b = 7   (pre-increment: avval oshirish) */
```

Eng ko'p ishlatiladigan idioma:

```c
buf[n++] = c;       /* c ni n-o'ringa yoz, keyin n ni oshir */
*p++ = c;           /* p ko'rsatgan joyga yoz, keyin p ni sur (7-bob) */
```

**Taqiqlangan:** bitta ifodada bir o'zgaruvchini ikki marta o'zgartirish — `i = i++;`,
`a[i] = i++;`, `f(i++, i++)` — UB (tartib aniqlanmagan). `-Wall` ogohlantiradi (`operation on 'i' may be undefined`).

## 3.7. Ternar operator `?:`

```c
int max = a > b ? a : b;
printf("%d ta fayl%s\n", n, n == 1 ? "" : "lar");
```

`shart ? rost_bo'lsa : yolg'on_bo'lsa` — qiymat qaytaradigan qisqa `if`.

## 3.8. Vergul operatori

```c
for (i = 0, j = n - 1; i < j; i++, j--)    /* ikki o'zgaruvchi birga */
```

Chapdan o'ngga bajariladi, qiymati — eng o'ngdagi. Faqat `for` ichida ishlating.

## 3.9. Ustuvorlik (precedence) — tuzoqlar

Hamma ustuvorlik jadvalini yodlash shart emas. **Qavs qo'ying.** Lekin quyidagi tuzoqlarni biling:

| Yozuv | Kutganingiz | Aslida | Nega |
|---|---|---|---|
| `x & 1 == 0` | `(x & 1) == 0` | `x & (1 == 0)` → `x & 0` → 0 | `==` bitli `&` dan kuchliroq! |
| `a << 2 + 1` | `(a << 2) + 1` | `a << 3` | `+` surishdan kuchliroq |
| `*p++` | `(*p)++` | `*(p++)` | postfiks `++` `*` dan kuchliroq |
| `a & b \|\| c` | | `(a & b) \|\| c` | to'g'ri, lekin o'qish qiyin |

`-Wall` ularning ko'pchiligini ogohlantiradi (`suggest parentheses`). Bitli amallar bilan taqqoslash
aralashsa — **doim qavs**: `if ((flags & MASK) == VALUE)`.

## 3.10. `sizeof` va cast — operatorlar ham

`sizeof(tur)`, `sizeof ifoda` — hajm (kompilyatsiya paytida). `(tur)ifoda` — aylantirish. Ikkalasi ham
unary operator va ustuvorligi yuqori: `(long)a * b` — avval `a` aylantiriladi, keyin ko'paytiriladi
(aynan shu kerak edi — 01-mashq).

## 3.11. Savol-javob

**Nega `-7 / 2` C'da −3, Python'da −4?**
C "nolga tomon kesadi" (CPU'ning `idiv` buyrug'i shunday ishlaydi, tez). Python "pastga yaxlitlaydi"
(matematik jihatdan izchil). Manfiy sonlarni bo'lish va `%` bilan ehtiyot bo'ling — masalan, halqa
bufer indeksida `(i - 1) % n` manfiy bo'lib qolishi mumkin; to'g'risi `(i + n - 1) % n`.

**Bitli amallar nega shunchalik muhim?**
Apparat bitlar bilan gaplashadi: qurilma registrining har bir biti alohida sozlama (yoqish, uzilish,
xato). Xotira manzillari ham bitlarga bo'lingan: virtual manzilning 39–47-bitlari — PML4 indeksi,
30–38 — PDPT va hokazo (MyOS: `kernel/mm/vmm.c`). Bitlarni erkin o'qiy olmasangiz, yadro kodini
o'qiy olmaysiz.

**`x * 2` o'rniga `x << 1` yozish kerakmi?**
Yo'q — kompilyator buni o'zi qiladi. Ma'nosi bo'yicha yozing: arifmetika uchun `*`, bitlar uchun `<<`.

## 3.12. O'zingizni tekshiring

1. `5 & 3`, `5 | 3`, `5 ^ 3`, `~0u >> 28` nechaga teng?
2. `if (p->x && p)` da nima xato?
3. `x & 0x80 == 0` qanday bajariladi?
4. `int i = 3; int j = i++ + 1;` — `i` va `j`?
5. 4096 ga tekislangan eng yaqin kichik manzilni qanday topasiz (`addr` dan)?

<details><summary>Javoblar</summary>

1. 1, 7, 6, 15 (0xF).
2. `p` NULL bo'lsa, `p->x` oldin bajarilib qulaydi. To'g'risi: `p && p->x`.
3. `x & (0x80 == 0)` → `x & 0` → 0. To'g'risi: `(x & 0x80) == 0`.
4. `i = 4`, `j = 4`.
5. `addr & ~0xFFFull` (yoki `addr & ~(uint64_t)4095`).
</details>

## 3.13. Mashqlar

- **02**, **03** — taqqoslash va toshishni oldindan tekshirish.
- **04** (bitlar) — bu bobning asosiy mashqi. Hal qilgach, `kernel/mm/vmm.c` ni oching va bitli
  amallarni topib, har birini o'qib chiqing.

Keyingi bob: [4-bob. Boshqaruv oqimi](04-boshqaruv.md)
