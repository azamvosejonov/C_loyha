# Asos bob. Kompyuter ichi: bit, bayt va ma'lumot

> **Bu bobda nima o'rganasiz:** kompyuter ma'lumotni **bitlar** bilan qanday saqlashini; bitlar sonni qanday ifodalashini (ikkilik, o'n oltilik, `0x` va `0b` nima ekanini); nega `0x0001` kabi **oldidagi nollar** yoziladi; quti to'lsa nima bo'lishini;
> bir xil baytlar nega har xil **ma'no** olishini (son, matn, kasr); matn va xotira qanday tuzilganini; qo'shish amali **mantiqiy kalitlar** bilan qanday bajarilishini; bitlar **jismonan** (tranzistor, zaryad) qanday saqlanishini.
> **Oldindan nima kerak:** hech narsa. Bu bobni **0-bobdan oldin** o'qing.   **Vaqt:** 5–7 soat.

> **Muhim eslatma:** bu bobdagi dasturlar sizga hozircha to'liq tushunarli bo'lmasligi mumkin — C tilini 0–3 boblarda o'rganasiz. **Hozirgi maqsad kodni yozish emas, natijani ko'rish va bitlarni tushunish.** Dasturlarni ishga tushiring (yoki natijani o'qing),
> "Kodda nimalar bor" jadvallariga qarang. Bu bobni tugatgach, boshqa boblar ancha osonlashadi.

## Bu bob nima haqida?

Boshqa hamma boblar uchta savolga tayanadi:

1. **Bit nima va u ma'lumotni qanday saqlaydi?**
2. **Bitlar sonni qanday ifodalaydi?** (`0x8000`, `0b1010`, `0xFF` — bular nima va nega aynan shunday yoziladi?)
3. **Kompyuter bitlarning "ma'nosini" qanday biladi?** (Bir xil bitlar nega ba'zan son, ba'zan harf?)

Bu savollarga javob bermasdan C va yadro dasturlashni o'rganish — poydevorsiz uy qurishga o'xshaydi. Shuning uchun avval shu poydevorni quramiz.

**Hayotdan misol: chiroqlar paneli.** Uyingizda 8 ta chiroq kaliti bor. Har biri yoki **yoniq**, yoki **o'chiq** — uchinchi holat yo'q. Siz bu 8 ta kalitning holati bilan **istalgan ma'lumotni** kodlay olasiz: "birinchi kalit yoniq = ha, o'chiq = yo'q", yoki
"kalitlar birgalikda o'qilsa — 0 dan 255 gacha son". Kompyuter ham aynan shunday: ichida milliardlab "kalit" bor (bit), va hamma narsa — son, matn, rasm, dastur — ularning holatidan iborat.

| Chiroqlar panelida | Kompyuterda |
|---|---|
| bitta kalit (yoniq/o'chiq) | **bit** (1/0) |
| 8 ta kalit | **bayt** |
| kalitlar holatini o'qish qoidasi ("birinchi kalit — 128 ni bildiradi...") | **kodlash** (sonlar, harflar) |
| kalitlar o'zi nimani bildirishini bilmaydi | bitlar o'zi ma'noni **bilmaydi** — ma'noni dastur beradi |

## A.1. Bit — bitta kalit

**Oddiy qilib aytganda:** **bit** (inglizcha *binary digit* — "ikkilik raqam") — eng kichik ma'lumot birligi. U faqat ikki qiymatdan birini oladi: **0** yoki **1**.

Nega aynan ikki? Chunki tranzistor (kompyuterning eng kichik elektron kaliti) ikki holatda **ishonchli** ishlaydi: "tok bor" va "tok yo'q". Agar 10 ta holat (0 dan 9 gacha) ishlatmoqchi bo'lsak, elektr kuchlanishining 10 xil darajasini
shovqin ichida aniq ajratish kerak bo'lardi — bu juda qiyin va xatoga moyil. Ikki holatni ajratish esa oson va juda tez.

> **Eslab qoling:** bit = bitta kalit = 0 yoki 1. Kompyuterdagi hamma narsa — shularning ketma-ketligi.

## A.2. Bayt — sakkizta kalit

**Oddiy qilib aytganda:** bitta bit juda kam ma'lumot (faqat "ha/yo'q"). Shuning uchun bitlar **guruhlanadi**. Eng ko'p ishlatiladigan guruh — **bayt** = **8 bit**.

Bir baytga necha xil qiymat sig'adi? Har bit 2 xil holatga ega, 8 ta bit bor: 2 × 2 × 2 × 2 × 2 × 2 × 2 × 2 = 2⁸ = **256**. Ya'ni 8 kalit **256 xil kombinatsiya** beradi: 0 dan 255 gacha.

| Bitlar soni | Xil qiymatlar soni | Ko'rinishi | Qayerda uchraydi |
|---|---|---|---|
| 1 | 2 | 0 yoki 1 | bayroq (ha/yo'q) |
| 4 | 16 | 0 … 15 | **bitta hex raqam** (A.4) |
| 8 (1 bayt) | 256 | 0 … 255 | `uint8_t`, `char` |
| 16 (2 bayt) | 65 536 | 0 … 65 535 | `uint16_t`, port raqami |
| 32 (4 bayt) | ≈ 4,3 milliard | 0 … 4 294 967 295 | `uint32_t`, `int` |
| 64 (8 bayt) | ≈ 18 kvintillion | 0 … 1,8·10¹⁹ | `uint64_t`, manzil |

**Qoida:** n bit → 2ⁿ xil qiymat. Bitni bittaga qo'shsangiz, qiymatlar soni **ikki barobar** oshadi.

> **Eslab qoling:** 1 bayt = 8 bit = 256 xil qiymat (0…255). n bit = 2ⁿ qiymat.

## A.3. Bitlar sonni qanday ifodalaydi

**Oddiy qilib aytganda:** o'nlik tizimda raqamning **o'rniga** qarab qiymati o'zgaradi: `347` da 3 — yuzlik, 4 — o'nlik, 7 — birlik. Har o'rin **10 ning darajasi**:

```text
347 = 3·100 + 4·10 + 7·1
```

Ikkilik tizim aynan shunday, faqat har o'rin **2 ning darajasi** (o'ngdan chapga: 1, 2, 4, 8, 16, 32, 64, 128, ...). Raqamlar esa faqat 0 va 1:

```text
o'rin og'irligi:   8   4   2   1
bitlar:            1   0   1   1      = 8 + 0 + 2 + 1 = 11
```

Ya'ni qaysi o'rinda `1` bo'lsa, o'sha o'rinning og'irligini **qo'shamiz**; `0` bo'lsa — qo'shmaymiz.

**Bu dastur nima qiladi (umumiy):** 0 dan 15 gacha hamma sonni o'nlik, 4 bitli ikkilik va o'n oltilik (A.4) ko'rinishda jadval qilib chiqaradi, so'ng 11 sonining har bir biti qiymatga qancha hissa qo'shishini ko'rsatadi.

```c
/* bitlar_son.c - bitlar sonni qanday ifodalaydi: 0 dan 15 gacha */
#include <stdio.h>

int main(void)
{
    printf("o'nlik | ikkilik (4 bit) | o'n oltilik\n");
    for (int n = 0; n <= 15; n++) {
        printf("%6d | ", n);
        for (int bit = 3; bit >= 0; bit--)          /* chapdan o'ngga: 3-bit, 2-bit, 1-bit, 0-bit */
            putchar((n >> bit) & 1 ? '1' : '0');
        printf("            | %X\n", n);
    }

    int son = 11;
    printf("\n%d sonining bitlari (har bitning og'irligi 8, 4, 2, 1):\n", son);
    int jami = 0;
    for (int bit = 3; bit >= 0; bit--) {
        int qiymat = (son >> bit) & 1;
        int ogirlik = 1 << bit;
        printf("  %d-bit: og'irligi %d, qiymati %d, hissasi %d\n", bit, ogirlik, qiymat, qiymat * ogirlik);
        jami += qiymat * ogirlik;
    }
    printf("  yig'indi = %d\n", jami);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 bitlar_son.c -o bitlar_son
$ ./bitlar_son
o'nlik | ikkilik (4 bit) | o'n oltilik
     0 | 0000            | 0
     1 | 0001            | 1
     2 | 0010            | 2
     3 | 0011            | 3
     4 | 0100            | 4
     5 | 0101            | 5
     6 | 0110            | 6
     7 | 0111            | 7
     8 | 1000            | 8
     9 | 1001            | 9
    10 | 1010            | A
    11 | 1011            | B
    12 | 1100            | C
    13 | 1101            | D
    14 | 1110            | E
    15 | 1111            | F

11 sonining bitlari (har bitning og'irligi 8, 4, 2, 1):
  3-bit: og'irligi 8, qiymati 1, hissasi 8
  2-bit: og'irligi 4, qiymati 0, hissasi 0
  1-bit: og'irligi 2, qiymati 1, hissasi 2
  0-bit: og'irligi 1, qiymati 1, hissasi 1
  yig'indi = 11
```

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `for (int n = 0; n <= 15; n++)` | `n` ni 0 dan 15 gacha oshirib takrorlaydi |
| `(n >> bit) & 1` | `n` ning `bit`-o'rindagi bitini oladi (0 yoki 1). Qanday ishlashi 3-bobda to'liq tushuntiriladi; hozir "n ning bit-chi biti" deb qabul qiling |
| `putchar(... ? '1' : '0')` | bit 1 bo'lsa `1` belgisini, aks holda `0` belgisini chiqaradi |
| `1 << bit` | shu o'rinning **og'irligi** (1, 2, 4, 8...) |
| `qiymat * ogirlik` | shu bitning yig'indiga **hissasi** |

**Nima ko'rdik:** 11 = `1011`: 8-o'rin (1) → 8, 4-o'rin (0) → 0, 2-o'rin (1) → 2, 1-o'rin (1) → 1; yig'indi 8 + 0 + 2 + 1 = 11. Jadvalning har bir qatorida ham shunday: masalan 13 = `1101` = 8 + 4 + 0 + 1.

> **Eslab qoling:** ikkilik son = yoniq o'rinlarning og'irliklari yig'indisi. O'ngdan chapga og'irliklar: 1, 2, 4, 8, 16, 32, 64, 128...

## A.4. O'n oltilik (hex) yozuv

### Muammo

Ikkilik yozuv uzun va ko'zni charchatadi. 32768 soni ikkilikda `1000000000000000` — 16 ta raqam. Bunda xato qilish oson (bitta nolni tashlab ketdingiz — boshqa son bo'ldi).

### Yechim: 4 bitni bitta belgi bilan yozish

4 bit **16** xil qiymat beradi (0000 dan 1111 gacha — yuqoridagi jadvalda ko'rdingiz). 16 xil qiymat uchun 16 ta belgi kerak: `0–9` (o'nta) va `A–F` (oltita):

| Ikkilik | O'nlik | Hex | | Ikkilik | O'nlik | Hex |
|---|---|---|---|---|---|---|
| `0000` | 0 | `0` | | `1000` | 8 | `8` |
| `0001` | 1 | `1` | | `1001` | 9 | `9` |
| `0010` | 2 | `2` | | `1010` | 10 | **`A`** |
| `0011` | 3 | `3` | | `1011` | 11 | **`B`** |
| `0100` | 4 | `4` | | `1100` | 12 | **`C`** |
| `0101` | 5 | `5` | | `1101` | 13 | **`D`** |
| `0110` | 6 | `6` | | `1110` | 14 | **`E`** |
| `0111` | 7 | `7` | | `1111` | 15 | **`F`** |

Nega aynan 16 (8 yoki 10 emas)? Chunki 16 = 2⁴: **bir hex belgi = aynan 4 bit**, **ikki hex belgi = aynan 1 bayt**. O'nlikda bunday moslik yo'q: 32768 ga qarab qaysi bit yoniqligini ko'ra olmaysiz, `0x8000` ga qarab esa ko'rasiz.

> Hex va ikkilik — **bitta sonning ikki xil yozilishi**, xuddi "o'n bir", "11" va "XI" kabi. Kompyuter faqat bitlarni saqlaydi. Hex — **bizning (odamlar) qulayligimiz uchun**.

### Prefikslar: `0x`, `0b`

Yozuvga qarab kompyuter qaysi sanoq tizimi ekanini bilishi kerak, aks holda `10` ning ma'nosi noaniq (o'n? ikki? o'n olti?). Shuning uchun son boshiga **belgi** qo'yiladi:

| Yozuv | Qaysi tizim | Misol | O'nlikda |
|---|---|---|---|
| hech narsa | o'nlik | `42` | 42 |
| `0x` | **o'n oltilik** (hexadecimal) | `0x2A` | 42 |
| `0b` | **ikkilik** (binary) | `0b101010` | 42 |
| `0` (bitta nol) | sakkizlik (octal) | `052` | 42 |

`0x` dagi `x` — *hexadecimal*, `0b` dagi `b` — *binary* so'zidan. `0x` va `0b` o'zi hech qanday qiymat emas, faqat "bundan keyingisi shu tizimda" degan belgi.

**`0b8000` nega xato?** `0b` dan keyin faqat **ikkilik raqamlar** (`0` va `1`) bo'lishi mumkin. `8` ikkilik raqam emas. Shuning uchun kompilyator bunday yozuvni rad etadi.

**Ehtiyot bo'ling — sakkizlik tuzoq:** `010` — o'nlikda **8**, o'n emas! Boshidagi `0` "sakkizlik" degani. Shuning uchun sonlarni boshiga nol qo'yib yozmang.

### `0x8000` ni o'qish

Har hex belgini 4 bitga yoyamiz (yuqoridagi jadvaldan):

| Belgi | 4 bit |
|---|---|
| `8` | `1000` |
| `0` | `0000` |
| `0` | `0000` |
| `0` | `0000` |

Birlashtirsak: `1000 0000 0000 0000` — 16 bit, **eng chap bit yoniq**, qolgan 15 tasi o'chiq. O'nlikda 2¹⁵ = **32768**. Teskarisi ham xuddi shunday: 4 bitni hex belgiga o'tkazasiz.

**Mashhur hex sonlar (16 bitli):**

| Son | 16 bit | Nega muhim |
|---|---|---|
| `0x0001` | `0000 0000 0000 0001` | eng o'ng (0-chi) bit yoniq |
| `0x8000` | `1000 0000 0000 0000` | eng chap (15-chi) bit yoniq; ishorali sonlarda **ishora biti** (20-bob) |
| `0x00FF` | `0000 0000 1111 1111` | past bayt to'liq yoniq — **niqob** (maska): `&` bilan faqat past baytni ajratib olishda (3-bob) |
| `0xFF00` | `1111 1111 0000 0000` | yuqori bayt to'liq yoniq |
| `0xFFFF` | `1111 1111 1111 1111` | **hamma** bit yoniq — 16 bitli qutiga sig'adigan eng katta son (65535) |

**F harfi:** `F` = `1111` — 4 ta yoniq kalit. Nechta `F` bo'lsa, shuncha 4-bitli guruh to'liq yoniq: `0xF` — 4 bit, `0xFF` — 8 bit (255), `0xFFFF` — 16 bit (65535).

> **Eslab qoling:** hex belgi = 4 bit; `0x` — hex, `0b` — ikkilik, `0` — sakkizlik. `A=10, B=11, C=12, D=13, E=14, F=15`. Hex ↔ ikkilik o'tkazish — har belgini 4 bitga yoyish, hisoblash shart emas.

## A.5. Oldidagi nollar: `0x0001` va `0x1`

**Oddiy qilib aytganda:** mashinaning kilometr hisoblagichi 5 xonali va 7 km yurgan bo'lsa, `00007` ko'rsatadi. Oldidagi nollar qiymatni o'zgartirmaydi — ular hisoblagichning **bo'sh xonalari**, chunki har xona nimadir ko'rsatishi shart.

Kompyuterdagi quti ham xuddi shunday: `uint16_t` — **doim 16 katak**. Songa 1 yozsak, bitta katak 1 bo'ladi, qolgan 15 tasi **doim 0** turadi:

```text
katak:   15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0
qiymat:   0  0  0  0 |  0  0  0  0 |  0  0  0  0 |  0  0  0  1
```

Hex da bu 4 belgi (4 × 4 = 16 katak): `0x 0001`. Biz oldidagi nollarni yozishimiz **ixtiyoriy**: `0x1` va `0x0001` kompyuter uchun **aynan bir xil**. Nollarni "bu 16 bitli quti" ekanini ko'rsatish uchun yozamiz.

**Bu dastur nima qiladi (umumiy):** `0x1`, `0x0001` va `0x000001` bir xil son ekanini ko'rsatadi, `printf` da kenglikni (`%04X`, `%08X`) tanlashni va **o'ng** tomondagi nollar qiymatni o'zgartirishini ko'rsatadi.

```c
/* nollar.c - oldidagi nollar qiymatni o'zgartirmaydi, faqat kenglikni ko'rsatadi */
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    uint16_t a = 0x1, b = 0x0001, c = 0x000001;

    printf("a = 0x1      -> %u\n", a);
    printf("b = 0x0001   -> %u\n", b);
    printf("c = 0x000001 -> %u\n", c);
    printf("hammasi tengmi? %s\n", (a == b && b == c) ? "ha" : "yo'q");

    printf("\nbitta son, uch xil kenglikda chiqarish:\n");
    printf("  %%X    -> %X\n", a);
    printf("  %%04X  -> %04X   (4 hex belgi = 16 bit)\n", a);
    printf("  %%08X  -> %08X   (8 hex belgi = 32 bit)\n", a);

    printf("\nO'NG tomondagi nollar esa qiymatni o'zgartiradi:\n");
    printf("  0x8 = %d, 0x80 = %d, 0x800 = %d, 0x8000 = %d\n", 0x8, 0x80, 0x800, 0x8000);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 nollar.c -o nollar
$ ./nollar
a = 0x1      -> 1
b = 0x0001   -> 1
c = 0x000001 -> 1
hammasi tengmi? ha

bitta son, uch xil kenglikda chiqarish:
  %X    -> 1
  %04X  -> 0001   (4 hex belgi = 16 bit)
  %08X  -> 00000001   (8 hex belgi = 32 bit)

O'NG tomondagi nollar esa qiymatni o'zgartiradi:
  0x8 = 8, 0x80 = 128, 0x800 = 2048, 0x8000 = 32768
```

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `uint16_t a = 0x1, b = 0x0001, c = 0x000001;` | uch xil yozilgan bir xil son; `uint16_t` — aniq 16 bitli quti (`<stdint.h>`) |
| `%u` | ishorasiz butun sonni o'nlikda chiqaradi |
| `%X` / `%04X` / `%08X` | hex chiqarish; `04` — "kamida 4 belgi, yetmasa oldiga nol qo'y" |
| `0x8`, `0x80`, `0x800`, `0x8000` | **o'ngdagi** nollar qo'shilgani sari qiymat 16 barobar oshadi |

**Qoida:** **oldidagi** (chap tomondagi) nollar qiymatni **o'zgartirmaydi**; **orqadagi** (o'ng tomondagi) nollar **o'zgartiradi**. O'nlikda ham shunday: `008` = `8`, lekin `800` ≠ `8`.

## A.6. Quti to'lsa: toshish

**Oddiy qilib aytganda:** kilometr hisoblagichi `99999` ko'rsatib turibdi. 1 km yursa, `100000` sig'maydi va hisoblagich `00000` ga **aylanib qaytadi**. Kompyuter qutisi ham xuddi shunday: hamma katak 1 bo'lsa va yana 1 qo'shsak, natijaga yangi (katta) katak kerak bo'ladi,
lekin quti chegaralangan — ortiqcha bit **tashlanadi**.

**Bu dastur nima qiladi (umumiy):** 8 bitli va 16 bitli qutilarda "hammasi 1" holatiga 1 ni qo'shadi va natijani ko'rsatadi; keyin 250 + 10 ni 8 bitli qutida hisoblaydi.

```c
/* toshish_f.c - quti to'lganda: hammasi 1 ga 1 qo'shsak */
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    uint8_t  kichik = 0xFF;                      /* 8 katak, hammasi 1 */
    uint16_t orta   = 0xFFFF;                    /* 16 katak, hammasi 1 */

    printf("8 bit : 0x%02X = %3u\n", kichik, kichik);
    kichik = kichik + 1;
    printf("+ 1   : 0x%02X = %3u   (9-bit sig'madi, tashlandi)\n", kichik, kichik);

    printf("16 bit: 0x%04X = %5u\n", orta, orta);
    orta = orta + 1;
    printf("+ 1   : 0x%04X = %5u\n", orta, orta);

    uint8_t k = 250;
    k = k + 10;
    printf("250 + 10 (8 bitli quti) = %u   (260 - 256 = 4)\n", k);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 toshish_f.c -o toshish_f
$ ./toshish_f
8 bit : 0xFF = 255
+ 1   : 0x00 =   0   (9-bit sig'madi, tashlandi)
16 bit: 0xFFFF = 65535
+ 1   : 0x0000 =     0
250 + 10 (8 bitli quti) = 4   (260 - 256 = 4)
```

**Hisoblab ko'ring:** `1111 1111` + `0000 0001` = `1 0000 0000` (9 ta raqam!). 8 bitli quti faqat 8 katak oladi, eng chap `1` tashlanadi → `0000 0000` = 0. Xuddi shu: 250 + 10 = 260 = 256 + **4** → quti faqat qoldiqni (4) saqlaydi.

**Muhim:** bu **ishorasiz** (`uint8_t`) sonlarda oddiy "aylanish". **Ishorali** (`int`) sonlarda toshish C da **aniqlanmagan xatti-harakat** (2-bob, 13-bob) — u yerda "aylanadi" deb hisoblab bo'lmaydi.

> **Eslab qoling:** quti — chegaralangan katak soni. Hammasi 1 (`0xFF`, `0xFFFF`) ga 1 qo'shsak, nolga qaytadi (ishorasiz sonlarda). Qutiga sig'maydigan son — xatoning keng tarqalgan manbai.

## A.7. Bitlar ma'noni bilmaydi: bir xil baytlar, turli ma'no

**Oddiy qilib aytganda:** xotirada bitlar turibdi. Ularning ma'nosini kim biladi? **Hech kim — bitlarning o'zi ham, kompyuter ham.** Ma'noni **dastur** beradi: "bu 4 baytni **son** deb o'qi" yoki "**matn** deb o'qi". C tilidagi **tur** (`int`, `char`, `float`) — aynan shu: "bu baytlarni qanday o'qish kerak"ning qoidasi.

**Bu dastur nima qiladi (umumiy):** xotirada bitta aniq 4 bayt (`41 42 43 00`) ni oladi va bir xil baytlarni to'rt xil usulda o'qiydi: matn, butun son, kasr son va to'rtta kichik son.

```c
/* talqin.c - bir xil baytlar, turli ma'no */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    unsigned char b[4] = { 0x41, 0x42, 0x43, 0x00 };

    printf("baytlar:             %02X %02X %02X %02X\n", b[0], b[1], b[2], b[3]);
    printf("matn deb o'qisak:    \"%s\"\n", (char *)b);

    uint32_t son;
    memcpy(&son, b, sizeof(son));
    printf("butun son deb:       %u\n", son);

    float f;
    memcpy(&f, b, sizeof(f));
    printf("kasr son (float):    %g\n", f);

    printf("4 ta kichik son deb: %u %u %u %u\n", b[0], b[1], b[2], b[3]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 talqin.c -o talqin
$ ./talqin
baytlar:             41 42 43 00
matn deb o'qisak:    "ABC"
butun son deb:       4407873
kasr son (float):    6.17675e-39
4 ta kichik son deb: 65 66 67 0
```

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `unsigned char b[4] = { 0x41, 0x42, 0x43, 0x00 }` | 4 ta bayt: **bitta** xotira bo'lagi |
| `(char *)b` + `%s` | baytlarni **matn** deb o'qiydi (nol bayt — matn oxiri) |
| `memcpy(&son, b, ...)` | shu 4 baytni 32 bitli **butun son** qutisiga ko'chiradi |
| `memcpy(&f, b, ...)` | xuddi shu 4 baytni **float** (kasr son) qutisiga ko'chiradi |
| `b[0] ... b[3]` | har baytni alohida **kichik son** sifatida |

**Nima ko'rdik:** baytlar **o'zgarmadi**, lekin ma'no butunlay boshqa bo'ldi: `"ABC"`, `4407873`, `6.17675e-39`, `65 66 67 0`. Shuning uchun xotiradagi baytlarni to'g'ri turda o'qish dasturchining vazifasi — turni noto'g'ri tanlasangiz, **axlat** o'qiysiz.
Yadro dasturlashda bu har kuni uchraydi: tarmoq paketi, fayl sarlavhasi, qurilma registri — hammasi shunchaki baytlar; ularni **qaysi tur** deb o'qish — qoida (protokol, format) bilan belgilanadi.

> **Eslab qoling:** bitlar/baytlar ma'noni bilmaydi. **Tur** — o'qish qoidasi. Bir xil baytlar → turga qarab turli ma'no.

## A.8. Matn qanday saqlanadi

**Oddiy qilib aytganda:** harf ham — son. Kelishuv: har harfga raqam beriladi. Eng mashhur jadval — **ASCII**: `A` = 65, `a` = 97, `0` = 48 va hokazo (128 ta belgi, 7 bit). Kompyuter harfni emas, uning **kodini** saqlaydi; ekranga chiqarganda kod yana harfga aylanadi.

**Bu dastur nima qiladi (umumiy):** "Salom" so'zining har harfi kodini (o'nlik, hex, bitlar) ko'rsatadi; kichik va katta harf farqini; va bir nechta matnning **baytlarini** (UTF-8) chiqaradi — shu jumladan o'zbekcha `ʻ` belgisi bilan.

