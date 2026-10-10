# 7-bob. Ko'rsatkichlar (pointers)

> **Bu bob — C'ning va yadro dasturlashning yuragi.** Uni tushunmaguningizcha keyingi boblarga o'tmang.
> **Bu bobda nima o'rganasiz:** ko'rsatkich (manzil saqlaydigan quti) nima va nega kerakligini; `&`, `*`, `->` belgilari nima qilishini;
> `NULL`; ko'rsatkich arifmetikasi; `const` ko'rsatkichlar; `void *`; `char **`; funksiya ko'rsatkichlari.
> **Oldindan nima kerak:** 2–6-boblar (ayniqsa 5.4 "nusxa bo'yicha uzatish" va 6-bob massivlar).   **Vaqt:** 8–10 soat — shoshilmang.
> Mashqlar: isitish (bob oxirida), 06, 07, 08, 10. 16, 20 — 9-bobdan, 23 — 10-bobdan keyin.

> **To'liq ishlaydigan misol:** [misollar/07_korsatkichlar.c](misollar/07_korsatkichlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

0-bobdan beri bilasiz: xotira — raqamlangan baytlar qatori. Har bir o'zgaruvchi shu qatorning qaysidir joyida (**manzilda**) turadi.
**Ko'rsatkich** — **manzilni o'zida saqlaydigan o'zgaruvchi**. Ya'ni ko'rsatkich — boshqa narsaning "qayerda turishini" biladigan quti.

**Hayotdan misol: manzil yozilgan qog'oz.** Do'stingizga uyingizni ko'rsatmoqchisiz. Uyni ko'tarib olib bormaysiz — qog'ozga **manzilni** yozib berasiz:
"Navoiy ko'chasi, 15-uy". Qog'oz kichik, uy katta.

| Hayotda | C'da | Yozilishi |
|---|---|---|
| uy | o'zgaruvchi (masalan `x`) | `int x = 42;` |
| uyning manzili | `x` ning **manzili** | `&x` |
| manzil yozilgan qog'oz | **ko'rsatkich** o'zgaruvchisi | `int *p = &x;` |
| "qog'ozdagi manzilga bor va uyga kir" | manzil bo'yicha **borish** (dereference) | `*p` |

Shu uch belgi — `&` ("manzilini ayt"), `*` ("manzil bo'yicha bor"), `->` (struktura ichiga kirish) — butun bobning mohiyati.

## 7.1. Ko'rsatkich — manzilni saqlaydigan o'zgaruvchi

```c
int x = 42;
int *p = &x;
```

Ikki qator: (1) `x` — oddiy `int` quti, ichida 42; (2) `p` — **ko'rsatkich** quti, ichida **`x` ning manzili**. Xotirada:

```text
   o'zgaruvchi   manzil          qiymat (ichidagi narsa)
   x             0x7ffd1000      42
   p             0x7ffd1008      0x7ffd1000   <- x ning manzili (p "x ga ko'rsatadi")
```

```text
        p                      x
   ┌──────────┐          ┌────────┐
   │ 0x7ffd1000 │ ───────>│   42   │
   └──────────┘          └────────┘
   (manzil saqlaydi)      (haqiqiy qiymat)
```

| Yozuv | Ma'nosi | Qiymati |
|---|---|---|
| `x` | x ning qiymati | 42 |
| `&x` | x ning **manzili** ("x qayerda?") | 0x7ffd1000 |
| `p` | p ning qiymati — bu manzil | 0x7ffd1000 |
| `*p` | p **ko'rsatgan joydagi** qiymat ("o'sha manzilga bor") | 42 |
| `&p` | p ning o'zining manzili | 0x7ffd1008 |

**Bu dastur nima qiladi (umumiy):** ko'rsatkich nima ekanini ko'rsatadi: `&x` manzilni oladi, `*p` shu manzildagi qiymatni o'qiydi va o'zgartiradi.

```c
/* korsatkich_asos.c - &, * va manzil bo'yicha o'zgartirish */
#include <stdio.h>

int main(void)
{
    int x = 42;
    int *p = &x;                        /* p ga x ning manzili yozildi */

    printf("x = %d\n", x);
    printf("*p = %d  (p ko'rsatgan joydagi qiymat)\n", *p);
    printf("p == &x ? %d  (p aynan x ning manzilini saqlaydi)\n", p == &x);

    *p = 100;                           /* manzil bo'yicha borib, qiymatni o'zgartirdik */
    printf("*p = 100 dan keyin: x = %d  (x o'zgardi!)\n", x);

    printf("x ning manzili: %p\n", (void *)&x);    /* manzil har ishga tushirishda boshqacha chiqadi */
    return 0;
}
```

```console
$ gcc -Wall -Wextra korsatkich_asos.c -o korsatkich_asos
$ ./korsatkich_asos
x = 42
*p = 42  (p ko'rsatgan joydagi qiymat)
p == &x ? 1  (p aynan x ning manzilini saqlaydi)
*p = 100 dan keyin: x = 100  (x o'zgardi!)
x ning manzili: 0x7ffd308d235c
```

**Kodda nimalar bor:**

| Qator | Nima qiladi | Nega |
|---|---|---|
| `int x = 42;` | `int` quti, ichida 42 | |
| `int *p = &x;` | `p` — "`int` ning manzilini saqlaydigan quti"; unga `x` ning manzili yoziladi | `&x` — "x qayerda turibdi?" savoliga javob |
| `*p` | `p` ichidagi manzilga **borib**, u yerdagi `int` ni o'qiydi | = `x` (42) |
| `p == &x` | ikki manzilni solishtiradi → 1 (rost) | `p` aynan `x` ning manzilini saqlaydi |
| `*p = 100;` | `p` ko'rsatgan joyga 100 yozadi | **`x` ning o'zi o'zgardi**, chunki `p` va `x` bir xil quti haqida |
| `%p` | manzilni chiqarish formati; `(void *)` bilan beriladi | manzil qiymati sizda boshqa bo'ladi (xavfsizlik uchun har gal tasodifiy surilishi — ASLR) |

> **Eslab qoling:** `&x` — "x ning **manzili**"; `*p` — "p ko'rsatgan joydagi **qiymat**". `&` va `*` — bir-biriga teskari amallar: `*&x` = `x`.

### `*` belgisining ikki ma'nosi (eng ko'p chalkashlik shu yerda)

```c
int *p;             /* E'LONDA: "p - int ga ko'rsatkich" (turning bir qismi) */
*p = 5;             /* IFODADA: "p ko'rsatgan joy" (dereference - manzil bo'yicha borish) */
```

E'londa `*` "bu ko'rsatkich" degani, ifodada — "shu manzilga bor". Ko'paytirish (`a * b`) — uchinchi ma'no. Qaysi ekanini **joyiga** qarab bilasiz:
tur nomidan keyin (`int *`) — e'lon; ifoda ichida bitta operand oldida (`*p`) — borish.

**Tuzoq:** `int *a, b;` — `a` ko'rsatkich, `b` esa oddiy `int`! `*` nomga yopishadi, turga emas.

**Bu dastur nima qiladi (umumiy):** `int *a, b;` yozuvidagi tuzoq: `*` faqat `a` ga tegishli, `b` esa oddiy `int` — ataylab xatoli.

