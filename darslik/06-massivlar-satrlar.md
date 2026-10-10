# 6-bob. Massivlar va satrlar

> **Bu bobda nima o'rganasiz:** massiv (bir xil turdagi qutilar qatori) xotirada qanday turishini; nega u o'z uzunligini **bilmasligini**;
> satr — oxirida maxsus `'\0'` belgisi turgan `char` massivi ekanini; **bufer to'lishi** (buffer overflow) qanday paydo bo'lishini va undan qanday qochishni.
> **Oldindan nima kerak:** 2–5-boblar (turlar, sikllar, funksiyalar).   **Vaqt:** 6–7 soat.
> Mashqlar: isitish (bob oxirida), 05, 07, 08 (`strchr` — 7-bobdan keyin), 09, 10, 11, 12.

> **To'liq ishlaydigan misol:** [misollar/06_satrlar.c](misollar/06_satrlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Hozirgacha har bir qiymat uchun alohida o'zgaruvchi yaratdik. 100 ta talabaning bahosini saqlash uchun 100 ta nom o'ylab topish
mumkin emas. **Massiv** — bir xil turdagi **ko'p qutini bitta nom** ostida ketma-ket joylashtirish. **Satr** — harflar massivi.

**Hayotdan misol: poyezd vagonlari.** Poyezdda vagonlar ketma-ket ulangan va raqamlangan. Faqat C'da raqamlash **0 dan** boshlanadi:
5 vagonli poyezdda vagonlar 0, 1, 2, 3, 4. Vagonlar bir xil o'lchamda va bir-biriga yopishgan — shuning uchun 3-vagon qayerdaligini hisoblash oson:
"boshidan 3 vagon uzunligi".

| Poyezd | C massivi |
|---|---|
| butun poyezd | massiv (`int a[5]`) |
| vagon | element (`a[0]`, `a[1]` ...) |
| vagon raqami | **indeks** (0 dan boshlanadi!) |
| vagonlar soni | massiv uzunligi (o'zgarmas) |

Bu bobning ikki xavfli haqiqati: (1) C massivning chegarasini **tekshirmaydi**; (2) C satrining uzunligi **saqlanmaydi**. Ikkalasini
sinchiklab o'rganamiz — chunki xavfsizlik teshiklarining katta qismi shundan.

## 6.1. Massiv

```c
int a[5] = { 10, 20, 30, 40, 50 };
```

Bu qator: "`int` turidagi **5 ta** quti yarat, nomi `a`, ichiga 10, 20, 30, 40, 50 yoz". Xotirada — **ketma-ket** 5 ta `int`, jami 5 × 4 = 20 bayt:

```text
manzil:  1000   1004   1008   1012   1016
         [ 10 ][ 20 ][ 30 ][ 40 ][ 50 ]
          a[0]  a[1]  a[2]  a[3]  a[4]
```

- Indeks **0 dan** boshlanadi: oxirgisi `a[4]`; `a[5]` — massivdan **tashqari**!
- O'lcham **qat'iy**: `a` ni 6 elementga "o'stirib" bo'lmaydi.
- `a[i]` ning manzili = `a` ning manzili + `i * sizeof(int)`. Shuning uchun murojaat juda tez (har qanday `i` uchun bir xil vaqt).

**Nega raqamlash 0 dan?** Indeks — "boshidan **necha qadam** nariga". Birinchi element boshida turibdi — 0 qadam. Shu sababli manzil formulasi
oddiy: `boshi + indeks * element_hajmi`.

**Bu dastur nima qiladi (umumiy):** massiv elementlarini va ularning xotiradagi o'lchamini (butun massiv, bitta element, qo'shni elementlar orasidagi masofa) chiqaradi.

```c
/* massiv_asos.c - massiv xotirada */
#include <stdio.h>

int main(void)
{
    int a[5] = { 10, 20, 30, 40, 50 };

    for (int i = 0; i < 5; i++)
        printf("a[%d] = %d\n", i, a[i]);

    printf("butun massiv: %zu bayt\n", sizeof(a));
    printf("bitta element: %zu bayt\n", sizeof(a[0]));
    printf("a[1] a[0] dan %td bayt narida\n", (char *)&a[1] - (char *)&a[0]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra massiv_asos.c -o massiv_asos
$ ./massiv_asos
a[0] = 10
a[1] = 20
a[2] = 30
a[3] = 40
a[4] = 50
butun massiv: 20 bayt
bitta element: 4 bayt
a[1] a[0] dan 4 bayt narida
```

**Kodda nimalar bor:**

| Qator | Nima qiladi | Nega |
|---|---|---|
| `int a[5] = {10, ..., 50};` | 5 ta `int` qutisi; boshlang'ich qiymatlar | `[5]` — uzunlik, `{ }` — qiymatlar ro'yxati |
| `for (int i = 0; i < 5; i++)` | `i` = 0, 1, 2, 3, 4 | `i < 5` (**`<=` emas**!) — oxirgi indeks `4` |
| `a[i]` | `i`-elementni o'qiydi | |
| `sizeof(a)` | **butun massiv** hajmi: 5 × 4 = 20 | |
| `(char *)&a[1] - (char *)&a[0]` | ikki qo'shni element manzillari orasidagi farq (**bayt**da) | `&a[1]` — "`a[1]` ning manzili" (7-bob); farq `4` = `sizeof(int)` |

### Boshlash usullari

```c
int a[5];                       /* lokal bo'lsa - AXLAT qiymatlar (2.2) */
int b[5] = { 0 };               /* hammasi 0 (qolganlari avtomatik 0) */
int c[] = { 1, 2, 3 };          /* o'lchamni kompilyator hisoblaydi: 3 */
int d[100] = { [10] = 5, [99] = 7 };    /* nomlangan indekslar, qolgani 0 */
```

**Bu dastur nima qiladi (umumiy):** massivni boshlashning uch usuli: hammasini nol qilish, uzunlikni kompilyatorga sanatish va aniq indekslarni berish.

```c
/* boshlash.c - massivni boshlash usullari */
#include <stdio.h>

int main(void)
{
    int b[5] = { 0 };
    int c[] = { 1, 2, 3 };
    int d[100] = { [10] = 5, [99] = 7 };

    printf("b: %d %d %d %d %d\n", b[0], b[1], b[2], b[3], b[4]);
    printf("c uzunligi: %zu\n", sizeof(c) / sizeof(c[0]));
    printf("d[10]=%d d[50]=%d d[99]=%d\n", d[10], d[50], d[99]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra boshlash.c -o boshlash
$ ./boshlash
b: 0 0 0 0 0
c uzunligi: 3
d[10]=5 d[50]=0 d[99]=7
```

`{ 0 }` yozilganda **birinchi** element 0 bo'ladi, **qolganlari ham** avtomatik 0 (qisman berilgan ro'yxatning qolgani nol bilan to'ldiriladi).
Bu massivni **tozalashning** eng qisqa yo'li.

### Massiv uzunligi

```c
size_t n = sizeof(a) / sizeof(a[0]);    /* 20 / 4 = 5 */
```

"Butun hajm ÷ bitta element hajmi" = elementlar soni. Bu **faqat massiv e'lon qilingan joyda** ishlaydi! Funksiyaga uzatilganda massiv
**ko'rsatkichga aylanadi** va `sizeof` ko'rsatkich hajmini (8) beradi:

**Bu dastur nima qiladi (umumiy):** funksiyaga uzatilgan massiv ko'rsatkichga aylanib, uzunligi yo'qolishini (`sizeof` noto'g'ri natija berishini) ko'rsatadi.