```c
/* matn_kod.c - harf ham son: ASCII va UTF-8 */
#include <stdio.h>
#include <string.h>

static void baytlar(const char *s)
{
    printf("\"%s\": %zu bayt ->", s, strlen(s));
    for (const unsigned char *p = (const unsigned char *)s; *p; p++)
        printf(" %02X", *p);
    printf("\n");
}

int main(void)
{
    const char *soz = "Salom";
    printf("harf   kod(o'nlik)  kod(hex)  bitlar\n");
    for (const char *p = soz; *p; p++) {
        printf("  %c     %3d         %02X       ", *p, *p, *p);
        for (int bit = 7; bit >= 0; bit--)
            putchar((*p >> bit) & 1 ? '1' : '0');
        printf("\n");
    }

    printf("\nKichik va katta harf orasidagi farq:\n");
    printf("  'a' - 'A' = %d  (faqat bitta bit farq qiladi: 0x20)\n", 'a' - 'A');

    printf("\nMatn baytlari (UTF-8):\n");
    baytlar("Salom");
    baytlar("Oʻzbek");                           /* ʻ - alohida belgi (U+02BB), 2 bayt */
    baytlar("5 so'm");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 matn_kod.c -o matn_kod
$ ./matn_kod
harf   kod(o'nlik)  kod(hex)  bitlar
  S      83         53       01010011
  a      97         61       01100001
  l     108         6C       01101100
  o     111         6F       01101111
  m     109         6D       01101101

Kichik va katta harf orasidagi farq:
  'a' - 'A' = 32  (faqat bitta bit farq qiladi: 0x20)

Matn baytlari (UTF-8):
"Salom": 5 bayt -> 53 61 6C 6F 6D
"Oʻzbek": 7 bayt -> 4F CA BB 7A 62 65 6B
"5 so'm": 6 bayt -> 35 20 73 6F 27 6D
```