```c
/* yulduz_tuzoq.c - int *a, b; */
#include <stdio.h>

int main(void)
{
    int x = 5;
    int *a, b;                          /* a - ko'rsatkich, b - oddiy int! */
    a = &x;
    b = &x;                             /* xato: b ga manzil yozib bo'lmaydi */
    printf("%d %d\n", *a, b);
    return 0;
}
```

```console
$ gcc -Wall -Wextra yulduz_tuzoq.c -o yulduz_tuzoq # xato kutiladi
yulduz_tuzoq.c: In function ‘main’:
yulduz_tuzoq.c:9:7: warning: assignment to ‘int’ from ‘int *’ makes integer from pointer without a cast [-Wint-conversion]
    9 |     b = &x;                             /* xato: b ga manzil yozib bo'lmaydi */
      |       ^
```

Shuning uchun har bir ko'rsatkichni **alohida qatorda** e'lon qiling: `int *a;` va `int b;`.

### Ko'rsatkichning turi nega kerak

`int *`, `char *`, `double *` — hammasi ham manzil (x86-64 da 8 bayt). Tur kompilyatorga ikki narsani aytadi:

1. `*p` qilganda **necha bayt** o'qish kerak (`int *` → 4, `char *` → 1);
2. `p + 1` qilganda manzil **qanchaga** siljiydi (7.4).

## 7.2. Nega ko'rsatkichlar kerak — to'rt asosiy sabab

**Hayotdan misol: ustaga uy.** Usta uyingizni ta'mirlashi kerak. Unga uyingizning **fotosuratini** (nusxa, 5-bob) bersangiz, u suratni bo'yaydi — uyingiz o'zgarmaydi.
**Manzilni** bersangiz — borib, haqiqiy uyni ta'mirlaydi. Funksiya chaqiruvchining o'zgaruvchisini o'zgartirishi uchun ham shunday: manzil beriladi.

1. **Chaqiruvchining o'zgaruvchisini o'zgartirish** (argumentlar nusxa — 5-bob).
2. **Katta ma'lumotni nusxalamasdan uzatish:** 4 KB struct o'rniga 8 baytli manzil.
3. **Dinamik xotira:** `malloc` xotira beradi va uning **manzilini** qaytaradi (8-bob).
4. **Bog'langan tuzilmalar:** ro'yxat, daraxt — har bir tugun keyingisining manzilini saqlaydi.

Yadroda esa beshinchisi: **apparatga murojaat**. Ekran xotirasi, qurilma registrlari aniq manzillarda turadi — ko'rsatkich bilan ularga yozasiz:

```text
volatile uint16_t *vga = (volatile uint16_t *)0xB8000;     /* VGA matn rejimi xotirasi */
vga[0] = 0x0F00 | 'A';      /* ekranning chap yuqori burchagiga oq 'A' */
```

(MyOS: `kernel/drivers/vga.c`. `volatile` — 16-bob.)

### Birinchi sabab amalda: `almashtir`

