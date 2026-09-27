# 20-bob. Sonlar kompyuterda: butun sonlar va kasr sonlar (IEEE 754)

> **Bu bobdan keyin:** istalgan sonni ikkilik va o'n oltilik tizimda boshingizda aylantira olasiz,
> ikkiga to'ldirish (two's complement) nega ishlashini **isbotlay** olasiz, ishora kengayishi va qirqishni,
> `float` ning bitlarini va `0.1 + 0.2 != 0.3` sababini bilasiz. (Mavzu odatda "Computer Systems:
> A Programmer's Perspective" kitobining 2-bobidan o'rganiladi.) Mashqlar: 03, 04, 41.

> **To'liq ishlaydigan misol:** [misollar/20_sonlar.c](misollar/20_sonlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

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

Shuning uchun `0xFFFFFFFF80100000` ni ko'rib, bitlarini darhol "ko'rish" mumkin. Ikkilikda yozilsa 64 ta
raqam, o'n oltilikda — 16 ta.

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

Ikkining darajalari: 2¹⁰ = 1024 (1 KiB), 2¹² = 4096 (sahifa), 2²⁰ ≈ million (1 MiB), 2³⁰ ≈ milliard (1 GiB),
2³² ≈ 4.29 milliard, 2⁴⁸ = 256 TiB (x86-64 virtual manzil maydoni), 2⁶⁴ ≈ 1.8·10¹⁹.

`2ⁿ` ni tez hisoblash: `2^(10a + b) = 2^b · 1024^a`. Masalan 2³⁹ = 2⁹ · 2³⁰ = 512 GiB — bitta PML4
yozuvi qamraydigan hudud (31-mashq).

## 20.2. Ishorasiz butun sonlar

w bitli ishorasiz son: `B = b[w-1]·2^(w-1) + ... + b[1]·2 + b[0]`. Oraliq: `0 .. 2^w − 1`.
Arifmetika **mod 2^w**: natijaning ortiqcha bitlari tashlanadi.

```text
8 bit:  255 + 1 = 1 0000 0000  ->  0000 0000 = 0
        0 - 1   = 1111 1111 = 255
```

C'da bu **aniqlangan** xatti-harakat (2-bob). Shuning uchun xesh funksiyalar, tasodifiy son
generatorlari, bitli hisoblar ishorasiz turda yoziladi.

## 20.3. Ikkiga to'ldirish (two's complement) — nega aynan shunday

w bitli ishorali son: eng yuqori bitning "og'irligi" **manfiy**:

```text
B = -b[w-1]·2^(w-1) + b[w-2]·2^(w-2) + ... + b[0]
```

8 bit uchun: `1000 0000 = -128`, `1111 1111 = -128 + 127 = -1`, `0111 1111 = 127`.
Oraliq: `-2^(w-1) .. 2^(w-1) - 1` (masalan int: −2147483648 .. 2147483647). Manfiylar bittaga ko'p.

**Ishorani o'zgartirish qoidasi:** `-x = ~x + 1` (hamma bitlarni teskari qilib, 1 qo'shish):

```text
 5 = 0000 0101
~5 = 1111 1010
+1 = 1111 1011 = -5     tekshirish: -128 + 64+32+16+8+2+1 = -128 + 123 = -5 ✓
```

**Nega kompyuterlar shuni tanladi:** qo'shish/ayirish/ko'paytirish **ishorali va ishorasiz uchun bir xil
sxema** bilan bajariladi — faqat natijani talqin qilish boshqa. `1111 1111 + 0000 0001`: ishorasiz
talqinda 255 + 1 = 0 (mod 256), ishorali talqinda −1 + 1 = 0. CPU'da bitta qo'shuvchi — ikki xil ma'no.
Shu sababli CPU'da `add` bitta, lekin taqqoslashdan keyingi sakrashlar ikki xil: `jl/jg` (ishorali)
va `jb/ja` (ishorasiz) — 17-bob.

**Muhim nosimmetriklik:** `-INT_MIN` — sig'maydi (`~1000...0 + 1 = 1000...0` — yana o'zi!). Shuning
uchun `abs(INT_MIN)`, `INT_MIN / -1` — UB (03, 20-mashqlar).

## 20.4. Ishora kengayishi va qirqish

Kichik turdan kattasiga o'tkazish:
- **ishorasiz** → yuqoriga **nollar** qo'shiladi (zero extension);
- **ishorali** → yuqoriga **ishora biti** takrorlanadi (sign extension).

```c
int8_t  a = -5;         /* 1111 1011 */
int32_t b = a;          /* 1111 1111 1111 1111 1111 1111 1111 1011 = -5 (qiymat saqlandi) */
uint8_t c = 0xFB;       /* 251 */
uint32_t d = c;         /* 0000 ... 1111 1011 = 251 */
```

Kattadan kichikka — **qirqish**: pastki bitlar qoladi.

```c
int32_t x = 300;        /* ... 0001 0010 1100 */
int8_t  y = (int8_t)x;  /* 0010 1100 = 44 */
```

**Klassik tuzoq — `char` va EOF:**

```c
char c;                          /* x86 da ishorali */
while ((c = getchar()) != EOF)   /* 0xFF bayt -> c = -1 -> EOF bilan teng -> sikl oldin tugaydi! */
```

`getchar` `int` qaytaradi (0..255 yoki −1). Natijani `int` da saqlang.