**Nima ko'rdik:**

- `S` = 83 = `0x53` = `0101 0011`; `a` = 97 = `0x61`. Harflar — oddiy sonlar.
- `'a' - 'A' = 32 = 0x20` — kichik va katta harf orasida **faqat bitta bit** (5-bit) farq qiladi. Shuning uchun katta harfni kichikka o'tkazish — bitta bitni yoqish.
- `"Salom"` = 5 bayt (har harf 1 bayt).
- `"Oʻzbek"` = **7** bayt, 6 harf bo'lsa ham! Chunki `ʻ` (o' dagi tutuq belgisi, Unicode U+02BB) ASCII da yo'q va **2 bayt** (`CA BB`) bilan yoziladi. Bu **UTF-8** kodlash: ASCII belgilar 1 bayt, boshqa tillar belgilari 2–4 bayt.

Shuning uchun matn uzunligini **belgilarda** va **baytlarda** sanash har xil bo'lishi mumkin — 6-bobdagi `strlen` baytlarni sanaydi.

> **Eslab qoling:** harf = kod (son). ASCII: `A`=65, `a`=97, `0`=48. UTF-8: ASCII 1 bayt, boshqalar ko'proq bayt.

## A.9. Xotira: baytlar qatori va manzillar

**Oddiy qilib aytganda:** kompyuter xotirasi — juda uzun **baytlar qatori**, har baytning o'z **raqami** bor — bu uning **manzili**. **Hayotdan misol:** ko'cha bo'ylab uylar, har uyning raqami bor. Bitta uy — bitta bayt. 4 baytli `int` — ketma-ket 4 uy.

