# 9-bob. `struct`, `union`, `enum`, `typedef`

> **Bu bobda nima o'rganasiz:** o'z turlaringizni yaratishni (`struct`); struktura xotirada qanday joylashishini (tekislash va to'ldiruvchi baytlar);
> `packed` va bit maydonlarini; `union` va `enum` ni; `typedef` ni; yashirin ("opaque") turlarni. Apparat va disk tuzilmalarini C'da tasvirlash uchun hammasi kerak.
> **Oldindan nima kerak:** 2-, 7-, 8-boblar.   **Vaqt:** 6–7 soat.
> Mashqlar: 13, 16, 17, 19, 22, 23.

> **To'liq ishlaydigan misol:** [misollar/09_struct.c](misollar/09_struct.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Hozirgacha o'zgaruvchilar bitta qiymat saqlardi. Lekin dunyodagi narsalar **bir nechta xususiyatdan** iborat: talaba — ism, yosh, ball; avtomobil — raqam, rusum, yil.
Shu xususiyatlarni **bitta paketga** yig'ib, uni bitta nom bilan ishlatish kerak. C'da buning vositasi — **`struct`** (struktura). Python'dagi `@dataclass` ga o'xshaydi, lekin metodlar yo'q — faqat ma'lumot.

**Hayotdan misol: texpasport.** Avtomobil texpasportida bir nechta ma'lumot bor: davlat raqami, rusumi, yili, rangi. Ular alohida qog'ozlarda emas — **bitta hujjatda**.
`struct mashina` ham shunday: bir nechta turli maydon, bitta nom ostida. Hujjatni nusxalash mumkin (`b = a;`) — hamma maydonlar birga ko'chadi.

| Texpasport | C'da |
|---|---|
| hujjat turi ("texpasport blanki") | `struct mashina { ... };` — **tur** (qolip) |
| to'ldirilgan hujjat | `struct mashina m = {...};` — o'zgaruvchi |
| hujjatdagi grafa (raqam, yil) | **maydon**: `m.raqam`, `m.yil` |
| hujjatning nusxasi | `b = a;` |

## 9.1. `struct` — bir nechta qiymat bitta nom ostida

```c
/* talaba_struct.c - struct e'lon qilish va ishlatish */
#include <stdio.h>

struct talaba {                 /* yangi TUR: "talaba" */
    char ism[32];
    int yosh;
    double ball;
};                              /* ; SHART! */

int main(void)
{
    struct talaba t = { "Ali", 20, 87.5 };                  /* tartib bilan */
    struct talaba u = { .ism = "Vali", .ball = 91.0 };      /* nomli maydonlar; yosh = 0 */

    t.yosh = 21;                    /* . - qiymat orqali */
    struct talaba *p = &t;
    p->ball = 90.0;                 /* -> - ko'rsatkich orqali (7.5) */

    printf("t: %s, %d yosh, %.1f ball\n", t.ism, t.yosh, t.ball);
    printf("u: %s, %d yosh, %.1f ball\n", u.ism, u.yosh, u.ball);
    printf("sizeof(struct talaba) = %zu\n", sizeof(struct talaba));
    return 0;
}
```

```console
$ gcc -Wall -Wextra talaba_struct.c -o talaba_struct
$ ./talaba_struct
t: Ali, 21 yosh, 90.0 ball
u: Vali, 0 yosh, 91.0 ball
sizeof(struct talaba) = 48
```

**Kodda nimalar bor:**

| Qator | Nima qiladi | Nega |
|---|---|---|
| `struct talaba { ... };` | yangi **tur** e'lon qiladi (hali o'zgaruvchi yaratilmadi!) | Qolip: "talaba — ism + yosh + ball" |
| `char ism[32]` | ism uchun 32 baytli massiv | satr (6-bob) |
| `struct talaba t = {...}` | `t` — shu turdagi **o'zgaruvchi**; qiymatlar maydonlar tartibida | `"Ali"` → `ism`, `20` → `yosh`, `87.5` → `ball` |
| `{ .ism = "Vali", .ball = 91.0 }` | **nomlangan** maydonlar; ko'rsatilmagan maydon (`yosh`) = 0 | Tartibni eslab qolish shart emas |
| `t.yosh = 21;` | maydonga yozish | `.` — struktura **qiymati** uchun |
| `p->ball = 90.0;` | ko'rsatkich orqali yozish | `->` — struktura **ko'rsatkichi** uchun |

**Qadam-baqadam (xotirada `t`):**

```text
t:  [ ism: "Ali\0..........(32 bayt)" ][ yosh: 20→21 ][ (to'ldiruvchi) ][ ball: 87.5→90.0 ]
     0                            31    32          35  36        39      40            47      -> jami 48 bayt
```

**Savol: nega `}` dan keyin `;` kerak?** Chunki `struct ... { }` — bu **tur**, va tur nomidan keyin darhol o'zgaruvchi e'lon qilish mumkin:

```c
struct nuqta { int x, y; } a, b;    /* tur ta'rifi + ikki o'zgaruvchi */
```

`;` "e'lon tugadi" degani. Unutilsa, kompilyator keyingi qatordagi narsani shu turdagi o'zgaruvchi deb o'qishga harakat qiladi va g'alati xato beradi:

```c
/* nuqtali_vergul_yoq.c - struct oxirida ; unutildi */
struct nuqta { int x, y; }

int main(void)
{
    return 0;
}
```

```console
$ gcc -Wall -Wextra nuqtali_vergul_yoq.c -o nuqtali_vergul_yoq # xato kutiladi
nuqtali_vergul_yoq.c:4:1: error: expected ‘;’, identifier or ‘(’ before ‘int’
    4 | int main(void)
      | ^~~
```

Xato "keyingi" qatorda ko'rsatildi: kompilyator `int main` ni nuqta o'zgaruvchisi deb o'qimoqchi bo'ldi.

### Struct bilan ishlash qoidalari

