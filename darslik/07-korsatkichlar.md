# 7-bob. Ko'rsatkichlar (pointers)

> **Bu bob — C'ning va yadro dasturlashning yuragi.** Uni tushunmaguningizcha keyingi boblarga o'tmang.
> Bobdan keyin: `&`, `*`, `->`, `NULL`, ko'rsatkich arifmetikasi, `void *`, `char **`, `const` ko'rsatkichlar
> va funksiya ko'rsatkichlarini bilasiz. Mashqlar: 06, 07, 08, 10, 16, 20, 23.

> **To'liq ishlaydigan misol:** [misollar/07_korsatkichlar.c](misollar/07_korsatkichlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Ko'rsatkich — manzil yozilgan qog'oz (7.1).** Do'stingizga uyingizni ko'rsatmoqchisiz. Uyni ko'tarib
olib bormaysiz — qog'ozga **manzilni** yozib berasiz: "Navoiy ko'chasi, 15-uy". Qog'oz kichik, uy katta.
- `int *p` — "manzil yoziladigan qog'oz" (bu qog'ozda `int` turadigan uyning manzili bo'ladi).
- `&x` — "x ning manzilini ayt" — qog'ozga yozish uchun.
- `*p` — "qog'ozdagi manzilga bor va ichiga qara (yoki o'zgartir)".

**Nega kerak (7.2).** Usta uyingizni ta'mirlashi kerak. Unga uyingizning **fotosuratini** (nusxa, 5-bob)
bersangiz, u suratni bo'yaydi — uyingiz o'zgarmaydi. **Manzilni** bersangiz — borib, haqiqiy uyni ta'mirlaydi.
Funksiya chaqiruvchining o'zgaruvchisini o'zgartirishi uchun ham shunday: manzil beriladi.

**`NULL` — bo'sh konvert (7.3).** Manzil yozilmagan konvert. Uni pochtachiga bersangiz, u hech qayerga
bora olmaydi. `NULL` ko'rsatkich orqali o'qishga urinish — dastur darhol qulaydi (Segmentation fault).
Shuning uchun konvertni ishlatishdan oldin tekshiring: `if (p != NULL)`.

**Ko'rsatkich arifmetikasi — ko'chadagi uylar (7.4).** `p + 1` — "keyingi uy". Agar uylar 4 metr
enlikda bo'lsa (`int` — 4 bayt), keyingi uy 4 metr naridagi uy. `char` uylari 1 metr — `p + 1` 1 metr
nari. Kompilyator uy enini o'zi biladi, siz faqat "nechta uy" deysiz.

**`->` — manzilga borib, aniq xonaga kirish (7.5).** `p->oshxona` — "`p` dagi manzilga bor va
oshxonaga kir". Bu `(*p).oshxona` ning qisqa yozuvi.

**`int **` — manzillar daftarining manzili (7.8).** Siz do'stingizga "manzillar daftarim javonda
turibdi" deysiz. U avval daftarni topadi, keyin daftardan kerakli manzilni o'qiydi, keyin o'sha uyga boradi.
Ikki marta "borish" — `**pp`.

**Funksiya ko'rsatkichi — telefon raqami (7.9).** Raqamni saqlab qo'yasiz va kerak paytda qo'ng'iroq
qilasiz. Raqamni boshqasiga almashtirsangiz — boshqa odam javob beradi. Yadroda drayverlar aynan shunday:
"o'qish kerak bo'lsa — mana bu raqamga qo'ng'iroq qil".

**Osilib qolgan ko'rsatkich — ko'chib ketgan odamning eski manzili (7.11).** Do'stingiz ko'chib
ketdi, siz esa eski manzilga xat yuborasiz. U yerda endi boshqa odam yashaydi — xatingizni u oladi.
`free` qilingan xotiraga ko'rsatkich orqali yozish aynan shunday.

### To'liq dastur: pochta xizmati