```c
/* uzunlik_xato.c - funksiyaga uzatilgan massiv uzunligi yo'qoladi */
#include <stdio.h>

static void f(int a[])                  /* aslida: int *a */
{
    size_t n = sizeof(a) / sizeof(a[0]);    /* 8 / 4 = 2 - XATO! */
    printf("funksiya ichida n = %zu (noto'g'ri)\n", n);
}

int main(void)
{
    int a[5] = { 1, 2, 3, 4, 5 };
    printf("main da n = %zu (to'g'ri)\n", sizeof(a) / sizeof(a[0]));
    f(a);
    return 0;
}
```

```console
$ gcc -Wall -Wextra uzunlik_xato.c -o uzunlik_xato # xato kutiladi
uzunlik_xato.c: In function ‘f’:
uzunlik_xato.c:6:22: warning: ‘sizeof’ on array function parameter ‘a’ will return size of ‘int *’ [-Wsizeof-array-argument]
    6 |     size_t n = sizeof(a) / sizeof(a[0]);    /* 8 / 4 = 2 - XATO! */
      |                      ^
uzunlik_xato.c:4:19: note: declared here
    4 | static void f(int a[])                  /* aslida: int *a */
      |               ~~~~^~~
$ ./uzunlik_xato
main da n = 5 (to'g'ri)
funksiya ichida n = 2 (noto'g'ri)
```

GCC ogohlantirdi (`sizeof on array function parameter`). **Nega?** Massiv funksiyaga uzatilganda uning **nusxasi** ko'chirilmaydi (juda qimmat): faqat
**birinchi elementning manzili** (8 bayt) uzatiladi. Funksiya uzunlikni **bilmaydi**.

Shuning uchun C'da **massiv doim uzunligi bilan birga uzatiladi**: `void f(const int *a, size_t n)`. Standart kutubxonada ham, yadroda ham — `buf, len` juftligi
hamma joyda (`read(fd, buf, len)`, `memcpy(dst, src, n)`).

> **Eslab qoling:** massivni funksiyaga bersangiz — **uzunligini ham bering**. `sizeof(a)/sizeof(a[0])` faqat e'lon qilingan joyda ishlaydi.

## 6.2. Chegaradan chiqish — C'ning eng xavfli xatosi

**Hayotdan misol: 5 qavatli uyda 6-qavat tugmasi.** Python'da bunday tugmani bossangiz, lift "bunday qavat yo'q" deydi (`IndexError`). C'da lift **hech narsa demaydi**
— to'g'ridan-to'g'ri tomga, yoki qo'shni binoning kvartirasiga olib chiqadi. Siz **boshqa birovning xotirasiga** yozasiz va xato ancha keyin, butunlay boshqa joyda chiqadi.

```c
int a[5];
a[5] = 99;          /* massivdan tashqariga yozish - a[0]..a[4] bor, a[5] YO'Q */
a[-1] = 0;          /* bu ham */
```