- `=` bilan **to'liq nusxalanadi** (ichidagi massiv ham!): `u = t;`. Python'dagi obyektlardan farqli — bu havola emas, **haqiqiy nusxa**.
- `==` bilan solishtirib **bo'lmaydi** — maydonma-maydon solishtiring (`memcmp` ham ishonchsiz — pastda to'ldiruvchi baytlar).
- Funksiyaga katta structni ko'rsatkich bilan uzating: `void chop_et(const struct talaba *t)`.

> **Eslab qoling:** `struct nom { maydonlar };` — yangi **tur** (oxirida `;`). O'zgaruvchi: `struct nom x;`. Maydon: `x.maydon` yoki `p->maydon`.

## 9.2. Xotirada joylashish: tekislash (alignment) va to'ldiruvchi (padding)

**Hayotdan misol: kitob javoni.** Javonda kichik va katta kitoblar bor. Qoida: katta kitob faqat maxsus bo'linma **boshidan** qo'yiladi (protsessor 4 baytlik sonni 4 ga
bo'linadigan manzildan tez o'qiydi). Kichik kitobdan keyin katta kitob kelsa, orada bo'sh joy qoladi — bu **padding** (to'ldiruvchi). Kitoblarni kattadan kichikka qarab tersangiz, bo'sh joy kamayadi.

```c
/* tekislash.c - struct hajmi va maydonlar o'rni */
#include <stddef.h>
#include <stdio.h>

struct a { char c; int i; char d; };        /* kichik, katta, kichik */
struct b { int i; char c; char d; };        /* katta, kichik, kichik */

int main(void)
{
    printf("struct a: sizeof = %zu\n", sizeof(struct a));
    printf("  c: %zu, i: %zu, d: %zu\n", offsetof(struct a, c), offsetof(struct a, i), offsetof(struct a, d));
    printf("struct b: sizeof = %zu\n", sizeof(struct b));
    printf("  i: %zu, c: %zu, d: %zu\n", offsetof(struct b, i), offsetof(struct b, c), offsetof(struct b, d));
    return 0;
}
```

```console
$ gcc -Wall -Wextra tekislash.c -o tekislash
$ ./tekislash
struct a: sizeof = 12
  c: 0, i: 4, d: 8
struct b: sizeof = 8
  i: 0, c: 4, d: 5
```

`struct a` maydonlari 1 + 4 + 1 = 6 bayt, lekin `sizeof` = **12**! Nega:

```text
struct a:
siljish: 0    1  2  3    4  5  6  7    8    9 10 11
        [c ][ to'ldiruvchi ][   i    ][d ][ to'ldiruvchi ]
         1 bayt  3 bayt (bo'sh)  4 bayt  1 bayt  3 bayt (bo'sh)

struct b:
siljish: 0  1  2  3    4    5    6 7
        [   i    ][c ][d ][ to'ldiruvchi ]    -> 8 bayt
```

**Nega:** CPU `int` ni 4 ga karrali manzildan o'qishni afzal ko'radi (ba'zi arxitekturalarda boshqacha umuman o'qiy olmaydi). Kompilyator har bir maydonni o'z **tekislanishiga**
qo'yadi (`char` → 1, `int` → 4, `long` → 8, ko'rsatkich → 8): `i` 4 ga bo'linadigan siljishda (4-baytdan) boshlanishi kerak, shuning uchun `c` dan keyin 3 bayt bo'sh qoladi.
Struct oxirini ham eng katta tekislanishga karrali qiladi (massivda keyingi element ham to'g'ri tekislansin): 6 → 12.

**Qoida:** maydonlarni **kattadan kichikka** tartiblasangiz, joy tejaladi (`struct b` = 8 bayt, `struct a` = 12).

**`offsetof`** — maydonning struktura boshidan siljishi: `offsetof(struct a, i)` = 4. `container_of` shu bilan ishlaydi (7-bob).

## 9.3. `__attribute__((packed))` — to'ldiruvchisiz

**Hayotdan misol: chamadonni zich taxlash.** Samolyotga chiqishda har bir santimetr hisobda: bo'sh joy qoldirmay taxlaysiz. Olish biroz noqulay (sekinroq), lekin joy tejaladi.
Disk va tarmoq formatlarida aynan shunday: har bir bayt standartda qat'iy belgilangan.

Apparat va disk formatlari aniq baytma-bayt tuzilishga ega — to'ldiruvchi bo'lmasligi kerak:

```c
/* packed_misol.c - disk bo'lim jadvali yozuvi: aniq 16 bayt */
#include <stdint.h>
#include <stdio.h>

struct __attribute__((packed)) mbr_yozuv {
    uint8_t  holat;
    uint8_t  chs_boshi[3];
    uint8_t  tur;
    uint8_t  chs_oxiri[3];
    uint32_t lba_boshi;
    uint32_t sektorlar;
};
_Static_assert(sizeof(struct mbr_yozuv) == 16, "MBR yozuvi 16 bayt bo'lishi kerak");

struct oddiy_mbr {                          /* packed SIZ - taqqoslash uchun */
    uint8_t  holat;
    uint8_t  chs_boshi[3];
    uint8_t  tur;
    uint8_t  chs_oxiri[3];
    uint32_t lba_boshi;
    uint32_t sektorlar;
};

int main(void)
{
    printf("packed: %zu bayt, packed siz: %zu bayt\n",
           sizeof(struct mbr_yozuv), sizeof(struct oddiy_mbr));
    return 0;
}
```

```console
$ gcc -Wall -Wextra packed_misol.c -o packed_misol
$ ./packed_misol
packed: 16 bayt, packed siz: 16 bayt
```

Bu yerda maydonlar **tabiiy** to'ldiruvchisiz sig'adi (1+3+1+3 = 8, keyin 4 ga karrali `uint32_t`) — ikkala hajm bir xil. Lekin `packed` **kafolat** beradi: kompilyator, platforma yoki
maydon qo'shilsa ham, tuzilma **aynan 16 bayt**. Shuning uchun `_Static_assert` bilan birga yoziladi:

`_Static_assert(shart, "xabar")` — **kompilyatsiya paytidagi** tekshiruv: shart yolg'on bo'lsa, kod umuman yig'ilmaydi. Mana, maydonni noto'g'ri qo'shib ko'ramiz:

```c
/* assert_xato.c - hajm noto'g'ri bo'lsa, yig'ilmaydi */
#include <stdint.h>

struct yozuv {
    uint8_t tur;
    uint32_t qiymat;                    /* tur dan keyin 3 bayt to'ldiruvchi */
};
_Static_assert(sizeof(struct yozuv) == 5, "yozuv aniq 5 bayt bo'lishi kerak");

int main(void)
{
    return 0;
}
```

```console
$ gcc -Wall -Wextra assert_xato.c -o assert_xato # xato kutiladi
assert_xato.c:8:1: error: static assertion failed: "yozuv aniq 5 bayt bo\'lishi kerak"
    8 | _Static_assert(sizeof(struct yozuv) == 5, "yozuv aniq 5 bayt bo'lishi kerak");
      | ^~~~~~~~~~~~~~
```

`sizeof(struct yozuv)` 8 chiqdi (5 emas), kompilyator **darhol** to'xtatdi. Bunday xato ish vaqtida topilsa, disk buzilgan bo'lardi. Apparat tuzilmalarida doim yozing — bitta noto'g'ri maydon
butun tuzilmani siljitadi.

MyOS'da: GDT yozuvlari (`kernel/arch/gdt.c`), ext2 superbloki va inode (`kernel/fs/ext2.c`), ELF sarlavhasi (`kernel/sys/elf.c`), MBR/GPT (`kernel/fs/block.c`), Multiboot2 teglari.

**Ehtiyot:** packed struct maydonining manzilini olish (`&s->lba_boshi`) tekislanmagan ko'rsatkich beradi — ba'zi arxitekturalarda qulaydi. GCC `-Waddress-of-packed-member` bilan ogohlantiradi.
Maydonni avval oddiy o'zgaruvchiga nusxalang.

> **Eslab qoling:** apparat/disk/tarmoq tuzilmasi = `packed` + `_Static_assert(sizeof(...) == aniq_son)`.

## 9.4. Bit maydonlari

```c
/* bit_maydon.c - struct ichida bitlar */
#include <stdio.h>

struct bayroqlar {
    unsigned faol    : 1;       /* 1 bit */
    unsigned rejim   : 2;       /* 2 bit: 0..3 */
    unsigned ustuvor : 5;
};

int main(void)
{
    struct bayroqlar b = { 1, 3, 17 };
    printf("sizeof = %zu bayt (8 bit ishlatilgan, lekin unsigned birligi 4 bayt)\n", sizeof(b));
    printf("faol=%u rejim=%u ustuvor=%u\n", b.faol, b.rejim, b.ustuvor);

    b.rejim = 4;                /* 2 bitga sig'maydi: 4 = 100 -> pastki 2 bit = 00 */
    printf("rejim = 4 yozildi, o'qildi: %u (qirqildi!)\n", b.rejim);
    return 0;
}
```

```console
$ gcc -Wall -Wextra bit_maydon.c -o bit_maydon # xato kutiladi
bit_maydon.c: In function ‘main’:
bit_maydon.c:16:15: warning: unsigned conversion from ‘int’ to ‘unsigned char:2’ changes value from ‘4’ to ‘0’ [-Woverflow]
   16 |     b.rejim = 4;                /* 2 bitga sig'maydi: 4 = 100 -> pastki 2 bit = 00 */
      |               ^
$ ./bit_maydon
sizeof = 4 bayt (8 bit ishlatilgan, lekin unsigned birligi 4 bayt)
faol=1 rejim=3 ustuvor=17
rejim = 4 yozildi, o'qildi: 0 (qirqildi!)
```

`: 2` — "bu maydon faqat 2 bit" (1 + 2 + 5 = 8 bit ishlatilgan, lekin kompilyator ularni `unsigned` ning 4 baytiga joylaydi). Sig'maydigan qiymat **qirqiladi** (kompilyator ogohlantirdi). Ixcham, lekin bitlarning xotiradagi tartibi **kompilyatorga bog'liq**.
Shuning uchun apparat registrlari uchun ko'pincha 3-bobdagi bit maskalar (`#define FLAG (1u << 3)`) afzal ko'riladi — natija aniq.

## 9.5. `union` — bitta xotira, turli ko'rinishlar

**Hayotdan misol: transformer divan.** Kunduzi divan, kechasi karavot — lekin **bir vaqtda faqat bittasi**, joy esa bitta. `union` da ham barcha maydonlar **bitta xotirani bo'lishadi**.
Qaysi rejimda ekanini alohida belgi (`enum`) bilan yozib qo'yish kerak — aks holda divanda uxlayotgan odamga mehmon o'tirib qoladi.

```c
/* union_misol.c - bir xil baytlar, turlicha o'qish */
#include <stdint.h>
#include <stdio.h>

union qiymat {
    uint32_t u32;
    uint8_t  bayt[4];
    float    f;
};

int main(void)
{
    union qiymat q;
    q.u32 = 0x11223344;
    printf("sizeof(union) = %zu\n", sizeof(q));
    printf("baytlar: %02x %02x %02x %02x\n", q.bayt[0], q.bayt[1], q.bayt[2], q.bayt[3]);

    q.f = 1.0f;
    printf("1.0f ning bitlari: 0x%08x\n", q.u32);
    return 0;
}
```

```console
$ gcc -Wall -Wextra union_misol.c -o union_misol
$ ./union_misol
sizeof(union) = 4
baytlar: 44 33 22 11
1.0f ning bitlari: 0x3f800000
```

**Nima ko'rdik:**