**Bu dastur nima qiladi (umumiy):** ikki o'zgaruvchining qiymatini ko'rsatkichlar yordamida funksiya orqali almashtiradi (qiymat bo'yicha uzatish buni qila olmaydi).

```c
/* almashtir.c - ikki qiymatni almashtirish: faqat manzil bilan mumkin */
#include <stdio.h>

static void almashtir(int *a, int *b)
{
    int t = *a;                         /* a ko'rsatgan qiymatni t ga saqlab qo'yamiz */
    *a = *b;                            /* a ko'rsatgan joyga b ko'rsatgan qiymatni yozamiz */
    *b = t;                             /* b ko'rsatgan joyga eski a qiymatini yozamiz */
}

int main(void)
{
    int x = 3, y = 7;
    printf("oldin:  x = %d, y = %d\n", x, y);
    almashtir(&x, &y);                  /* ikkalasining MANZILINI beramiz */
    printf("keyin:  x = %d, y = %d\n", x, y);
    return 0;
}
```

```console
$ gcc -Wall -Wextra almashtir.c -o almashtir
$ ./almashtir
oldin:  x = 3, y = 7
keyin:  x = 7, y = 3
```

**Qiymatlar qanday o'zgaradi:**

| Qadam | `*a` (= x) | `*b` (= y) | `t` |
|---|---|---|---|
| boshida | 3 | 7 | — |
| `int t = *a;` | 3 | 7 | **3** |
| `*a = *b;` | **7** | 7 | 3 |
| `*b = t;` | 7 | **3** | 3 |

`almashtir(x, y)` (nusxalar) yozilganida faqat nusxalar almashgan bo'lardi — `main` dagi `x`, `y` o'zgarmasdi (5.4). Shuning uchun **manzil** beramiz: `&x`, `&y`.

## 7.3. `NULL` — "hech qayerga ko'rsatmaydi"

**Hayotdan misol: bo'sh konvert.** Manzil yozilmagan konvert. Uni pochtachiga bersangiz, u hech qayerga bora olmaydi. `NULL` ko'rsatkich orqali o'qishga urinish —
dastur darhol qulaydi (Segmentation fault). Shuning uchun konvertni ishlatishdan oldin tekshiring: `if (p != NULL)`.

```c
int *p = NULL;
if (p != NULL)      /* yoki: if (p) */
    *p = 5;
```

- `NULL` — 0 manzili, "ko'rsatkich hali biror narsaga bog'lanmagan" belgisi.
- `*p` qilish (`p == NULL` bo'lganda) — **NULL dereference**: user dasturda `Segmentation fault`, yadroda — page fault va **PANIC** (0-sahifa ataylab xaritalanmaydi — xato darhol ko'rinsin).
- Ko'rsatkich qaytaradigan funksiyalar xatoni `NULL` bilan bildiradi: `malloc`, `fopen`, `strchr`. **Doim tekshiring.**

**Bu dastur nima qiladi (umumiy):** `NULL` ko'rsatkich orqali yozishga urinadi — dastur qulaydi (ataylab xatoli).

```c
/* null_xato.c - NULL orqali yozish */
#include <stdio.h>

int main(void)
{
    int *volatile p = NULL;             /* volatile: kompilyator oldindan "ko'rib" qo'ymasligi uchun */
    printf("yozishdan oldin\n");
    fflush(stdout);
    *p = 5;                             /* NULL dereference */
    printf("bu qatorga yetib kelmaymiz\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra null_xato.c -o null_xato
$ bash -c './null_xato; echo "chiqish kodi: $?"' 2>&1 | sed 's/[0-9][0-9]* Segm/Segm/'
yozishdan oldin
bash: line 1:  Segmentation fault      ./null_xato
chiqish kodi: 139
```

Ikkinchi `printf` bajarilmadi: dastur `Segmentation fault` (signal 11, chiqish kodi 139 = 128 + 11) bilan o'ldi.

### Boshlang'ich qiymatsiz ko'rsatkich — "yovvoyi" ko'rsatkich

```c
int *p;             /* ichida AXLAT - tasodifiy manzil */
*p = 5;             /* tasodifiy joyga yozish: qulash yoki jim buzilish */
```

> **Eslab qoling:** ko'rsatkichni doim yo **haqiqiy manzil**, yo **`NULL`** bilan boshlang. Ishlatishdan oldin `NULL` ni tekshiring.

## 7.4. Ko'rsatkich arifmetikasi

**Hayotdan misol: ko'chadagi uylar.** `p + 1` — "keyingi uy". Agar uylar 4 metr enlikda bo'lsa (`int` — 4 bayt), keyingi uy 4 metr naridagi uy. `char` uylari 1 metr —
`p + 1` 1 metr nari. Kompilyator uy enini o'zi biladi, siz faqat "nechta uy" deysiz.

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

**Bu dastur nima qiladi (umumiy):** ko'rsatkich arifmetikasini ko'rsatadi: `p + 1` keyingi elementga o'tadi, `p++` siljiydi, ikki ko'rsatkich ayirmasi elementlar sonini beradi.

```c
/* arifmetika.c - ko'rsatkich arifmetikasi */
#include <stdio.h>

int main(void)
{
    int a[5] = { 10, 20, 30, 40, 50 };
    int *p = a;                                     /* a[0] ga ko'rsatadi */

    printf("*p = %d\n", *p);
    printf("*(p + 2) = %d, p[2] = %d\n", *(p + 2), p[2]);

    printf("p + 1 manzili p dan %td bayt narida\n", (char *)(p + 1) - (char *)p);

    p++;                                            /* p endi a[1] ga ko'rsatadi */
    printf("p++ dan keyin *p = %d\n", *p);

    int *q = &a[4];
    printf("q - p = %td element\n", q - p);

    char *c = (char *)a;                            /* BAYT sifatida qarash */
    printf("(char *)a + 4 -> a[1] = %d\n", *(int *)(c + 4));
    return 0;
}
```

```console
$ gcc -Wall -Wextra arifmetika.c -o arifmetika
$ ./arifmetika
*p = 10
*(p + 2) = 30, p[2] = 30
p + 1 manzili p dan 4 bayt narida
p++ dan keyin *p = 20
q - p = 3 element
(char *)a + 4 -> a[1] = 20
```

**Qadamlar (xotira rasmi):**

```text
indeks:      a[0]   a[1]   a[2]   a[3]   a[4]
qiymat:      [10]   [20]   [30]   [40]   [50]
manzil:      1000   1004   1008   1012   1016
p = a:        ^
p + 1:               ^      (1000 + 1*4 = 1004)
p + 2:                      ^      (1000 + 2*4 = 1008)  -> *(p+2) = 30
p++ keyin:           ^      (p = 1004)
q = &a[4]:                                 ^
q - p = (1016 - 1004) / 4 = 3
```

**Muhim qoida:** `p + n` manzilni `n * sizeof(*p)` baytga siljitadi. `char *` uchun — `n` bayt, `int *` uchun — `4n` bayt, `struct x *` uchun — `n` × struct hajmi.
`q - p` ham manzillar farqini **elementlar soniga** aylantiradi.

**Yana muhimroq:** `a[i]` — bu **ta'rifan** `*(a + i)`. Massiv indeksi ko'rsatkich arifmetikasining qisqa yozuvi (6.1 dagi manzil formulasi shu!). Qiziq natija: `a[2]` va `2[a]` bir xil (hech qachon yozmang, lekin tushuning).

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

**Bu dastur nima qiladi (umumiy):** `strlen` ni ko'rsatkich bilan o'zimiz yozamiz: oxirgi `'\0'` ga yetguncha siljib, ayirmani qaytaramiz.

```c
/* mening_strlen.c - strlen ni ko'rsatkich bilan yozish */
#include <stdio.h>

static size_t mening_strlen(const char *s)
{
    const char *p = s;
    while (*p)                          /* *p != '\0' */
        p++;
    return (size_t)(p - s);             /* ko'rsatkichlar ayirmasi = uzunlik */
}

int main(void)
{
    printf("mening_strlen(\"abc\") = %zu\n", mening_strlen("abc"));
    printf("mening_strlen(\"\") = %zu\n", mening_strlen(""));
    return 0;
}
```

```console
$ gcc -Wall -Wextra mening_strlen.c -o mening_strlen
$ ./mening_strlen
mening_strlen("abc") = 3
mening_strlen("") = 0
```

**Qadam-baqadam (`"abc"`, `s` = 1000):**

| Qadam | `p` | `*p` | `while (*p)` | Natija |
|---|---|---|---|---|
| boshida | 1000 | `'a'` | rost | `p++` |
| | 1001 | `'b'` | rost | `p++` |
| | 1002 | `'c'` | rost | `p++` |
| | 1003 | `'\0'` (0) | **yolg'on** | sikl tugadi |

`p - s = 1003 - 1000 = 3` — uzunlik. Kodni qog'ozda har bir qadamda `p` qayerga ko'rsatishini chizib kuzating. 08-mashqda o'zingiz yozasiz.

## 7.5. `->` — struktura ko'rsatkichi orqali a'zo

Struktura (9-bob) — turli turdagi maydonlarni birlashtirgan "paket". `.` — struktura **qiymati**ning maydoniga, `->` — struktura **ko'rsatkichi**ning maydoniga:

**Bu dastur nima qiladi (umumiy):** struktura maydoniga `.` bilan va ko'rsatkich orqali `->` bilan murojaat qilishni ko'rsatadi.

```c
/* strelka.c - . va -> */
#include <stdio.h>

struct nuqta { int x, y; };

int main(void)
{
    struct nuqta n = { 1, 2 };
    struct nuqta *p = &n;

    (*p).x = 10;                        /* p ko'rsatgan struktura, uning x maydoni */
    p->y = 20;                          /* aynan o'sha - qisqa yozuv */
    printf("n.x = %d, n.y = %d\n", n.x, n.y);
    return 0;
}
```

```console
$ gcc -Wall -Wextra strelka.c -o strelka
$ ./strelka
n.x = 10, n.y = 20
```

**Hayotdan misol:** `p->oshxona` — "`p` dagi manzilga bor va **oshxonaga kir**". Bu `(*p).oshxona` ning qisqa yozuvi (qavs kerak, chunki `.` `*` dan kuchliroq: `*p.x` noto'g'ri bo'lardi).

Yadroda deyarli hamma narsa ko'rsatkich orqali, shuning uchun `->` hamma joyda: `current->mm`, `f->fops->read(f, buf, len, f->offset)`. (9-bob.)

> **Eslab qoling:** `s.maydon` — `s` oddiy struktura; `p->maydon` — `p` struktura **ko'rsatkichi**.

## 7.6. `const` va ko'rsatkichlar

```c
const char *p;          /* ko'rsatilgan MA'LUMOT o'zgarmas:  *p = 'x' - xato,  p++ - mumkin */
char *const p;          /* KO'RSATKICHNING O'ZI o'zgarmas:   p++ - xato,  *p = 'x' - mumkin */
const char *const p;    /* ikkalasi ham o'zgarmas */
```

O'qish qoidasi: **o'ngdan chapga**. `const char *p` → "p — ko'rsatkich — `const char` ga". `char *const p` → "p — o'zgarmas (`const`) ko'rsatkich — `char` ga".

**Bu dastur nima qiladi (umumiy):** `const char *` (ma'lumot qulflangan) va `char *const` (ko'rsatkich qulflangan) farqini ko'rsatadi — ikkalasini buzish ham xato beradi.

```c
/* const_korsatkich.c - const qaysi qismni qulflaydi */
int main(void)
{
    char harf = 'a';
    const char *p = &harf;              /* ma'lumot o'zgarmas */
    char *const q = &harf;              /* ko'rsatkichning o'zi o'zgarmas */

    *p = 'x';                           /* xato: ma'lumotni o'zgartirib bo'lmaydi */
    q++;                                /* xato: q ni siljitib bo'lmaydi */
    return 0;
}
```

```console
$ gcc -Wall -Wextra -c const_korsatkich.c -o const_korsatkich.o # xato kutiladi
const_korsatkich.c: In function ‘main’:
const_korsatkich.c:8:8: error: assignment of read-only location ‘*p’
    8 |     *p = 'x';                           /* xato: ma'lumotni o'zgartirib bo'lmaydi */
      |        ^
const_korsatkich.c:9:6: error: increment of read-only variable ‘q’
    9 |     q++;                                /* xato: q ni siljitib bo'lmaydi */
      |      ^~
```

Funksiya parametrlarida `const` — **va'da**: `size_t strlen(const char *s)` — "satringizni o'zgartirmayman". Kompilyator va'dani tekshiradi, o'quvchi esa funksiyaning nima qilishini imzosidan tushunadi.

> **Eslab qoling:** funksiya o'zgartirmaydigan har bir ko'rsatkich parametrini `const` qiling.

## 7.7. `void *` — turi noma'lum manzil

**Bu dastur nima qiladi (umumiy):** `void *` har qanday ko'rsatkichni saqlay olishini, lekin ishlatishdan oldin aniq turga aylantirish kerakligini ko'rsatadi.

```c
/* void_korsatkich.c - void * ning ishlatilishi */
#include <stdio.h>

int main(void)
{
    int x = 7;
    void *p = &x;                       /* istalgan ko'rsatkich void * ga sig'adi */
    int *a = p;                         /* C'da void * istalgan ko'rsatkichga avtomatik aylanadi */
    printf("*a = %d\n", *a);

    /* *p qilib bo'lmaydi: qancha bayt o'qishni bilmaydi.
       Avval aniq turga aylantiring: */
    printf("*(int *)p = %d\n", *(int *)p);
    return 0;
}
```

```console
$ gcc -Wall -Wextra void_korsatkich.c -o void_korsatkich
$ ./void_korsatkich
*a = 7
*(int *)p = 7
```

- `void *` ga har qanday ko'rsatkich sig'adi va u har qanday ko'rsatkichga aylanadi (cast shart emas).
- `*p` qilib **bo'lmaydi** (qancha bayt o'qishni bilmaydi) va `p + 1` standartda ruxsat etilmagan (GCC 1 bayt deb hisoblaydi). Avval aniq turga aylantiring.
- "Umumiy" funksiyalar shuni ishlatadi: `memcpy(void *d, const void *s, size_t n)`, `qsort`, oqim funksiyasining `void *arg` i (29-mashq), `struct file` dagi `void *priv`
  (MyOS VFS: har bir fayl tizimi o'z ma'lumotini shunda saqlaydi).

## 7.8. Ko'rsatkichga ko'rsatkich: `int **`, `char **`

**Hayotdan misol: manzillar daftarining manzili.** Siz do'stingizga "manzillar daftarim javonda turibdi" deysiz. U avval daftarni topadi, keyin daftardan kerakli manzilni
o'qiydi, keyin o'sha uyga boradi. Ikki marta "borish" — `**pp`.

```c
/* ikki_yulduz.c - ko'rsatkichga ko'rsatkich */
#include <stdio.h>

int main(void)
{
    int x = 5;
    int *p = &x;                        /* p x ga ko'rsatadi */
    int **pp = &p;                      /* pp p ga ko'rsatadi */

    **pp = 7;                           /* pp -> p -> x: ikki marta borish */
    printf("x = %d, *p = %d, **pp = %d\n", x, *p, **pp);
    return 0;
}
```

```console
$ gcc -Wall -Wextra ikki_yulduz.c -o ikki_yulduz
$ ./ikki_yulduz
x = 7, *p = 7, **pp = 7
```

```text
   pp ───> p ───> x
  (int **) (int *) (int)       **pp = 7  =>  x = 7
```

Qayerda kerak:

1. **Satrlar massivi:** `char **argv` — har bir element `char *` (satr manzili).
   ```text
   argv ──> [ ptr0 ][ ptr1 ][ ptr2 ][ NULL ]
               │       │       └──> "42\0"
               │       └──────────> "salom\0"
               └──────────────────> "./dastur\0"
   ```
2. **Funksiya chaqiruvchining KO'RSATKICHINI o'zgartirishi kerak bo'lsa** (`int *` ni o'zgartirish uchun uning manzili — `int **` — kerak):
   ```text
   int yarat(struct tugun **natija)
   {
       *natija = malloc(sizeof(struct tugun));
       return *natija ? 0 : -1;
   }
   struct tugun *t;
   yarat(&t);
   ```
   MyOS'da: `int pipe_create(struct file **rf, struct file **wf)` — ikkita yangi fayl yaratib, ularning manzillarini chaqiruvchiga qaytaradi.
