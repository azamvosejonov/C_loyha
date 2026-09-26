# 7-bob. Ko'rsatkichlar (pointers)

> **Bu bob — C'ning va yadro dasturlashning yuragi.** Uni tushunmaguningizcha keyingi boblarga o'tmang.
> Bobdan keyin: `&`, `*`, `->`, `NULL`, ko'rsatkich arifmetikasi, `void *`, `char **`, `const` ko'rsatkichlar
> va funksiya ko'rsatkichlarini bilasiz. Mashqlar: 06, 07, 08, 10, 16, 20, 23.

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

Keyingi bob: [8-bob. Xotira: stek, heap, statik](08-xotira.md)