- `sizeof(union)` = 4: barcha maydonlar **bitta joyda** boshlanadi; hajmi — eng katta maydonniki.
- `u32 = 0x11223344` ni baytlarda o'qiganda **`44` birinchi** chiqdi: x86 da kichik bayt oldin saqlanadi (little-endian, 16-bob).
- `f = 1.0f` ni yozib, `u32` ni o'qisak — `0x3f800000`: bu **floatning ichki bitlari** (20-bob).

Qayerda kerak:

- **Bir xil baytlarni turlicha o'qish:** tarmoq paketi, disk sektori, registr.
- **"Belgilangan union" (tagged union)** — bir nechta turdan biri:

```c
/* tagged_union.c - belgilangan union: divan va uning rejimi */
#include <stdio.h>

struct token {
    enum { SON, AMAL } tur;             /* qaysi maydon haqiqiy ekanini bildiradi */
    union {
        long son;
        char amal;
    };
};

static void chop(const struct token *t)
{
    if (t->tur == SON)
        printf("SON: %ld\n", t->son);
    else
        printf("AMAL: %c\n", t->amal);
}

int main(void)
{
    struct token a = { SON, { .son = 42 } };
    struct token b = { AMAL, { .amal = '+' } };
    chop(&a);
    chop(&b);
    return 0;
}
```

```console
$ gcc -Wall -Wextra tagged_union.c -o tagged_union
$ ./tagged_union
SON: 42
AMAL: +
```

`tur` maydoni — "divan hozir qaysi rejimda" belgisi. U `SON` bo'lsa `son` o'qiladi, `AMAL` bo'lsa — `amal`. Belgisiz `union` ni o'qish xavfli: nima yozilganini bilmaysiz.
Python'dagi "istalgan tur" o'zgaruvchining C'dagi qo'lda yasalgan varianti.

## 9.6. `enum` — nomlangan butun sonlar

**Hayotdan misol: svetofor ranglari.** Svetoforda faqat uchta holat bor: qizil, sariq, yashil. "0, 1, 2" deb yozish o'rniga nom beriladi: `QIZIL, SARIQ, YASHIL`.
Kod o'qilishi oson bo'ladi va `switch` da biror holatni unutsangiz, kompilyator ogohlantiradi.

```c
/* enum_misol.c - enum va switch */
#include <stdio.h>

enum holat { TAYYOR, ISHLAYAPTI, UXLAYAPTI, TUGADI };    /* 0, 1, 2, 3 */
enum { SEKTOR = 512, SAHIFA = 4096 };                    /* aniq qiymatlar */

static const char *nom(enum holat h)
{
    switch (h) {
    case TAYYOR:      return "tayyor";
    case ISHLAYAPTI:  return "ishlayapti";
    case UXLAYAPTI:   return "uxlayapti";
    }                                    /* TUGADI yo'q! */
    return "?";
}

int main(void)
{
    enum holat h = ISHLAYAPTI;
    printf("h = %d, nomi: %s\n", h, nom(h));
    printf("TUGADI = %d, SEKTOR = %d, SAHIFA = %d\n", TUGADI, SEKTOR, SAHIFA);
    printf("nom(TUGADI) = %s\n", nom(TUGADI));
    return 0;
}
```

```console
$ gcc -Wall -Wextra enum_misol.c -o enum_misol # xato kutiladi
enum_misol.c: In function ‘nom’:
enum_misol.c:9:5: warning: enumeration value ‘TUGADI’ not handled in switch [-Wswitch]
    9 |     switch (h) {
      |     ^~~~~~
$ ./enum_misol
h = 1, nomi: ishlayapti
TUGADI = 3, SEKTOR = 512, SAHIFA = 4096
nom(TUGADI) = ?
```

- `enum holat { TAYYOR, ... }` — nomlar avtomatik `0, 1, 2, 3` oladi. `enum { SEKTOR = 512 }` — aniq qiymatlar.
- Kompilyator ogohlantirdi: `switch` da `TUGADI` **yo'q** (`-Wswitch`) — shuning uchun `enum` `#define` dan afzal. Yangi holat qo'shsangiz, unutilgan joylarni kompilyator ko'rsatadi.
- Debuggerda ham nomi ko'rinadi. MyOS'da: jarayon holatlari (`kernel/proc/process.h`), signal raqamlari.

## 9.7. `typedef` — turga yangi nom

**Hayotdan misol: laqab.** "Abdurahmon Abdullayevich" o'rniga "Rahmon aka" — bitta odam, qisqa nom.

```c
/* typedef_misol.c - turga laqab */
#include <stdio.h>

typedef unsigned long ulong;
struct nuqta { int x, y; };
typedef struct nuqta nuqta_t;
typedef int (*amal_fn)(int, int);       /* funksiya ko'rsatkichi - eng foydali ishlatilishi */

static int qoshish(int a, int b) { return a + b; }

int main(void)
{
    ulong katta = 4000000000UL;
    nuqta_t n = { 3, 4 };
    amal_fn amal = qoshish;             /* int (*amal)(int, int) o'rniga */
    printf("%lu, (%d, %d), amal(2, 3) = %d\n", katta, n.x, n.y, amal(2, 3));
    return 0;
}
```

```console
$ gcc -Wall -Wextra typedef_misol.c -o typedef_misol
$ ./typedef_misol
4000000000, (3, 4), amal(2, 3) = 5
```

`typedef <eski tur> <yangi nom>;` — yangi tur **yaratmaydi**, faqat ikkinchi nom beradi.

**Linux (va MyOS) uslubi:** strukturalar uchun `typedef` **ishlatilmaydi** — `struct process *p` yozuvi "bu struktura" ekanini darhol ko'rsatadi. `typedef` faqat: aniq o'lchamli turlar (`uint32_t`),
funksiya ko'rsatkichlari va ichki tuzilishi ataylab yashirilgan ("opaque") turlar uchun.

## 9.8. Opaque (yashirin) tur — interfeys va amalga oshirishni ajratish