C **tekshirmaydi**. Kompilyator ham (ko'pincha), ish vaqtida ham. Natija:

- yonidagi o'zgaruvchi **jim** buziladi;
- funksiyaning qaytish manzili ustidan yoziladi → dastur boshqa joyga "sakraydi";
- yoki hech narsa bo'lmaydi — bugun. Ertaga boshqa kompilyator bilan qulaydi.

Tarixdagi eng ko'p xavfsizlik hujumlari (Morris qurti, 1988-yildan beri) aynan shundan boshlangan: foydalanuvchi bergan uzun satr stekdagi buferdan toshib,
qaytish manzilini o'zgartiradi.

Xatoni **ushlash** uchun yig'ish vaqtida **AddressSanitizer** (xotira xatolarini topuvchi vosita) yoqiladi:

**Bu dastur nima qiladi (umumiy):** massiv chegarasidan tashqariga yozish (`a[5]`) — ataylab xatoli.

```c
/* chegara.c - massivdan tashqariga yozish */
#include <stdio.h>

int main(int argc, char **argv)
{
    (void)argv;
    int a[5] = { 1, 2, 3, 4, 5 };
    int i = 4 + argc;                   /* argc = 1 -> i = 5: chegaradan tashqari */
    a[i] = 99;                          /* XATO: a[5] yo'q */
    printf("a[0] = %d\n", a[0]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address chegara.c -o chegara
$ ./chegara 2>&1 | grep -E 'ERROR|WRITE of size|#0' | head -3 | sed -E 's/==[0-9]+==//; s/0x[0-9a-f]+/0x.../g; s/ in main .*chegara/ in main chegara/'
ERROR: AddressSanitizer: stack-buffer-overflow on address 0x... at pc 0x... bp 0x... sp 0x...
WRITE of size 4 at 0x... thread T0
    #0 0x... in main chegara.c:9
```

Sanitizer xatoni **aniq qatori bilan** topdi: `stack-buffer-overflow` ("stekdagi bufer toshdi"), `WRITE of size 4` ("4 bayt yozdingiz"). Sanitizersiz dastur "ishlab ketardi".

**Himoya:** `-fsanitize=address` (o'rganishda), har bir indeksni tekshirish, uzunlikni doim uzatish. Yadroda sanitizer yo'q — faqat intizom.

> **Eslab qoling:** `a[5]` — 5 elementli massivda **mavjud emas**. Tsiklda `i < n` yozing, `i <= n` emas. C sizni ogohlantirmaydi.

## 6.3. Ko'p o'lchamli massivlar

**Hayotdan misol: kinoteatr zali.** Qator va o'rin: `zal[3][7]` — 3-qator, 7-o'rin. Xotirada esa qatorlar bitta uzun chiziqda ketma-ket turadi:
0-qatorning hamma o'rinlari, keyin 1-qator...

```c
/* ikki_olchamli.c - 3 qator, 4 ustun */
#include <stdio.h>

int main(void)
{
    int m[3][4];                        /* 3 qator, 4 ustun = 12 ta int */

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            m[i][j] = i * 10 + j;       /* qator*10 + ustun */

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++)
            printf("%2d ", m[i][j]);
        printf("\n");
    }

    int *tekis = &m[0][0];              /* butun massivga bitta uzun qator sifatida qarash */
    printf("m[1][2] = %d, tekis[1*4 + 2] = %d\n", m[1][2], tekis[1 * 4 + 2]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra ikki_olchamli.c -o ikki_olchamli
$ ./ikki_olchamli
 0  1  2  3 
10 11 12 13 
20 21 22 23 
m[1][2] = 12, tekis[1*4 + 2] = 12
```

**Qadamlar:** `m[i][j] = i * 10 + j` — har katakka o'zining o'rni (qator, ustun) yoziladi. Xotirada 12 ta qiymat ketma-ket:

```text
xotirada:   0  1  2  3 | 10 11 12 13 | 20 21 22 23        <- qatorma-qator
indeks:     0  1  2  3 |  4  5  6  7 |  8  9 10 11
```

`m[1][2]` ning tekis indeksi = `1 * 4 + 2 = 6` (4 — ustunlar soni) → xotirada 6-o'rinda `12` turibdi ✓. Formula: **`m[i][j]` manzili = `m` + `(i * ustunlar + j) * 4`**.

Katta matritsalarni qatorma-qator aylanish (`for i` tashqarida, `for j` ichkarida) keshga mos va tez (21-bob).

## 6.4. Satr — `'\0'` bilan tugaydigan `char` massivi

C'da alohida "satr" turi **yo'q**. Satr — oxirida **nol bayt** (`'\0'`) bo'lgan `char` massivi:

```c
char s[] = "salom";
```

```text
s[0] s[1] s[2] s[3] s[4] s[5]
 's'  'a'  'l'  'o'  'm'  '\0'      -> jami 6 bayt!
```

**Hayotdan misol: ip ustidagi munchoqlar va tugun.** Satr — harflar ketma-ketligi, oxirida esa **tugun** — `'\0'`. Tugunsiz munchoqlar sochilib ketadi: `printf` harflarni
tugunni topguncha o'qiydi, topmasa — xotira bo'ylab keyin nima bo'lsa, hammasini o'qib ketaveradi. `"salom"` 5 harf, lekin 6 bayt joy oladi.

```c
/* satr_asos.c - satr xotirada */
#include <stdio.h>
#include <string.h>

int main(void)
{
    char s[] = "salom";

    printf("sizeof(s) = %zu, strlen(s) = %zu\n", sizeof(s), strlen(s));
    for (size_t i = 0; i < sizeof(s); i++)
        printf("s[%zu] = %3d  (belgi: %c)\n", i, s[i], s[i] >= 32 ? s[i] : '?');

    printf("'\\0' = %d, '0' = %d\n", '\0', '0');
    return 0;
}
```

```console
$ gcc -Wall -Wextra satr_asos.c -o satr_asos
$ ./satr_asos
sizeof(s) = 6, strlen(s) = 5
s[0] = 115  (belgi: s)
s[1] =  97  (belgi: a)
s[2] = 108  (belgi: l)
s[3] = 111  (belgi: o)
s[4] = 109  (belgi: m)
s[5] =   0  (belgi: ?)
'\0' = 0, '0' = 48
```

**Kodda nimalar bor:**

- `sizeof(s)` = **6** (`s`, `a`, `l`, `o`, `m` + `'\0'`); `strlen(s)` = **5** (`'\0'` sanalmaydi). Ikkalasini adashtirmang!
- Oxirgi qatorda `s[5] = 0`: belgining kodi **nol**. `'?'` — ko'rinmas belgi o'rniga (`s[i] >= 32 ? ... : '?'` — 3.7 dagi `?:`).
- `'\0'` — son **0** (belgi `'0'` esa 48!). Uni "terminator" deyiladi.
- Satr uzunligi hech qayerda saqlanmaydi — uni bilish uchun `'\0'` gacha **sanash** kerak (har safar). Python satri uzunligini ichida saqlaydi.

**Savol: nega shunday qilingan?** 1970-yillarda xotira juda qimmat edi: 1 bayt terminator uzunlik maydonidan tejamliroq. Bugun bu qaror "milliard dollarlik xato"
deb ataladi — son-sanoqsiz bufer to'lishlari shu yerdan. Lekin C va unga asoslangan barcha tizimlar (Linux syscall'lari ham) shunday ishlaydi, shuning uchun uni
mukammal bilishingiz shart.

### Satr literali va massiv farqi

```c
char s[] = "salom";         /* MASSIV: 6 baytli nusxa stekda - o'zgartirish mumkin */
char *p = "salom";          /* KO'RSATKICH: faqat o'qiladigan xotiradagi literalga */

s[0] = 'S';                 /* OK */
p[0] = 'S';                 /* UB - odatda Segmentation fault (literal .rodata da) */
```

**Nega farq bor?** `"salom"` literali dastur fayli ichida **faqat o'qiladigan** xotira bo'limida (`.rodata`) yotadi. `char s[] = "salom";` shu literaldan **o'zingizning
nusxangizni** stekda yasaydi — uni o'zgartirsangiz bo'ladi. `char *p = "salom";` esa **o'sha asl literalga** ko'rsatadi — o'zgartirib bo'lmaydi.

**Bu dastur nima qiladi (umumiy):** satr literalini o'zgartirishga urinish: `char s[]` nusxa (mumkin), `const char *p` literal (mumkin emas).

```c
/* literal_xato.c - literalni o'zgartirishga urinish */
#include <stdio.h>

int main(void)
{
    char s[] = "salom";                 /* o'zimizning nusxa */
    s[0] = 'S';
    printf("massiv: %s\n", s);

    const char *p = "salom";            /* const: literalga ko'rsatadi */
    p[0] = 'S';                         /* xato! kompilyator to'xtatadi */
    return 0;
}
```

```console
$ gcc -Wall -Wextra literal_xato.c -o literal_xato # xato kutiladi
literal_xato.c: In function ‘main’:
literal_xato.c:11:10: error: assignment of read-only location ‘*p’
   11 |     p[0] = 'S';                         /* xato! kompilyator to'xtatadi */
      |          ^
```

Literalga ko'rsatkichni **doim** `const char *p = "salom";` deb yozing — shunda kompilyator `p[0] = ...` ni darhol xato deydi (ish vaqtida qulashini kutmaysiz).

> **Eslab qoling:** satr = `'\0'` bilan tugaydigan baytlar. `"salom"` — 6 bayt. Literalni `const char *` bilan oling; o'zgartirmoqchi bo'lsangiz — `char s[] = "..."`.

## 6.5. `<string.h>` — asosiy funksiyalar

| Funksiya | Nima qiladi | Tuzoq |
|---|---|---|
| `strlen(s)` | `'\0'` gacha uzunlik | `'\0'` bo'lmasa — xotira bo'ylab "yuguradi" |
| `strcpy(d, s)` | s ni d ga nusxalash | **d ning hajmini tekshirmaydi** — bufer to'lishi. Ishlatmang |
| `strncpy(d, s, n)` | ko'pi bilan n bayt | s uzun bo'lsa, `'\0'` **qo'ymaydi**! Chalkash |
| `snprintf(d, n, "%s", s)` | xavfsiz nusxa/format | tavsiya etiladi |
| `strcmp(a, b)` | solishtirish: <0, 0, >0 | `a == b` satrlarni emas, **manzillarni** solishtiradi! |
| `strncmp(a, b, n)` | birinchi n bayt | prefiks tekshirish uchun |
| `strchr(s, c)` | c ning birinchi o'rni yoki NULL | |
| `strstr(s, t)` | t ning s ichidagi o'rni | |
| `memcpy(d, s, n)` | n baytni nusxalash | d va s ustma-ust tushmasligi kerak |
| `memmove(d, s, n)` | ustma-ust tushsa ham to'g'ri | |
| `memset(d, c, n)` | n baytni c bilan to'ldirish | |
| `memcmp(a, b, n)` | n baytni solishtirish | |

**`mem*` va `str*` farqi:** `str*` — `'\0'` gacha ishlaydi (matn uchun). `mem*` — aniq n bayt (har qanday ma'lumot: struct, rasm, disk sektori). Yadroda `mem*` ko'proq ishlatiladi.

MyOS'da bularning hammasi **o'zimiz yozgan**: `user/libc/string.c` (user dasturlar uchun) va `kernel/lib/string.c` (yadro uchun). 08-mashqda ulardan uchtasini o'zingiz yozasiz,
lab'larda esa yadrodagisini.

**Bu dastur nima qiladi (umumiy):** `<string.h>` ning asosiy funksiyalarini (`strlen`, `strcmp`, `strchr` va boshqalar) ishlatib, natijalarini chiqaradi.

```c
/* string_h.c - asosiy funksiyalar amalda */
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *s = "salom dunyo";

    printf("strlen: %zu\n", strlen(s));
    printf("strcmp(\"abc\", \"abd\") = %d (manfiy: abc < abd)\n", strcmp("abc", "abd") < 0 ? -1 : 1);
    printf("strcmp(\"abc\", \"abc\") = %d (nol: teng)\n", strcmp("abc", "abc"));

    const char *joy = strchr(s, 'd');           /* 'd' belgisining o'rni */
    printf("strchr 'd' dan boshlab: \"%s\"\n", joy);
    printf("strstr \"dun\": indeks %td\n", strstr(s, "dun") - s);

    char buf[16];
    memset(buf, 'x', sizeof(buf) - 1);          /* 15 ta 'x' */
    buf[15] = '\0';
    printf("memset: %s\n", buf);

    char nusxa[16];
    memcpy(nusxa, "abcdef", 7);                 /* 6 harf + '\0' = 7 bayt */
    printf("memcpy: %s\n", nusxa);
    return 0;
}
```

```console
$ gcc -Wall -Wextra string_h.c -o string_h
$ ./string_h
strlen: 11
strcmp("abc", "abd") = -1 (manfiy: abc < abd)
strcmp("abc", "abc") = 0 (nol: teng)
strchr 'd' dan boshlab: "dunyo"
strstr "dun": indeks 6
memset: xxxxxxxxxxxxxxx
memcpy: abcdef
```

**Nima ko'rdik:** `strchr` — topilgan joydan **boshlab** satrning qolgan qismini ko'rsatadi (`"dunyo"`); `strstr(s, "dun") - s` — topilgan joy manzili minus satr boshi = **indeks** (6).
`memcpy(..., 7)` da `'\0'` ni ham nusxalash uchun `7` bayt berdik (6 harf + terminator) — aks holda nusxa tugatilmagan bo'lib qolardi.

## 6.6. Satrni belgima-belgi aylanish

**Bu dastur nima qiladi (umumiy):** satrdagi unli harflar sonini sanaydi (`strchr` yordamida).

```c
/* unli.c - satrdagi unli harflarni sanash */
#include <stdio.h>
#include <string.h>

static size_t unli_soni(const char *s)
{
    size_t n = 0;
    for (size_t i = 0; s[i] != '\0'; i++)       /* '\0' ni uchratguncha */
        if (strchr("aeiouAEIOU", s[i]))         /* s[i] shu belgilar ichida bormi? */
            n++;
    return n;
}

int main(void)
{
    printf("\"Salom Dunyo\" da %zu ta unli\n", unli_soni("Salom Dunyo"));
    return 0;
}
```

```console
$ gcc -Wall -Wextra unli.c -o unli
$ ./unli
"Salom Dunyo" da 4 ta unli
```

**Qadamlar:** sikl `s[0]`, `s[1]`, … `s[i] == '\0'` bo'lguncha ketadi (uzunlikni oldindan bilish shart emas). Har belgi uchun `strchr("aeiouAEIOU", belgi)` — "bu belgi unlilar
ro'yxatida bormi?" (topilsa manzil — rost; topilmasa `NULL` — yolg'on). `"Salom Dunyo"`: `a`, `o`, `u`, `o` = **4**.

Ko'rsatkich bilan (7-bobdan keyin shunday yozasiz): `for (const char *p = s; *p; p++) ...`.

## 6.7. `<ctype.h>` — belgilarni tekshirish

`isdigit(c)` (raqammi?), `isalpha(c)` (harfmi?), `isalnum(c)`, `isspace(c)` (bo'shliqmi?), `isupper(c)`, `tolower(c)`, `toupper(c)`.

**Tuzoq:** ularga faqat `unsigned char` qiymati yoki `EOF` berish mumkin. x86'da `char` ishorali: UTF-8 dagi `'é'` baytlari (0xC3 0xA9) manfiy son bo'lib qoladi → **UB**.
To'g'ri yozuv:

```c
if (isalpha((unsigned char)s[i]))
```

**Bu dastur nima qiladi (umumiy):** satrdagi belgilarni harf, raqam va boshqa turlarga ajratib sanaydi.

```c
/* ctype_misol.c - belgilarni turlarga ajratish */
#include <ctype.h>
#include <stdio.h>

int main(void)
{
    const char *s = "Salom, 2026-yil!";
    int harf = 0, raqam = 0, boshqa = 0;

    for (int i = 0; s[i] != '\0'; i++) {
        unsigned char c = (unsigned char)s[i];      /* manfiy bo'lmasligi uchun */
        if (isalpha(c))
            harf++;
        else if (isdigit(c))
            raqam++;
        else
            boshqa++;
    }
    printf("harf: %d, raqam: %d, boshqa: %d\n", harf, raqam, boshqa);
    return 0;
}
```

```console
$ gcc -Wall -Wextra ctype_misol.c -o ctype_misol
$ ./ctype_misol
harf: 8, raqam: 4, boshqa: 4
```

(10-mashq aynan shu `unsigned char` tuzog'ini tekshiradi.)

## 6.8. Satrlarni solishtirish va nusxalash — to'g'ri usullar

**Hayotdan misol: ikki kitob.** Qo'lingizda ikki nusxa "O'tkan kunlar" bor. Matni bir xilmi? Ha (`strcmp` = 0). Bu **bitta** kitobmi? Yo'q (`==` manzillarni solishtiradi — ular har xil).

```c
/* solishtir.c - == va strcmp farqi */
#include <stdio.h>
#include <string.h>

int main(void)
{
    char a[] = "exit";
    char b[] = "exit";                  /* matni bir xil, lekin ALOHIDA massiv */

    if (a == b)
        printf("a == b: teng\n");
    else
        printf("a == b: teng emas (manzillar farq qiladi)\n");

    if (strcmp(a, b) == 0)
        printf("strcmp: matnlar teng\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra solishtir.c -o solishtir # xato kutiladi
solishtir.c: In function ‘main’:
solishtir.c:10:11: warning: comparison between two arrays [-Warray-compare]
   10 |     if (a == b)
      |           ^~
solishtir.c:10:11: note: use ‘&a[0] == &b[0]’ to compare the addresses
$ ./solishtir
a == b: teng emas (manzillar farq qiladi)
strcmp: matnlar teng
```

Kompilyator ham ogohlantirdi (`comparison between two arrays`). Satrlarni **faqat `strcmp`** bilan solishtiring: `if (strcmp(buyruq, "exit") == 0)`.

### Xavfsiz nusxalash: `snprintf`

**Hayotdan misol: chelakka ko'p suv.** 10 litrlik chelakka 15 litr quysangiz, 5 litr polga to'kiladi. `strcpy` chelak o'lchamini **bilmaydi** — quyib ketaveradi.
`snprintf(buf, sizeof(buf), ...)` esa chelak o'lchamini biladi va ortig'ini quymaydi.

```c
/* snprintf_misol.c - xavfsiz nusxalash */
#include <stdio.h>

int main(void)
{
    char buf[16];                       /* 15 belgi + '\0' */
    /* volatile: kompilyator satr uzunligini oldindan ko'rib ogohlantirmasligi uchun
       (haqiqiy dasturda matn foydalanuvchidan keladi) */
    const char *volatile papka = "/home/foydalanuvchi";
    const char *volatile fayl = "hujjat.txt";

    int n = snprintf(buf, sizeof(buf), "%s/%s", papka, fayl);
    printf("buferda: \"%s\"\n", buf);
    printf("kerak edi: %d belgi, sig'di: %zu\n", n, sizeof(buf) - 1);
    if (n >= (int)sizeof(buf))
        printf("natija QISQARTIRILDI!\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra snprintf_misol.c -o snprintf_misol
$ ./snprintf_misol
buferda: "/home/foydalanu"
kerak edi: 30 belgi, sig'di: 15
natija QISQARTIRILDI!
```

`snprintf` qaytaradi: **to'liq natija uchun kerak bo'lgan** uzunlikni. Agar u `>= sizeof(buf)` bo'lsa — natija **qisqartirilgan**. Bufer to'lmadi (xotira buzilmadi),
va biz qisqartirilganini bilamiz. Bu "joy — n, lekin haqiqiy kerakli sonni qaytaraman" uslubi (09-mashq, BSD `strlcpy`) C'ning xavfsiz satr funksiyalari uchun standart.

## 6.9. Satrni o'qish (klaviaturadan)

**Bu dastur nima qiladi (umumiy):** klaviaturadan bitta qator o'qiydi (`fgets`), oxiridagi yangi qator belgisini olib tashlaydi va uzunligini chiqaradi.

```c
/* qator_oqish.c - klaviaturadan qator o'qish */
#include <stdio.h>
#include <string.h>

int main(void)
{
    char qator[256];
    if (fgets(qator, sizeof(qator), stdin)) {       /* ko'pi bilan 255 belgi + '\0' */
        qator[strcspn(qator, "\n")] = '\0';          /* oxiridagi '\n' ni olib tashlash */
        printf("o'qildi: \"%s\" (%zu belgi)\n", qator, strlen(qator));
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra qator_oqish.c -o qator_oqish
$ echo "salom dunyo" | ./qator_oqish
o'qildi: "salom dunyo" (11 belgi)
```

**Qadamlar:** `fgets(qator, sizeof(qator), stdin)` — "klaviatura (`stdin`)dan qator o'qi, lekin `qator` ga **ko'pi bilan** `sizeof(qator) - 1` belgi yoz" — **bufer hajmini biladi**.
`fgets` qator oxiridagi `'\n'` ni ham saqlaydi; `strcspn(qator, "\n")` — "birinchi `'\n'` indeksi" — shu joyga `'\0'` yozib, uni olib tashlaymiz. (Biz `echo ... |` bilan "klaviatura" o'rniga matn berdik.)

**Hech qachon `gets` ishlatmang** — u bufer hajmini bilmaydi va standartdan olib tashlangan. `scanf("%s", buf)` ham xuddi shunday xavfli (kenglik ko'rsatilmasa).

## 6.10. Satr ↔ son

**Bu dastur nima qiladi (umumiy):** satrni songa aylantirish: `strtol` (xatoni aniqlaydi) va `atoi` (aniqlay olmaydi) farqini ko'rsatadi.

```c
/* satr_son.c - satrni songa aylantirish */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char *end;
    long x = strtol("123", &end, 10);
    printf("strtol(\"123\"): %ld, tugadi: %s\n", x, *end == '\0' ? "to'liq" : "qoldiq bor");

    long y = strtol("12abc", &end, 10);
    printf("strtol(\"12abc\"): %ld, qoldiq: \"%s\"\n", y, end);

    printf("atoi(\"abc\") = %d (xato haqida xabar bermaydi!)\n", atoi("abc"));

    char buf[16];
    snprintf(buf, sizeof(buf), "%d", 42);       /* son -> satr */
    printf("son -> satr: \"%s\"\n", buf);
    return 0;
}
```

```console
$ gcc -Wall -Wextra satr_son.c -o satr_son
$ ./satr_son
strtol("123"): 123, tugadi: to'liq
strtol("12abc"): 12, qoldiq: "abc"
atoi("abc") = 0 (xato haqida xabar bermaydi!)
son -> satr: "42"
```

**Kodda nimalar bor:**

- `strtol(satr, &end, 10)` — "satrni **10** lik sanoq tizimida songa aylantir; qayergacha o'qiganingni `end` ga yoz". `&end` — `end` ning **manzili** berilyapti, chunki funksiya uni o'zgartirishi kerak (5.4 dagi usul).
- `*end == '\0'` — "hamma belgi o'qildi". `"12abc"` da son 12, qoldiq `"abc"` — shuning uchun xato aniqlanadi.
- `atoi("abc")` jim `0` qaytaradi — "xato" va "haqiqiy 0" ni ajratib bo'lmaydi. Shuning uchun `strtol` yaxshiroq.
- `snprintf(buf, sizeof(buf), "%d", 42)` — son → satr.

11 va 12-mashqlarda bularni **o'zingiz** yozasiz — `printf` ning yuragi (`kernel/lib/kprintf.c`) aynan shunday ishlaydi.

## Hayotdan misol va to'liq dastur

**SMS va haftalik ob-havo.** Bu dasturda: satr (SMS), bufer o'lchami, satr ichida qidirish va ikki o'lchamli massiv.

```c
/* sms.c - satrlar, bufer o'lchami va ikki o'lchamli massiv */
#include <stdio.h>
#include <string.h>

int main(void)
{
    /* SMS: 30 belgili ekran (29 harf + '\0') */
    char sms[30];
    /* volatile: haqiqiy dasturda ism va matn foydalanuvchidan keladi, oldindan noma'lum.
     * Usiz kompilyator satr sig'masligini oldindan ko'rib, ogohlantirish beradi. */
    const char *volatile ism = "Dilnoza";
    const char *volatile matn = "Ertaga soat 9 da uchrashamiz.";
    int n = snprintf(sms, sizeof(sms), "Salom %s! %s", ism, matn);
    printf("Ekranda: \"%s\"\n", sms);
    printf("Xabar %d belgi, ekranga %zu tasi sig'di\n", n, strlen(sms));

    /* Satr ichida qidirish */
    const char *xabar = "Kod: 4821. Hech kimga aytmang!";
    const char *topildi = strstr(xabar, "Kod: ");
    if (topildi)
        printf("Tasdiqlash kodi: %.4s\n", topildi + 5);

    /* Hafta x (ertalab, kechqurun) - ikki o'lchamli massiv */
    const char *kunlar[7] = { "Du", "Se", "Ch", "Pa", "Ju", "Sh", "Ya" };
    int harorat[7][2] = { { 12, 20 }, { 10, 18 }, { 14, 25 }, { 15, 27 },
                          { 11, 19 }, { 9, 16 }, { 13, 22 } };
    int eng_issiq = 0;
    for (int kun = 0; kun < 7; kun++) {
        printf("%s: %2d..%2d  ", kunlar[kun], harorat[kun][0], harorat[kun][1]);
        if (harorat[kun][1] > harorat[eng_issiq][1])
            eng_issiq = kun;
    }
    printf("\nEng issiq kun: %s (%d gradus)\n", kunlar[eng_issiq], harorat[eng_issiq][1]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra sms.c -o sms
$ ./sms
Ekranda: "Salom Dilnoza! Ertaga soat 9 "
Xabar 44 belgi, ekranga 29 tasi sig'di
Tasdiqlash kodi: 4821
Du: 12..20  Se: 10..18  Ch: 14..25  Pa: 15..27  Ju: 11..19  Sh:  9..16  Ya: 13..22  
Eng issiq kun: Pa (27 gradus)
```

**Kodda nimalar bor:**

| Nom | Tur | Nima uchun |
|---|---|---|
| `sms` | `char[30]` | 30 baytli bufer: 29 belgi + `'\0'`. SMS ekraniga sig'adigan matn |
| `ism`, `matn` | `const char *volatile` | matn manzillari. `volatile` — kompilyator oldindan hisoblab ogohlantirmasligi uchun (haqiqiy dasturda matn tashqaridan keladi) |
| `xabar` | `const char *` | qidiriladigan satr |
| `kunlar[7]` | 7 ta satr manzili | kun nomlari |
| `harorat[7][2]` | 7 qator × 2 ustun | har kun: `[0]` — ertalab, `[1]` — kechqurun |
| `eng_issiq` | `int` | hozirgacha eng issiq kechqurunning **indeksi** |

**Qadamlar:**

1. `snprintf` "Salom Dilnoza! Ertaga soat 9 da uchrashamiz." (44 belgi) ni 30 baytli buferga yozdi: faqat 29 tasi sig'di, qolgani **kesildi** (xotira buzilmadi). Qaytgan `n = 44` — kerak bo'lgan uzunlik.
2. `strstr(xabar, "Kod: ")` — "Kod: " ning o'rnini topadi; `topildi + 5` — undan 5 belgi keyin (kod boshi); `%.4s` — **faqat 4 belgi** chiqar → `4821`.
3. Sikl har kunning haroratini chiqaradi; `harorat[kun][1] > harorat[eng_issiq][1]` — joriy kun hozirgi rekorddan issiqmi? Ha → `eng_issiq = kun`. Oxirida `Pa` (27).

**Sinab ko'ring:** `char sms[30]` ni `char sms[100]` qiling. `harorat[kun][1]` ni `harorat[kun][2]` qilib, `-fsanitize=address` bilan yig'ing — sanitizer nima deydi (6.2)?

<!-- katta:boshi -->
## Katta loyiha: Ombor — 6-bosqich: massivlar va satrlar

**Oldingi bosqichdan:** har mahsulot uchun alohida o'zgaruvchi va hamma joyda `switch (id)`. Yangi mahsulot qo'shib bo'lmaydi: dastur **faqat 3 ta** mahsulotni biladi.

### Bu bosqichda nima qilamiz

Ma'lumotni **massivlarda** saqlaymiz: `nom[i]`, `narx[i]`, `soni[i]` — **parallel massivlar** ("i-mahsulot" ma'lumoti uchala massivning `i`-katagida). Natijada:

- mahsulot qo'shish — **runtime da** (dastur ishlayotganda) — menyudan mumkin;
- hamma `switch` lar `for` sikliga aylanadi;
- mahsulotni **nomi** bilan topamiz (`"sut"`), raqam bilan emas — foydalanuvchi uchun qulayroq.

**Satrlar:** C da matn — `char` massivi, oxirida **`'\0'`** (nol bayt) bilan tugaydi (6-bob). Shuning uchun:
- nomlar `char nom[MAKS][NOM_UZ]` — "MAKS ta satr, har biri NOM_UZ belgigacha";
- satrni `==` bilan solishtirib **bo'lmaydi** (u manzillarni solishtiradi) — `strcmp` ishlatamiz;
- satrni `=` bilan ko'chirib **bo'lmaydi** — `snprintf`/`strcpy` ishlatamiz;
- uzun nom massivdan **tashqariga yozib yubormasligi** uchun `scanf("%23s", ...)` va `snprintf` — **chegarali** funksiyalar (bufer to'lishidan himoya, 6-bob, 13-bob).

```c
/* ombor.c - Ombor, 6-bosqich: massivlar va satrlar - istalgancha (MAKS gacha) mahsulot, nom bo'yicha qidirish */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ombor_chop.h"

#define MAKS 8                                  /* eng ko'pi bilan nechta mahsulot */
#define NOM_UZ 24                               /* nom uchun joy (oxirgi '\0' bilan) */

enum { OK = 0, TOLA = -1, NOM_BAND = -2, TOPILMADI = -3, NOTOGRI_MIQDOR = -4, YETARLI_EMAS = -5 };

/* "parallel massivlar": i-mahsulotning ma'lumoti nom[i], narx[i], soni[i] da */
static char nom[MAKS][NOM_UZ];
static long narx[MAKS];
static uint16_t soni[MAKS];
static int n;                                   /* hozir nechta mahsulot bor */

/* nom bo'yicha qidiradi: indeksni yoki -1 ni qaytaradi */
static int topish(const char *qidirilgan)
{
    for (int i = 0; i < n; i++)
        if (strcmp(nom[i], qidirilgan) == 0)    /* satrlarni == bilan emas, strcmp bilan solishtiramiz */
            return i;
    return -1;
}

static int qosh(const char *yangi_nom, long yangi_narx, uint16_t yangi_soni)
{
    if (n == MAKS)
        return TOLA;
    if (topish(yangi_nom) >= 0)
        return NOM_BAND;
    snprintf(nom[n], NOM_UZ, "%s", yangi_nom);  /* uzun bo'lsa qirqiladi, '\0' doim qo'yiladi */
    narx[n] = yangi_narx;
    soni[n] = yangi_soni;
    n++;
    return OK;
}

static int sot(int i, int miqdor)
{
    if (miqdor <= 0)
        return NOTOGRI_MIQDOR;
    if (miqdor > soni[i])
        return YETARLI_EMAS;
    soni[i] -= miqdor;
    return OK;
}

static void royxat(void)
{
    long jami = 0;
    chop_sarlavha();
    for (int i = 0; i < n; i++) {
        chop_qator(nom[i], narx[i], soni[i]);
        jami += narx[i] * soni[i];
    }
    chop_jami(jami, 12);
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
    if (scanf("%23s %ld %d", s, &so_m, &miqdor) != 3)   /* %23s: 23 belgidan ko'pini o'qimaydi */
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
    int r = i < 0 ? TOPILMADI : sot(i, miqdor);
    if (r == OK)
        printf("  Sotildi: %d dona %s. Qoldi: %u dona\n", miqdor, nom[i], soni[i]);
    else
        printf("  XATO: %s\n", xato_matni(r));
}

static void qidirish_menyusi(void)
{
    char s[NOM_UZ];
    printf("Nom boshlanishi?\n");
    if (scanf("%23s", s) != 1)
        return;
    int topildi = 0;
    for (int i = 0; i < n; i++)
        if (strncmp(nom[i], s, strlen(s)) == 0) {       /* faqat boshidagi belgilarni solishtiramiz */
            printf("  %s (%u dona)\n", nom[i], soni[i]);
            topildi++;
        }
    if (!topildi)
        printf("  hech narsa topilmadi\n");
}

int main(void)
{
    qosh("non", 400000, 120);
    qosh("sut", 1200000, 45);
    qosh("guruch", 1800000, 8);

    int tanlov;
    while (printf("\n1) ro'yxat  2) sotish  3) qo'shish  4) qidirish  0) chiqish\nTanlov:\n"),
           scanf("%d", &tanlov) == 1 && tanlov != 0) {
        switch (tanlov) {
        case 1: royxat(); break;
        case 2: sotish_menyusi(); break;
        case 3: qoshish_menyusi(); break;
        case 4: qidirish_menyusi(); break;
        default: printf("  XATO: menyuda bunday band yo'q\n");
        }
    }
    printf("\nXayr!\n");
    return 0;
}
```

```console
$ cd katta_loyiha/ombor/06_massivlar
$ printf '3\nshakar 15000 60\n3\nnon 5000 10\n2\nsut 5\n2\nolma 1\n4\nsh\n4\nxyz\n2\nguruch 100\n1\n0\n' > kirish.txt
$ gcc -Wall -Wextra ombor.c ombor_chop.c -o ombor
$ ./ombor < kirish.txt

1) ro'yxat  2) sotish  3) qo'shish  4) qidirish  0) chiqish
Tanlov:
Nom, narx (so'mda) va soni?
  Qo'shildi: shakar

1) ro'yxat  2) sotish  3) qo'shish  4) qidirish  0) chiqish
Tanlov:
Nom, narx (so'mda) va soni?
  XATO: bunday nomli mahsulot allaqachon bor

1) ro'yxat  2) sotish  3) qo'shish  4) qidirish  0) chiqish
Tanlov:
Mahsulot nomi va necha dona?
  Sotildi: 5 dona sut. Qoldi: 40 dona

1) ro'yxat  2) sotish  3) qo'shish  4) qidirish  0) chiqish
Tanlov:
Mahsulot nomi va necha dona?
  XATO: bunday mahsulot topilmadi

1) ro'yxat  2) sotish  3) qo'shish  4) qidirish  0) chiqish
Tanlov:
Nom boshlanishi?
  shakar (60 dona)

1) ro'yxat  2) sotish  3) qo'shish  4) qidirish  0) chiqish
Tanlov:
Nom boshlanishi?
  hech narsa topilmadi

1) ro'yxat  2) sotish  3) qo'shish  4) qidirish  0) chiqish
Tanlov:
Mahsulot nomi va necha dona?
  XATO: omborda yetarli emas

1) ro'yxat  2) sotish  3) qo'shish  4) qidirish  0) chiqish
Tanlov:
================ OMBOR ================
Mahsulot         Narx   Soni          Summa
---------------------------------------
non           4000.00    120      480000.00
sut          12000.00     40      480000.00
guruch       18000.00      8      144000.00
shakar       15000.00     60      900000.00
---------------------------------------
Jami qiymat:                2004000.00
QQS stavkasi:               12%
QQS summasi:                240480.00

1) ro'yxat  2) sotish  3) qo'shish  4) qidirish  0) chiqish
Tanlov:

Xayr!
```

**Kiritilgan ketma-ketlik:**

| Kirish | Natija |
|---|---|
| `3`, `shakar 15000 60` | yangi mahsulot **qo'shildi** (3 ta o'rniga 4 ta bo'ldi) |
| `3`, `non 5000 10` | **XATO:** "non" nomi allaqachon bor (nom takrorlanmaydi) |
| `2`, `sut 5` | 5 dona sut sotildi → qoldi 40 |
| `2`, `olma 1` | **XATO:** bunday mahsulot topilmadi |
| `4`, `sh` | nomi `sh` bilan boshlanuvchi: **shakar** |
| `4`, `xyz` | hech narsa topilmadi |
| `2`, `guruch 100` | **XATO:** omborda yetarli emas (8 dona) |
| `1` | ro'yxat: 4 mahsulot, jami 2004000.00 |

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `static char nom[MAKS][NOM_UZ];` | 8 ta satr uchun joy, har biri 24 baytgacha (23 belgi + `'\0'`) |
| `static int n;` | hozir nechta katak band: massivning "uzunligi" (C massivning uzunligini o'zi bilmaydi!) |
| `topish(const char *qidirilgan)` | `for` bilan hamma nomni `strcmp` bilan solishtiradi; topilgan **indeksni** yoki `-1` qaytaradi |
| `snprintf(nom[n], NOM_UZ, "%s", yangi_nom);` | nomni **chegarasi bilan** ko'chiradi: uzun bo'lsa qirqadi, `'\0'` ni doim qo'yadi |
| `scanf("%23s", s)` | ko'pi bilan 23 belgi o'qiydi — `s[24]` dan oshib ketmaydi |
| `strncmp(nom[i], s, strlen(s))` | faqat **boshidagi** `strlen(s)` ta belgini solishtiradi — "boshlanishi bo'yicha qidirish" |
| `so_m * 100` | foydalanuvchi so'mda kiritadi, biz tiyinda saqlaymiz |

**Muhim xavf:** `n == MAKS` bo'lganda yangi mahsulot **qo'shilmaydi** (`TOLA` xatosi) — massiv o'lchami **doimiy**. Agar bu tekshiruv bo'lmasa, `nom[n]` massivdan tashqariga yozilar edi (**bufer to'lishi**). 8-bobda massivni dinamik qilib, bu cheklovni yo'qotamiz.

> **Eslab qoling:** C massivi o'z uzunligini bilmaydi — `n` ni o'zingiz saqlang. Satr = `char[]` + `'\0'`. Solishtirish — `strcmp`, ko'chirish — `snprintf`, o'qish — `%23s` (**chegara bilan**). Massiv chegarasini **doim** tekshiring.

**O'zingiz qo'shing (yechimsiz):**

1. `MAKS` ni 3 ga tushirib yig'ing va 4-mahsulotni qo'shib ko'ring: dastur nima deydi?
2. Nomlari bo'sh joy bilan yozilgan mahsulotlar ("olma sharbati") bilan nima bo'ladi? `scanf("%23s")` nega buni buzadi? (Maslahat: 6-bobdagi `fgets`.)
3. `qidirish_menyusi` ga katta-kichik harfni farqlamaydigan qidiruvni qo'shing. (Maslahat: `strncasecmp` yoki har belgini `tolower` bilan o'tkazish.)
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. Massiv — bir xil turdagi **ketma-ket** qutilar; indeks **0 dan**; uzunlik **qat'iy**; `a[i]` manzili = boshi + `i * hajm`.
2. C chegarani **tekshirmaydi** (`a[5]`, 5 elementli massivda — xato). Funksiyaga massivni **uzunligi bilan** bering; `-fsanitize=address` bilan sinang.
3. Satr = `'\0'` bilan tugaydigan `char` massivi: `"salom"` = 6 bayt (`sizeof`), `strlen` = 5.
4. Satrni `==` bilan emas, **`strcmp`** bilan solishtiring; nusxalash — **`snprintf`** (`strcpy` emas); o'qish — `fgets` (`gets` emas).
5. Literal — `const char *p = "..."` (o'zgartirib bo'lmaydi); o'zgartiriladigan satr — `char s[] = "..."`.

## Savol-javob

**`char s[10] = "salom";` — qolgan 4 bayt nima?**
Nol. Boshlang'ich qiymat qisman berilsa, qolgani 0 bilan to'ldiriladi.

**`char s[5] = "salom";` — xato bo'ladimi?**
C'da **yo'q** (C++'da xato): 5 ta harf sig'adi, lekin `'\0'` uchun joy yo'q — `s` satr emas, oddiy massiv bo'lib qoladi. `strlen(s)` xotira bo'ylab yuguradi. Klassik jim xato.

**UTF-8 (o'zbekcha `o'`, `g'`, kirill) qanday saqlanadi?**
Lotin o'zbek alifbosi ASCII'da (apostrof ham). Kirill yoki `é` — bir belgi 2–4 bayt. `strlen` **baytlarni** sanaydi, belgilarni emas: `strlen("дом")` = 6.
MyOS terminal emulyatori UTF-8 ni qanday dekodlashini `kernel/drivers/vt.c` da ko'rishingiz mumkin.

**Nega C massivga `.length` bermaydi?**
Massiv — shunchaki xotira bo'lagi; hech qayerda uzunlik yozilmagan (Python ro'yxati esa uzunligini ichida saqlaydi). Uzunlikni **siz** saqlaysiz va uzatasiz.

## O'zingizni tekshiring

1. `int a[10];` — `a[10]` ga yozish nima?
2. `char s[] = "abc";` — `sizeof(s)` va `strlen(s)`?
3. `if (s == "exit")` nima uchun deyarli doim yolg'on?
4. Funksiya ichida massiv uzunligini `sizeof` bilan bilsa bo'ladimi?
5. `strcpy` o'rniga nimani ishlatish kerak?
6. `int m[3][4];` — `m[2][1]` xotirada nechanchi (0 dan) o'rinda turadi?

<details><summary>Javoblar</summary>

1. Chegaradan tashqariga yozish — UB (indekslar 0..9).
2. 4 va 3.
3. Ikki manzil solishtiriladi (massiv va literal manzili), mazmun emas. `strcmp(s, "exit") == 0` kerak.
4. Yo'q — parametr ko'rsatkichga aylanadi; uzunlikni alohida uzating.
5. `snprintf(d, sizeof(d), "%s", s)` yoki o'z `strlcpy` (09-mashq).
6. `2 * 4 + 1 = 9`.
</details>

## Mashq

### Isitish: ballar va shahar nomi ★☆☆ — eng osoni, avval shuni qiling

Faqat 0–6-boblar kerak (massiv, satr, `strlen`).
Skeletni `isitish.c` ga **qo'lda** yozing (ko'chirmang), izohlarni o'qing va `TODO` joylarini to'ldiring.
"Namuna" qismlar tayyor — qolganini qanday yozishni ko'rsatadi. Skelet hozir ham ogohlantirishsiz yig'iladi:
har `TODO` dan keyin yig'ib, ishga tushirib boring.

```c
/* isitish.c - 6-bob, isitish: ballar va shahar nomi. Ko'rsatkich arifmetikasi kerak emas - faqat indekslar. */
#include <stdio.h>
#include <string.h>

int main(void)
{
    int ballar[] = {72, 95, 58, 88, 64};
    size_t n = sizeof(ballar) / sizeof(ballar[0]);  /* butun massiv baytlari / bitta element = 5 (6.1) */
    int yigindi = 0;
    int eng = ballar[0];                /* "eng katta" ni 0 dan emas, birinchi elementdan boshlaymiz */

    /* 1) TODO: bitta sikl, i = 0 .. n-1: yigindi += ballar[i]; ballar[i] > eng bo'lsa - eng = ballar[i]. */

    printf("%zu ta ball, yig'indi %d, o'rtacha %d, eng katta %d\n", n, yigindi, yigindi / (int)n, eng);

    char ism[16] = "Toshkent";          /* 8 harf + '\0' = 9 bayt band, 16 ta joy bor (6.4) */
    size_t uz = strlen(ism);            /* '\0' gacha bo'lgan harflar soni: 8 */
    int unlilar = 0;

    /* 2) TODO: ism[0] .. ism[uz - 1] ni aylanib, 'a', 'e', 'i', 'o', 'u' larni sanang (|| bilan, 3.3). */

    printf("\"%s\": %zu harf, %d unli, oxirgi harf '%c'\n", ism, uz, unlilar, ism[uz - 1]);

    /* 3) TODO: printf("teskari: "); keyin harflarni oxiridan %c bilan, oxirida printf("\n").
     *    DIQQAT: for (size_t i = uz - 1; i >= 0; i--) - CHEKSIZ sikl: size_t hech qachon < 0 bo'lmaydi,
     *    0 dan keyin eng katta songa aylanadi. To'g'ri: for (size_t i = uz; i > 0; i--) va ism[i - 1].
     *    Natija: teskari: tnekhsoT */
    return 0;
}
```

**Kutilgan natija** (`darslik/loyihalar/06_anagram/isitish.txt`):

```text
5 ta ball, yig'indi 377, o'rtacha 75, eng katta 95
"Toshkent": 8 harf, 2 unli, oxirgi harf 't'
teskari: tnekhsoT
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined isitish.c -o isitish
$ ./isitish | diff - ~/C_loyha/darslik/loyihalar/06_anagram/isitish.txt && echo "TO'G'RI"
TO'G'RI
```

### Keyingi mashqlar

- **05** — massivlar (4-bobda qoldirilgan edi). **07** — chiqish parametrlari (5-bobda qoldirilgan edi).
- **08** — `strlen`, `strcmp` ni o'zingiz yozish. `strchr` ko'rsatkich qaytaradi (`s + i`, 7.4) — uni 7-bobdan keyin qo'shing.
- **09** — xavfsiz nusxalash. **10** — teskari satr va palindrom (indekslar bilan ham yechiladi).
- **11** — son → satr. **12** — satr → son.

<!-- loyiha:boshi -->
## Loyiha: ovoz berish natijalari

**Maqsad:** massivni **hisoblagich** sifatida ishlatish — C dasturchining eng ko'p qo'llaydigan hiyla-si.
**Bobdan ishlatiladi:** massiv, indeks bo'yicha murojaat, ikki o'lchamli `char` massivi (nomlar), `sizeof`.

**Talab:** 15 ta ovoz berilgan, har biri nomzod raqami (0..3). Har nomzod nechta ovoz olganini hisoblang,
gistogramma chizing va g'olibni aniqlang.
**Ma'lumotlar:** `nomlar[4][8]` — ismlar; `ovozlar[]` — berilgan ovozlar; `hisob[4]` — hisoblagichlar.
**Asosiy g'oya:** ovoz raqamining o'zi `hisob` massivining **indeksi**: `hisob[ovoz]++`. Sikl ichida `if` shart emas.

```c
/* sorov.c - ovoz berish natijalari */
#include <stdio.h>

int main(void)
{
    char nomlar[4][8] = { "Ali", "Vali", "Zarina", "Madina" };
    int ovozlar[] = { 2, 0, 1, 2, 2, 3, 0, 2, 1, 3, 2, 2, 0, 3, 2 };
    int n = sizeof(ovozlar) / sizeof(ovozlar[0]);   /* massiv e'lon qilingan joyda ishlaydi (6.1) */
    int hisob[4] = { 0 };                           /* hammasi nol */

    for (int i = 0; i < n; i++)
        hisob[ovozlar[i]]++;                        /* ovoz raqami = indeks */

    printf("Jami ovoz: %d\n", n);
    int golib = 0;
    for (int k = 0; k < 4; k++) {
        printf("%-7s |", nomlar[k]);
        for (int j = 0; j < hisob[k]; j++)
            putchar('#');
        printf(" %d\n", hisob[k]);
        if (hisob[k] > hisob[golib])
            golib = k;
    }
    printf("G'olib: %s (%d ovoz, %d%%)\n", nomlar[golib], hisob[golib], hisob[golib] * 100 / n);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined sorov.c -o sorov
$ ./sorov
Jami ovoz: 15
Ali     |### 3
Vali    |## 2
Zarina  |####### 7
Madina  |### 3
G'olib: Zarina (7 ovoz, 46%)
```

**Kengaytiring:** `ovozlar` ga `7` ni qo'shing (mavjud bo'lmagan nomzod) va sanitizer bilan ishga tushiring — u
`hisob[7]` chegaradan chiqishini ushlaydi (6.2). Haqiqiy dasturda `ovoz < 0 || ovoz > 3` ni oldindan tekshirasiz.

## Mustaqil loyiha: palindrom, anagram va so'zlar ★★☆

**Vazifa:** uchta satr funksiyasini yozing. Fayl: `satrlar.c`. `<string.h>` va `<ctype.h>` dan foydalanishingiz mumkin.

1. `int palindrom_mi(const char s[])` — harflarni katta-kichikligiga **qaramay** o'qing; **harf bo'lmagan**
   belgilarni (probel, tinish belgilari) e'tiborga olmang. `"A man a plan a canal Panama"` — palindrom.
2. `int anagram_mi(const char a[], const char b[])` — ikki satr bir xil harflardan iborat (tartibi boshqa),
   katta-kichik va harf bo'lmagan belgilarga qaramay. Massiv `int soni[26]` bilan hisoblang.
3. `int sozlar_soni(const char s[])` — probel bilan ajratilgan so'zlar soni. Ketma-ket probellar,
   boshidagi va oxiridagi probellar so'z bo'lib sanalmaydi. Belgi ajratgichi faqat probel.

`main` ichida sinov ma'lumotlari sizniki (quyidagi natijaga qarab yozing) va aynan shunday chiqarsin:

**Kutilgan natija** (`darslik/loyihalar/06_anagram/kutilgan.txt`):

```text
Palindrom tekshiruvi:
  "Level" -> ha
  "A man a plan a canal Panama" -> ha
  "Salom" -> yo'q
  "Was it a car or a cat I saw" -> ha
  "abcd" -> yo'q
Anagram tekshiruvi:
  "listen" va "silent" -> ha
  "Dormitory" va "dirty room" -> ha
  "hello" va "world" -> yo'q
  "aab" va "abb" -> yo'q
  "Astronomer" va "Moon starer" -> ha
So'zlar soni:
  "  salom   dunyo  bu  C  " -> 4
  "" -> 0
  "bitta" -> 1
  "bir, ikki; uch" -> 3
```

Sinov satrlari, tartib bilan: palindrom uchun `"Level"`, `"A man a plan a canal Panama"`, `"Salom"`,
`"Was it a car or a cat I saw"`, `"abcd"`; anagram juftlari `listen/silent`, `Dormitory/dirty room`, `hello/world`,
`aab/abb`, `Astronomer/Moon starer`; so'zlar uchun `"  salom   dunyo  bu  C  "`, `""` (bo'sh satr), `"bitta"`,
`"bir, ikki; uch"`.

**Maslahat** (yechim emas):
- `isalpha`, `tolower` ga `(unsigned char)` cast bering — 6.7-bo'lim, nega?
- Palindrom: ikki indeks (`i` boshdan, `j` oxirdan), ikkalasi ham harf bo'lmagan belgini o'tkazib yuboradi, `i < j` bo'lguncha.
- Anagram: birinchi satr harflari uchun `soni[h - 'a']++`, ikkinchisi uchun `--`. Oxirida hammasi nolmi?
- So'zlar: "so'z ichidamanmi?" degan bayroq (0/1). Probeldan harfga o'tgan joyda sanagich oshadi.
- Bo'sh satr `""` uchun `strlen(s) - 1` — `size_t` bilan nima bo'ladi? (2.4-bo'lim: unsigned toshishi!)

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined satrlar.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/06_anagram/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [7-bob. Ko'rsatkichlar](07-korsatkichlar.md)