```c
/* pochta.c - manzillar: &, *, ->, NULL, arifmetika, funksiya ko'rsatkichi */
#include <stdio.h>

struct uy {
    int raqam;
    int xatlar;                                 /* qutidagi xatlar soni */
};

/* Manzil orqali - haqiqiy uyning qutisiga xat tashlanadi */
static void xat_tashla(struct uy *u, int soni)
{
    if (u == NULL) {                            /* bo'sh konvert */
        printf("  manzil yo'q - xat qaytarildi\n");
        return;
    }
    u->xatlar += soni;
}

static void oddiy(struct uy *u) { xat_tashla(u, 1); printf("  oddiy pochta: %d-uyga 1 xat\n", u->raqam); }
static void tezkor(struct uy *u) { xat_tashla(u, 3); printf("  tezkor pochta: %d-uyga 3 xat\n", u->raqam); }

int main(void)
{
    struct uy kocha[4] = { { 1, 0 }, { 3, 0 }, { 5, 0 }, { 7, 0 } };

    struct uy *p = &kocha[0];                   /* qog'ozga 1-uyning manzili yozildi */
    xat_tashla(p, 2);
    p = p + 2;                                  /* 2 ta uy nariga */
    xat_tashla(p, 1);
    printf("p endi %d-uyni ko'rsatadi\n", p->raqam);

    xat_tashla(NULL, 1);

    void (*xizmat)(struct uy *) = oddiy;        /* telefon raqami saqlandi */
    xizmat(&kocha[1]);
    xizmat = tezkor;                            /* raqam almashdi - boshqa xizmat javob beradi */
    xizmat(&kocha[3]);

    printf("Ko'chadagi qutilar:\n");
    for (struct uy *u = kocha; u < kocha + 4; u++)
        printf("  %d-uy: %d ta xat\n", u->raqam, u->xatlar);
    return 0;
}
```

```console
$ gcc -Wall -Wextra pochta.c -o pochta
$ ./pochta
p endi 5-uyni ko'rsatadi
  manzil yo'q - xat qaytarildi
  oddiy pochta: 3-uyga 1 xat
  tezkor pochta: 7-uyga 3 xat
Ko'chadagi qutilar:
  1-uy: 2 ta xat
  3-uy: 1 ta xat
  5-uy: 1 ta xat
  7-uy: 3 ta xat
```

**Sinab ko'ring:** `p = p + 2;` ni `p = p + 5;` qilib, `-fsanitize=address` bilan yig'ing — 5 ta uy
nariga borsak, ko'chada uy bormi? `xat_tashla` ni `struct uy u` (yulduzsiz, nusxa) qabul qiladigan qilib
yozsangiz, qutilar nega bo'sh qoladi?

## 7.1. Ko'rsatkich — manzilni saqlaydigan o'zgaruvchi

Xotira — raqamlangan baytlar qatori (0-bob). Har bir o'zgaruvchi qaysidir **manzilda** turadi.
Ko'rsatkich — shunday manzilni saqlaydigan o'zgaruvchi.

```c
int x = 42;
int *p = &x;
```

```text
   o'zgaruvchi   manzil          qiymat
   x             0x7ffd1000      42
   p             0x7ffd1008      0x7ffd1000   <- x ning manzili
```

| Yozuv | Ma'nosi | Qiymati |
|---|---|---|
| `x` | x ning qiymati | 42 |
| `&x` | x ning **manzili** ("x qayerda?") | 0x7ffd1000 |
| `p` | p ning qiymati — bu manzil | 0x7ffd1000 |
| `*p` | p **ko'rsatgan joydagi** qiymat ("o'sha manzilga bor") | 42 |
| `&p` | p ning o'zining manzili | 0x7ffd1008 |

```c
*p = 100;           /* x ning qiymati endi 100 - p orqali o'zgartirildi */
printf("%d\n", x);  /* 100 */
```