**Hayotdan misol: televizor pulti.** Pultda tugmalar bor (funksiyalar), lekin televizor ichidagi platani ko'rmaysiz va unga tegolmaysiz. Ishlab chiqaruvchi ichini o'zgartirsa ham, pult ishlayveradi.
`FILE *` aynan shunday: siz `fopen`, `fprintf` tugmalarini bosasiz, `FILE` ichida nima borligini bilmaysiz.

Uch faylli kichik misol (1-bobdagi `.h` / `.c` / `main.c` bo'linishi): **hisoblagich**, ichida nima borligini tashqi dunyo bilmaydi.

```c
/* hisob.h - tashqi dunyo faqat shuni ko'radi */
#pragma once

struct hisob;                                   /* "shunday tur bor" - ichi noma'lum */
struct hisob *hisob_yarat(void);
void hisob_oshir(struct hisob *h, int n);
int hisob_qiymat(const struct hisob *h);
void hisob_ozod(struct hisob *h);
```

```c
/* hisob.c - ichki tafsilot */
#include <stdlib.h>
#include "hisob.h"

struct hisob {
    int qiymat;                                 /* ichki maydon: tashqaridan ko'rinmaydi */
    int oshirishlar;
};

struct hisob *hisob_yarat(void)
{
    return calloc(1, sizeof(struct hisob));     /* nollangan */
}

void hisob_oshir(struct hisob *h, int n)
{
    h->qiymat += n;
    h->oshirishlar++;
}

int hisob_qiymat(const struct hisob *h)
{
    return h->qiymat;
}

void hisob_ozod(struct hisob *h)
{
    free(h);
}
```

```c
/* hisob_main.c - foydalanuvchi */
#include <stdio.h>
#include "hisob.h"

int main(void)
{
    struct hisob *h = hisob_yarat();
    hisob_oshir(h, 5);
    hisob_oshir(h, 7);
    printf("qiymat = %d\n", hisob_qiymat(h));
    hisob_ozod(h);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -c hisob.c
$ gcc -Wall -Wextra -c hisob_main.c
$ gcc hisob.o hisob_main.o -o hisob_dastur
$ ./hisob_dastur
qiymat = 12
```

Endi tashqaridan ichki maydonga tegib ko'ramiz:

```c
/* hisob_xato.c - yashirin maydonga tegish */
#include "hisob.h"

int main(void)
{
    struct hisob *h = hisob_yarat();
    h->qiymat = 999;                            /* xato: struct ichi ko'rinmaydi */
    hisob_ozod(h);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -c hisob_xato.c # xato kutiladi
hisob_xato.c: In function ‘main’:
hisob_xato.c:7:6: error: invalid use of undefined type ‘struct hisob’
    7 |     h->qiymat = 999;                            /* xato: struct ichi ko'rinmaydi */
      |      ^~
```

Kompilyator: "`struct hisob` ning ichini bilmayman" (`dereferencing pointer to incomplete type`). Foydalanuvchi faqat ko'rsatkich bilan ishlaydi va ichki maydonlarga tega olmaydi —
amalga oshirishni keyin butunlay o'zgartirsangiz (masalan, `qiymat` ni `long` qilsangiz), uni ishlatadigan kod buzilmaydi. `FILE *` (stdio) aynan shunday. 17-mashq shu uslubda.

## 9.9. Struktura ichida struktura va ko'rsatkichlar

Yadro tuzilmalari odatda boshqa tuzilmalarga ko'rsatkichlar tarmog'i:

```text
struct process {
    int pid;
    enum holat holat;
    struct mm *mm;                  /* xotira xaritasi */
    struct file *fayllar[64];       /* ochiq fayllar jadvali */
    struct process *ota;
    struct list_head node;          /* scheduler navbatiga ulanish uchun */
};
```

(MyOS'dagi haqiqiy `struct process` — `kernel/proc/process.h`. Uni oching va har bir maydon nimaga kerakligini izohlardan o'qing.)

## Hayotdan misol va to'liq dastur

**Avtomobil ro'yxatga olish.** Bir dasturda: `struct`, `enum`, `union` (to'lov turi), padding va struct nusxasi.

```c
/* avto.c - struct, enum, union, padding, struct nusxasi */
#include <stddef.h>
#include <stdio.h>

enum yoqilgi { BENZIN, GAZ, ELEKTR };
static const char *yoqilgi_nomi[] = { "benzin", "gaz", "elektr" };

enum tolov_turi { NAQD, KARTA };
struct tolov {
    enum tolov_turi tur;                        /* divan hozir qaysi rejimda */
    union {
        long summa;                             /* NAQD bo'lsa */
        char karta[20];                         /* KARTA bo'lsa */
    } u;
};

struct mashina {
    char raqam[12];
    char rusum[16];
    int yil;
    enum yoqilgi yoqilgi;
};

/* Maydonlar tartibi hajmga ta'sir qiladi */
struct yomon { char a; long b; char c; };       /* 1 + 7 bo'sh + 8 + 1 + 7 bo'sh = 24 */
struct yaxshi { long b; char a; char c; };      /* 8 + 1 + 1 + 6 bo'sh = 16 */

static void korsat(const struct mashina *m)
{
    printf("  %-10s %-8s %d-yil, %s\n", m->raqam, m->rusum, m->yil, yoqilgi_nomi[m->yoqilgi]);
}

static void tolov_korsat(const struct tolov *t)
{
    switch (t->tur) {
    case NAQD:
        printf("  to'lov: naqd %ld so'm\n", t->u.summa);
        break;
    case KARTA:
        printf("  to'lov: karta %s\n", t->u.karta);
        break;
    }
}

int main(void)
{
    struct mashina garaj[] = {
        { "01A777AA", "Cobalt", 2021, GAZ },
        { "10B123CD", "Tracker", 2023, BENZIN },
        { "30Z555ZZ", "BYD", 2025, ELEKTR },
    };
    printf("Ro'yxatdagi mashinalar:\n");
    for (size_t i = 0; i < sizeof(garaj) / sizeof(garaj[0]); i++)
        korsat(&garaj[i]);

    struct mashina nusxa = garaj[0];            /* butun hujjat nusxalandi */
    nusxa.yil = 1999;
    printf("Nusxa o'zgardi: %d, asli: %d\n", nusxa.yil, garaj[0].yil);

    struct tolov t1 = { NAQD, { .summa = 150000 } };
    struct tolov t2 = { KARTA, { .karta = "8600 **** **** 1234" } };
    tolov_korsat(&t1);
    tolov_korsat(&t2);

    printf("sizeof(struct yomon) = %zu, sizeof(struct yaxshi) = %zu\n",
           sizeof(struct yomon), sizeof(struct yaxshi));
    printf("struct mashina: %zu bayt, yil maydoni %zu-baytdan boshlanadi\n",
           sizeof(struct mashina), offsetof(struct mashina, yil));
    return 0;
}
```

