# 5-bob. Funksiyalar

> **Bu bobda nima o'rganasiz:** funksiya nima va nega kerakligini; uni qanday yozish va chaqirishni; `void` so'zining hamma ma'nolarini;
> argumentlar nega **nusxa** bo'lib uzatilishini; `static` funksiyalarni; stek va rekursiya nima ekanini.
> **Oldindan nima kerak:** 1–4-boblar.   **Vaqt:** 5–6 soat.
> Mashqlar: isitish (bob oxirida), 03, 06. 07 — 6-bobdan keyin.

> **To'liq ishlaydigan misol:** [misollar/05_funksiyalar.c](misollar/05_funksiyalar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Dastur o'sgan sari bir xil kodni ko'p joyda yozasiz: "narxni hisobla", "xatoni chiqar". **Funksiya** — shu kodga **nom berib**, bir marta yozib,
istalgancha chaqirish usuli.

**Hayotdan misol: kir yuvish mashinasi.** Ichiga kir va kukun solasiz (**argumentlar**), tugmani bosasiz (**chaqiruv**), toza kir olasiz
(**qaytish qiymati**). Mashina ichida nima bo'layotganini bilishingiz shart emas. Bitta mashinadan har kuni foydalanasiz — kodni ham bir marta
yozib, ko'p marta chaqirasiz.

| Kir yuvish mashinasi | C funksiyasi |
|---|---|
| mashinaning nomi | funksiya nomi (`hisobla`) |
| solinadigan narsalar (kir, kukun) | **parametrlar** (`narx`, `soni`) |
| toza kir | **qaytish qiymati** (`return jami;`) |
| tugmani bosish | **chaqiruv** (`hisobla(12000, 3)`) |
| mashinaning ichi | funksiya **tanasi** `{ ... }` |

## 5.1. Funksiyaning tuzilishi

**Bu dastur nima qiladi (umumiy):** birinchi funksiya: `kvadrat(int)` sonni o'ziga ko'paytirib qaytaradi, `main` uni chaqiradi — funksiya yozilishining qismlari.

```c
/* kvadrat.c - birinchi funksiyamiz */
#include <stdio.h>

/*  qaytish   nom       parametrlar
     turi      |         |           */
long kvadrat(int x)
{                                   /* tana boshi */
    return (long)x * x;             /* qaytarish */
}                                   /* tana oxiri - ; YO'Q */

int main(void)
{
    long r = kvadrat(7);            /* chaqiruv: x = 7 */
    printf("kvadrat(7) = %ld\n", r);
    printf("kvadrat(12) = %ld\n", kvadrat(12));
    return 0;
}
```

```console
$ gcc -Wall -Wextra kvadrat.c -o kvadrat
$ ./kvadrat
kvadrat(7) = 49
kvadrat(12) = 144
```

**Kodda nimalar bor:**

| Qism | Nima | Nega |
|---|---|---|
| `long` (nomdan oldin) | **qaytish turi**: funksiya `long` turidagi qiymat beradi | Kompilyator natija uchun qancha joy kerakligini bilishi kerak |
| `kvadrat` | funksiya **nomi** | Chaqirish uchun |
| `(int x)` | **parametr**: funksiya `int` turidagi bitta qiymat oladi va uni `x` deb ataydi | Har parametrning turi **majburiy** (Python'da yo'q) |
| `return (long)x * x;` | natijani hisoblab **qaytaradi**, funksiya shu yerda tugaydi | `(long)` — toshmaslik uchun (2-bob) |
| `kvadrat(7)` | **chaqiruv**: `7` — **argument** (haqiqiy qiymat), `x` ga nusxalanadi | |

**Parametr va argument farqi:** parametr — funksiya ta'rifidagi **nom** (`x`); argument — chaqirishda berilgan **qiymat** (`7`).

