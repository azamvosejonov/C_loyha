# 9-bob. `struct`, `union`, `enum`, `typedef`

> **Bu bobdan keyin:** o'z turlaringizni yarata olasiz, struktura xotirada qanday joylashishini
> (tekislash va to'ldiruvchi baytlar), `packed` va bit maydonlarini, `union` va `enum` ni bilasiz —
> apparat va disk tuzilmalarini C'da tasvirlash uchun hammasi kerak. Mashqlar: 13, 16, 17, 19, 22, 23.

> **To'liq ishlaydigan misol:** [misollar/09_struct.c](misollar/09_struct.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**`struct` — texpasport (9.1).** Avtomobil texpasportida bir nechta ma'lumot bor: davlat raqami, rusumi,
yili, rangi. Ular alohida qog'ozlarda emas — **bitta hujjatda**. `struct mashina` ham shunday: bir nechta
turli maydon, bitta nom ostida. Hujjatni nusxalash mumkin (`b = a;`) — hamma maydonlar birga ko'chadi.

**Tekislash va to'ldiruvchi — kitob javoni (9.2).** Javonda kichik va katta kitoblar bor. Qoida: katta
kitob faqat maxsus bo'linma **boshidan** qo'yiladi (protsessor 4 baytlik sonni 4 ga bo'linadigan manzildan
tez o'qiydi). Kichik kitobdan keyin katta kitob kelsa, orada bo'sh joy qoladi — bu **padding**. Kitoblarni
kattadan kichikka qarab tersangiz, bo'sh joy kamayadi.

**`packed` — chamadonni zich taxlash (9.3).** Samolyotga chiqishda har bir santimetr hisobda: bo'sh joy
qoldirmay taxlaysiz. Olish biroz noqulay (sekinroq), lekin joy tejaladi. Disk va tarmoq formatlarida
aynan shunday: har bir bayt standartda qat'iy belgilangan.

**`union` — transformer divan (9.5).** Kunduzi divan, kechasi karavot — lekin **bir vaqtda faqat
bittasi**, joy esa bitta. `union` da ham barcha maydonlar bitta xotirani bo'lishadi. Qaysi rejimda
ekanini alohida belgi (`enum`) bilan yozib qo'yish kerak — aks holda divanda uxlayotgan odamga
mehmon o'tirib qoladi.

**`enum` — svetofor ranglari (9.6).** Svetoforda faqat uchta holat bor: qizil, sariq, yashil.
"0, 1, 2" deb yozish o'rniga nom beriladi: `QIZIL, SARIQ, YASHIL`. Kod o'qilishi oson bo'ladi va
`switch` da biror holatni unutsangiz, kompilyator ogohlantiradi.

**`typedef` — laqab (9.7).** "Abdurahmon Abdullayevich" o'rniga "Rahmon aka" — bitta odam, qisqa nom.

**Opaque tur — televizor pulti (9.8).** Pultda tugmalar bor (funksiyalar), lekin televizor ichidagi
platani ko'rmaysiz va unga tegolmaysiz. Ishlab chiqaruvchi ichini o'zgartirsa ham, pult ishlayveradi.
`FILE *` aynan shunday: siz `fopen`, `fprintf` tugmalarini bosasiz, `FILE` ichida nima borligini bilmaysiz.

### To'liq dastur: avtomobil ro'yxatga olish

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

**Sinab ko'ring:** `enum yoqilgi` ga `GIBRID` qo'shing (va `yoqilgi_nomi` ga "gibrid"). `tolov_korsat`
dagi `case KARTA:` ni o'chiring — `-Wall` qanday ogohlantirish beradi?

## 9.1. `struct` — bir nechta qiymat bitta nom ostida

```c
struct talaba {
    char ism[32];
    int yosh;
    double ball;
};                              /* ; SHART! */

struct talaba t = { "Ali", 20, 87.5 };
struct talaba u = { .ism = "Vali", .ball = 91.0 };  /* nomlangan maydonlar; yosh = 0 */

t.yosh = 21;                    /* . - qiymat orqali */
struct talaba *p = &t;
p->ball = 90.0;                 /* -> - ko'rsatkich orqali */
```

Python'dagi `@dataclass` ga o'xshaydi, lekin metodlar yo'q — faqat ma'lumot.

**Savol: nega `}` dan keyin `;` kerak?**
Chunki `struct ... { }` — bu **tur**, va tur nomidan keyin darhol o'zgaruvchi e'lon qilish mumkin:

```c
struct nuqta { int x, y; } a, b;    /* tur ta'rifi + ikki o'zgaruvchi */
```

`;` "e'lon tugadi" degani. Unutilsa, kompilyator keyingi qatordagi narsani shu turdagi o'zgaruvchi
deb o'qishga harakat qiladi va g'alati xato beradi (ko'pincha keyingi funksiya ta'rifida).

### Struct bilan ishlash qoidalari

- `=` bilan **to'liq nusxalanadi** (ichidagi massiv ham!): `u = t;`. Python'dagi obyektlardan farqli —
  bu havola emas, haqiqiy nusxa.
- `==` bilan solishtirib **bo'lmaydi** — maydonma-maydon solishtiring (`memcmp` ham ishonchsiz — pastda to'ldiruvchi baytlar).
- Funksiyaga katta structni ko'rsatkich bilan uzating: `void chop_et(const struct talaba *t)`.

## 9.2. Xotirada joylashish: tekislash (alignment) va to'ldiruvchi (padding)

```c
struct a {
    char c;         /* 1 bayt */
    int  i;         /* 4 bayt */
    char d;         /* 1 bayt */
};
printf("%zu\n", sizeof(struct a));      /* 6 emas - 12! */
```

```text
siljish: 0    1  2  3    4  5  6  7    8    9 10 11
        [c ][ to'ldiruvchi ][   i    ][d ][ to'ldiruvchi ]
```

**Nega:** CPU `int` ni 4 ga karrali manzildan o'qishni afzal ko'radi (ba'zi arxitekturalarda boshqacha
umuman o'qiy olmaydi). Kompilyator har bir maydonni o'z **tekislanishiga** qo'yadi (`int` → 4, `long` → 8,
ko'rsatkich → 8) va struct oxirini ham eng katta tekislanishga karrali qiladi (massivda keyingi element
ham to'g'ri tekislansin).

Maydonlarni kattadan kichikka tartiblash joyni tejaydi:

```c
struct b { int i; char c; char d; };    /* sizeof = 8 */
```

**`offsetof`** — maydonning siljishi: `offsetof(struct a, i)` = 4. `container_of` shu bilan ishlaydi (7-bob).

## 9.3. `__attribute__((packed))` — to'ldiruvchisiz

Apparat va disk formatlari aniq baytma-bayt tuzilishga ega — to'ldiruvchi bo'lmasligi kerak:

```c
struct __attribute__((packed)) mbr_yozuv {     /* disk bo'lim jadvali yozuvi - aniq 16 bayt */
    uint8_t  holat;
    uint8_t  chs_boshi[3];
    uint8_t  tur;
    uint8_t  chs_oxiri[3];
    uint32_t lba_boshi;
    uint32_t sektorlar;
};
_Static_assert(sizeof(struct mbr_yozuv) == 16, "MBR yozuvi 16 bayt bo'lishi kerak");
```

`_Static_assert` — **kompilyatsiya paytidagi** tekshiruv: shart yolg'on bo'lsa, kod umuman yig'ilmaydi.
Apparat tuzilmalarida doim yozing — bitta noto'g'ri maydon butun tuzilmani siljitadi.

MyOS'da: GDT yozuvlari (`kernel/arch/gdt.c`), ext2 superbloki va inode (`kernel/fs/ext2.c`),
ELF sarlavhasi (`kernel/sys/elf.c`), MBR/GPT (`kernel/fs/block.c`), Multiboot2 teglari.

**Ehtiyot:** packed struct maydonining manzilini olish (`&s->lba_boshi`) tekislanmagan ko'rsatkich
beradi — ba'zi arxitekturalarda qulaydi. GCC `-Waddress-of-packed-member` bilan ogohlantiradi. Maydonni
avval oddiy o'zgaruvchiga nusxalang.

## 9.4. Bit maydonlari

```c
struct bayroqlar {
    unsigned faol    : 1;       /* 1 bit */
    unsigned rejim   : 2;       /* 2 bit: 0..3 */
    unsigned ustuvor : 5;
};
```

Ixcham, lekin bitlarning xotiradagi tartibi **kompilyatorga bog'liq**. Shuning uchun apparat
registrlari uchun ko'pincha bit maskalar (`#define FLAG (1u << 3)`) afzal ko'riladi — natija aniq.

## 9.5. `union` — bitta xotira, turli ko'rinishlar

```c
union qiymat {
    uint32_t u32;
    uint8_t  bayt[4];
    float    f;
};
union qiymat q;
q.u32 = 0x11223344;
printf("%02x\n", q.bayt[0]);    /* 44 - x86 da kichik bayt oldin (16-bob) */
```

Barcha maydonlar **bitta joyda** boshlanadi; hajmi — eng katta maydonniki. Qayerda kerak:

- **Bir xil baytlarni turlicha o'qish:** tarmoq paketi, disk sektori, registr.
- **"Belgilangan union" (tagged union)** — bir nechta turdan biri:
  ```c
  struct token {
      enum { SON, AMAL } tur;           /* qaysi maydon haqiqiy ekanini bildiradi */
      union {
          long son;
          char amal;
      };
  };
  ```
  Python'dagi "istalgan tur" o'zgaruvchining C'dagi qo'lda yasalgan varianti.

## 9.6. `enum` — nomlangan butun sonlar

```c
enum holat { TAYYOR, ISHLAYAPTI, UXLAYAPTI, TUGADI };    /* 0, 1, 2, 3 */
enum holat h = ISHLAYAPTI;

enum { SEKTOR = 512, SAHIFA = 4096 };                   /* aniq qiymatlar */
```

- `#define` dan afzalligi: debuggerda nomi ko'rinadi, `switch` da `-Wswitch` hamma holat
  qamrab olinmasa ogohlantiradi.
- MyOS'da: jarayon holatlari (`kernel/proc/process.h`), signal raqamlari.

## 9.7. `typedef` — turga yangi nom

```c
typedef unsigned long ulong;
typedef struct nuqta nuqta_t;
typedef int (*amal_fn)(int, int);   /* funksiya ko'rsatkichi - eng foydali ishlatilishi */
```

**Linux (va MyOS) uslubi:** strukturalar uchun `typedef` **ishlatilmaydi** — `struct process *p`
yozuvi "bu struktura" ekanini darhol ko'rsatadi. `typedef` faqat: aniq o'lchamli turlar (`uint32_t`),
funksiya ko'rsatkichlari va ichki tuzilishi ataylab yashirilgan ("opaque") turlar uchun.

## 9.8. Opaque (yashirin) tur — interfeys va amalga oshirishni ajratish

```c
/* xesh.h - tashqi dunyo faqat shuni ko'radi */
struct xesh;                                /* "shunday tur bor" - ichi noma'lum */
struct xesh *xesh_yarat(void);
int xesh_qoy(struct xesh *h, const char *k, int v);
```

```c
/* xesh.c - ichki tafsilot */
struct xesh {
    struct yozuv **chelaklar;
    size_t soni;
};
```

Foydalanuvchi faqat ko'rsatkich bilan ishlaydi va ichki maydonlarga tega olmaydi — amalga oshirishni
keyin butunlay o'zgartirsangiz ham, uni ishlatadigan kod buzilmaydi. `FILE *` (stdio) aynan shunday.
17-mashq shu uslubda.

## 9.9. Struktura ichida struktura va ko'rsatkichlar

Yadro tuzilmalari odatda boshqa tuzilmalarga ko'rsatkichlar tarmog'i:

```c
struct process {
    int pid;
    enum holat holat;
    struct mm *mm;                  /* xotira xaritasi */
    struct file *fayllar[64];       /* ochiq fayllar jadvali */
    struct process *ota;
    struct list_head node;          /* scheduler navbatiga ulanish uchun */
};
```

(MyOS'dagi haqiqiy `struct process` — `kernel/proc/process.h`. Uni oching va har bir maydon nimaga
kerakligini izohlardan o'qing.)

## 9.10. Savol-javob

**Struct'ni `malloc` bilan qanday yarataman?**
`struct talaba *t = malloc(sizeof(*t));` va tekshiring. Nollangan kerak bo'lsa — `calloc(1, sizeof(*t))`.

**Nega `memcmp` bilan structlarni solishtirish ishonchsiz?**
To'ldiruvchi baytlarda axlat bo'lishi mumkin — maydonlar teng bo'lsa ham `memcmp` farq topadi.

**Flexible array member nima?**
```c
struct paket { uint32_t uzunlik; uint8_t data[]; };   /* oxirgi maydon - o'lchamsiz massiv */
struct paket *p = malloc(sizeof(*p) + n);            /* sarlavha + n bayt ma'lumot bitta blokda */
```
Yadroda tez-tez uchraydi: sarlavha va ma'lumot bitta ajratmada.

## 9.11. O'zingizni tekshiring

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

## 9.12. Mashqlar

- **13** (struct vec), **16** (tugun), **17** (opaque xesh), **19** (struct saralash),
  **22** (halqa bufer struct'i), **23** (`container_of`, `offsetof`).
- Qo'shimcha: `sizeof` va `offsetof` bilan 9.2 va 9.11-savoldagi tuzilmalarning joylashuvini chiqarib,
  qog'ozdagi rasmingiz bilan solishtiring.

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