```console
$ gcc -Wall -Wextra avto.c -o avto
$ ./avto
Ro'yxatdagi mashinalar:
  01A777AA   Cobalt   2021-yil, gaz
  10B123CD   Tracker  2023-yil, benzin
  30Z555ZZ   BYD      2025-yil, elektr
Nusxa o'zgardi: 1999, asli: 2021
  to'lov: naqd 150000 so'm
  to'lov: karta 8600 **** **** 1234
sizeof(struct yomon) = 24, sizeof(struct yaxshi) = 16
struct mashina: 36 bayt, yil maydoni 28-baytdan boshlanadi
```

**Kodda nimalar bor:**

| Qism | Nima | Nega |
|---|---|---|
| `enum yoqilgi { BENZIN, GAZ, ELEKTR }` | 0, 1, 2 ga nom | yoqilg'i turini raqam emas, **nom** bilan yozamiz |
| `yoqilgi_nomi[]` | satrlar massivi; indeks = enum qiymati | `yoqilgi_nomi[GAZ]` = `"gaz"` — `enum` raqam bo'lgani uchun indeks sifatida ishlaydi |
| `struct tolov` | `tur` (rejim belgisi) + `union` (`summa` yoki `karta`) | to'lov **yo naqd, yo karta** — joy tejaladi, `tur` qaysi biri ekanini aytadi |
| `struct mashina` | `raqam`, `rusum`, `yil`, `yoqilgi` | texpasport |
| `struct yomon` / `yaxshi` | bir xil maydonlar, boshqacha tartib | padding hajmga ta'sirini ko'rsatish (9.2) |
| `korsat(const struct mashina *m)` | struktura **ko'rsatkichi** bilan o'qiydi, `const` — o'zgartirmaydi | nusxalamaslik (5.4) |
| `nusxa = garaj[0]` | **butun** struktura nusxalandi | `nusxa.yil` ni o'zgartirdik, asl `garaj[0]` **o'zgarmadi** |

**Hajmlar:** `struct yomon`: `a` (1) + 7 bo'sh + `b` (8) + `c` (1) + 7 bo'sh = **24**; `struct yaxshi`: `b` (8) + `a` (1) + `c` (1) + 6 bo'sh = **16**. `struct mashina`: 12 + 16 = 28 bayt, `yil` aynan 28-baytdan (4 ga karrali),
`yoqilgi` (4) → jami **36**.

**Sinab ko'ring:** `enum yoqilgi` ga `GIBRID` qo'shing (va `yoqilgi_nomi` ga "gibrid"). `tolov_korsat` dagi `case KARTA:` ni o'chiring — `-Wall` qanday ogohlantirish beradi?

## Bob xulosasi (yodlash uchun)

1. `struct` — bir nechta maydonni bitta turga yig'adi; `.` (qiymat) va `->` (ko'rsatkich) bilan ishlanadi; `=` to'liq nusxalaydi; oxirida `;`.
2. Kompilyator maydonlar orasiga **to'ldiruvchi** (padding) qo'yadi; kattadan kichikka tartiblasangiz tejaladi. `offsetof` — maydon siljishi.
3. Apparat/disk tuzilmasi uchun — `packed` + `_Static_assert(sizeof == ...)`.
4. `union` — bitta xotira, turli ko'rinish (qaysi biri haqiqiy ekanini `enum` bilan belgilang); `enum` — nomlangan butun sonlar.
5. `typedef` — laqab (strukturalarga Linux uslubida **qilinmaydi**); opaque tur — `.h` da faqat `struct x;` va funksiyalar, ichi `.c` da.

## Savol-javob

**Struct'ni `malloc` bilan qanday yarataman?**
`struct talaba *t = malloc(sizeof(*t));` va tekshiring. Nollangan kerak bo'lsa — `calloc(1, sizeof(*t))`.

**Nega `memcmp` bilan structlarni solishtirish ishonchsiz?**
To'ldiruvchi baytlarda axlat bo'lishi mumkin — maydonlar teng bo'lsa ham `memcmp` farq topadi.

**Flexible array member nima?**
Oxirgi maydon — o'lchamsiz massiv: sarlavha va ma'lumot bitta blokda.

```c
/* flexible.c - o'lchamsiz massiv maydoni */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct paket {
    uint32_t uzunlik;
    uint8_t data[];                             /* oxirgi maydon - o'lchamsiz massiv */
};

int main(void)
{
    size_t n = 5;
    struct paket *p = malloc(sizeof(*p) + n);   /* sarlavha + n bayt ma'lumot bitta blokda */
    if (!p)
        return 1;
    p->uzunlik = (uint32_t)n;
    memcpy(p->data, "salom", n);
    printf("sizeof(struct paket) = %zu, ma'lumot: %.5s\n", sizeof(*p), (char *)p->data);
    free(p);
    return 0;
}
```

```console
$ gcc -Wall -Wextra flexible.c -o flexible
$ ./flexible
sizeof(struct paket) = 4, ma'lumot: salom
```

`sizeof(*p)` faqat sarlavhani (4 bayt) o'z ichiga oladi; `data` uchun joyni `malloc` da qo'shib so'raymiz. Yadroda tez-tez uchraydi: sarlavha va ma'lumot bitta ajratmada.