3. **Ro'yxatdan o'chirishning "yaxshi did" usuli** (16-mashq): `struct tugun **pp` — "o'zgartirilishi kerak bo'lgan ko'rsatkichning manzili", boshni alohida holat qilmasdan.

## 7.9. Funksiya ko'rsatkichlari

**Hayotdan misol: telefon raqami.** Raqamni saqlab qo'yasiz va kerak paytda qo'ng'iroq qilasiz. Raqamni boshqasiga almashtirsangiz — boshqa odam javob beradi. Yadroda drayverlar
aynan shunday: "o'qish kerak bo'lsa — mana bu raqamga qo'ng'iroq qil".

5.10 da ko'rgansiz: `int (*amal)(int, int) = qoshish;`. Sintaksis qo'rqinchli ko'rinadi — `typedef` bilan soddalashtiriladi:

```c
typedef int (*amal_fn)(int, int);
amal_fn amal = qoshish;
```

### Funksiya jadvali — C'dagi "polimorfizm", yadroning asosi

Quyida yadroning **asosiy hiylasini** kichik dasturda ko'rsatamiz: bir xil `fayl_oqi(f)` chaqiruvi, lekin `f` turiga qarab turli funksiya ishlaydi.

**Bu dastur nima qiladi (umumiy):** VFS ning kichik nusxasi: har bir fayl o'z funksiyalar jadvalini (`pipe`, `tty`) ko'rsatadi, umumiy `fayl_oqi` esa turini bilmay shu jadval orqali chaqiradi.