**Bu dastur nima qiladi (umumiy):** `int` va `char` massivlarida qo'shni elementlar xotirada necha bayt narida turishini va turlarning o'lchamini ko'rsatadi; keyin `0x12345678` sonining baytlari xotirada qanday tartibda yotishini chiqaradi.

```c
/* xotira_manzil.c - xotira: har bayt o'z manzilida, int esa bir nechta baytda */
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    int a[3] = { 10, 20, 30 };
    char c[3] = { 'x', 'y', 'z' };

    printf("int  massivida qo'shni elementlar orasida: %td bayt\n", (char *)&a[1] - (char *)&a[0]);
    printf("char massivida qo'shni elementlar orasida: %td bayt\n", (char *)&c[1] - (char *)&c[0]);
    printf("sizeof(int) = %zu, sizeof(char) = %zu, sizeof(a) = %zu\n", sizeof(int), sizeof(char), sizeof(a));

    uint32_t son = 0x12345678;
    unsigned char *bayt = (unsigned char *)&son;
    printf("\n0x12345678 xotirada (past manzildan yuqoriga):");
    for (int i = 0; i < 4; i++)
        printf(" %02X", bayt[i]);
    printf("\n(x86 da avval PAST bayt turadi: 78 56 34 12 - bu 'little-endian')\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 xotira_manzil.c -o xotira_manzil
$ ./xotira_manzil
int  massivida qo'shni elementlar orasida: 4 bayt
char massivida qo'shni elementlar orasida: 1 bayt
sizeof(int) = 4, sizeof(char) = 1, sizeof(a) = 12

0x12345678 xotirada (past manzildan yuqoriga): 78 56 34 12
(x86 da avval PAST bayt turadi: 78 56 34 12 - bu 'little-endian')
```