### `*` belgisining ikki ma'nosi (eng ko'p chalkashlik shu yerda)

```c
int *p;             /* E'LONDA: "p - int ga ko'rsatkich" (turning bir qismi) */
*p = 5;             /* IFODADA: "p ko'rsatgan joy" (dereference - manzil bo'yicha borish) */
```

E'londa `*` "bu ko'rsatkich" degani, ifodada — "shu manzilga bor". Ko'paytirish (`a * b`) — uchinchi ma'no.

**Tuzoq:** `int *a, b;` — `a` ko'rsatkich, `b` esa oddiy `int`! `*` nomga yopishadi, turga emas.
Shuning uchun har bir ko'rsatkichni alohida qatorda e'lon qiling.

### Ko'rsatkichning turi nega kerak

`int *`, `char *`, `double *` — hammasi ham manzil (x86-64 da 8 bayt). Tur kompilyatorga ikki
narsani aytadi:
1. `*p` qilganda **necha bayt** o'qish kerak (`int *` → 4, `char *` → 1);
2. `p + 1` qilganda manzil **qanchaga** siljiydi (7.4).

## 7.2. Nega ko'rsatkichlar kerak — to'rt asosiy sabab

1. **Chaqiruvchining o'zgaruvchisini o'zgartirish** (argumentlar nusxa — 5-bob):
   ```c
   void almashtir(int *a, int *b) { int t = *a; *a = *b; *b = t; }
   almashtir(&x, &y);
   ```
2. **Katta ma'lumotni nusxalamasdan uzatish:** 4 KB struct o'rniga 8 baytli manzil.
3. **Dinamik xotira:** `malloc` xotira beradi va uning **manzilini** qaytaradi (8-bob).
4. **Bog'langan tuzilmalar:** ro'yxat, daraxt — har bir tugun keyingisining manzilini saqlaydi.

Yadroda esa beshinchisi: **apparatga murojaat**. Ekran xotirasi, qurilma registrlari aniq manzillarda
turadi — ko'rsatkich bilan ularga yozasiz:

```c
volatile uint16_t *vga = (volatile uint16_t *)0xB8000;     /* VGA matn rejimi xotirasi */
vga[0] = 0x0F00 | 'A';      /* ekranning chap yuqori burchagiga oq 'A' */
```

(MyOS: `kernel/drivers/vga.c`. `volatile` — 16-bob.)

## 7.3. `NULL` — "hech qayerga ko'rsatmaydi"

```c
int *p = NULL;
if (p != NULL)      /* yoki: if (p) */
    *p = 5;
```

