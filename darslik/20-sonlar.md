# 20-bob. Sonlar kompyuterda: butun sonlar va kasr sonlar (IEEE 754)

> **Bu bobdan keyin:** istalgan sonni ikkilik va o'n oltilik tizimda boshingizda aylantira olasiz,
> ikkiga to'ldirish (two's complement) nega ishlashini **isbotlay** olasiz, ishora kengayishi va qirqishni,
> `float` ning bitlarini va `0.1 + 0.2 != 0.3` sababini bilasiz. (Mavzu odatda "Computer Systems:
> A Programmer's Perspective" kitobining 2-bobidan o'rganiladi.) Mashqlar: 03, 04, 41.

> **To'liq ishlaydigan misol:** [misollar/20_sonlar.c](misollar/20_sonlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Pozitsion sanoq tizimi — kupyuralar (20.1).** 3 745 so'm — bu 3 ta mingtalik, 7 ta yuztalik, 4 ta
o'ntalik va 5 ta birtalik. Raqamning **joyi** uning qiymatini belgilaydi. Ikkilik tizimda ham xuddi
shunday, faqat "kupyuralar" 1, 2, 4, 8, 16... va har biridan ko'pi bilan bitta: `1101` = 8 + 4 + 1 = 13.
O'n oltilik tizim — ikkilikning qisqa yozuvi: har bir o'n oltilik raqam — to'rtta bit.

**Ikkiga to'ldirish — orqaga aylanadigan hisoblagich (20.3).** Uch xonali hisoblagich 000 dan bitta
orqaga aylantirilsa — 999 ko'rsatadi. Demak "−1" ni 999 deb kelishish mumkin: 999 + 1 = 000 — to'g'ri!
8 bitli sonlarda ham: −1 = `11111111`. Shu kelishuv tufayli protsessorga ayirish uchun alohida
sxema kerak emas — qo'shish sxemasi ishorali sonlar uchun ham ishlaydi.

**Ishora kengayishi — narxni kattaroq blankaga ko'chirish (20.4).** Kichik blankada "−5" yozilgan.
Katta blankaga ko'chirganda bo'sh katakchalarni to'ldirish kerak: musbat son uchun 0 lar bilan,
manfiy son uchun — 1 lar bilan (ikkiga to'ldirishda manfiy sonning boshi 1 lardan iborat).
Aks holda −5 katta musbat songa aylanib qoladi.

**Qirqish — sig'magan raqamlarni kesib tashlash (20.4).** 4 xonali displeyga 12 345 ni yozsangiz,
faqat oxirgi 4 raqami qoladi: 2345. Katta sonni kichik turga o'tkazganda ham yuqori bitlar kesiladi.

**`float` — ilmiy yozuv va kalkulyator ekrani (20.6).** Kalkulyator ekranida faqat 8–10 raqam sig'adi.
Juda katta son `6.02e23` ko'rinishida yoziladi: raqamlar (mantissa) va daraja. `float` da ~7 ta aniq
raqam bor. Shuning uchun 16 777 217 ni `float` da aniq saqlab bo'lmaydi, 0.1 ni esa umuman aniq
saqlab bo'lmaydi (ikkilikda u cheksiz kasr — o'nlikdagi 1/3 = 0.3333... kabi).

**Qat'iy nuqta — pulni tiyinda sanash (20.7).** Bankda hisob "12 650.55 so'm" deb emas, "1 265 055 tiyin"
deb saqlanadi — butun son, hech qanday yaxlitlash xatosi yo'q. Yadroda `float` ishlatilmaydi, shuning
uchun foizlar, vaqt va boshqalar aynan shunday butun sonlar bilan hisoblanadi.

### To'liq dastur: valyuta ayirboshlash shoxobchasi

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

`float` bilan 100 000 ta 0.01 ni qo'shish 1000 dan sezilarli farq qildi — har bir qo'shishdagi
kichik yaxlitlash xatosi yig'ilib boradi. Sentlar esa doim aniq.

**Sinab ko'ring:** `float` ni `double` ga almashtiring — xato kamayadimi, yo'qoladimi? 12345 ni 8 bitga
qirqish natijasini qo'lda hisoblang: 12345 % 256.

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