**Chaqiruv qanday ishlaydi (vaqt bo'yicha):**

1. `main` `kvadrat(7)` ni chaqiradi → `7` yangi quti `x` ga **nusxalanadi**.
2. Boshqaruv `kvadrat` ichiga o'tadi: `return (long)x * x` → `49`.
3. `49` chaqirilgan joyga **qaytadi**, `kvadrat` ning qutilari (`x`) yo'qoladi.
4. `main` davom etadi: `r = 49`.

Python bilan solishtirish:

```text
def kvadrat(x):            long kvadrat(int x)
    return x * x           {
                               return (long)x * x;
                           }
```

C'da qo'shimcha: **qaytish turi**, **har bir parametrning turi**, `{ }`, va `;` — tanadan keyin **yo'q**.

> **Eslab qoling:** funksiya = **qaytish turi + nom + (parametrlar) + { tana }**. Bitta funksiya — bitta aniq ish.

## 5.2. `void` — barcha ma'nolari

`void` — "hech narsa / turi yo'q". Kontekstga qarab ma'nosi o'zgaradi:

| Yozuv | Ma'nosi |
|---|---|
| `void f(int x)` | `f` **hech narsa qaytarmaydi** |
| `int f(void)` | `f` **hech narsa olmaydi** (parametr yo'q) |
| `void *p` | `p` — **turi noma'lum** xotira manzili (7-bob); `malloc` shuni qaytaradi |
| `(void)x;` | "bu qiymatni **ataylab** ishlatmayapman" (ogohlantirishni o'chiradi) |

**Bu dastur nima qiladi (umumiy):** `void` ning ikki ma'nosini ko'rsatadi: "hech narsa qaytarmaydi" va "argument olmaydi".

```c
/* void_misol.c - void ning ikki ma'nosi */
#include <stdio.h>

void salom(void)                    /* 1-void: hech narsa qaytarmaydi; 2-void: argument olmaydi */
{
    printf("salom\n");
    return;                         /* qiymatsiz return - ixtiyoriy, funksiya oxirida o'zi qaytadi */
}

static void ishlatilmaydi(int n)
{
    (void)n;                        /* "n ni bilaman, hozircha ishlatmayman" */
}

int main(void)
{
    salom();
    ishlatilmaydi(5);
    (void)printf("printf ning natijasini ataylab tashladik\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra void_misol.c -o void_misol
$ ./void_misol
salom
printf ning natijasini ataylab tashladik
```

**Savol: `void` qaytaradigan funksiya natijasini ishlatsam?** Qiymat yo'q — olib bo'lmaydi:

**Bu dastur nima qiladi (umumiy):** qiymat qaytarmaydigan (`void`) funksiya natijasini o'zgaruvchiga yozishga urinish — ataylab xatoli.

```c
/* void_xato.c - void natijani ishlatish */
void salom(void) { }

int main(void)
{
    int x = salom();                /* xato! */
    return x;
}
```

```console
$ gcc -Wall -Wextra void_xato.c -o void_xato # xato kutiladi
void_xato.c: In function ‘main’:
void_xato.c:6:13: error: void value not ignored as it ought to be
    6 |     int x = salom();                /* xato! */
      |             ^~~~~
```

**Savol: mashqlardagi `(void)n;` nima uchun?** Bo'sh funksiyada (`TODO`) parametr ishlatilmaydi va `-Wextra` "unused parameter" ogohlantiradi,
`-Werror` esa uni xatoga aylantiradi. `(void)n;` — "bilaman, hozircha ishlatmayapman" deb kompilyatorga aytish. Yechimni yozganingizda o'chirasiz.
MyOS'da ham bor: `pipe_read` dagi `(void)off;` — pipe'da fayl pozitsiyasi ma'nosiz, lekin VFS hamma `read` funksiyalariga bir xil parametrlar beradi.

## 5.3. E'lon (prototip) va ta'rif

1-bobda e'lon va ta'rifni o'rgandingiz. Bir faylning ichida ham shu qoida: kompilyator faylni **yuqoridan pastga bir marta** o'qiydi.
`main` ichida `kvadrat` ni uchratganda u haqida **allaqachon** bilishi kerak. Ikki yo'l bor: ta'rifni yuqoriga yozish yoki **prototip** (tanasiz e'lon) yozish.

**Bu dastur nima qiladi (umumiy):** funksiya prototipi (e'lon) `main` dan oldin, ta'rifi esa keyinroq turishini ko'rsatadi.

```c
/* prototip.c - prototip va ta'rif */
#include <stdio.h>

long kvadrat(int x);                /* PROTOTIP: shunday funksiya bor (tanasi keyinroq) */

int main(void)
{
    printf("%ld\n", kvadrat(12));   /* kompilyator prototipdan turlarni biladi */
    return 0;
}

long kvadrat(int x)                 /* TA'RIF */
{
    return (long)x * x;
}
```

```console
$ gcc -Wall -Wextra prototip.c -o prototip
$ ./prototip
144
```

Prototip bermaydigan narsa yo'q: argumentlar soni va turi **tekshiriladi**; kerak bo'lsa avtomatik aylantiriladi (`kvadrat(3.7)` → `double` `int` ga, ogohlantirish bilan).
Boshqa fayldagi funksiyalar uchun prototiplar `.h` faylda turadi (1-bob).

Prototipsiz xato ko'rinishi (ta'rif `main` dan **pastda** va prototip yo'q):

**Bu dastur nima qiladi (umumiy):** prototipsiz variant: funksiya `main` dan keyin ta'riflangan va oldin e'lon qilinmagan — xato hosil qiladi.

```c
/* prototipsiz.c - e'lon yo'q */
#include <stdio.h>

int main(void)
{
    printf("%ld\n", kvadrat(12));
    return 0;
}

long kvadrat(int x)
{
    return (long)x * x;
}
```

```console
$ gcc -Wall -Wextra prototipsiz.c -o prototipsiz # xato kutiladi
prototipsiz.c: In function ‘main’:
prototipsiz.c:6:21: warning: implicit declaration of function ‘kvadrat’ [-Wimplicit-function-declaration]
    6 |     printf("%ld\n", kvadrat(12));
      |                     ^~~~~~~
prototipsiz.c:6:15: warning: format ‘%ld’ expects argument of type ‘long int’, but argument 2 has type ‘int’ [-Wformat=]
    6 |     printf("%ld\n", kvadrat(12));
      |             ~~^     ~~~~~~~~~~~
      |               |     |
      |               |     int
      |               long int
      |             %d
prototipsiz.c: At top level:
prototipsiz.c:10:6: error: conflicting types for ‘kvadrat’; have ‘long int(int)’
   10 | long kvadrat(int x)
      |      ^~~~~~~
prototipsiz.c:6:21: note: previous implicit declaration of ‘kvadrat’ with type ‘int()’
    6 |     printf("%ld\n", kvadrat(12));
      |                     ^~~~~~~
```

Kompilyator `main` ichida `kvadrat` ni ko'rganda uni hali bilmasdi (`implicit declaration`); keyin ta'rifni ko'rib, "bu avval taxmin qilganimdan farq qiladi" deb xato berdi (`conflicting types`).

> **Eslab qoling:** funksiyani **ishlatishdan oldin** u e'lon qilingan bo'lishi kerak: yo ta'rif yuqorida, yo prototip.

## 5.4. Argumentlar NUSXA sifatida uzatiladi

**Hayotdan misol: hujjat nusxasi.** Idoraga pasportingizning **ksero nusxasini** berasiz. Ular nusxaga nima yozsa ham, sizning pasportingiz o'zgarmaydi.
C funksiyaga oddiy o'zgaruvchi berganda ham faqat **nusxa** beriladi.

```c
/* nusxa.c - funksiya nusxani o'zgartiradi, asl qiymat emas */
#include <stdio.h>

static void oshir_nusxa(int x)
{
    x = x + 1;                      /* faqat NUSXA o'zgaradi */
    printf("  funksiya ichida x = %d\n", x);
}

static void oshir_asl(int *x)       /* x - int ning MANZILI */
{
    *x = *x + 1;                    /* shu manzildagi qiymatni o'zgartirish */
}

int main(void)
{
    int a = 5;
    oshir_nusxa(a);
    printf("nusxa bilan: a = %d (o'zgarmadi!)\n", a);

    oshir_asl(&a);                  /* a ning manzilini berish */
    printf("manzil bilan: a = %d (o'zgardi)\n", a);
    return 0;
}
```

```console
$ gcc -Wall -Wextra nusxa.c -o nusxa
$ ./nusxa
  funksiya ichida x = 6
nusxa bilan: a = 5 (o'zgarmadi!)
manzil bilan: a = 6 (o'zgardi)
```

**Qadam-baqadam (xotira rasmi):**

```text
oshir_nusxa(a) chaqirilganda:
    main:   a [ 5 ]                    <- asl quti (o'zgarmaydi)
    funksiya: x [ 5 ]  -> x = x+1 -> x [ 6 ]     <- ALOHIDA quti (nusxa); funksiya tugasa yo'qoladi

oshir_asl(&a) chaqirilganda:
    main:   a [ 5 ]  <---------+
    funksiya: x [ manzil_a ] --+      x - a ning manzilini saqlaydi; *x = a ning o'zi
              *x = *x + 1   ->   a [ 6 ]       <- asl quti o'zgardi
```

- `&a` — "`a` ning **manzili**" (7-bob). `int *x` — "`x` — `int` manzili saqlaydigan quti". `*x` — "shu manzilga **borib**, ichidagi qiymat".
- Funksiya chaqirilganda har bir argument **yangi o'zgaruvchiga nusxalanadi**. Chaqiruvchining o'zgaruvchisiga funksiya tega olmaydi,
  agar unga manzil bermasangiz. (Python'da ham `int` uchun shunday, lekin `list` uzatilsa — o'sha obyektning o'zi uzatiladi.)

Bu 7-bobning asosiy mavzusi va 06-mashqda amalda. Yadroda deyarli hamma funksiya shunday ishlaydi.

**Struktura ham nusxalanadi:** `void f(struct katta s)` — butun struktura (masalan 4 KB) stekka nusxalanadi. Shuning uchun katta strukturalar doim
ko'rsatkich bilan uzatiladi: `void f(const struct katta *s)`.

**Massiv esa nusxalanMAYdi:** `void f(int a[])` aslida `void f(int *a)` — birinchi elementning manzili uzatiladi (6-bob). Shuning uchun funksiya massivni
o'zgartira oladi va uning uzunligini **bilmaydi**.

> **Eslab qoling:** oddiy argument — **nusxa** (asl qiymat o'zgarmaydi). O'zgartirmoqchi bo'lsangiz — **manzil** bering (`&a`).

## 5.5. Qaytish qiymati va xato kodlari

**Hayotdan misol: kvitansiya.** Pul o'tkazdingiz — kvitansiya olasiz: "muvaffaqiyatli" yoki "xato: hisobda mablag' yo'q". Yadroda funksiyalar shunday:
**0 — muvaffaqiyat, manfiy son — xato kodi**. Kvitansiyani o'qimay tashlab yuborish — eng ko'p uchraydigan xato.

C'da exception yo'q. Xato haqida xabar berishning ikki asosiy usuli:

```text
/* 1) Manfiy son = xato (yadro va POSIX uslubi) */
int fayl_och(const char *yol);     /* >= 0: fd, < 0: -ENOENT, -EACCES ... */

/* 2) Natija chiqish parametrida, qaytish qiymati - holat */
int satr_songa(const char *s, long *natija);   /* 0 - OK, -1 - noto'g'ri, -2 - toshish */
```

Ikkinchi usulni kichik dasturda ko'ramiz — nolga bo'lishni **xavfsiz** qilamiz:

**Bu dastur nima qiladi (umumiy):** bo'lish funksiyasi xato kodini (0 yoki −1) qaytaradi, natijani esa ko'rsatkich orqali beradi — xatoni bildirish naqshi.

```c
/* bolish.c - xato kodi va chiqish parametri */
#include <stdio.h>

/* a / b ni natija ga yozadi. Qaytaradi: 0 - OK, -1 - nolga bo'lish */
static int bol(int a, int b, int *natija)
{
    if (b == 0)
        return -1;                  /* xato: natija ga tegmaymiz */
    *natija = a / b;
    return 0;
}

int main(void)
{
    int r = 0;

    if (bol(20, 4, &r) == 0)
        printf("20 / 4 = %d\n", r);

    if (bol(20, 0, &r) != 0)
        printf("20 / 0 -> xato: nolga bo'lib bo'lmaydi\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra bolish.c -o bolish
$ ./bolish
20 / 4 = 5
20 / 0 -> xato: nolga bo'lib bo'lmaydi
```

**Kodda nimalar bor:**

| Qator | Nima qiladi | Nega |
|---|---|---|
| `int bol(int a, int b, int *natija)` | uchta narsa oladi; **holat** (`int`) qaytaradi, natijani esa `natija` manzili orqali beradi | Bitta `return` da ham natija, ham xato kodini berib bo'lmaydi |
| `if (b == 0) return -1;` | nolga bo'lishdan **oldin** to'xtaydi | Aks holda dastur qulaydi (3.1) |
| `*natija = a / b;` | `natija` ko'rsatgan qutiga yozadi | Chaqiruvchi (`main`) o'z `r` ini beradi |
| `bol(20, 4, &r) == 0` | holatni **tekshirish** | Chaqiruvchi **doim** tekshirishi kerak |

Birinchi chaqiruvda `r = 5`; ikkinchisida funksiya `-1` qaytardi va `r` ga tegmadi.

MyOS yadrosida qoida: **manfiy = xato kodi** (`-ENOMEM`, `-EINVAL`, `-EFAULT`). Syscall natijasi user'ga shunday qaytadi, libc esa uni `errno` ga yozib
`-1` qaytaradi (`user/libc/syscall.h`: `__sysret`). 07 va 12-mashqlar shu uslubda.

**Nega Python'dagi kabi bir nechta qiymat qaytarib bo'lmaydi?** Qaytish qiymati bitta registrda (`rax`) qaytadi. Bir nechta natija uchun: chiqish parametrlari
(07-mashq) yoki struct qaytarish (`struct natija f(void)` — kichik struct ham registrlarda qaytadi).

> **Eslab qoling:** funksiya bitta qiymat qaytaradi. Ko'p natija kerak bo'lsa — **chiqish parametrlari** (`int *natija`). Xatoni **qaytish qiymatida** bering va **doim tekshiring**.

## 5.6. `static` — ikki xil ma'no

### `static` funksiya — faqat shu fayl uchun

```c
static int yordamchi(int x)     /* boshqa .c fayllar buni ko'rmaydi */
{
    return x * 2;
}
```

- Nomlar to'qnashuvining oldi olinadi: ikki faylda ikkita `static int yordamchi` — muammo emas.
- "Ichki" va "tashqi" funksiyalar aniq ajraladi: `.h` da e'lon qilinganlari — tashqi interfeys, `static` lar — ichki tafsilot.
- Kompilyator `static` funksiyani bemalol **inline** qila oladi (chaqirilgan joyga ko'chiradi).

MyOS'da: `kernel/fs/pipe.c` dagi `pipe_read`, `pipe_write` — `static`. Tashqariga faqat `pipe_create` va `pipe_fops` jadvali orqali chiqadi.
Linux yadrosida ham qoida: tashqariga kerak bo'lmagan hamma narsa `static`.

### `static` lokal o'zgaruvchi — eslab qoluvchi quti

**Hayotdan misol: metro turniketi.** Har kim o'tganda hisoblagich bittaga oshadi va **keyingi odam kelganda ham eslab qoladi**. Oddiy lokal o'zgaruvchi esa har
chaqiruvda **noldan** boshlanadi — xuddi har yo'lovchi uchun yangi turniket qo'yilgandek.

```c
/* turniket.c - oddiy va static lokal o'zgaruvchi */
#include <stdio.h>

static int oddiy(void)
{
    int soni = 0;                   /* har chaqiruvda yangidan yaratiladi */
    return ++soni;
}

static int eslab_qoladi(void)
{
    static int soni = 0;            /* BIR MARTA yaratiladi, chaqiruvlar orasida saqlanadi */
    return ++soni;
}

int main(void)
{
    for (int i = 0; i < 3; i++)
        printf("oddiy: %d   static: %d\n", oddiy(), eslab_qoladi());
    return 0;
}
```

```console
$ gcc -Wall -Wextra turniket.c -o turniket
$ ./turniket
oddiy: 1   static: 1
oddiy: 1   static: 2
oddiy: 1   static: 3
```

| Chaqiruv | `oddiy()` ichida `soni` | `eslab_qoladi()` ichida `soni` |
|---|---|---|
| 1 | 0 → 1 (**yangidan** 0) | 0 → 1 |
| 2 | 0 → 1 (yana 0 dan) | **1** → 2 (oldingisini eslab qoldi) |
| 3 | 0 → 1 | 2 → 3 |

`static int soni = 0;` dagi `= 0` — **faqat bir marta**, dastur boshida bajariladi (har chaqiruvda emas). (`static` o'zgaruvchilarning xotiradagi o'rni — 8-bob.)

## 5.7. Stek: funksiya chaqirilganda nima bo'ladi

**Hayotdan misol: likopchalar ustuni.** Yangi likopcha doim **ustiga** qo'yiladi va doim **ustidan** olinadi. `main` → `a()` → `b()` chaqirilsa, `b` ning
likopchasi eng ustida. `b` tugashi bilan uning likopchasi olinadi va `a` davom etadi. Likopchalar juda ko'payib, shiftga yetsa — **stack overflow** (stek to'lishi).

Har bir chaqiruv **stekda** "kadr" (frame) ochadi: qaytish manzili, lokal o'zgaruvchilar, saqlangan registrlar. Funksiya qaytganda kadr yo'qoladi.

```text
            yuqori manzillar
   ┌──────────────────────┐
   │ main ning kadri      │  a = 5
   ├──────────────────────┤
   │ qaytish manzili      │  (main ichidagi keyingi instruksiya)
   │ oshir ning kadri     │  x (nusxa)
   └──────────────────────┘  <- rsp (stek ko'rsatkichi), stek PASTGA o'sadi
            quyi manzillar
```

Bundan muhim natija: **lokal o'zgaruvchining manzilini qaytarib bo'lmaydi**.

**Bu dastur nima qiladi (umumiy):** lokal o'zgaruvchining manzilini qaytaradigan funksiya (osilib qolgan ko'rsatkich) — ataylab xatoli.

```c
/* yomon.c - o'lik xotiraga manzil */
int *yomon(void)
{
    int x = 42;
    return &x;              /* XATO: x ning kadri funksiya qaytishi bilan yo'qoladi */
}
```

```console
$ gcc -Wall -Wextra -c yomon.c -o yomon.o # xato kutiladi
yomon.c: In function ‘yomon’:
yomon.c:5:12: warning: function returns address of local variable [-Wreturn-local-addr]
    5 |     return &x;              /* XATO: x ning kadri funksiya qaytishi bilan yo'qoladi */
      |            ^~
```

GCC ogohlantiradi (`function returns address of local variable`). Qaytarilgan manzil "o'lik" xotiraga ko'rsatadi: kadr yo'qolgan, o'sha joyni keyingi chaqiruvlar egallaydi.
Uzoq yashashi kerak bo'lgan ma'lumot — `malloc` bilan **heap**'da (8-bob).

**Stek cheklangan:** user dasturda odatda 8 MB, **yadroda esa har bir oqimga atigi 8–16 KB**. Yadro funksiyasida `char buf[8192];` yozish — stekni to'ldirishning
oson yo'li. MyOS'da buning namoyishi bor: `make run-nographic APPEND=demo=stack` (double fault).

## 5.8. Rekursiya

**Hayotdan misol: matryoshka.** Katta qo'g'irchoqni ochsangiz, ichidan kichigi chiqadi, uning ichidan yana kichigi... Eng kichigini ochib bo'lmaydi — bu **to'xtash sharti**.
To'xtash sharti bo'lmasa, matryoshka hech qachon tugamaydi — dastur stack overflow bilan qulaydi.

**Rekursiya** — funksiya **o'zini o'zi chaqirishi**. Faktorial: `5! = 5 × 4 × 3 × 2 × 1`; ya'ni `n! = n × (n-1)!`, `1! = 1`.

```c
/* rekursiya.c - faktorial va chaqiruvlar chuqurligi */
#include <stdio.h>

static unsigned long faktorial(unsigned n, int chuqurlik)
{
    printf("%*s faktorial(%u) boshlandi\n", chuqurlik * 2, "", n);
    if (n <= 1) {
        printf("%*s -> 1 (to'xtash sharti)\n", chuqurlik * 2, "");
        return 1;                               /* to'xtash sharti - BO'LISHI SHART */
    }
    unsigned long r = n * faktorial(n - 1, chuqurlik + 1);
    printf("%*s -> %lu\n", chuqurlik * 2, "", r);
    return r;
}

int main(void)
{
    printf("natija: %lu\n", faktorial(4, 0));
    return 0;
}
```

```console
$ gcc -Wall -Wextra rekursiya.c -o rekursiya
$ ./rekursiya
 faktorial(4) boshlandi
   faktorial(3) boshlandi
     faktorial(2) boshlandi
       faktorial(1) boshlandi
       -> 1 (to'xtash sharti)
     -> 2
   -> 6
 -> 24
natija: 24
```

**Kodda nimalar bor:** `faktorial(n, chuqurlik)` — `chuqurlik` faqat chiroyli chop etish uchun (chekinish). `%*s` — "kengligi argumentdan olinadigan matn": `chuqurlik * 2` ta bo'shliq chiqaradi.
Asosiy mantiq: `n <= 1` bo'lsa `1` qaytar (**to'xtash sharti**); aks holda `n * faktorial(n - 1)`.

**Qadam-baqadam (`faktorial(4)`):**

| Qadam | Chaqiruv | Nima bo'ladi |
|---|---|---|
| 1 | `faktorial(4)` | `4 * faktorial(3)` — javob kutadi |
| 2 | `faktorial(3)` | `3 * faktorial(2)` — kutadi |
| 3 | `faktorial(2)` | `2 * faktorial(1)` — kutadi |
| 4 | `faktorial(1)` | **to'xtash**: `1` qaytaradi |
| 5 | qaytish | `2 * 1 = 2` → `3 * 2 = 6` → `4 * 6 = 24` |

Har chaqiruv — stekda **yangi kadr** (5.7). Chuqur rekursiya stekni to'ldiradi. To'xtash sharti **yo'q** bo'lsa nima bo'ladi — ko'ramiz:

**Bu dastur nima qiladi (umumiy):** to'xtash sharti yo'q rekursiya: stek to'lib, dastur qulaydi (stack overflow).

```c
/* cheksiz.c - to'xtash sharti yo'q */
#include <stdio.h>

static int ichiga(int n)
{
    return ichiga(n + 1) + 1;           /* to'xtamaydi! */
}

int main(void)
{
    printf("boshladik\n");
    fflush(stdout);
    return ichiga(0);
}
```

```console
$ gcc -Wall -Wextra cheksiz.c -o cheksiz # xato kutiladi
cheksiz.c: In function ‘ichiga’:
cheksiz.c:4:12: warning: infinite recursion detected [-Winfinite-recursion]
    4 | static int ichiga(int n)
      |            ^~~~~~
cheksiz.c:6:12: note: recursive call
    6 |     return ichiga(n + 1) + 1;           /* to'xtamaydi! */
      |            ^~~~~~~~~~~~~
$ bash -c './cheksiz; echo "chiqish kodi: $?"' 2>&1 | sed 's/[0-9][0-9]* Segm/Segm/'
boshladik
bash: line 1:  Segmentation fault      ./cheksiz
chiqish kodi: 139
```

Stek to'ldi → operatsion tizim dasturni **`Segmentation fault`** bilan o'ldirdi (chiqish kodi 139 = 128 + 11, signal 11). Kompilyator ham ogohlantirgan (`infinite recursion`).

**Yadroda rekursiyadan qochiladi** — stek juda kichik (8–16 KB). Daraxtlar ham ko'pincha sikl + qo'lda boshqariladigan stek bilan aylanadi.

> **Eslab qoling:** rekursiya = **to'xtash sharti** + o'zini **kichikroq** masala bilan chaqirish. To'xtash sharti bo'lmasa — stek to'ladi.

## 5.9. `inline` va makro-funksiyalar

```c
static inline int max(int a, int b) { return a > b ? a : b; }
```

`inline` — "bu funksiyani chaqirmasdan, chaqirilgan joyga ko'chirishing mumkin" degan maslahat. Kichik, tez-tez chaqiriladigan funksiyalar uchun `.h` faylda
`static inline` yoziladi. MyOS'da: `kernel/arch/io.h` (`inb`, `outb`), `kernel/arch/cpu.h`. Makrolardan (`#define MAX(a,b) ...`) afzal — turlar tekshiriladi va
argumentlar ikki marta hisoblanmaydi (10-bob).

## 5.10. Funksiya ko'rsatkichlari — qisqacha

Funksiya ham xotirada turadi — uning **manzilini** o'zgaruvchida saqlash mumkin:

**Bu dastur nima qiladi (umumiy):** bitta funksiya ko'rsatkichi avval `qoshish`, keyin `ayirish` funksiyasini ko'rsatib, ikkalasini ham chaqirishini ko'rsatadi.

```c
/* funksiya_korsatkich.c - funksiyaning manzili */
#include <stdio.h>

static int qoshish(int a, int b) { return a + b; }
static int ayirish(int a, int b) { return a - b; }

int main(void)
{
    int (*amal)(int, int) = qoshish;    /* amal - funksiyaga ko'rsatkich */
    printf("qoshish: %d\n", amal(2, 3));

    amal = ayirish;                     /* endi boshqa funksiyani ko'rsatadi */
    printf("ayirish: %d\n", amal(2, 3));
    return 0;
}
```

```console
$ gcc -Wall -Wextra funksiya_korsatkich.c -o funksiya_korsatkich
$ ./funksiya_korsatkich
qoshish: 5
ayirish: -1
```

`int (*amal)(int, int)` o'qilishi: "`amal` — `(int, int)` oladigan va `int` qaytaradigan funksiyaga **ko'rsatkich**". Xuddi qutiga "qaysi tugma bosilsa" degan yozuv yopishtirgandek:
`amal` boshqa funksiyani ko'rsata oladi. `qsort` taqqoslash funksiyasini shunday oladi; VFS har bir fayl tizimining `read`/`write` ini shunday chaqiradi. 7-bob va 20-mashqda batafsil.

## 5.11. `main` ning argumentlari

**Bu dastur nima qiladi (umumiy):** buyruq qatori argumentlarini (`argc`, `argv`) birma-bir chiqaradi.

```c
/* argumentlar.c - buyruq qatori argumentlari */
#include <stdio.h>

int main(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
        printf("argv[%d] = %s\n", i, argv[i]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra argumentlar.c -o argumentlar
$ ./argumentlar salom 42
argv[0] = ./argumentlar
argv[1] = salom
argv[2] = 42
```

- `argc` — argumentlar **soni** (dastur nomi bilan birga); `argv` — ularning **matnlari** ro'yxati.
- `./dastur salom 42` → `argc = 3`, `argv = {"./dastur", "salom", "42", NULL}`. Sonlar ham **satr** bo'lib keladi — ularni o'zingiz aylantirasiz (12-mashq).
- MyOS'da `argv` ni kim yasaydi: yadro `exec` paytida yangi dasturning stekiga yozadi (`kernel/proc/exec.c`), `user/libc/crt0.asm` esa uni `main` ga uzatadi.

## Hayotdan misol va to'liq dastur

**Do'kon kassasi.** Bitta dasturda funksiyalarning hamma turi: qiymat qaytarish, chiqish parametrlari, `static` hisoblagich, rekursiya.

```c
/* kassa.c - funksiyalar: qiymat qaytarish, chiqish parametrlari, static, rekursiya */
#include <stdio.h>

/* Narx: soni x narx, chegirma foizda. Natijani QAYTARADI. */
static long hisobla(long narx, int soni, int chegirma_foiz)
{
    long jami = narx * soni;
    return jami - jami * chegirma_foiz / 100;
}

/* Qaytim: nechta 10 000 lik va nechta 1 000 lik. Ikki natija - CHIQISH PARAMETRLARI orqali. */
static void qaytim_ber(long qaytim, int *on_minglik, int *minglik)
{
    *on_minglik = (int)(qaytim / 10000);
    *minglik = (int)(qaytim % 10000 / 1000);
}

/* Har chaqiruvda chek raqami oshadi - static tufayli ESLAB QOLADI */
static int yangi_chek(void)
{
    static int raqam = 100;
    return ++raqam;
}

/* Rekursiya: n kun davomida har kuni narx 10% oshsa */
static long narx_n_kundan_keyin(long narx, int n)
{
    if (n == 0)
        return narx;                            /* to'xtash sharti - eng kichik matryoshka */
    return narx_n_kundan_keyin(narx + narx / 10, n - 1);
}

int main(void)
{
    long jami = hisobla(12000, 3, 10);          /* 3 ta non, 10% chegirma */
    printf("Chek #%d: jami %ld so'm\n", yangi_chek(), jami);

    long berildi = 50000;
    int on, bir;
    qaytim_ber(berildi - jami, &on, &bir);
    printf("  berildi %ld, qaytim %ld: %d x 10 000 + %d x 1 000\n", berildi, berildi - jami, on, bir);

    printf("Chek #%d: jami %ld so'm\n", yangi_chek(), hisobla(25000, 2, 0));
    printf("Chek #%d: jami %ld so'm\n", yangi_chek(), hisobla(7000, 10, 5));

    printf("10 000 so'mlik narx 3 kundan keyin: %ld so'm\n", narx_n_kundan_keyin(10000, 3));
    return 0;
}
```

```console
$ gcc -Wall -Wextra kassa.c -o kassa
$ ./kassa
Chek #101: jami 32400 so'm
  berildi 50000, qaytim 17600: 1 x 10 000 + 7 x 1 000
Chek #102: jami 50000 so'm
Chek #103: jami 66500 so'm
10 000 so'mlik narx 3 kundan keyin: 13310 so'm
```

**Kodda nimalar bor:**

| Funksiya | Nima oladi | Nima qaytaradi | Nima uchun |
|---|---|---|---|
| `hisobla(narx, soni, chegirma_foiz)` | uchta son | `long` — jami | narxni hisoblash; **natija `return` bilan** |
| `qaytim_ber(qaytim, &on, &bir)` | qaytim summasi va ikki **manzil** | hech narsa (`void`) | ikkita natija kerak edi, shuning uchun `*on_minglik`, `*minglik` orqali **chiqish parametrlari** |
| `yangi_chek()` | hech narsa (`void`) | `int` — chek raqami | `static int raqam = 100;` — chaqiruvlar orasida **eslab qoladi**: 101, 102, 103 |
| `narx_n_kundan_keyin(narx, n)` | narx, kunlar | `long` | **rekursiya**: `n == 0` — to'xtash; aks holda 1 kun o'tdi, `n - 1` kun qoldi |

**Hisoblash (qo'lda tekshirish):**

- `hisobla(12000, 3, 10)`: `jami = 36000`; chegirma `36000 * 10 / 100 = 3600`; `36000 - 3600 = 32400` ✓.
- Qaytim: `50000 - 32400 = 17600` → `17600 / 10000 = 1` ta o'n minglik; `17600 % 10000 = 7600`, `7600 / 1000 = 7` ta minglik ✓.
- Rekursiya: `10000 → 11000 → 12100 → 13310` (har kuni +10%) ✓.

**Sinab ko'ring:** `yangi_chek` dagi `static` ni o'chiring — chek raqamlari qanday bo'ladi? `narx_n_kundan_keyin` dagi `if (n == 0)` ni o'chiring va dasturni ishga tushiring — nima bo'ladi (5.8)?

<!-- katta:boshi -->
## Katta loyiha: Ombor — 5-bosqich: funksiyalar va xato kodlari

**Oldingi bosqichdan:** `main` ichida hamma narsa — menyu, sotish tekshiruvlari, zaxirani kamaytirish. U uzun, ichma-ich (`switch` ichida `switch`) va takrorlanuvchi. O'qish va o'zgartirish qiyin.

### Bu bosqichda nima qilamiz

`main` ni **kichik funksiyalarga** bo'lamiz. Har funksiya **bitta ish** qiladi va nomi ishini aytadi:

| Funksiya | Vazifasi |
|---|---|
| `nom_ol(id)`, `soni_ol(id)`, `soni_yoz(id, ...)` | id bo'yicha mahsulot ma'lumotini olish/yozish (takrorlanuvchi `switch` bitta joyda) |
| `royxat()` | jadvalni chiqarish |
| `sot(id, miqdor)` | sotish mantiqi: tekshiradi, zaxirani kamaytiradi, **natija kodini qaytaradi** |
| `xato_matni(kod)` | kodni odam o'qiydigan matnga aylantiradi |
| `sotish_menyusi()` | foydalanuvchidan so'rash va natijani ko'rsatish |

**Xato kodlari:** `sot` ichida `printf("XATO...")` yozish o'rniga, **kod qaytaramiz**: `OK = 0`, manfiy sonlar — turli xatolar. Nega?

- **Mantiq va ko'rinish ajraladi:** `sot` hisoblaydi, xabar chiqarish — chaqiruvchining ishi (balki kelajakda xabar boshqa tilda yoki faylga yoziladi).
- **Yadro shunday ishlaydi:** Linux funksiyalari muvaffaqiyatda `0`, xatoda **manfiy** kod qaytaradi (`-ENOMEM`, `-EINVAL`). Siz ham shu uslubga o'rganasiz.
- **`enum`** (nomlangan butun sonlar) — kodlarga nom beradi: `YETARLI_EMAS` ma'nosi `-3` dan ko'ra ancha aniq.

**`static`** — ikki xil ishlatilmoqda: (1) fayl darajasidagi `static long non_narx...` — "bu o'zgaruvchi faqat shu faylga tegishli", (2) `static` funksiyalar — "bu funksiya faqat shu faylda ko'rinadi" (5-bob, 11-bob).

```c
/* ombor.c - Ombor, 5-bosqich: hamma ish funksiyalarga bo'lingan, xatolar kod bilan qaytariladi */
#include <stdint.h>
#include <stdio.h>

#include "ombor_chop.h"

/* sotish natijasi kodlari (0 - hammasi joyida, manfiy - xato; yadroda ham shunday) */
enum { OK = 0, YOQ_ID = -1, NOTOGRI_MIQDOR = -2, YETARLI_EMAS = -3 };

/* ombor ma'lumotlari: shu faylga tegishli (static), funksiyalar ularni ko'radi */
static long non_narx = 400000, sut_narx = 1200000, guruch_narx = 1800000;
static uint16_t non_soni = 120, sut_soni = 45, guruch_soni = 8;

/* --- "kirish" funksiyalari: id bo'yicha ma'lumotni olish va yozish --- */
static const char *nom_ol(int id)
{
    switch (id) {
    case 1: return "Non";
    case 2: return "Sut";
    case 3: return "Guruch";
    default: return "?";
    }
}

static uint16_t soni_ol(int id)
{
    switch (id) {
    case 1: return non_soni;
    case 2: return sut_soni;
    default: return guruch_soni;
    }
}

static void soni_yoz(int id, uint16_t yangi)
{
    switch (id) {
    case 1: non_soni = yangi; break;
    case 2: sut_soni = yangi; break;
    default: guruch_soni = yangi; break;
    }
}

/* --- asosiy ishlar --- */
static void royxat(void)
{
    chop_sarlavha();
    chop_qator("Non", non_narx, non_soni);
    chop_qator("Sut", sut_narx, sut_soni);
    chop_qator("Guruch", guruch_narx, guruch_soni);
    chop_jami(non_narx * non_soni + sut_narx * sut_soni + guruch_narx * guruch_soni, 12);
}

/* id mahsulotdan miqdor dona sotadi. Qaytaradi: OK yoki xato kodi */
static int sot(int id, int miqdor)
{
    if (id < 1 || id > 3)
        return YOQ_ID;
    if (miqdor <= 0)
        return NOTOGRI_MIQDOR;
    if (miqdor > soni_ol(id))
        return YETARLI_EMAS;
    soni_yoz(id, soni_ol(id) - miqdor);
    return OK;
}

static const char *xato_matni(int kod)
{
    switch (kod) {
    case YOQ_ID: return "bunday mahsulot yo'q";
    case NOTOGRI_MIQDOR: return "miqdor musbat bo'lishi kerak";
    case YETARLI_EMAS: return "omborda yetarli emas";
    default: return "noma'lum xato";
    }
}

static void sotish_menyusi(void)
{
    int id, miqdor;
    printf("Qaysi mahsulot (1-non, 2-sut, 3-guruch) va necha dona?\n");
    if (scanf("%d %d", &id, &miqdor) != 2)
        return;
    int r = sot(id, miqdor);
    if (r == OK)
        printf("  Sotildi: %d dona %s. Qoldi: %u dona\n", miqdor, nom_ol(id), soni_ol(id));
    else
        printf("  XATO: %s\n", xato_matni(r));
}

int main(void)
{
    int tanlov;
    while (printf("\n1) ro'yxat   2) sotish   0) chiqish\nTanlov:\n"), scanf("%d", &tanlov) == 1 && tanlov != 0) {
        if (tanlov == 1)
            royxat();
        else if (tanlov == 2)
            sotish_menyusi();
        else
            printf("  XATO: menyuda bunday band yo'q\n");
    }
    printf("\nXayr!\n");
    return 0;
}
```

```console
$ cd katta_loyiha/ombor/05_funksiyalar
$ printf '1\n2\n1 20\n2\n3 100\n2\n9 1\n2\n2 -5\n7\n1\n0\n' > kirish.txt
$ gcc -Wall -Wextra ombor.c ombor_chop.c -o ombor
$ ./ombor < kirish.txt

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
================ OMBOR ================
Mahsulot         Narx   Soni          Summa
---------------------------------------
Non           4000.00    120      480000.00
Sut          12000.00     45      540000.00
Guruch       18000.00      8      144000.00
---------------------------------------
Jami qiymat:                1164000.00
QQS stavkasi:               12%
QQS summasi:                139680.00

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
Qaysi mahsulot (1-non, 2-sut, 3-guruch) va necha dona?
  Sotildi: 20 dona Non. Qoldi: 100 dona

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
Qaysi mahsulot (1-non, 2-sut, 3-guruch) va necha dona?
  XATO: omborda yetarli emas

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
Qaysi mahsulot (1-non, 2-sut, 3-guruch) va necha dona?
  XATO: bunday mahsulot yo'q

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
Qaysi mahsulot (1-non, 2-sut, 3-guruch) va necha dona?
  XATO: miqdor musbat bo'lishi kerak

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
  XATO: menyuda bunday band yo'q

1) ro'yxat   2) sotish   0) chiqish
Tanlov:
================ OMBOR ================
Mahsulot         Narx   Soni          Summa
---------------------------------------
Non           4000.00    100      400000.00
Sut          12000.00     45      540000.00
Guruch       18000.00      8      144000.00
---------------------------------------
Jami qiymat:                1084000.00
QQS stavkasi:               12%
QQS summasi:                130080.00

1) ro'yxat   2) sotish   0) chiqish
Tanlov:

Xayr!
```

**Natija 4-bosqichdagi bilan bir xil mantiqda** (faqat xabar matnlari biroz boshqa): yana refaktoring — ishlash o'zgarmadi, **tuzilma** o'zgardi. Endi `main` ning menyu sikli 10 qatorga tushdi.

**Funksiya chaqirilganda nima bo'ladi** (5-bob, stek): `sot(1, 20)` chaqirilganda **nusxalar** uzatiladi: `id` va `miqdor` o'zlari emas, qiymat nusxalari. `sot` ichida ularni o'zgartirsangiz, `sotish_menyusi` dagi asl o'zgaruvchilar o'zgarmaydi. Natijani esa `return` bilan qaytaramiz.

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `enum { OK = 0, YOQ_ID = -1, ... };` | natija kodlariga nom |
| `static int sot(int id, int miqdor)` | `int` natija kodi qaytaradi; tekshiruvlar tartibi: id → miqdor → zaxira |
| `switch (kod) { case ...: return "..."; }` | `xato_matni` ichida har tarmoq `return` qiladi (`break` kerak emas) |
| `while (printf(...), scanf(...) == 1 && tanlov != 0)` | **vergul operatori**: avval menyu chiqadi, keyin `scanf` o'qiydi; ikkala shart bitta qatorda |
| `else if (tanlov == 2)` | `switch` o'rniga `if/else if`: ikkita band uchun yetarli |

**Hali ham muammo:** `nom_ol`, `soni_ol`, `soni_yoz` ichidagi `switch (id)` — **mahsulot qo'shilsa har uchalasini** o'zgartirish kerak. Ma'lumotni **massivda** saqlasak, bu funksiyalar bitta qatorga qisqaradi. Bu — 6-bobning mavzusi.

> **Eslab qoling:** funksiya = bitta ish + aniq nom. Xatoni **kod bilan qaytaring** (`0` — OK, manfiy — xato), xabarni chaqiruvchi chiqarsin. Argumentlar **nusxa** bo'lib uzatiladi.

**O'zingiz qo'shing (yechimsiz):**

1. `soni_ol(id)` ni nolinchi bo'lmagan hamma id uchun tekshiring: `soni_ol(7)` nima qaytaradi? Bu yaxshi dizaynmi? Qanday yaxshilash mumkin?
2. `royxat()` ichidagi uchta `chop_qator` chaqiruvini `nom_ol`/`soni_ol` va `for` sikli bilan qisqartiring (narx uchun `narx_ol` funksiyasi kerak bo'ladi).
3. `sot` ga yangi xato kodi `BLOKLANGAN` qo'shing (id=3 sotilmaydi, deylik) va `xato_matni` ni yangilang. Nechta joyni tahrirlashingiz kerak bo'ldi?
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. Funksiya = **qaytish turi + nom + (parametrlar) + { tana }**; bir marta yozib, ko'p marta chaqiriladi.
2. `void` = "hech narsa": `void f()` — qaytarmaydi; `f(void)` — olmaydi; `(void)x;` — ataylab ishlatmayapman.
3. Argument **nusxa** bo'lib uzatiladi; asl qiymatni o'zgartirish uchun **manzil** (`&a`) bering.
4. Xato = qaytish qiymati (manfiy son / nolga teng emas); **doim tekshiring**. `static` funksiya — faqat shu fayl uchun; `static` lokal — eslab qoladi.
5. Har chaqiruv stekda kadr ochadi; rekursiya = to'xtash sharti + kichikroq masala; lokal o'zgaruvchi manzilini **qaytarmang**.

## Savol-javob

**Default argument, nomli argument bormi?**
Yo'q. Funksiya aynan e'lon qilingan parametrlar bilan chaqiriladi.

**Funksiya ichida funksiya yozsa bo'ladimi?**
Standart C'da yo'q (GCC kengaytmasi bor, lekin ishlatmang). Yordamchi funksiyani `static` qilib yuqorida yozing.

**Nega `main` ga `static` yozmaymiz?**
`main` ni operatsion tizim **tashqaridan** chaqiradi — u boshqa fayllarga ko'rinishi shart.

**Nega funksiyani chaqirish "bepul" emas?**
Har chaqiruv kadr ochadi, argumentlarni nusxalaydi. Juda kichik funksiyalar uchun `static inline` (5.9) bu xarajatni yo'qotadi.

## O'zingizni tekshiring

1. `void f(void)` dagi ikkala `void` nimani bildiradi?
2. `void f(int x) { x = 10; }` — chaqiruvchining o'zgaruvchisi o'zgaradimi? Qanday qilib o'zgartirish mumkin?
3. Funksiyani `main` dan pastda yozsam va prototip bo'lmasa nima bo'ladi?
4. `static` funksiya nima uchun kerak?
5. Nega lokal o'zgaruvchining manzilini qaytarish xato?
6. `static int n = 0; return ++n;` ichki funksiya uch marta chaqirilsa nima qaytaradi?

<details><summary>Javoblar</summary>

1. Hech narsa qaytarmaydi; argument olmaydi.
2. Yo'q — nusxa o'zgaradi. `void f(int *x) { *x = 10; }` va `f(&a)`.
3. `implicit declaration` xatosi (zamonaviy GCC).
4. Faqat shu faylda ko'rinishi uchun: nomlar to'qnashmaydi, interfeys aniq bo'ladi, inline qilish oson.
5. Funksiya qaytgach uning stek kadri yo'qoladi va o'sha joyni keyingi chaqiruvlar egallaydi.
6. 1, 2, 3 — `static` o'zgaruvchi chaqiruvlar orasida eslab qoladi.
</details>

## Mashq

### Isitish: kichik funksiyalar ★☆☆ — eng osoni, avval shuni qiling

Faqat 0–5-boblar kerak (funksiya, `return`, `static`).
Skeletni `isitish.c` ga **qo'lda** yozing (ko'chirmang), izohlarni o'qing va `TODO` joylarini to'ldiring.
"Namuna" qismlar tayyor — qolganini qanday yozishni ko'rsatadi. Skelet hozir ham ogohlantirishsiz yig'iladi:
har `TODO` dan keyin yig'ib, ishga tushirib boring.

```c
/* isitish.c - 5-bob, isitish: kichik funksiyalar. Massiv va ko'rsatkich kerak emas. */
#include <stdio.h>

/* static - funksiya faqat shu faylda ko'rinadi (5.6). (Namuna - tayyor.) */
static int kattasi(int a, int b)
{
    return a > b ? a : b;               /* ternar (3.7): shart ? rost bo'lsa : yolg'on bo'lsa */
}

/* TODO: x * x ni qaytaring. */
static int kvadrat(int x)
{
    (void)x;
    return 0;
}

/* TODO: 1 * 2 * ... * n - sikl bilan. Nega long: 13! int ga sig'maydi.
 *       Keyin 5.8 dagi kabi rekursiya bilan ham yozib ko'ring. */
static long faktorial(int n)
{
    (void)n;
    return 0;
}

/* TODO: har chaqirilganda 1, 2, 3, ... qaytarsin - global o'zgaruvchisiz.
 *       Funksiya ichidagi `static int marta = 0;` qiymatini chaqiruvlar orasida saqlaydi (5.6);
 *       oddiy lokal o'zgaruvchi esa har chaqiruvda qaytadan 0 bo'lardi. */
static int sanagich(void)
{
    return 0;
}

int main(void)
{
    printf("kattasi(7, 3) = %d\n", kattasi(7, 3));
    printf("kattasi(-2, -9) = %d\n", kattasi(-2, -9));
    printf("kvadrat(9) = %d\n", kvadrat(9));
    printf("faktorial(10) = %ld\n", faktorial(10));

    /* Nega avval o'zgaruvchilarga olamiz? printf argumentlari QAYSI TARTIBDA hisoblanishi C'da
     * kafolatlanmagan: printf("%d %d %d", sanagich(), sanagich(), sanagich()) "3 2 1" ham berishi mumkin. */
    int a = sanagich();
    int b = sanagich();
    int c = sanagich();
    printf("sanagich: %d %d %d\n", a, b, c);
    return 0;
}
```

**Kutilgan natija** (`darslik/loyihalar/05_hanoy_paskal/isitish.txt`):

```text
kattasi(7, 3) = 7
kattasi(-2, -9) = -2
kvadrat(9) = 81
faktorial(10) = 3628800
sanagich: 1 2 3
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined isitish.c -o isitish
$ ./isitish | diff - ~/C_loyha/darslik/loyihalar/05_hanoy_paskal/isitish.txt && echo "TO'G'RI"
TO'G'RI
```

### Keyingi mashqlar

- **03** — xato kodini qaytarish va chiqish parametri (2–3-boblarda qoldirilgan; 5.4 dagi `int *` usuli).
- **06** — manzil orqali o'zgartirish (bu bobning asosiy mashqi).
- **07** — chiqish parametrlari va bufer hajmi, lekin massiv (6-bob) kerak: 6-bobdan keyin.

<!-- loyiha:boshi -->
## Loyiha: sonlar laboratoriyasi

**Maqsad:** kichik, aniq vazifali funksiyalar to'plamini yozish va ularni bir-biriga tayantirish.
**Bobdan ishlatiladi:** funksiya yozish, `static`, qiymat qaytarish, rekursiya, bitta funksiya — bitta ish.

**Talab:** tub son tekshiruvi, EKUB va EKUK, Fibonachchi, daraja va raqamlar yig'indisi.
**Ma'lumotlar:** hammasi funksiya argumentlari — global holat yo'q.
**Funksiyalar** (avval imzo, keyin tana):
`tub_mi(n)`, `eub(a, b)`, `eukk(a, b)` (EKUB'ga tayanadi!), `fib(n)`, `daraja(a, n)`, `raqamlar_yigindisi(n)`.

```c
/* sonlar.c - sonlar laboratoriyasi */
#include <stdio.h>

static int tub_mi(int n)
{
    if (n < 2)
        return 0;
    for (int d = 2; d * d <= n; d++)            /* d*d <= n : ildizgacha tekshirish yetadi */
        if (n % d == 0)
            return 0;
    return 1;
}

static int eub(int a, int b)                    /* Evklid algoritmi */
{
    while (b != 0) {
        int t = a % b;
        a = b;
        b = t;
    }
    return a;
}

static int eukk(int a, int b)
{
    return a / eub(a, b) * b;                   /* avval bo'lamiz - toshmaslik uchun */
}

static long fib(int n)                          /* rekursiya: to'xtash sharti + o'ziga chaqiruv */
{
    return n < 2 ? n : fib(n - 1) + fib(n - 2);
}

static long daraja(long a, int n)               /* tez darajaga ko'tarish: n ni har safar yarimlaymiz */
{
    if (n == 0)
        return 1;
    long yarim = daraja(a, n / 2);
    return n % 2 ? yarim * yarim * a : yarim * yarim;
}

static int raqamlar_yigindisi(long n)
{
    return n < 10 ? (int)n : (int)(n % 10) + raqamlar_yigindisi(n / 10);
}

int main(void)
{
    printf("50 gacha tub sonlar:");
    for (int i = 1; i <= 50; i++)
        if (tub_mi(i))
            printf(" %d", i);
    printf("\n");
    printf("EUB(48, 36) = %d, EUKK(4, 6) = %d\n", eub(48, 36), eukk(4, 6));
    printf("Fibonachchi:");
    for (int i = 0; i < 15; i++)
        printf(" %ld", fib(i));
    printf("\n");
    printf("2^10 = %ld, 3^13 = %ld\n", daraja(2, 10), daraja(3, 13));
    printf("98765 raqamlari yig'indisi = %d\n", raqamlar_yigindisi(98765));
    return 0;
}
```

```console
$ gcc -Wall -Wextra sonlar.c -o sonlar
$ ./sonlar
50 gacha tub sonlar: 2 3 5 7 11 13 17 19 23 29 31 37 41 43 47
EUB(48, 36) = 12, EUKK(4, 6) = 12
Fibonachchi: 0 1 1 2 3 5 8 13 21 34 55 89 144 233 377
2^10 = 1024, 3^13 = 1594323
98765 raqamlari yig'indisi = 35
```

Funksiyalarni ajratish foydasi: `eukk` ichida EKUB formulasi qayta yozilmadi — `eub` chaqirildi.
Xatoni topish ham oson: `tub_mi(9)` noto'g'ri chiqsa, faqat bitta 7 qatorli funksiyani ko'rasiz.

**Kengaytiring:** `fib(40)` ni chaqiring — nega sekin? (Har chaqiruv ikkitaga bo'linadi, bir xil qiymatlar qayta-qayta
hisoblanadi.) `daraja` ni oddiy siklga almashtiring — tez rekursiv variant bilan 60 marta ko'paytirish sonini solishtiring.

## Mustaqil loyiha: Hanoy minorasi va Paskal uchburchagi ★★☆

**Vazifa:** rekursiv fikrlashni mashq qiling. Ikki mashhur masala bir dasturda. Fayl: `rekursiya.c`.

**1. Hanoy minorasi.** Uch tayoq: A, B, C. A da `n` ta disk (pastda kattasi). Hamma disklarni C ga
o'tkazing: bir yurishda faqat bitta disk, katta disk kichik disk ustiga tushmasin.
- `void hanoy(int n, char dan, char ga, char oraliq)` — yurishlarni tartib raqami bilan chiqaradi: `3: A -> B`.
  Raqam **global** hisoblagichdan olinadi (`static long yurish;`).
- `long hanoy_soni(int n)` — yurishlar sonini **rekursiv** hisoblaydi (formulasiz: `2*hanoy_soni(n-1) + 1`).

**2. Paskal uchburchagi.** `long c(int n, int k)` — rekursiv: chekkalari 1, ichkarisi yuqoridagi ikkitasining yig'indisi.

**Chiqish tartibi** (aniq shakli kutilgan natijada):
1. `hanoy(3, 'A', 'C', 'B')` ning barcha yurishlari;
2. `hanoy_soni(n)` n = 1, 2, 3, 4, 10, 20 uchun;
3. Paskalning 0..6-qatorlari (har katak `%4ld`);
4. `C(20, 10)`.

**Kutilgan natija** (`darslik/loyihalar/05_hanoy_paskal/kutilgan.txt`):

```text
Hanoy, 3 disk (A -> C, oraliq B):
1: A -> C
2: A -> B
3: C -> B
4: A -> C
5: B -> A
6: B -> C
7: A -> C
Yurishlar soni:
n = 1: 1
n = 2: 3
n = 3: 7
n = 4: 15
n = 10: 1023
n = 20: 1048575
Paskal uchburchagi:
   1
   1   1
   1   2   1
   1   3   3   1
   1   4   6   4   1
   1   5  10  10   5   1
   1   6  15  20  15   6   1
C(20, 10) = 184756
```

**Maslahat** (yechim emas):
- Hanoy: `n` ta diskni `dan` dan `ga` ga o'tkazish uchun avval `n−1` tasini `oraliq` ga, keyin eng katta diskni
  `ga` ga, keyin `n−1` tasini `oraliq` dan `ga` ga o'tkazasiz. Bu uch qadamning ikkitasi — o'sha funksiyaning
  o'zi (kichikroq `n` bilan). To'xtash sharti nima?
- Qog'ozda `n = 2` uchun 3 ta yurishni chizing, keyin `n = 3` ni.
- Paskal: `c(n, 0) = c(n, n) = 1`, boshqasi `c(n-1, k-1) + c(n-1, k)`.
- `c(20, 10)` rekursiv ~370 ming chaqiruv qiladi — tez. Lekin `c(40, 20)` ni sinab ko'rmang, kuting va nega
  sekinligini o'ylang.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g rekursiya.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/05_hanoy_paskal/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [6-bob. Massivlar va satrlar](06-massivlar-satrlar.md)