## O'zingizni tekshiring

1. `struct { char a; long b; char c; }` hajmi? Qanday qilib 16 ga tushirish mumkin?
2. Disk tuzilmasi uchun nega `packed` va `_Static_assert` kerak?
3. `union { uint32_t x; uint8_t b[4]; } u; u.x = 1;` — `u.b[0]` x86'da nechaga teng?
4. Linux uslubida nega structlarga `typedef` qilinmaydi?
5. Opaque tur nima beradi?

<details><summary>Javoblar</summary>

1. 24 (1 + 7 to'ldiruvchi + 8 + 1 + 7). Tartib `long b; char a; char c;` → 16.
2. Disk formati bayt-baybayt aniq: kompilyator qo'shadigan to'ldiruvchi maydonlarni siljitadi; `_Static_assert` hajm xatosini kompilyatsiyada ushlaydi.
3. 1 (kichik bayt oldin — little-endian).
4. `struct x *` yozuvi turning struktura ekanini aniq ko'rsatadi; yashirish kodni tushunishni qiyinlashtiradi.
5. Foydalanuvchi ichki maydonlarga tega olmaydi; ichki tuzilishni interfeysni buzmasdan o'zgartirish mumkin.
</details>

## Mashq

- **13** (struct vec), **16** (tugun), **17** (opaque xesh), **19** (struct saralash), **22** (halqa bufer struct'i), **23** (`container_of`, `offsetof`).
- Qo'shimcha: `sizeof` va `offsetof` bilan 9.2 va 9.11-savoldagi tuzilmalarning joylashuvini chiqarib, qog'ozdagi rasmingiz bilan solishtiring.

<!-- loyiha:boshi -->
## Loyiha: geometriya — nuqta va to'g'ri to'rtburchak

**Maqsad:** ma'lumotni **tuzilma** sifatida modellashtirish: `struct` ichida `struct`, `enum` va `typedef`.
**Bobdan ishlatiladi:** `struct`, ichma-ich `struct`, `enum`, `typedef`, struktura qiymat sifatida uzatish.

**Talab:** to'g'ri to'rtburchak (ikki burchak nuqtasi bilan) uchun: yuza, nuqta ichidami, ikki to'rtburchak
kesishishi, ularni o'rab oluvchi eng kichik to'rtburchak.
**Ma'lumotlar:**
```text
struct nuqta       { x, y }
struct tortburchak { pastchap (nuqta), yuqoriong (nuqta) }
enum joy           { TASHQARIDA, CHEGARADA, ICHIDA }
```
**Funksiyalar:** `yuza`, `joylashuv`, `kesishma` (natijani chiqish parametri orqali), `oraydi`.

```c
/* geometriya.c - nuqta va to'g'ri to'rtburchak */
#include <stdio.h>

struct nuqta {
    int x, y;
};
struct tortburchak {
    struct nuqta pastchap, yuqoriong;
};
typedef struct tortburchak Tb;                  /* qisqa nom */
enum joy { TASHQARIDA, CHEGARADA, ICHIDA };

static int min(int a, int b) { return a < b ? a : b; }
static int max(int a, int b) { return a > b ? a : b; }

static int yuza(Tb t)                           /* struktura NUSXA sifatida uzatiladi */
{
    return (t.yuqoriong.x - t.pastchap.x) * (t.yuqoriong.y - t.pastchap.y);
}

static enum joy joylashuv(Tb t, struct nuqta p)
{
    if (p.x < t.pastchap.x || p.x > t.yuqoriong.x || p.y < t.pastchap.y || p.y > t.yuqoriong.y)
        return TASHQARIDA;
    if (p.x == t.pastchap.x || p.x == t.yuqoriong.x || p.y == t.pastchap.y || p.y == t.yuqoriong.y)
        return CHEGARADA;
    return ICHIDA;
}

static int kesishma(Tb a, Tb b, Tb *natija)     /* 1 - kesishadi, natija to'ldiriladi */
{
    struct nuqta pc = { max(a.pastchap.x, b.pastchap.x), max(a.pastchap.y, b.pastchap.y) };
    struct nuqta yo = { min(a.yuqoriong.x, b.yuqoriong.x), min(a.yuqoriong.y, b.yuqoriong.y) };
    if (pc.x >= yo.x || pc.y >= yo.y)
        return 0;
    natija->pastchap = pc;
    natija->yuqoriong = yo;
    return 1;
}

static Tb oraydi(Tb a, Tb b)
{
    Tb r = { { min(a.pastchap.x, b.pastchap.x), min(a.pastchap.y, b.pastchap.y) },
             { max(a.yuqoriong.x, b.yuqoriong.x), max(a.yuqoriong.y, b.yuqoriong.y) } };
    return r;
}

static void chiqar(const char *nom, Tb t)
{
    printf("%s: (%d,%d)-(%d,%d), yuza %d\n", nom, t.pastchap.x, t.pastchap.y, t.yuqoriong.x,
           t.yuqoriong.y, yuza(t));
}

int main(void)
{
    Tb a = { { 0, 0 }, { 6, 4 } };
    Tb b = { { 4, 2 }, { 10, 8 } };
    Tb c = { { 20, 20 }, { 25, 25 } };
    const char *joy_nomi[] = { "tashqarida", "chegarada", "ichida" };

    chiqar("A", a);
    chiqar("B", b);
    struct nuqta sinov[] = { { 3, 2 }, { 6, 1 }, { 7, 7 } };
    for (int i = 0; i < 3; i++)
        printf("(%d,%d) A ga nisbatan: %s\n", sinov[i].x, sinov[i].y, joy_nomi[joylashuv(a, sinov[i])]);

    Tb k;
    if (kesishma(a, b, &k))
        chiqar("A va B kesishmasi", k);
    printf("A va C kesishadimi? %s\n", kesishma(a, c, &k) ? "ha" : "yo'q");
    chiqar("A va C ni o'rovchi", oraydi(a, c));
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined geometriya.c -o geometriya
$ ./geometriya
A: (0,0)-(6,4), yuza 24
B: (4,2)-(10,8), yuza 36
(3,2) A ga nisbatan: ichida
(6,1) A ga nisbatan: chegarada
(7,7) A ga nisbatan: tashqarida
A va B kesishmasi: (4,2)-(6,4), yuza 4
A va C kesishadimi? yo'q
A va C ni o'rovchi: (0,0)-(25,25), yuza 625
```

Nega `Tb *natija` ko'rsatkich, `Tb` qiymat emas? Funksiya ikki natija qaytarishi kerak: "kesishadimi" (`return`) va
"kesishma to'rtburchagi" (chiqish parametri) — 5.4 va 7-boblar. Struktura kichik bo'lgani uchun `yuza(Tb t)` nusxa
bilan ishlaydi; yadroda katta struktura **doim** ko'rsatkich bilan uzatiladi.

**Kengaytiring:** `perimetr(Tb)` qo'shing. `enum joy` ga `BURCHAKDA` qiymatini qo'shing va `joylashuv` ni yangilang.

## Mustaqil loyiha: poker qo'llari ★★★

**Vazifa:** 5 kartalik poker qo'lini baholang va tasodifiy taqsimlashni simulyatsiya qiling. Fayl: `poker.c`.

**Ma'lumotlar.** Karta — `struct karta { int rank; enum masti masti; }`. Rank: 2..14 (`T`=10, `J`=11, `Q`=12,
`K`=13, `A`=14). Mastlar: `enum masti { S, H, D, C }`. Matn belgisi: rank `"23456789TJQKA"`, mast `"SHDC"`
(masalan `TH` — o'n cherva, `AS` — tuz).

**Kombinatsiyalar** (kuchli → kuchsiz) va chiqariladigan nomi:

| Nomi | Shart |
|---|---|
| `Strit-flesh` | bir mast **va** ketma-ket 5 rank |
| `Kare` | to'rt bir xil rank |
| `Full-xaus` | uchlik + juftlik |
| `Flesh` | hammasi bir mast |
| `Strit` | ketma-ket 5 rank (A **faqat eng yuqori**: `T J Q K A` — strit, `A 2 3 4 5` — emas) |
| `Uchlik` | uchta bir xil rank |
| `Ikki juft` | ikkita juftlik |
| `Bir juft` | bitta juftlik |
| `Yuqori karta` | boshqa hech narsa |

**A qism — aniq qo'llar.** Har birini `"TH JH QH KH AH"` shaklidagi satrdan o'qing (`strchr` bilan
belgi indeksini toping), baholang va `qo'l -> nom` ko'rinishida chiqaring. Qo'llar (tartib bilan):
`TH JH QH KH AH`, `9S 9H 9D 9C 2H`, `KS KH KD 4C 4H`, `2D 7D 9D JD KD`, `5S 6H 7D 8C 9H`, `QS QH QD 3C 8H`,
`JS JH 4D 4C AH`, `8S 8H 2D 5C KH`, `2S 5H 9D JC KH`, `AS 2H 3D 4C 5H`.

**B qism — taqsimlash.** Karta to'plami: mast bo'yicha `S,H,D,C`, har mastda rank `2..14`
(`indeks = mast*13 + (rank-2)`). Aralashtirish (Fisher–Yates), tasodifiy son generatori aniq berilgan:

```text
uint32_t davlat = 2026;
sonni_ol():  davlat = davlat * 1103515245u + 12345u;  return (davlat >> 16) & 0x7fff;
i = 51 dan 1 gacha:  j = sonni_ol() % (i + 1);  to'plam[i] va to'plam[j] almashtiriladi
```

Keyin 6 ta qo'l: `h`-qo'l = `to'plam[5h .. 5h+4]`. Qo'l chiqarilishidan oldin kartalar **rank kamayishi**
bo'yicha (teng rankda mast tartibi `S,H,D,C`) saralansin.

**Kutilgan natija** (`darslik/loyihalar/09_poker/kutilgan.txt`):

```text
A qism:
  TH JH QH KH AH -> Strit-flesh
  9S 9H 9D 9C 2H -> Kare
  KS KH KD 4C 4H -> Full-xaus
  2D 7D 9D JD KD -> Flesh
  5S 6H 7D 8C 9H -> Strit
  QS QH QD 3C 8H -> Uchlik
  JS JH 4D 4C AH -> Ikki juft
  8S 8H 2D 5C KH -> Bir juft
  2S 5H 9D JC KH -> Yuqori karta
  AS 2H 3D 4C 5H -> Yuqori karta
B qism:
  Qo'l 1: AS 7H 7D 3C 2H -> Bir juft
  Qo'l 2: JD TH 4H 3D 2C -> Yuqori karta
  Qo'l 3: JH TD 9D 5H 4C -> Yuqori karta
  Qo'l 4: AC KS 9H 6S 2D -> Yuqori karta
  Qo'l 5: JC 9C 6C 5S 5C -> Bir juft
  Qo'l 6: JS 9S 8S 8H 5D -> Bir juft
```

**Maslahat** (yechim emas):
- Avval har rankdan nechta borligini sanang (`int soni[15]`). Juft/uchlik/kare/full-xaus shu sanoqdan aniqlanadi.
- Strit: saralangan qo'lda `rank[i] == rank[i+1] + 1` har bir i uchun. `A 2 3 4 5` da tuz 14 — shart bajarilmaydi.
- Ustuvorlik tartibini `if` zanjirida qattiq bering: yuqori kombinatsiya avval tekshiriladi.
- `strchr("23456789TJQKA", c) - "23456789TJQKA"` — belgi indeksi (7.4: ko'rsatkich ayirmasi).
- LCG aniq shu formulada bo'lsa, sizniki ham aynan mening natijamni beradi — birinchi qo'l orqali tekshiring.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined poker.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/09_poker/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [10-bob. Preprotsessor](10-preprotsessor.md)