```c
/* fops.c - funksiya jadvali: yadro VFS ning kichik nusxasi */
#include <stdio.h>

struct fayl;                            /* oldindan e'lon: ichida jadvalga ko'rsatkich bor */

struct fayl_amallar {                   /* "jadval": funksiyalarning manzillari */
    int (*oqi)(struct fayl *f);
    const char *nom;
};

struct fayl {
    const struct fayl_amallar *amal;    /* bu fayl qaysi jadvalni ishlatadi */
    int ichki;
};

static int pipe_oqi(struct fayl *f) { printf("  pipe_oqi: bufer %d\n", f->ichki); return 1; }
static int tty_oqi(struct fayl *f)  { printf("  tty_oqi: klaviatura %d\n", f->ichki); return 2; }

static const struct fayl_amallar pipe_amallar = { pipe_oqi, "pipe" };
static const struct fayl_amallar tty_amallar  = { tty_oqi, "tty" };

/* VFS: qaysi fayl turi ekanini BILMASDAN chaqiradi */
static int fayl_oqi(struct fayl *f)
{
    printf("%s ni o'qiymiz:\n", f->amal->nom);
    return f->amal->oqi(f);
}

int main(void)
{
    struct fayl a = { &pipe_amallar, 10 };
    struct fayl b = { &tty_amallar, 20 };

    fayl_oqi(&a);
    fayl_oqi(&b);
    return 0;
}
```

```console
$ gcc -Wall -Wextra fops.c -o fops
$ ./fops
pipe ni o'qiymiz:
  pipe_oqi: bufer 10
tty ni o'qiymiz:
  tty_oqi: klaviatura 20
```

**Kodda nimalar bor:**

| Qism | Nima | Nega |
|---|---|---|
| `struct fayl_amallar` | funksiya **manzillari** to'plami (`oqi`) va nom | "bu turdagi fayl nima qila oladi" ro'yxati |
| `pipe_oqi`, `tty_oqi` | ikki xil o'qish funksiyasi | har fayl turi o'zicha |
| `pipe_amallar`, `tty_amallar` | ikki **jadval**; har birida o'z funksiyasi | |
| `struct fayl` | har fayl o'ziga mos **jadvalga** ko'rsatadi | |
| `f->amal->oqi(f)` | "`f` ning jadvalini ol, undagi `oqi` ni chaqir" | `fayl_oqi` qaysi tur ekanini **bilmaydi**: jadval hal qiladi |

Haqiqiy MyOS'da:

```text
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

n = f->fops->read(f, buf, len, f->offset);      /* kernel/fs/vfs.c */
```

`f` pipe bo'lsa — `pipe_read`, ext2 fayli bo'lsa — `ext2_read`, terminal bo'lsa — `tty_read` ishlaydi. Python'dagi klass metodlari aynan shunday amalga oshirilgan. Linux'da
`file_operations`, `inode_operations`, `pci_driver`... — minglab shunday jadvallar. MyOS: `kernel/fs/vfs.h`, `kernel/fs/pipe.c`, `kernel/drivers/ahci.c`. 20-mashq — shu uslubning amaliyoti.

## 7.10. `container_of` — a'zodan butun strukturaga

Linux'ning eng mashhur makrosi (MyOS: `kernel/lib/common.h`):

```c
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
```

**Masala:** sizda strukturaning **ichidagi maydonning** manzili bor (masalan, ro'yxat tuguni `t`), lekin tugun joylashgan **butun struktura**ni topish kerak.
`offsetof(type, member)` — a'zoning struktura boshidan siljishi (baytda). A'zoning manzilidan shu siljishni ayirsak — strukturaning boshini topamiz.
`(char *)` ga aylantirish shart: ayirish **bayt** bo'yicha bo'lishi uchun (7.4-dagi qoida!).

**Bu dastur nima qiladi (umumiy):** `container_of` makrosi: struktura ichidagi a'zoning manzilidan butun strukturaning manzilini topadi (Linux yadrosining mashhur hiylasi).

```c
/* container_misol.c - a'zodan butun strukturani topish */
#include <stddef.h>
#include <stdio.h>

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

struct tugun {
    struct tugun *keyingi;
};

struct talaba {
    int id;
    char ism[8];
    struct tugun t;                     /* ro'yxatga ulash uchun ichki tugun */
};

int main(void)
{
    struct talaba s = { 42, "Ali", { NULL } };
    struct tugun *tp = &s.t;            /* bizda faqat tugun manzili bor */

    struct talaba *butun = container_of(tp, struct talaba, t);
    printf("offsetof(t) = %zu bayt\n", offsetof(struct talaba, t));
    printf("topildi: id = %d, ism = %s, to'g'ri manzil? %d\n", butun->id, butun->ism, butun == &s);
    return 0;
}
```

```console
$ gcc -Wall -Wextra container_misol.c -o container_misol
$ ./container_misol
offsetof(t) = 16 bayt
topildi: id = 42, ism = Ali, to'g'ri manzil? 1
```

**Qadam:** `tp` = `s` boshidan 16 bayt narida (id: 4, ism: 8, tekislash: 4 → `t` 16-baytdan). `tp - 16` = `s` ning boshi → `butun == &s` ✓. 23-mashqda buni amalda qo'llaysiz.

## 7.11. Ko'rsatkichlar bilan tipik xatolar — ro'yxat

**Hayotdan misol: ko'chib ketgan odamning eski manzili.** Do'stingiz ko'chib ketdi, siz esa eski manzilga xat yuborasiz. U yerda endi boshqa odam yashaydi — xatingizni u oladi.
`free` qilingan xotiraga ko'rsatkich orqali yozish aynan shunday (**osilib qolgan ko'rsatkich**, dangling pointer).

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

```text
struct tugun *t = malloc(sizeof(*t));       /* sizeof(*t) - t ko'rsatgan turning hajmi */
```

Tur o'zgarsa ham to'g'ri qoladi.

## Hayotdan misol va to'liq dastur

**Pochta xizmati.** Ko'cha — uylar massivi. Pochtachi (`xat_tashla`) uyning **manzili** bilan keladi va qutiga xat tashlaydi. Bitta dasturda: `&`, `*`, `->`, `NULL`, arifmetika va funksiya ko'rsatkichi.

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

**Kodda nimalar bor:**

