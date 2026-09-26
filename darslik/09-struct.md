# 9-bob. `struct`, `union`, `enum`, `typedef`

> **Bu bobdan keyin:** o'z turlaringizni yarata olasiz, struktura xotirada qanday joylashishini
> (tekislash va to'ldiruvchi baytlar), `packed` va bit maydonlarini, `union` va `enum` ni bilasiz —
> apparat va disk tuzilmalarini C'da tasvirlash uchun hammasi kerak. Mashqlar: 13, 16, 17, 19, 22, 23.

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

Keyingi bob: [10-bob. Preprotsessor](10-preprotsessor.md)