**Yadrodagi misol — kanonik manzillar:** x86-64 da virtual manzil 48 bitli, lekin registr 64 bit.
63..48-bitlar 47-bitning **ishora kengayishi** bo'lishi shart: shuning uchun yadro manzillari
`0xFFFF8000...` dan boshlanadi (47-bit = 1 → yuqori bitlar ham 1). 31-mashqdagi kanonik tekshiruv — shu.

## 20.5. Butun son arifmetikasining xossalari

- Qo'shish, ko'paytirish **mod 2^w** — halqa: kommutativ, assotsiativ, hatto toshish bo'lganda ham
  (ishorasiz uchun; ishorali uchun C'da UB, lekin apparat darajasida xuddi shu).
- Ikkining darajasiga ko'paytirish = chapga surish: `x * 8 == x << 3`.
- Ikkining darajasiga bo'lish = o'ngga surish, **lekin** manfiy son uchun yaxlitlash farq qiladi:
  `-7 / 2 == -3` (C, nolga tomon), `-7 >> 1 == -4` (pastga). Kompilyator buni to'g'rilash uchun
  qo'shimcha buyruq qo'yadi — shuning uchun `unsigned` bo'lish tezroq.
- `x % 2^k == x & (2^k - 1)` — faqat ishorasiz (yoki manfiy bo'lmagan) x uchun.

## 20.6. Kasr sonlar: IEEE 754 standarti

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
- `E` — **siljitilgan** (biased) eksponenta: haqiqiy daraja + 127. Nega siljitilgan: shunda musbat
  sonlarning bitlari butun son sifatida solishtirilsa ham tartib to'g'ri chiqadi.
- `1.M` — "yashirin bir": normal sonda mantissa doim `1.xxx` ko'rinishida, shuning uchun 1 saqlanmaydi —
  bitta bit tejaladi.

**Misol: 6.5 ni float'ga**

```text
6.5 = 110.1₂ = 1.101₂ × 2²
S = 0,  E = 2 + 127 = 129 = 1000 0001,  M = 101 0000 ... (1. dan keyingi qism)
bitlar: 0 10000001 10100000000000000000000 = 0x40D00000
```

**Maxsus qiymatlar:**

| E | M | Nima |
|---|---|---|
| 0 | 0 | ±0 (ha, −0 ham bor!) |
| 0 | ≠0 | **denormal**: `0.M × 2^(-126)` — nolga juda yaqin sonlar, asta-sekin yo'qolish |
| 1..254 | istalgan | normal son |
| 255 | 0 | ±∞ (`1.0/0.0`) |
| 255 | ≠0 | NaN ("son emas": `0.0/0.0`, `sqrt(-1)`). `NaN != NaN` — hatto o'zi bilan ham! |

**Nega `0.1 + 0.2 != 0.3`:** 0.1 ikkilikda cheksiz davriy kasr: `0.0001100110011...₂` (xuddi 1/3 o'nlikda
0.333... kabi). 52 bitda qirqiladi → kichik xato. Ikki xatoli son yig'indisi 0.3 ning eng yaqin
tasviriga teng chiqmaydi. Qoida: kasr sonlarni `==` bilan solishtirmang, `fabs(a - b) < eps` ishlating;
pul hisobida — butun sonlar (tiyinlar).

**Yaxlitlash:** standart bo'yicha — "eng yaqiniga, teng bo'lsa juftiga" (round-to-nearest-even):
2.5 → 2, 3.5 → 4. Nega juftiga: ko'p yaxlitlashda xatolar bir tomonga to'planmaydi.

**Aniqlik:** `float` ~7 o'nlik raqam, `double` ~15–16. 2²⁴ + 1 = 16777217 ni `float` aniq saqlay olmaydi:
`(float)16777217 == 16777216`. Shuning uchun `int` → `float` aylantirish ma'lumot yo'qotishi mumkin.

## 20.7. Yadroda kasr sonlar o'rniga — qat'iy nuqtali (fixed-point) arifmetika

Yadro FPU registrlarini ishlatmaydi (18-bob). Kasr kerak bo'lsa — butun sonni "masshtab" bilan:

```c
/* CPU yuklamasi foizda, 2 xona aniqlik: 12.34% -> 1234 */
uint32_t yuklama_x100 = (band_tiklar * 10000) / jami_tiklar;
kprintf("%u.%02u%%\n", yuklama_x100 / 100, yuklama_x100 % 100);

/* TSC chastotasi kHz da saqlanadi (MyOS: tsc_khz) - Hz da float emas */
uint64_t mikrosekund = tsc_farq * 1000 / tsc_khz;
```

Linux ham shunday qiladi: yuklama o'rtachalari (load average), scheduler og'irliklari — hammasi
butun sonlarda. Tartib muhim: avval ko'paytirish, keyin bo'lish (aks holda aniqlik yo'qoladi), lekin
toshishga e'tibor (64 bit).

## 20.8. O'zingizni tekshiring

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

## 20.9. Mashqlar

- **41** (float bitlari) — float'ni sign/eksponenta/mantissaga ajratish va butun sonni qo'lda float'ga
  aylantirish (yaxlitlash bilan).
- **04** (bitlar) va **03** (toshish) — bu bob bilan qayta ko'ring.
- Qo'shimcha: `union { float f; uint32_t u; }` bilan 1.0, −2.5, 0.1 ning bitlarini chiqarib, qo'lda hisoblaganingiz bilan solishtiring.

Keyingi bob: [21-bob. Xotira ierarxiyasi va kesh](21-kesh.md)