**Nima ko'rdik:**

| Natija | Ma'nosi |
|---|---|
| `int` massivida qo'shni elementlar orasida 4 bayt | har `int` 4 ta ketma-ket baytni egallaydi |
| `char` massivida 1 bayt | har `char` 1 bayt |
| `sizeof(a) = 12` | 3 ta `int` × 4 bayt |
| `78 56 34 12` | `0x12345678` ning baytlari **teskari** tartibda: avval **past** bayt (`78`), oxirida **yuqori** bayt (`12`) |

Oxirgi qator — **little-endian**: x86 protsessorlarda ko'p baytli sonning **past** baytini eng past manzilga yozadi. Bu 16-bobda batafsil. Hozir shuni biling: **sonni baytlar ko'rinishida o'qisangiz, tartib teskari ko'rinishi mumkin** (shuning uchun yuqoridagi `0x8000` xotirada `00 80` bo'lib yotadi).

> **Eslab qoling:** xotira = baytlar qatori, har bayt o'z manzilida. Ko'p baytli son ketma-ket baytlarni egallaydi. x86 da past bayt oldin turadi (little-endian).

## A.10. Qo'shish aslida kalitlar ishi

**Oddiy qilib aytganda:** kompyuter "qo'shishni biladi" deb o'ylamang. Qo'shish — oddiy **mantiqiy kalitlar** kombinatsiyasi. Bitta bitni qo'shish uchun ikki amal yetadi:

- **XOR** (`^`): bitlar **farq qilsa** 1. Bu **yig'indi biti**.
- **AND** (`&`): ikkalasi 1 bo'lsa 1. Bu **ko'chish biti** (1 + 1 = 10₂: 0 qoldi, 1 keyingi o'ringa ko'chdi).

O'nlikda ham shunday: 7 + 5 = 12 — 2 yozamiz, **1 ni ko'chiramiz** (qo'shni o'ringa). Ikkilikda faqat 0 va 1 bo'lgani uchun qoida soddaroq.

**Bu dastur nima qiladi (umumiy):** "yarim qo'shuvchi" (bitta bitni qo'shuvchi) jadvalini ko'rsatadi; keyin 8 bitli sonlarni **faqat XOR, AND, OR va siljitish** yordamida qo'shadi; 256 × 256 juftlikning hammasida odatdagi `+` bilan solishtirib xato yo'qligini tekshiradi.

```c
/* qoshuvchi.c - qo'shish aslida mantiqiy kalitlar (XOR va AND) ishi */
#include <stdio.h>

/* yarim qo'shuvchi: ikki BITni qo'shadi -> yig'indi biti va ko'chish biti */
static void yarim(int a, int b, int *yigindi, int *kochish)
{
    *yigindi = a ^ b;                           /* XOR: bitlar farq qilsa 1 */
    *kochish = a & b;                           /* AND: ikkalasi 1 bo'lsa keyingi o'ringa 1 ko'chadi */
}

/* to'liq qo'shuvchi: oldingi o'rindan kelgan ko'chishni ham hisobga oladi */
static void tolik(int a, int b, int kirish, int *yigindi, int *kochish)
{
    int s1, k1, k2;
    yarim(a, b, &s1, &k1);
    yarim(s1, kirish, yigindi, &k2);
    *kochish = k1 | k2;
}

/* 8 bitli sonni faqat mantiqiy amallar bilan qo'shamiz: har bit uchun bitta to'liq qo'shuvchi */
static int qosh8(int x, int y)
{
    int natija = 0, kochish = 0;
    for (int bit = 0; bit < 8; bit++) {
        int s;
        tolik((x >> bit) & 1, (y >> bit) & 1, kochish, &s, &kochish);
        natija |= s << bit;
    }
    return natija;                              /* oxirgi ko'chish tashlanadi: 8 bitli quti */
}

int main(void)
{
    printf("a b | yigindi kochish   (yarim qo'shuvchi jadvali)\n");
    for (int a = 0; a <= 1; a++)
        for (int b = 0; b <= 1; b++) {
            int s, k;
            yarim(a, b, &s, &k);
            printf("%d %d |    %d       %d\n", a, b, s, k);
        }

    printf("\n13 + 29 = %d  (kalitlar bilan)\n", qosh8(13, 29));
    printf("200 + 100 = %d  (8 bitli quti: 300 - 256 = 44)\n", qosh8(200, 100));

    int xato = 0;
    for (int x = 0; x < 256; x++)
        for (int y = 0; y < 256; y++)
            if (qosh8(x, y) != ((x + y) & 0xFF))
                xato++;
    printf("256 x 256 juftlikdan xato: %d ta\n", xato);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 qoshuvchi.c -o qoshuvchi
$ ./qoshuvchi
a b | yigindi kochish   (yarim qo'shuvchi jadvali)
0 0 |    0       0
0 1 |    1       0
1 0 |    1       0
1 1 |    0       1

13 + 29 = 42  (kalitlar bilan)
200 + 100 = 44  (8 bitli quti: 300 - 256 = 44)
256 x 256 juftlikdan xato: 0 ta
```

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `yarim(a, b, ...)` | ikki bitni qo'shadi: yig'indi = `a ^ b`, ko'chish = `a & b` |
| `tolik(a, b, kirish, ...)` | oldingi o'rindan kelgan ko'chishni ham hisobga oladi (ikkita yarim qo'shuvchi + OR) |
| `qosh8(x, y)` | 8 ta bit uchun ketma-ket `tolik` ni ishlatadi, ko'chishni keyingi bitga uzatadi |
| `& 0xFF` | natijani 8 bitga qisqartirish (A.6 dagi "quti") |

**Nima ko'rdik:** yarim qo'shuvchi jadvali (0+0, 0+1, 1+0 → yig'indi 0/1/1, ko'chish 0; **1+1 → yig'indi 0, ko'chish 1**); `13 + 29 = 42`; `200 + 100 = 44` (toshish: 300 − 256); va **65 536 juftlikning birortasida ham xato yo'q**. Haqiqiy protsessorda bu amallar tranzistorlardan yasalgan "eshiklar" (gates) bilan bajariladi — tez va parallel.

> **Eslab qoling:** qo'shish = XOR (yig'indi) + AND (ko'chish). Kompyuterning hisob-kitobi — bit amallari. 3-bobda `&`, `|`, `^`, `~`, `<<`, `>>` ni batafsil o'rganasiz.

## A.11. Bitlar jismonan qanday saqlanadi

**Oddiy qilib aytganda:** bit — "elektr holat". Qutiga ma'lumot yozish — shu holatni o'zgartirish; o'qish — holatni sezish. Har xil xotira turlari buni **turlicha** bajaradi:

| Xotira turi | Bitni qanday saqlaydi | Tezlik / xususiyat | Qayerda |
|---|---|---|---|
| **Registr, kesh (SRAM)** | 4–6 tranzistordan iborat "trigger" (flip-flop): ikki barqaror holatdan birida turadi | eng tez, qimmat, kam hajm | protsessor ichida |
| **Operativ xotira (DRAM)** | kichik **kondensator**da zaryad bor/yo'q (1 tranzistor + 1 kondensator) | tez, arzon, katta hajm; zaryad sizib ketadi, shuning uchun **doim yangilab turiladi** (refresh); tok o'chsa — yo'qoladi | RAM moduli |
| **SSD (flesh)** | tranzistor "mahkamlangan" katagida (floating gate) **elektronlar** tutib qolinadi | tok o'chsa ham saqlanadi; yozish sekinroq, katak cheklangan marta yeyiladi | SSD, USB flesh |
| **Qattiq disk (HDD)** | aylanuvchi plastinkada kichik **magnit** sohalar (shimol/janub) | arzon, katta hajm, mexanik — sekin | HDD |

**Qanday biladi?** Xotira "bilmaydi" — u faqat holatni saqlaydi. Protsessor manzil yuboradi ("1000-uy"), xotira shu uydagi 8 bitning holatini elektr signal sifatida qaytaradi. Signalni **o'qiydigan** va **hisoblaydigan** qurilma — protsessor — ham tranzistor "eshiklari"dan (AND, OR, NOT, XOR) tuzilgan.
Qanday saqlanishi esa **qatlamlar**ga bo'lingan: tranzistor → eshiklar → registrlar/xotira → buyruqlar → dastur → siz ko'radigan natija. Siz bu darslikda asosan **yuqoridagi** qatlamlar bilan ishlaysiz, pastki qatlamlarni bilish esa **nima uchun** ba'zi narsalar (kesh, tezlik, tok o'chganda yo'qolish) shunday ekanini tushuntiradi (21, 27-boblar).

> **Eslab qoling:** bit — fizik holat (zaryad, magnit). RAM — tez, lekin tok o'chsa yo'qoladi; SSD/disk — doimiy. Protsessor ham tranzistor "eshiklari"dan tuzilgan.

## Hayotdan misol va to'liq dastur

**Xotiraga ko'z bilan qarash.** Yuqorida bitlar, baytlar, hex, ma'no, matn, xotira tartibini alohida ko'rdik. Endi **bitta dastur** hamma narsani birlashtiradi: har xil turdagi o'zgaruvchilarning **xotiradagi baytlarini** hex va bitlarda ko'rsatadi.

**Bu dastur nima qiladi (umumiy):** 8 xil qiymatni (kichik son, hex son, 258, −1, harf, kasr son, matn, struktura) xotirada yotgan **aniq baytlari** bilan chiqaradi. `korsat` funksiyasi har qanday o'zgaruvchining manzilini va uzunligini olib, uning baytlarini hex va ikkilikda ko'rsatadi.

```c
/* bayt_korgich.c - har qanday o'zgaruvchining xotiradagi BAYTLARINI ko'rsatadi */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* nom - o'zgaruvchi nomi, p - uning manzili, n - necha bayt */
static void korsat(const char *nom, const void *p, size_t n)
{
    const unsigned char *b = p;                 /* xotirani baytma-bayt o'qiymiz */
    printf("%-14s %zu bayt:", nom, n);
    for (size_t i = 0; i < n; i++)
        printf(" %02X", b[i]);
    printf("   | bitlar:");
    for (size_t i = 0; i < n; i++) {
        putchar(' ');
        for (int bit = 7; bit >= 0; bit--)
            putchar((b[i] >> bit) & 1 ? '1' : '0');
    }
    printf("\n");
}

struct nuqta {
    char tur;                                   /* 1 bayt */
    int x;                                      /* 4 bayt: oldidan bo'sh joy qoldiriladi (tekislash) */
};

int main(void)
{
    uint8_t  a = 200;
    uint16_t b = 0x8000;
    int32_t  c = 258;                           /* 0x00000102 */
    int32_t  d = -1;                            /* hamma bit 1 (ikkiga to'ldirish) */
    char     e = 'A';
    float    f = 1.0f;
    char     g[] = "Hi";                        /* 'H' 'i' va oxirida '\0' */
    struct nuqta h;
    memset(&h, 0, sizeof(h));                   /* bo'sh joylar ham nol bo'lsin */
    h.tur = 'P';
    h.x = 7;

    korsat("a = 200", &a, sizeof(a));
    korsat("b = 0x8000", &b, sizeof(b));
    korsat("c = 258", &c, sizeof(c));
    korsat("d = -1", &d, sizeof(d));
    korsat("e = 'A'", &e, sizeof(e));
    korsat("f = 1.0f", &f, sizeof(f));
    korsat("g = \"Hi\"", g, sizeof(g));
    korsat("struct nuqta", &h, sizeof(h));
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 bayt_korgich.c -o bayt_korgich
$ ./bayt_korgich
a = 200        1 bayt: C8   | bitlar: 11001000
b = 0x8000     2 bayt: 00 80   | bitlar: 00000000 10000000
c = 258        4 bayt: 02 01 00 00   | bitlar: 00000010 00000001 00000000 00000000
d = -1         4 bayt: FF FF FF FF   | bitlar: 11111111 11111111 11111111 11111111
e = 'A'        1 bayt: 41   | bitlar: 01000001
f = 1.0f       4 bayt: 00 00 80 3F   | bitlar: 00000000 00000000 10000000 00111111
g = "Hi"       3 bayt: 48 69 00   | bitlar: 01001000 01101001 00000000
struct nuqta   8 bayt: 50 00 00 00 07 00 00 00   | bitlar: 01010000 00000000 00000000 00000000 00000111 00000000 00000000 00000000
```

**Natijani o'qish:**

| Qator | Nimani bildiradi |
|---|---|
| `a = 200` → `C8` | 200 = 128+64+8 = `1100 1000` = `0xC8`. 1 bayt |
| `b = 0x8000` → `00 80` | 2 bayt; xotirada **past bayt oldin**: `00`, so'ng `80` (little-endian, A.9) |
| `c = 258` → `02 01 00 00` | 258 = 256 + 2 = `0x0102`; 4 bayt, past bayt oldin |
| `d = -1` → `FF FF FF FF` | manfiy son: **hamma bit 1** (ikkiga to'ldirish, 20-bob) — shuning uchun `0xFFFFFFFF` bir vaqtning o'zida "−1" ham, "eng katta ishorasiz son" ham: **qanday o'qishga bog'liq** (A.7!) |
| `e = 'A'` → `41` | harf — kod 65 |
| `f = 1.0f` → `00 00 80 3F` | kasr son butunlay boshqa kodlashda (IEEE 754, 20-bob) — "1" ning bitlari butun sondagi 1 ga o'xshamaydi |
| `g = "Hi"` → `48 69 00` | 'H', 'i' va **oxirida nol bayt** (matn oxiri belgisi `'\0'`, 6-bob) |
| `struct nuqta` → `50 00 00 00 07 00 00 00` | 8 bayt: `'P'` (1 bayt) + **3 bayt bo'sh joy** + `07 00 00 00` (4 baytli `int`). Kompilyator `int` ni 4 ga karrali manzilga qo'yadi (**tekislash**, 9-bob) |

## Katta loyiha: Bit laboratoriyasi (`konv`)

**Maqsad:** o'z qo'lingiz bilan **sonni har xil yozuvda ko'rsatadigan vosita** yasash. Dastur klaviatura (yoki fayl) dan sonlarni o'qiydi — `42`, `0x8000`, `0b1010`, `052` — va har biri uchun o'nlik, hex, ikkilik ko'rinishini, nechta bayt kerakligini, qaysi bitlar yoniqligini va 2 ning darajasi ekanini chiqaradi. Bu bobdagi **hamma** tushunchalar shu yerda ishlaydi.

**Qanday ishlaydi:**

| Qism | Vazifasi |
|---|---|
| `oqi(matn, &natija)` | prefiksga qarab (`0x`, `0b`, `0`, hech narsa) sanoq tizimini aniqlaydi va matnni songa aylantiradi; xato bo'lsa kod qaytaradi |
| `strtoull(s, &oxiri, asos)` | C kutubxonasi funksiyasi: matnni `asos` tizimida songa o'giradi; `oxiri` — qayergacha o'qiganini ko'rsatadi (hamma belgi yaroqlimi tekshirish uchun) |
| `bayt_soni(v)` | `v >>= 8` bilan sonni baytma-bayt "qirqib", nechta bayt kerakligini sanaydi |
| `korsat(...)` | ikkilik ko'rinishni 4 bitlik guruhlarga ajratib chiqaradi, yoniq bitlar o'rnini va sonini ko'rsatadi |
| `v & (v - 1)` | eng pastki yoniq bitni o'chiradi; natija 0 bo'lsa — faqat bitta bit yoniq edi, ya'ni **2 ning darajasi** |
| `fgets` sikli | stdin dan qator-qator o'qiydi (6-bobda batafsil) |

```c
/* konv.c - bit laboratoriyasi: sonni 4 xil yozuvda va bitlar jadvalida ko'rsatadi */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* matnni songa o'giradi: "42", "0x2A", "0b101010", "052". 0 - muvaffaqiyat */
static int oqi(const char *s, uint64_t *natija)
{
    int asos = 10;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        asos = 16;
        s += 2;
    } else if (s[0] == '0' && (s[1] == 'b' || s[1] == 'B')) {
        asos = 2;
        s += 2;
    } else if (s[0] == '0' && s[1] != '\0') {
        asos = 8;
        s += 1;
    }
    if (*s == '\0')
        return -1;                              /* "0x" dan keyin hech narsa yo'q */

    char *oxiri;
    errno = 0;
    uint64_t v = strtoull(s, &oxiri, asos);
    if (*oxiri != '\0')
        return -2;                              /* bu asosda yaroqsiz belgi bor */
    if (errno == ERANGE)
        return -3;                              /* 64 bitga sig'maydi */
    *natija = v;
    return 0;
}

static int bayt_soni(uint64_t v)                /* eng kamida necha bayt kerak */
{
    int n = 1;
    while (v >>= 8)
        n++;
    return n;
}

static void korsat(const char *matn, uint64_t v)
{
    int baytlar = bayt_soni(v);
    int bitlar = baytlar * 8;

    printf("%s\n", matn);
    printf("  o'nlik      : %llu\n", (unsigned long long)v);
    printf("  o'n oltilik : 0x%llX\n", (unsigned long long)v);
    printf("  ikkilik     : ");
    for (int bit = bitlar - 1; bit >= 0; bit--) {
        putchar((v >> bit) & 1 ? '1' : '0');
        if (bit % 4 == 0 && bit)
            putchar(' ');
    }
    printf("\n  hajm        : %d bayt (%d bit kerak)\n", baytlar, bitlar);

    int yoniq = 0;
    printf("  yoniq bitlar: ");
    for (int bit = 0; bit < 64; bit++)
        if ((v >> bit) & 1) {
            printf("%s%d", yoniq ? ", " : "", bit);
            yoniq++;
        }
    if (!yoniq)
        printf("yo'q");
    printf("\n  yoniq soni  : %d\n", yoniq);
    printf("  2 darajasi  : %s\n", (v && !(v & (v - 1))) ? "ha (bitta bit yoniq)" : "yo'q");
}

int main(void)
{
    char qator[128];
    while (fgets(qator, sizeof(qator), stdin)) {
        qator[strcspn(qator, "\r\n")] = '\0';
        if (qator[0] == '\0')
            continue;
        uint64_t v;
        int r = oqi(qator, &v);
        if (r == 0)
            korsat(qator, v);
        else
            printf("%s\n  XATO: %s\n", qator,
                   r == -1 ? "raqam yo'q" : r == -2 ? "bu sanoq tizimida yo'q belgi bor" : "juda katta son");
    }
    return 0;
}
```

```console
$ printf '42\n0x8000\n0b1010\n052\n255\n0b2\n0xFFFF\n0x\n1\n' > kirish.txt
$ gcc -Wall -Wextra -g -fsanitize=address,undefined konv.c -o konv
$ ./konv < kirish.txt
42
  o'nlik      : 42
  o'n oltilik : 0x2A
  ikkilik     : 0010 1010
  hajm        : 1 bayt (8 bit kerak)
  yoniq bitlar: 1, 3, 5
  yoniq soni  : 3
  2 darajasi  : yo'q
0x8000
  o'nlik      : 32768
  o'n oltilik : 0x8000
  ikkilik     : 1000 0000 0000 0000
  hajm        : 2 bayt (16 bit kerak)
  yoniq bitlar: 15
  yoniq soni  : 1
  2 darajasi  : ha (bitta bit yoniq)
0b1010
  o'nlik      : 10
  o'n oltilik : 0xA
  ikkilik     : 0000 1010
  hajm        : 1 bayt (8 bit kerak)
  yoniq bitlar: 1, 3
  yoniq soni  : 2
  2 darajasi  : yo'q
052
  o'nlik      : 42
  o'n oltilik : 0x2A
  ikkilik     : 0010 1010
  hajm        : 1 bayt (8 bit kerak)
  yoniq bitlar: 1, 3, 5
  yoniq soni  : 3
  2 darajasi  : yo'q
255
  o'nlik      : 255
  o'n oltilik : 0xFF
  ikkilik     : 1111 1111
  hajm        : 1 bayt (8 bit kerak)
  yoniq bitlar: 0, 1, 2, 3, 4, 5, 6, 7
  yoniq soni  : 8
  2 darajasi  : yo'q
0b2
  XATO: bu sanoq tizimida yo'q belgi bor
0xFFFF
  o'nlik      : 65535
  o'n oltilik : 0xFFFF
  ikkilik     : 1111 1111 1111 1111
  hajm        : 2 bayt (16 bit kerak)
  yoniq bitlar: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
  yoniq soni  : 16
  2 darajasi  : yo'q
0x
  XATO: raqam yo'q
1
  o'nlik      : 1
  o'n oltilik : 0x1
  ikkilik     : 0000 0001
  hajm        : 1 bayt (8 bit kerak)
  yoniq bitlar: 0
  yoniq soni  : 1
  2 darajasi  : ha (bitta bit yoniq)
```

**Natijani o'qish:**

- `42` va `052` (sakkizlik) — ikkalasi ham o'nlikda 42: **bir xil son**, ikki xil yozuv.
- `0x8000` — faqat **15-bit** yoniq, "2 darajasi: ha". `0xFFFF` — 16 ta yoniq bit, 2 baytni to'liq to'ldiradi.
- `0b2` — xato: "bu sanoq tizimida yo'q belgi bor" (A.4: `0b` dan keyin faqat 0/1).
- `0x` — xato: raqam yo'q.

**O'zingiz kengaytiring (yechim yo'q — faqat maslahat):**

1. **Sakkizlik chiqarish qo'shing** (`%llo`). Maslahat: `printf` ning `%o` formati.
2. **Manfiy sonlar:** `-5` ni qabul qilib, 8 bitli "ikkiga to'ldirish" ko'rinishini chiqaring (`0xFB`). Maslahat: `(uint8_t)(-5)` ni chiqaring; nega `0xFB` ekanini 20-bobda ko'rasiz.
3. **Bitni yoqish/o'chirish buyruqlari:** kirish `0x00 set 3` bo'lsa, 3-bitni yoqib natijani ko'rsating. Maslahat: `v | (1ULL << 3)` (yoqish), `v & ~(1ULL << 3)` (o'chirish) — 3-bobda o'rganasiz.
4. **Eng pastki yoniq bitning o'rni:** maslahat: `__builtin_ctzll(v)`.

## Bob xulosasi (yodlash uchun)

1. **Bit** — 0 yoki 1 (bitta kalit); **bayt** — 8 bit (256 xil qiymat). n bit = 2ⁿ qiymat.
2. Ikkilik son = yoniq o'rinlar og'irliklarining yig'indisi (1, 2, 4, 8, 16, ...). **Hex** — 4 bitni 1 belgi bilan yozish (`A=10 … F=15`); `0x` — hex, `0b` — ikkilik, `0` — sakkizlik. Hex va ikkilik — **bitta sonning ikki yozuvi**.
3. **Oldidagi nollar** qiymatni o'zgartirmaydi (`0x0001` = `0x1`), faqat quti kengligini (16 bit = 4 hex belgi) ko'rsatadi; **orqadagi** nollar qiymatni o'zgartiradi. Hamma bit 1 (`0xFF`, `0xFFFF`) + 1 → **toshish** → 0.
4. Bitlar **ma'noni bilmaydi**: tur — o'qish qoidasi (`41 42 43 00` = "ABC" yoki 4407873). Harf = kod (ASCII/UTF-8). Xotira = baytlar qatori, har bayt o'z manzilida; x86 da past bayt oldin (little-endian).
5. Qo'shish = XOR + AND (kalitlar ishi). Bit jismonan — zaryad/magnit holati: SRAM/DRAM (tez, tok o'chsa yo'qoladi), flesh/disk (doimiy).

## Savol-javob

**Savol:** `0xFFFF` ga yana son qo'shsam nima bo'ladi?
**Javob:** Agar tur `uint16_t` bo'lsa — natija nolga qaytadi (toshish, A.6). Qutiga sig'maydigan eng chap bit tashlanadi.

**Savol:** Nega hex? Axir kompyuter ikkilikda ishlaydi-ku.
**Javob:** Hex — **bizning** qulayligimiz uchun. 1 hex belgi = 4 bit, shuning uchun uzun ikkilik qatorni 4 baravar qisqartirib, xatosiz o'qiymiz va yozamiz. Kompyuter uchun ikkala yozuv bir xil.

**Savol:** `0b8000` desam bo'ladimi?
**Javob:** Yo'q. `0b` dan keyin faqat `0` va `1` yoziladi. `8` — ikkilik raqam emas.

**Savol:** Kompyuter bitlarning ma'nosini qanday biladi?
**Javob:** **Bilmaydi.** Ma'noni dastur beradi (tur orqali). Bir xil baytlar son, matn yoki kasr bo'lishi mumkin (A.7).

**Savol:** `uint8_t` va oddiy `int` farqi nima?
**Javob:** `uint8_t` — **aniq** 8 bit, ishorasiz (0…255). `int` — odatda 32 bit, ishorali. Aniq o'lcham kerak bo'lganda (apparat, tarmoq, fayl formati) `<stdint.h>` dagi turlar ishlatiladi (2-bob).

## O'zingizni tekshiring

1. 1 baytda necha xil qiymat saqlanadi? 2 baytdami?
2. `1101` ikkilik sonni o'nlikka aylantiring.
3. `0xA5` ni 8 bitga yoying.
4. `0x0007` va `0x7` bir xilmi? `0x700` va `0x7` bir xilmi?
5. `0xFF` ga 1 qo'shsak (8 bitli quti) nima bo'ladi?
6. `0b1010` o'nlikda necha? Hex da qanday yoziladi?
7. Nega xotirada `0x8000` ikki bayt `00 80` shaklida yotadi?
8. `'A'` (65) va `'a'` (97) orasidagi farq nimada?
9. Bir xil 4 bayt turlicha o'qilsa nima o'zgaradi: baytlarmi yoki ma'nomi?
10. Qo'shish uchun qaysi ikki bit amali yetadi?

<details><summary>Javoblar</summary>

1. 256 (2⁸); 2 baytda 65 536 (2¹⁶).
2. `1101` = 8 + 4 + 0 + 1 = **13**.
3. `A` = `1010`, `5` = `0101` → `1010 0101`.
4. `0x0007` = `0x7` (oldidagi nollar qiymatni o'zgartirmaydi). `0x700` ≠ `0x7`: `0x700` = 7 × 256 = 1792 (**orqadagi** nollar o'zgartiradi).
5. `1111 1111` + 1 = `1 0000 0000`; 9-bit tashlanadi → **0**.
6. 8 + 0 + 2 + 0 = **10**; hex da `0xA`.
7. x86 (little-endian) da ko'p baytli sonning **past** bayti (`00`) eng past manzilga yoziladi, yuqori bayt (`80`) keyingisiga.
8. Faqat bitta bit (5-bit, qiymati 32 = `0x20`): kichik harfda yoniq, katta harfda o'chiq.
9. **Ma'no** o'zgaradi, baytlar o'zgarmaydi: tur — o'qish qoidasi.
10. **XOR** (yig'indi biti) va **AND** (ko'chish biti).
</details>

## Mashq

- `konv` ga 1–4-kengaytirishlardan kamida ikkitasini qo'shing.
- Daftarga 10 ta hex sonni (`0x3C`, `0xE7`, `0x1F`, ...) qo'lda ikkilikka yoying, keyin `konv` bilan tekshiring.
- `bayt_korgich` ga o'zingizning 3 ta o'zgaruvchingizni qo'shing (`uint32_t`, `char[]`, `double`) va natijani oldindan **taxmin qiling**: nechta bayt? qaysi tartibda?
- `bitlar_son` ni o'zgartirib 0 dan 255 gacha (8 bitli) jadval chiqaring.

Keyingi bob: [0-bob. Kirish: C nima, o'rnatish, birinchi dastur](00-kirish.md)