- `NULL` — 0 manzili, "ko'rsatkich hali biror narsaga bog'lanmagan" belgisi.
- `*p` qilish (`p == NULL` bo'lganda) — **NULL dereference**: user dasturda `Segmentation fault`,
  yadroda — page fault va **PANIC** (0-sahifa ataylab xaritalanmaydi — xato darhol ko'rinsin).
- Ko'rsatkich qaytaradigan funksiyalar xatoni `NULL` bilan bildiradi: `malloc`, `fopen`, `strchr`.
  **Doim tekshiring.**

### Boshlang'ich qiymatsiz ko'rsatkich — "yovvoyi" ko'rsatkich

```c
int *p;             /* ichida AXLAT - tasodifiy manzil */
*p = 5;             /* tasodifiy joyga yozish: qulash yoki jim buzilish */
```

Qoida: ko'rsatkichni doim yo haqiqiy manzil, yo `NULL` bilan boshlang.

## 7.4. Ko'rsatkich arifmetikasi

```c
int a[5] = { 10, 20, 30, 40, 50 };
int *p = &a[0];         /* yoki shunchaki: int *p = a; */

p + 1                   /* keyingi int ning manzili: +4 BAYT (+1 emas!) */
*(p + 2)                /* 30 */
p[2]                    /* 30 - aynan *(p + 2) ning boshqa yozuvi */
p++;                    /* p endi a[1] ga ko'rsatadi */
int *q = &a[4];
q - p                   /* 3 - ikki ko'rsatkich orasidagi ELEMENTLAR soni */
```

**Muhim qoida:** `p + n` manzilni `n * sizeof(*p)` baytga siljitadi. `char *` uchun — n bayt,
`int *` uchun — 4n bayt, `struct x *` uchun — n × struct hajmi.

**Yana muhimroq:** `a[i]` — bu **ta'rifan** `*(a + i)`. Massiv indeksi ko'rsatkich arifmetikasining
qisqa yozuvi. Qiziq natija: `a[2]` va `2[a]` bir xil (hech qachon yozmang, lekin tushuning).

### Massiv va ko'rsatkich — bog'liq, lekin bir xil emas

```c
int a[5];
int *p = a;             /* massiv nomi ko'p joyda birinchi elementning manziliga "aylanadi" (decay) */
```

| | `a` (massiv) | `p` (ko'rsatkich) |
|---|---|---|
| `sizeof` | 20 (butun massiv) | 8 (manzil) |
| `a = ...` / `p = ...` | mumkin emas | mumkin |
| Xotira | 5 ta int uchun joy | faqat manzil uchun joy |

Funksiyaga uzatilganda massiv har doim ko'rsatkichga aylanadi (6-bob).

### Ko'rsatkich bilan satr aylanish — C'ning klassik uslubi

```c
size_t mening_strlen(const char *s)
{
    const char *p = s;
    while (*p)              /* *p != '\0' */
        p++;
    return p - s;           /* ko'rsatkichlar ayirmasi = uzunlik */
}
```

Bu kodni o'qib, har bir qadamda `p` qayerga ko'rsatishini qog'ozda chizing. 08-mashqda o'zingiz yozasiz.

## 7.5. `->` — struktura ko'rsatkichi orqali a'zo

```c
struct nuqta { int x, y; };
struct nuqta n = { 1, 2 };
struct nuqta *p = &n;

(*p).x = 10;        /* p ko'rsatgan struktura, uning x maydoni */
p->x = 10;          /* aynan o'sha - qisqa yozuv */
```

`.` — struktura **qiymati** uchun, `->` — struktura **ko'rsatkichi** uchun. Yadroda deyarli hamma narsa
ko'rsatkich orqali, shuning uchun `->` hamma joyda: `current->mm`, `f->fops->read(f, buf, len, f->offset)`.
(9-bob.)

## 7.6. `const` va ko'rsatkichlar

```c
const char *p;          /* ko'rsatilgan MA'LUMOT o'zgarmas:  *p = 'x' - xato,  p++ - mumkin */
char *const p;          /* KO'RSATKICHNING O'ZI o'zgarmas:   p++ - xato,  *p = 'x' - mumkin */
const char *const p;    /* ikkalasi ham o'zgarmas */
```

O'qish qoidasi: **o'ngdan chapga**. `const char *p` → "p — ko'rsatkich — `const char` ga".

Funksiya parametrlarida `const` — **va'da**: `size_t strlen(const char *s)` — "satringizni
o'zgartirmayman". Kompilyator va'dani tekshiradi, o'quvchi esa funksiyaning nima qilishini imzosidan
tushunadi. Qoida: funksiya o'zgartirmaydigan har bir ko'rsatkich parametrini `const` qiling.

## 7.7. `void *` — turi noma'lum manzil

```c
void *p = malloc(100);      /* malloc bu xotira nima uchun ekanini bilmaydi */
int *a = p;                 /* C'da void * istalgan ko'rsatkichga avtomatik aylanadi */
char *c = p;
```

- `void *` ga har qanday ko'rsatkich sig'adi va u har qanday ko'rsatkichga aylanadi (cast shart emas).
- `*p` qilib **bo'lmaydi** (qancha bayt o'qishni bilmaydi) va `p + 1` standartda ruxsat etilmagan
  (GCC 1 bayt deb hisoblaydi). Avval aniq turga aylantiring.
- "Umumiy" funksiyalar shuni ishlatadi: `memcpy(void *d, const void *s, size_t n)`, `qsort`,
  oqim funksiyasining `void *arg` i (29-mashq), `struct file` dagi `void *priv` (MyOS VFS: har bir
  fayl tizimi o'z ma'lumotini shunda saqlaydi).

## 7.8. Ko'rsatkichga ko'rsatkich: `int **`, `char **`

```c
int x = 5;
int *p = &x;
int **pp = &p;          /* p ning manzili */
**pp = 7;               /* x = 7 */
```

Qayerda kerak:

1. **Satrlar massivi:** `char **argv` — har bir element `char *` (satr manzili).
   ```text
   argv ──> [ ptr0 ][ ptr1 ][ ptr2 ][ NULL ]
               │       │       └──> "42\0"
               │       └──────────> "salom\0"
               └──────────────────> "./dastur\0"
   ```
2. **Funksiya chaqiruvchining KO'RSATKICHINI o'zgartirishi kerak bo'lsa:**
   ```c
   int yarat(struct tugun **natija)
   {
       *natija = malloc(sizeof(struct tugun));
       return *natija ? 0 : -1;
   }
   struct tugun *t;
   yarat(&t);
   ```
   MyOS'da: `int pipe_create(struct file **rf, struct file **wf)` — ikkita yangi fayl yaratib, ularning
   manzillarini chaqiruvchiga qaytaradi.
3. **Ro'yxatdan o'chirishning "yaxshi did" usuli** (16-mashq): `struct tugun **pp` — "o'zgartirilishi
   kerak bo'lgan ko'rsatkichning manzili", boshni alohida holat qilmasdan.

## 7.9. Funksiya ko'rsatkichlari

```c
int qoshish(int a, int b) { return a + b; }
int ayirish(int a, int b) { return a - b; }

int (*amal)(int, int);      /* amal - (int, int) olib int qaytaradigan funksiyaga ko'rsatkich */
amal = qoshish;
printf("%d\n", amal(5, 3));     /* 8 */
amal = ayirish;
printf("%d\n", amal(5, 3));     /* 2 */
```

Sintaksis qo'rqinchli ko'rinadi — `typedef` bilan soddalashtiriladi:

```c
typedef int (*amal_fn)(int, int);
amal_fn amal = qoshish;
```

### Funksiya jadvali — C'dagi "polimorfizm", yadroning asosi

```c
struct file_ops {                               /* MyOS: kernel/fs/vfs.h (qisqartirilgan) */
    int64_t (*read)(struct file *f, void *buf, size_t len, uint64_t off);
    int64_t (*write)(struct file *f, const void *buf, size_t len, uint64_t off);
    void    (*release)(struct file *f);
};

static const struct file_ops pipe_fops = {
    .read    = pipe_read,
    .write   = pipe_write,
    .release = pipe_release,
};

/* VFS - qaysi fayl turi ekanini BILMASDAN chaqiradi: */
n = f->fops->read(f, buf, len, f->offset);      /* kernel/fs/vfs.c */
```

`f` pipe bo'lsa — `pipe_read`, ext2 fayli bo'lsa — `ext2_read`, terminal bo'lsa — `tty_read` ishlaydi.
Python'dagi klass metodlari aynan shunday amalga oshirilgan. Linux'da `file_operations`,
`inode_operations`, `pci_driver`... — minglab shunday jadvallar. MyOS: `kernel/fs/vfs.h`,
`kernel/fs/pipe.c`, `kernel/drivers/ahci.c`. 20-mashq — shu uslubning amaliyoti.

## 7.10. `container_of` — a'zodan butun strukturaga

Linux'ning eng mashhur makrosi (MyOS: `kernel/lib/common.h`):

```c
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
```

`offsetof(type, member)` — a'zoning struktura boshidan siljishi (baytda). A'zoning manzilidan shu
siljishni ayirsak — strukturaning boshini topamiz. `(char *)` ga aylantirish shart: ayirish **bayt**
bo'yicha bo'lishi uchun (7.4-dagi qoida!). 23-mashqda buni amalda qo'llaysiz.

## 7.11. Ko'rsatkichlar bilan tipik xatolar — ro'yxat

| Xato | Misol | Oqibat |
|---|---|---|
| NULL dereference | `p = malloc(..); p->x = 1;` (tekshirmasdan) | qulash |
| Yovvoyi ko'rsatkich | `int *p; *p = 1;` | tasodifiy buzilish |
| Osilib qolgan ko'rsatkich (dangling) | `free(p); p->x = 1;` | use-after-free (8-bob) |
| Lokal o'zgaruvchi manzilini qaytarish | `return &lokal;` | o'lik stek xotirasi |
| Chegaradan chiqish | `p[n]` (n ta elementli massivda) | buzilish |
| Noto'g'ri tur arifmetikasi | `(int *)p + 1` va `(char *)p + 1` ni adashtirish | noto'g'ri manzil |
| `sizeof(p)` massiv o'rniga | `malloc(sizeof(p))` → 8 bayt | kichik bufer |

Oxirgisi ayniqsa ko'p uchraydi. To'g'ri idioma:

```c
struct tugun *t = malloc(sizeof(*t));       /* sizeof(*t) - t ko'rsatgan turning hajmi */
```

Tur o'zgarsa ham to'g'ri qoladi.

## 7.12. Savol-javob

**Ko'rsatkichni `printf` bilan qanday chiqaraman?**
`printf("%p\n", (void *)p);`. Stek manzillari odatda `0x7ff...`, heap — `0x55...`/`0x5...`,
yadro — `0xffff...`. MyOS'da `hello` dasturi o'z manzillarini ko'rsatadi.

**Ko'rsatkich va manzil — bir xil narsami?**
Manzil — son (xotiradagi joy). Ko'rsatkich — manzilni saqlaydigan **turli** o'zgaruvchi. Tur uni
qanday talqin qilishni belgilaydi.

**Bu manzillar "haqiqiy" RAM manzillarimi?**
User dasturda — yo'q, **virtual** manzillar. Har bir jarayonning o'z manzil maydoni bor, CPU ularni
sahifa jadvali orqali fizik manzilga aylantiradi. Ikki jarayonda bir xil `0x401000` manzili — turli
fizik xotira. Buni yadro boshqaradi: MyOS'da `kernel/mm/vmm.c`, `docs/04-virtual-xotira.md`. Mashq 31
(sahifa jadvali) shu mexanizmni simulyatsiya qiladi.

## 7.13. O'zingizni tekshiring

```c
int a[4] = { 1, 2, 3, 4 };
int *p = a + 1;
```

1. `*p`, `p[1]`, `*(p + 2)`, `p[-1]` qiymatlari?
2. `p - a` nechaga teng?
3. `char *c = (char *)a; c + 4` qaysi elementning manziliga ko'rsatadi?
4. `const int *q` va `int *const q` farqi?
5. `void swap(int *a, int *b)` ni qanday chaqirasiz?
6. `f->fops->read(f, ...)` qatorini so'z bilan o'qing.

<details><summary>Javoblar</summary>

1. 2, 3, 4, 1.
2. 1 (elementlar soni).
3. `a[1]` (4 bayt = bitta int).
4. Birinchisi: ko'rsatilgan qiymatni o'zgartirib bo'lmaydi; ikkinchisi: q ning o'zini (manzilni).
5. `swap(&x, &y);`
6. "f ko'rsatgan faylning fops maydoni ko'rsatgan jadvaldagi read funksiyasini f va ... argumentlar bilan chaqir".
</details>

## 7.14. Mashqlar

- **06** — `&` va `*` asoslari. **07** — chiqish parametrlari. **08** — ko'rsatkich arifmetikasi.
- **10** — ikki ko'rsatkich. **16** — ro'yxat, `->`, `**pp`. **20** — funksiya jadvali. **23** — `container_of`.
- Qo'shimcha: qog'ozda 7.13 dagi xotira rasmini chizing; `int x; int *p = &x; int **pp = &p;` uchun
  uchta o'zgaruvchining manzilini `%p` bilan chiqarib, rasmingiz bilan solishtiring.

<!-- loyiha:boshi -->
## Loyiha: map / filter / reduce

**Maqsad:** Python'dagi `map`, `filter`, `functools.reduce` ni C'da funksiya ko'rsatkichlari bilan yozish.
Bu yadroda ham uchraydigan naqsh: "hodisa kelganda **shu funksiyani** chaqir".
**Bobdan ishlatiladi:** massivni ko'rsatkich sifatida uzatish, `int *`, `const int *`, funksiya ko'rsatkichi.

**Talab:** uch umumiy funksiya; ular **nima qilishni** bilmaydi — buni argument sifatida berilgan funksiya hal qiladi.
- `map(a, n, f)` — har elementni `f(x)` bilan almashtiradi;
- `filter(src, n, dst, shart)` — sharti rost elementlarni `dst` ga ko'chiradi, sonini qaytaradi;
- `reduce(a, n, boshi, f)` — `r = f(r, a[i])` bilan bitta qiymatga keltiradi.

```c
/* funksional.c - map / filter / reduce */
#include <stdio.h>

static void map(int *a, int n, int (*f)(int))
{
    for (int i = 0; i < n; i++)
        a[i] = f(a[i]);
}

static int filter(const int *src, int n, int *dst, int (*shart)(int))
{
    int k = 0;
    for (int i = 0; i < n; i++)
        if (shart(src[i]))
            dst[k++] = src[i];
    return k;
}

static int reduce(const int *a, int n, int boshi, int (*f)(int, int))
{
    int r = boshi;
    for (int i = 0; i < n; i++)
        r = f(r, a[i]);
    return r;
}

static void chiqar(const char *nom, const int *a, int n)
{
    printf("%-14s:", nom);
    for (int i = 0; i < n; i++)
        printf(" %d", a[i]);
    printf("\n");
}

static int kvadrat(int x) { return x * x; }
static int juft_mi(int x) { return x % 2 == 0; }
static int qosh(int a, int b) { return a + b; }
static int katta(int a, int b) { return a > b ? a : b; }
static int kopaytir(int a, int b) { return a * b; }

int main(void)
{
    int a[] = { 3, 8, 1, 6, 4, 7, 2, 5 };
    int n = sizeof(a) / sizeof(a[0]);
    int juftlar[8];

    chiqar("asl massiv", a, n);
    int k = filter(a, n, juftlar, juft_mi);
    chiqar("juftlar", juftlar, k);

    map(juftlar, k, kvadrat);                   /* massivni joyida o'zgartiradi */
    chiqar("kvadratlari", juftlar, k);

    printf("yig'indi      : %d\n", reduce(a, n, 0, qosh));
    printf("eng katta     : %d\n", reduce(a, n, a[0], katta));
    printf("ko'paytma     : %d\n", reduce(a, n, 1, kopaytir));
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined funksional.c -o funksional
$ ./funksional
asl massiv    : 3 8 1 6 4 7 2 5
juftlar       : 8 6 4 2
kvadratlari   : 64 36 16 4
yig'indi      : 36
eng katta     : 8
ko'paytma     : 40320
```

Diqqat qiling: `map` ham, `filter` ham, `reduce` ham massiv **qanday** qayta ishlanishini bilmaydi. Yangi amal
kerakmi — yangi 1 qatorli funksiya yozasiz, sikllarni qayta yozmaysiz. `qsort(…, taqqosla)` (12-bob) ham shunday.

**Kengaytiring:** `manfiy_mi` sharti va `ikki_barobar` amalini qo'shing. `reduce` bilan eng kichik elementni toping.
`filter` ga `dst` uchun `n` dan kichik massiv bersangiz nima bo'ladi (sanitizer ko'rsatadi)?

## Mustaqil loyiha: massivni joyida qayta ishlash ★★★

**Vazifa:** qo'shimcha massiv **ochmasdan**, faqat ko'rsatkichlar va indekslar bilan uchta klassik
algoritmni yozing. Bu mashq ko'rsatkichlarni "ichidan" his qilish uchun. Fayl: `joyida.c`.

1. `int unique(int *a, int n)` — **saralangan** massivdan takroriy qiymatlarni olib tashlaydi (joyida),
   yangi uzunlikni qaytaradi. Ikki ko'rsatkich: o'qish va yozish.
2. `void rotate(int *a, int n, int k)` — massivni **o'ngga** `k` ta o'ringa aylantiradi
   (oxirgi `k` ta element boshiga o'tadi). `k` `n` dan katta bo'lishi mumkin. Qo'shimcha massiv yo'q!
   (Maslahat: `reverse` yordamchisi.)
3. `int partition(int *a, int n, int pivot)` — `pivot` dan **kichik** elementlarni oldinga o'tkazadi,
   ularning sonini qaytaradi. **Lomuto usuli**: `j` — "kichiklar" chegarasi (0 dan); `i` bo'ylab yuring,
   `a[i] < pivot` bo'lsa, `a[i]` va `a[j]` ni almashtiring va `j++`. (Usul aniq belgilangan, chunki natija tartibi unga bog'liq.)

Chiqarish tartibi aniq (quyida). Har bir funksiyani **yangi nusxa** massivda sinang.

**Kutilgan natija** (`darslik/loyihalar/07_joyida/kutilgan.txt`):

```text
unique: 1 1 2 3 3 3 4 4 5 7 7 -> 6 ta: 1 2 3 4 5 7
rotate(3): 1 2 3 4 5 6 7 -> 5 6 7 1 2 3 4
rotate(10): 1 2 3 4 5 6 7 -> 5 6 7 1 2 3 4
partition(5): 9 2 7 4 8 1 6 3 -> 4 ta: 2 4 1 3 8 7 6 9
partition(1): 9 2 7 4 8 1 6 3 -> 0 ta: 9 2 7 4 8 1 6 3
partition(100): 9 2 7 4 8 1 6 3 -> 8 ta: 9 2 7 4 8 1 6 3
```

Sinov ma'lumotlari: `unique` — `{1,1,2,3,3,3,4,4,5,7,7}`; `rotate` — `{1,2,3,4,5,6,7}` bilan `k = 3` va `k = 10`;
`partition` — `{9,2,7,4,8,1,6,3}` bilan `pivot = 5`, `1` va `100`.

**Maslahat** (yechim emas):
- `unique`: `yoz` ko'rsatkichi/indeksi "hozirgacha noyoblar soni". Har `o'qi` da `a[o'qi] != a[yoz-1]` bo'lsa yozing.
- `rotate`: 3 marta `reverse`: butun massiv, keyin birinchi `k` ta, keyin qolgan `n-k` ta. `k %= n` — `n == 0` ga ehtiyot bo'ling.
- Massiv uzunligini funksiyaga **doim** alohida uzating — `sizeof(a)` funksiya ichida ko'rsatkich o'lchamini beradi (7.2).
- `-fsanitize=address` bilan yig'ing: chegaradan chiqish darhol ko'rinadi.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined joyida.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/07_joyida/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [8-bob. Xotira: stek, heap, statik](08-xotira.md)