| Qism | Nima qiladi |
|---|---|
| `struct uy` | uy: `raqam` va `xatlar` (qutidagi xatlar soni) |
| `kocha[4]` | 4 ta uy: raqamlari 1, 3, 5, 7; xatlar 0 |
| `struct uy *p = &kocha[0];` | `p` — 1-uyning manzili |
| `xat_tashla(p, 2)` | `u->xatlar += 2` — haqiqiy uyning qutisiga yozadi (manzil bo'yicha) |
| `p = p + 2;` | `p` 2 ta uy (2 × `sizeof(struct uy)`) nariga siljidi → 5-uy |
| `xat_tashla(NULL, 1)` | `u == NULL` — xavfsiz tekshiruv: "manzil yo'q" |
| `void (*xizmat)(struct uy *) = oddiy;` | funksiya manzili saqlandi; `xizmat = tezkor` — boshqasiga almashdi |
| `for (struct uy *u = kocha; u < kocha + 4; u++)` | ko'rsatkich bilan massivni aylanish (`u` — joriy uy) |

**Qutilar qanday to'ladi:** 1-uy: 2 xat (birinchi `xat_tashla`); 5-uy: 1 xat (`p + 2`); 3-uy: 1 xat (`oddiy`); 7-uy: 3 xat (`tezkor`). Natija jadvalga mos.

**Sinab ko'ring:** `p = p + 2;` ni `p = p + 5;` qilib, `-fsanitize=address` bilan yig'ing — 5 ta uy nariga borsak, ko'chada uy bormi? `xat_tashla` ni `struct uy u` (yulduzsiz, nusxa) qabul qiladigan qilib yozsangiz, qutilar nega bo'sh qoladi?

<!-- katta:boshi -->
## Katta loyiha: Ombor — 7-bosqich: ko'rsatkichlar

**Oldingi bosqichdan:** ma'lumot massivlarda, mahsulot nomi bo'yicha topiladi. Lekin ikki narsa noqulay: (1) `sot` zaxirani o'zgartirish uchun **indeks** va global massivni bilishi kerak — funksiya "o'zi nimani o'zgartirayotganini" ko'rsatmaydi; (2) ma'lumotni saralash, eng qimmatni topish kabi amallar yo'q.

### Bu bosqichda nima qilamiz

**Ko'rsatkich** — o'zgaruvchining xotiradagi **manzilini** saqlaydigan o'zgaruvchi (7-bob). Uning yordamida funksiya **boshqa joydagi** qiymatni o'qiy va o'zgartira oladi. Ombor loyihasida to'rt joyda ishlatamiz:

| Qayerda | Ko'rsatkich nima beradi |
|---|---|
| `sot(uint16_t *zaxira, int miqdor)` | funksiya zaxira **katagining o'ziga** boradi va uni o'zgartiradi; chaqirishda `&soni[i]` — "i-katakning manzili" |
| `eng_qimmat(const long *a, int n)` | massivdagi **eng katta elementning manzilini** qaytaradi (`NULL` — massiv bo'sh) |
| `jami_qiymat(const long *a_narx, const uint16_t *a_soni, int n)` | massivni **ko'rsatkich** sifatida oladi (massiv funksiyaga uzatilganda ko'rsatkichga aylanadi) |
| `almashtir_long(long *a, long *b)` | ikki katakning qiymatini **almashtiradi** (nusxa bo'yicha uzatishda bu mumkin emas edi) |

**Nega `const`?** `const long *a` — "bu funksiya `a` ko'rsatgan ma'lumotni **o'zgartirmaydi**" degan va'da. Kompilyator va'dani **tekshiradi**: o'zgartirmoqchi bo'lsangiz xato beradi. O'qiydigan funksiyalarda doim `const` yozing.

**Pufakcha (bubble) saralash:** qo'shni elementlarni solishtirib, noto'g'ri tartibda bo'lsa almashtiradi; eng qimmati "pufakcha kabi" boshiga ko'tariladi. Parallel massivlar bo'lgani uchun **uchala massivni birga** almashtirish kerak — bu noqulaylik 9-bobda (`struct`) yo'qoladi.

```c
/* ombor.c - Ombor, 7-bosqich: ko'rsatkichlar - zaxirani manzil bo'yicha o'zgartirish, saralash, eng qimmatni topish */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ombor_chop.h"

#define MAKS 8
#define NOM_UZ 24

enum { OK = 0, TOLA = -1, NOM_BAND = -2, TOPILMADI = -3, NOTOGRI_MIQDOR = -4, YETARLI_EMAS = -5 };

static char nom[MAKS][NOM_UZ];
static long narx[MAKS];
static uint16_t soni[MAKS];
static int n;

static int topish(const char *qidirilgan)
{
    for (int i = 0; i < n; i++)
        if (strcmp(nom[i], qidirilgan) == 0)
            return i;
    return -1;
}

static int qosh(const char *yangi_nom, long yangi_narx, uint16_t yangi_soni)
{
    if (n == MAKS)
        return TOLA;
    if (topish(yangi_nom) >= 0)
        return NOM_BAND;
    snprintf(nom[n], NOM_UZ, "%s", yangi_nom);
    narx[n] = yangi_narx;
    soni[n] = yangi_soni;
    n++;
    return OK;
}

/* zaxira - o'zgartiriladigan katakning MANZILI: funksiya shu katakka borib yozadi */
static int sot(uint16_t *zaxira, int miqdor)
{
    if (miqdor <= 0)
        return NOTOGRI_MIQDOR;
    if (miqdor > *zaxira)                       /* *zaxira - manzil ko'rsatgan joydagi qiymat */
        return YETARLI_EMAS;
    *zaxira -= miqdor;
    return OK;
}

/* massivdagi eng katta narxning MANZILINI qaytaradi (bo'sh bo'lsa NULL) */
static const long *eng_qimmat(const long *a, int soni_a)
{
    if (soni_a == 0)
        return NULL;
    const long *eng = a;
    for (const long *p = a + 1; p < a + soni_a; p++)    /* p++ - keyingi elementga o'tish */
        if (*p > *eng)
            eng = p;
    return eng;
}

static long jami_qiymat(const long *a_narx, const uint16_t *a_soni, int soni_a)
{
    long jami = 0;
    for (int i = 0; i < soni_a; i++)
        jami += *(a_narx + i) * *(a_soni + i);          /* *(a + i) xuddi a[i] */
    return jami;
}

static void almashtir_long(long *a, long *b)
{
    long t = *a;
    *a = *b;
    *b = t;
}

static void almashtir_soni(uint16_t *a, uint16_t *b)
{
    uint16_t t = *a;
    *a = *b;
    *b = t;
}

static void almashtir_nom(char *a, char *b)
{
    char t[NOM_UZ];
    strcpy(t, a);
    strcpy(a, b);
    strcpy(b, t);
}

/* narx bo'yicha kamayish tartibida (qimmati birinchi): "pufakcha" saralash */
static void narx_boyicha_saralash(void)
{
    for (int o = 0; o < n - 1; o++)
        for (int i = 0; i < n - 1 - o; i++)
            if (narx[i] < narx[i + 1]) {
                almashtir_long(&narx[i], &narx[i + 1]);         /* uchta massivni birga almashtirish kerak */
                almashtir_soni(&soni[i], &soni[i + 1]);
                almashtir_nom(nom[i], nom[i + 1]);
            }
}

static void royxat(void)
{
    chop_sarlavha();
    for (int i = 0; i < n; i++)
        chop_qator(nom[i], narx[i], soni[i]);
    chop_jami(jami_qiymat(narx, soni, n), 12);
}

static const char *xato_matni(int kod)
{
    switch (kod) {
    case TOLA: return "ombor to'lgan";
    case NOM_BAND: return "bunday nomli mahsulot allaqachon bor";
    case TOPILMADI: return "bunday mahsulot topilmadi";
    case NOTOGRI_MIQDOR: return "miqdor musbat bo'lishi kerak";
    case YETARLI_EMAS: return "omborda yetarli emas";
    default: return "noma'lum xato";
    }
}

static void qoshish_menyusi(void)
{
    char s[NOM_UZ];
    long so_m;
    int miqdor;
    printf("Nom, narx (so'mda) va soni?\n");
    if (scanf("%23s %ld %d", s, &so_m, &miqdor) != 3)
        return;
    int r = qosh(s, so_m * 100, (uint16_t)miqdor);
    if (r == OK)
        printf("  Qo'shildi: %s\n", s);
    else
        printf("  XATO: %s\n", xato_matni(r));
}

static void sotish_menyusi(void)
{
    char s[NOM_UZ];
    int miqdor;
    printf("Mahsulot nomi va necha dona?\n");
    if (scanf("%23s %d", s, &miqdor) != 2)
        return;
    int i = topish(s);
    int r = i < 0 ? TOPILMADI : sot(&soni[i], miqdor);      /* &soni[i] - i-katakning manzili */
    if (r == OK)
        printf("  Sotildi: %d dona %s. Qoldi: %u dona\n", miqdor, nom[i], soni[i]);
    else
        printf("  XATO: %s\n", xato_matni(r));
}

static void hisobot(void)
{
    const long *q = eng_qimmat(narx, n);
    if (!q) {
        printf("  ombor bo'sh\n");
        return;
    }
    int i = (int)(q - narx);                    /* ikki ko'rsatkich ayirmasi = elementlar soni */
    printf("  Eng qimmat: %s (", nom[i]);
    chop_pul(*q);
    printf(" so'm)\n  Jami qiymat: ");
    chop_pul(jami_qiymat(narx, soni, n));
    printf(" so'm\n");
}

int main(void)
{
    qosh("non", 400000, 120);
    qosh("sut", 1200000, 45);
    qosh("guruch", 1800000, 8);

    int tanlov;
    while (printf("\n1) ro'yxat  2) sotish  3) qo'shish  4) saralash  5) hisobot  0) chiqish\nTanlov:\n"),
           scanf("%d", &tanlov) == 1 && tanlov != 0) {
        switch (tanlov) {
        case 1: royxat(); break;
        case 2: sotish_menyusi(); break;
        case 3: qoshish_menyusi(); break;
        case 4: narx_boyicha_saralash(); printf("  Narx bo'yicha saralandi\n"); break;
        case 5: hisobot(); break;
        default: printf("  XATO: menyuda bunday band yo'q\n");
        }
    }
    printf("\nXayr!\n");
    return 0;
}
```

```console
$ cd katta_loyiha/ombor/07_korsatkichlar
$ printf '3\nshakar 15000 60\n5\n4\n1\n2\nsut 50\n2\nsut 5\n5\n0\n' > kirish.txt
$ gcc -Wall -Wextra ombor.c ombor_chop.c -o ombor
$ ./ombor < kirish.txt

1) ro'yxat  2) sotish  3) qo'shish  4) saralash  5) hisobot  0) chiqish
Tanlov:
Nom, narx (so'mda) va soni?
  Qo'shildi: shakar

1) ro'yxat  2) sotish  3) qo'shish  4) saralash  5) hisobot  0) chiqish
Tanlov:
  Eng qimmat: guruch (18000.00 so'm)
  Jami qiymat: 2064000.00 so'm

1) ro'yxat  2) sotish  3) qo'shish  4) saralash  5) hisobot  0) chiqish
Tanlov:
  Narx bo'yicha saralandi

1) ro'yxat  2) sotish  3) qo'shish  4) saralash  5) hisobot  0) chiqish
Tanlov:
================ OMBOR ================
Mahsulot         Narx   Soni          Summa
---------------------------------------
guruch       18000.00      8      144000.00
shakar       15000.00     60      900000.00
sut          12000.00     45      540000.00
non           4000.00    120      480000.00
---------------------------------------
Jami qiymat:                2064000.00
QQS stavkasi:               12%
QQS summasi:                247680.00

1) ro'yxat  2) sotish  3) qo'shish  4) saralash  5) hisobot  0) chiqish
Tanlov:
Mahsulot nomi va necha dona?
  XATO: omborda yetarli emas

1) ro'yxat  2) sotish  3) qo'shish  4) saralash  5) hisobot  0) chiqish
Tanlov:
Mahsulot nomi va necha dona?
  Sotildi: 5 dona sut. Qoldi: 40 dona

1) ro'yxat  2) sotish  3) qo'shish  4) saralash  5) hisobot  0) chiqish
Tanlov:
  Eng qimmat: guruch (18000.00 so'm)
  Jami qiymat: 2004000.00 so'm

1) ro'yxat  2) sotish  3) qo'shish  4) saralash  5) hisobot  0) chiqish
Tanlov:

Xayr!
```

**Kiritilgan ketma-ketlik:** `3` shakar qo'shildi → `5` hisobot (eng qimmat: guruch 18000; jami 2064000) → `4` saralash → `1` ro'yxat (**narx bo'yicha kamayish**: guruch, shakar, sut, non) → `2 sut 50` (xato: faqat 45) → `2 sut 5` (sotildi) → `5` hisobot (jami 2004000: sut 5 donaga kamaydi).

**Ko'rsatkich amallari — qatorma-qator:**

| Kod | Ma'nosi |
|---|---|
| `uint16_t *zaxira` | "`uint16_t` turidagi katakning manzili" |
| `*zaxira` | shu manzildagi **qiymat**ni o'qish (yoki yozish: `*zaxira -= miqdor`) |
| `&soni[i]` | `soni[i]` katagining **manzili** |
| `const long *eng = a;` | massiv boshiga ko'rsatkich (`a` — birinchi elementning manzili) |
| `p++` | ko'rsatkichni **keyingi elementga** siljitish (`long` bo'lsa 8 baytga: kompilyator o'zi hisoblaydi) |
| `*(a_narx + i)` | `a_narx[i]` bilan **bir xil** — indeks aslida "manzil + siljish" |
| `q - narx` | ikki ko'rsatkich ayirmasi = oralaridagi **elementlar soni** (`i` indeksini topish) |
| `NULL` | "hech narsaga ko'rsatmaydi": tekshirmasdan `*NULL` — **dastur qulaydi** |

**Nima ko'rdik:** `hisobot` eng qimmat mahsulotni ko'rsatkich bilan topdi va uning **indeksini** `q - narx` bilan hisobladi — shu indeks bo'yicha `nom[i]` ni chiqardi. `sot(&soni[i], miqdor)` — funksiya zaxirani o'zi o'zgartirdi: "Qoldi: 40 dona" — chaqiruvchi (`sotish_menyusi`) uni `soni[i]` dan o'qidi.

**Xavf:** `NULL` ko'rsatkichni, yoki massivdan tashqarini ko'rsatayotgan ko'rsatkichni `*` bilan ishlatish — **aniqlanmagan xatti-harakat** (qulash yoki jim buzilish). Shuning uchun `eng_qimmat` bo'sh massivda `NULL` qaytaradi va `hisobot` uni **tekshiradi** (`if (!q)`).

> **Eslab qoling:** `&x` — x ning manzili; `*p` — p ko'rsatgan qiymat. Funksiya **o'zgartirishi** kerak bo'lgan narsaning manzilini oladi. O'qiydigan funksiyada `const`. Ko'rsatkichni ishlatishdan oldin `NULL` ni tekshiring.

**O'zingiz qo'shing (yechimsiz):**

1. `eng_arzon` funksiyasini yozing (xuddi `eng_qimmat` kabi) va hisobotga qo'shing.
2. `sot` ga qo'shimcha: zaxira 10 dan pastga tushsa `*tugayapti = 1` degan **ikkinchi chiqish ko'rsatkichi** orqali xabar bering. (Maslahat: `int *tugayapti` parametri.)
3. `almashtir_long` ni **bitta** umumiy funksiyaga aylantirib bo'ladimi? Maslahat: `void *` va hajm — 7-bob (`void *`), `memcpy` — 6-bob. Qiyinroq: nega har tur uchun alohida funksiya yozdik?
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. Ko'rsatkich — **manzil saqlaydigan quti**: `int *p = &x;`. `&x` — "x ning manzili", `*p` — "p ko'rsatgan joydagi qiymat".
2. Oddiy argument — nusxa; funksiya asl qiymatni o'zgartirishi uchun **manzil** (`&x`) beriladi, funksiya `*p = ...` qiladi.
3. Ko'rsatkichni **`NULL`** yoki haqiqiy manzil bilan boshlang va ishlatishdan oldin **`NULL`** ni tekshiring.
4. `p + n` manzilni `n * sizeof(*p)` baytga siljitadi; `a[i]` = `*(a + i)`. `->` — struktura ko'rsatkichi orqali maydon.
5. `const char *p` — ma'lumot o'zgarmas; `void *` — turi noma'lum (aniq turga aylantirib ishlating); `char **` — satrlar ro'yxati; funksiya ko'rsatkichi — "qaysi funksiya chaqirilsin" ni o'zgaruvchida saqlash (yadro jadvallari).

## Savol-javob

**Ko'rsatkichni `printf` bilan qanday chiqaraman?**
`printf("%p\n", (void *)p);`. Stek manzillari odatda `0x7ff...`, heap — `0x55...`/`0x5...`, yadro — `0xffff...`. MyOS'da `hello` dasturi o'z manzillarini ko'rsatadi.

**Ko'rsatkich va manzil — bir xil narsami?**
Manzil — son (xotiradagi joy). Ko'rsatkich — manzilni saqlaydigan **turli** o'zgaruvchi. Tur uni qanday talqin qilishni belgilaydi.

**Bu manzillar "haqiqiy" RAM manzillarimi?**
User dasturda — yo'q, **virtual** manzillar. Har bir jarayonning o'z manzil maydoni bor, CPU ularni sahifa jadvali orqali fizik manzilga aylantiradi. Ikki jarayonda bir xil
`0x401000` manzili — turli fizik xotira. Buni yadro boshqaradi: MyOS'da `kernel/mm/vmm.c`, `docs/04-virtual-xotira.md`. Mashq 31 (sahifa jadvali) shu mexanizmni simulyatsiya qiladi.

**Nega ko'rsatkich kerak, Python'da yo'q-ku?**
Python'da ham bor — har bir o'zgaruvchi aslida obyektga "ko'rsatkich" (yorliq), faqat u yashirin. C'da uni **ko'rasiz va boshqarasiz** — yadro uchun shu kerak (apparat manzillari bilan ishlash).

## O'zingizni tekshiring

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

## Mashq

### Isitish: ko'rsatkichlarni kuzatish ★☆☆ — eng osoni, avval shuni qiling

Faqat 0–7-boblar kerak (`&`, `*`, `**`, ko'rsatkich arifmetikasi).
Skeletni `isitish.c` ga **qo'lda** yozing (ko'chirmang), izohlarni o'qing va `TODO` joylarini to'ldiring.
"Namuna" qismlar tayyor — qolganini qanday yozishni ko'rsatadi. Skelet hozir ham ogohlantirishsiz yig'iladi:
har `TODO` dan keyin yig'ib, ishga tushirib boring.

```c
/* isitish.c - 7-bob, isitish: ko'rsatkichlarni kuzatish. malloc va struct kerak emas. */
#include <stdio.h>

int main(void)
{
    int x = 10;
    int *p = &x;                        /* p ichida - x ning MANZILI:            p -> x */
    int **pp = &p;                      /* pp ichida - p ning manzili (7.8):   pp -> p -> x */

    /* 1) Bitta qiymatga uch yo'l. (Namuna - tayyor.) */
    printf("x = %d, *p = %d, **pp = %d\n", x, *p, **pp);

    /* 2) TODO: *p = 20;  keyin  "*p = 20 dan keyin: x = %d\n"   (x ga to'g'ridan-to'g'ri tegmang!)
     *          **pp = 30; keyin  "**pp = 30 dan keyin: x = %d\n"
     *    *p - "p dagi manzilga bor"; **pp - "pp dagi manzilga bor, u yerdagi manzilga yana bor". */

    int a[] = {3, 1, 4, 1, 5};
    int *q = a;                         /* massiv nomi - birinchi elementning manzili: q = &a[0] */

    /* 3) TODO: *q, *(q + 2), q[4]. q + 2 - "2 ta INT keyin" (2 bayt emas, 7.4); q[4] - bu *(q + 4).
     *    Natija: *q = 3, *(q + 2) = 4, q[4] = 5 */

    /* 4) TODO: q += 3; keyin *q va q - a. Ikki ko'rsatkich ayirmasi - orasidagi ELEMENTLAR soni,
     *    turi ptrdiff_t, formati %td.
     *    Natija: q += 3 dan keyin *q = 1, q - a = 3 */
    (void)q;                            /* 3-4-qadamlarni yozgach, bu qatorni o'chiring */

    /* 5) Yig'indi - indekssiz, faqat ko'rsatkich bilan. (Namuna - tayyor.)
     *    a + 5 - massivdan KEYINGI joy: unga solishtirish mumkin, *(a + 5) ni o'qish - mumkin emas. */
    int yig = 0;
    for (int *r = a; r < a + 5; r++)
        yig += *r;
    printf("ko'rsatkich bilan yig'indi: %d\n", yig);
    return 0;
}
```

**Kutilgan natija** (`darslik/loyihalar/07_joyida/isitish.txt`):

```text
x = 10, *p = 10, **pp = 10
*p = 20 dan keyin: x = 20
**pp = 30 dan keyin: x = 30
*q = 3, *(q + 2) = 4, q[4] = 5
q += 3 dan keyin *q = 1, q - a = 3
ko'rsatkich bilan yig'indi: 14
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined isitish.c -o isitish
$ ./isitish | diff - ~/C_loyha/darslik/loyihalar/07_joyida/isitish.txt && echo "TO'G'RI"
TO'G'RI
```

### Keyingi mashqlar

- **06** — `&` va `*` asoslari. **07** — chiqish parametrlari. **08** — ko'rsatkich arifmetikasi (endi `strchr` ham).
  **10** — ikki ko'rsatkich.
- Qo'shimcha: qog'ozda 7.13 dagi xotira rasmini chizing; `int x; int *p = &x; int **pp = &p;` uchun uchta o'zgaruvchining
  manzilini `%p` bilan chiqarib, rasmingiz bilan solishtiring.
- **16** (ro'yxat, `->`, `**pp`) — `struct` (9-bob) va `malloc` (8-bob) kerak: 9-bobdan keyin.
- **20** (funksiya jadvali) — `struct` va `typedef` (9-bob) kerak: 9-bobdan keyin.
- **23** (`container_of`) — `struct` (9-bob) va makrolar (10-bob) kerak: 10-bobdan keyin.

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
