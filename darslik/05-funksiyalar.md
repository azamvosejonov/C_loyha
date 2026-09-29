# 5-bob. Funksiyalar

> **Bu bobdan keyin:** funksiya e'loni va ta'rifini, `void` ning barcha ma'nolarini, argumentlar
> nega **nusxa** sifatida uzatilishini, `static` funksiyalarni, rekursiya va stek nima ekanini bilasiz.
> Mashqlar: 03, 06, 07.

> **To'liq ishlaydigan misol:** [misollar/05_funksiyalar.c](misollar/05_funksiyalar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Funksiya — kir yuvish mashinasi (5.1).** Ichiga kir va kukun solasiz (argumentlar), tugmani bosasiz
(chaqiruv), toza kir olasiz (qaytish qiymati). Mashina ichida nima bo'layotganini bilishingiz shart
emas. Bitta mashinadan har kuni foydalanasiz — kodni ham bir marta yozib, ko'p marta chaqirasiz.

**`void` — hech narsa bermaydigan yoki olmaydigan mashina (5.2).** Qo'ng'iroq tugmasi hech narsa olmaydi
va hech narsa qaytarmaydi — faqat jiringlaydi: `void jiringla(void)`.

**Nusxa bo'yicha uzatish — hujjat nusxasi (5.4).** Idoraga pasportingizning **ksero nusxasini** berasiz.
Ular nusxaga nima yozsa ham, sizning pasportingiz o'zgarmaydi. C funksiyaga oddiy o'zgaruvchi berganda
ham faqat **nusxa** beriladi. Asl nusxani o'zgartirish uchun esa — **manzilni** berasiz (7-bob): "pasport
mana shu tortmada, borib o'zgartiring".

**Qaytish qiymati — kvitansiya (5.5).** Pul o'tkazdingiz — kvitansiya olasiz: "muvaffaqiyatli" yoki
"xato: hisobda mablag' yo'q". Yadroda funksiyalar shunday: 0 — muvaffaqiyat, manfiy son — xato kodi.
Kvitansiyani o'qimay tashlab yuborish — eng ko'p uchraydigan xato.

**`static` lokal — turniket hisoblagichi (5.6).** Metro turniketidan har kim o'tganda hisoblagich
bittaga oshadi va **keyingi odam kelganda ham eslab qoladi**. Oddiy lokal o'zgaruvchi esa har chaqiruvda
noldan boshlanadi — xuddi har bir yo'lovchi uchun yangi turniket qo'yilgandek.

**Stek — likopchalar ustuni (5.7).** Yangi likopcha doim ustiga qo'yiladi va doim ustidan olinadi.
`main` → `a()` → `b()` chaqirilsa, `b` ning likopchasi eng ustida. `b` tugashi bilan uning likopchasi
olinadi va `a` davom etadi. Likopchalar juda ko'payib, shiftga yetsa — **stack overflow**.

**Rekursiya — matryoshka (5.8).** Katta qo'g'irchoqni ochsangiz, ichidan kichigi chiqadi, uning ichidan
yana kichigi... Eng kichigini ochib bo'lmaydi — bu **to'xtash sharti**. To'xtash sharti bo'lmasa,
matryoshka hech qachon tugamaydi — dastur stack overflow bilan qulaydi.

### To'liq dastur: do'kon kassasi

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

**Sinab ko'ring:** `yangi_chek` dagi `static` ni o'chiring — chek raqamlari qanday bo'ladi?
`narx_n_kundan_keyin` dagi `if (n == 0)` ni o'chiring va dasturni ishga tushiring — nima bo'ladi (5.7)?

## 5.1. Funksiyaning tuzilishi

```c
/*  qaytish   nom       parametrlar
     turi      |         |           */
    long   kvadrat(int x)
{                                   /* tana boshi */
    return (long)x * x;             /* qaytarish */
}                                   /* tana oxiri - ; YO'Q */
```

Python bilan solishtirish:

```python
def kvadrat(x):
    return x * x
```

C'da qo'shimcha: **qaytish turi**, **har bir parametrning turi**, `{ }`.

## 5.2. `void` — barcha ma'nolari (to'liq)

`void` — "hech narsa / turi yo'q". Kontekstga qarab:

```c
void salom(void)            /* 1-void: hech narsa qaytarmaydi; 2-void: argument olmaydi */
{
    printf("salom\n");
    return;                 /* qiymatsiz return - ixtiyoriy, funksiya oxirida o'zi qaytadi */
}

void *p = malloc(100);      /* void * - "turi noma'lum xotira manzili" (7-bob) */

(void)natija;               /* "bu qiymatni ataylab ishlatmayapman" */
(void)printf("x\n");        /* "qaytish qiymatini ataylab tashlab yuboryapman" */
```

**Savol: `void` qaytaradigan funksiya natijasini ishlatsam?**
`int x = salom();` → xato: `void value not ignored as it ought to be`. Qiymat yo'q — olib bo'lmaydi.

**Savol: mashqlardagi `(void)n;` nima uchun?**
Bo'sh funksiyada (`TODO`) parametr ishlatilmaydi va `-Wextra` "unused parameter" ogohlantiradi,
`-Werror` esa uni xatoga aylantiradi. `(void)n;` — "bilaman, hozircha ishlatmayapman" deb kompilyatorga
aytish. Yechimni yozganingizda o'chirasiz. MyOS'da ham bor: `pipe_read` dagi `(void)off;` — pipe'da
fayl pozitsiyasi ma'nosiz, lekin VFS hamma `read` funksiyalariga bir xil parametrlar beradi.

## 5.3. E'lon (prototip) va ta'rif

```c
#include <stdio.h>

long kvadrat(int x);            /* PROTOTIP: shunday funksiya bor (tanasi keyinroq) */

int main(void)
{
    printf("%ld\n", kvadrat(12));   /* kompilyator prototipdan turlarni biladi */
    return 0;
}

long kvadrat(int x)             /* TA'RIF */
{
    return (long)x * x;
}
```

Kompilyator faylni **yuqoridan pastga bir marta** o'qiydi. `main` ichida `kvadrat` ni uchratganda u
haqida allaqachon bilishi kerak — yoki ta'rif yuqorida bo'lishi, yoki prototip. Boshqa fayldagi
funksiyalar uchun prototiplar `.h` faylda turadi (1-bob).

Prototip nima beradi: argumentlar soni va turi **tekshiriladi**, kerak bo'lsa avtomatik aylantiriladi
(`kvadrat(3.7)` → `double` `int` ga, ogohlantirish bilan).

## 5.4. Argumentlar NUSXA sifatida uzatiladi

```c
void oshir(int x)
{
    x = x + 1;              /* faqat NUSXA o'zgaradi */
}

int main(void)
{
    int a = 5;
    oshir(a);
    printf("%d\n", a);      /* 5 - o'zgarmadi! */
}
```

Funksiya chaqirilganda har bir argument **yangi o'zgaruvchiga nusxalanadi**. Chaqiruvchining
o'zgaruvchisiga funksiya tega olmaydi. (Python'da ham `int` uchun xuddi shunday, lekin `list`
uzatilsa — o'sha obyektning o'zi uzatiladi va funksiya uni o'zgartira oladi.)

Funksiya chaqiruvchining o'zgaruvchisini o'zgartirishi kerak bo'lsa — **manzilini** berasiz:

```c
void oshir(int *x)          /* x - int ning MANZILI */
{
    *x = *x + 1;            /* shu manzildagi qiymatni o'zgartirish */
}

oshir(&a);                  /* a ning manzilini berish; endi a = 6 */
```

Bu 7-bobning asosiy mavzusi va 06-mashqda amalda. Yadroda deyarli hamma funksiya shunday ishlaydi.

**Struktura ham nusxalanadi:** `void f(struct katta s)` — butun struktura (masalan 4 KB) stekka
nusxalanadi. Shuning uchun katta strukturalar doim ko'rsatkich bilan uzatiladi: `void f(const struct katta *s)`.

**Massiv esa nusxalanMAYdi:** `void f(int a[])` aslida `void f(int *a)` — birinchi elementning
manzili uzatiladi (6-bob). Shuning uchun funksiya massivni o'zgartira oladi va uning uzunligini **bilmaydi**.

## 5.5. Qaytish qiymati va xato kodlari

C'da exception yo'q. Xato haqida xabar berishning ikki asosiy usuli:

```c
/* 1) Manfiy son = xato (yadro va POSIX uslubi) */
int fayl_och(const char *yol);     /* >= 0: fd, < 0: -ENOENT, -EACCES ... */

/* 2) Natija chiqish parametrida, qaytish qiymati - holat */
int satr_songa(const char *s, long *natija);   /* 0 - OK, -1 - noto'g'ri, -2 - toshish */
```

Chaqiruvchi **doim** tekshirishi kerak:

```c
long x;
if (satr_songa(argv[1], &x) != 0) {
    fprintf(stderr, "noto'g'ri son: %s\n", argv[1]);
    return 1;
}
```

MyOS yadrosida qoida: **manfiy = xato kodi** (`-ENOMEM`, `-EINVAL`, `-EFAULT`). Syscall natijasi
user'ga shunday qaytadi, libc esa uni `errno` ga yozib `-1` qaytaradi (`user/libc/syscall.h`: `__sysret`).
07 va 12-mashqlar shu uslubda.

## 5.6. `static` funksiya — faqat shu fayl uchun

```c
static int yordamchi(int x)     /* boshqa .c fayllar buni ko'rmaydi */
{
    return x * 2;
}
```

- Nomlar to'qnashuvining oldi olinadi: ikki faylda ikkita `static int yordamchi` — muammo emas.
- "Ichki" va "tashqi" funksiyalar aniq ajraladi: `.h` da e'lon qilinganlari — tashqi interfeys,
  `static` lar — ichki tafsilot.
- Kompilyator `static` funksiyani bemalol **inline** qila oladi (chaqirilgan joyga ko'chiradi).

MyOS'da: `kernel/fs/pipe.c` dagi `pipe_read`, `pipe_write` — `static`. Tashqariga faqat `pipe_create`
va `pipe_fops` jadvali orqali chiqadi. Linux yadrosida ham qoida: tashqariga kerak bo'lmagan hamma
narsa `static`.

(`static` o'zgaruvchilarda boshqa ma'noga ega — 8-bob.)

## 5.7. Stek: funksiya chaqirilganda nima bo'ladi

Har bir chaqiruv **stekda** "kadr" (frame) ochadi: qaytish manzili, lokal o'zgaruvchilar, saqlangan
registrlar. Funksiya qaytganda kadr yo'qoladi.

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

Bundan muhim natija:

```c
int *yomon(void)
{
    int x = 42;
    return &x;              /* XATO: x ning kadri funksiya qaytishi bilan yo'qoladi */
}
```

Qaytarilgan manzil "o'lik" xotiraga ko'rsatadi. GCC ogohlantiradi (`function returns address of local variable`).
Uzoq yashashi kerak bo'lgan ma'lumot — `malloc` bilan heap'da (8-bob).

**Stek cheklangan:** user dasturda odatda 8 MB, **yadroda esa har bir oqimga atigi 8–16 KB**.
Yadro funksiyasida `char buf[8192];` yozish — stekni to'ldirishning oson yo'li. MyOS'da buning
namoyishi bor: `make run-nographic APPEND=demo=stack` (double fault).

## 5.8. Rekursiya

```c
unsigned long faktorial(unsigned n)
{
    if (n <= 1)
        return 1;                       /* to'xtash sharti - BO'LISHI SHART */
    return n * faktorial(n - 1);
}
```

Har bir chaqiruv yangi kadr — chuqur rekursiya stekni to'ldiradi (user'da `Segmentation fault`).
**Yadroda rekursiyadan qochiladi** — stek juda kichik. Daraxtlar ham ko'pincha sikl + qo'lda boshqariladigan
stek bilan aylanadi.

## 5.9. `inline` va makro-funksiyalar

```c
static inline int max(int a, int b) { return a > b ? a : b; }
```

`inline` — "bu funksiyani chaqirmasdan, chaqirilgan joyga ko'chirishing mumkin" degan maslahat.
Kichik, tez-tez chaqiriladigan funksiyalar uchun `.h` faylda `static inline` yoziladi. MyOS'da:
`kernel/arch/io.h` (`inb`, `outb`), `kernel/arch/cpu.h`. Makrolardan (`#define MAX(a,b) ...`) afzal —
turlar tekshiriladi va argumentlar ikki marta hisoblanmaydi (10-bob).

## 5.10. Funksiya ko'rsatkichlari — qisqacha

Funksiya ham xotirada turadi — uning manzilini o'zgaruvchida saqlash mumkin:

```c
int qoshish(int a, int b) { return a + b; }

int (*amal)(int, int) = qoshish;    /* amal - funksiyaga ko'rsatkich */
int r = amal(2, 3);                 /* 5 */
```

`qsort` taqqoslash funksiyasini shunday oladi; VFS har bir fayl tizimining `read`/`write` ini shunday
chaqiradi. 7-bob va 20-mashqda batafsil.

## 5.11. `main` ning argumentlari

```c
int main(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
        printf("argv[%d] = %s\n", i, argv[i]);
    return 0;
}
```

`./dastur salom 42` → `argc = 3`, `argv = {"./dastur", "salom", "42", NULL}`. Sonlar ham **satr**
bo'lib keladi — ularni o'zingiz aylantirasiz (12-mashq). MyOS'da `argv` ni kim yasaydi: yadro exec
paytida yangi dasturning stekiga yozadi (`kernel/proc/exec.c`), `user/libc/crt0.asm` esa uni `main` ga uzatadi.

## 5.12. Savol-javob

**Nega Python'dagi kabi bir nechta qiymat qaytarib bo'lmaydi?**
Qaytish qiymati bitta registrda (`rax`) qaytadi. Bir nechta natija uchun: chiqish parametrlari
(07-mashq) yoki struct qaytarish (`struct natija f(void)` — kichik struct ham registrlarda qaytadi).

**Default argument, nomli argument bormi?**
Yo'q. Funksiya aynan e'lon qilingan parametrlar bilan chaqiriladi.

**Funksiya ichida funksiya yozsa bo'ladimi?**
Standart C'da yo'q (GCC kengaytmasi bor, lekin ishlatmang). Yordamchi funksiyani `static` qilib yuqorida yozing.

## 5.13. O'zingizni tekshiring

1. `void f(void)` dagi ikkala `void` nimani bildiradi?
2. `void f(int x) { x = 10; }` — chaqiruvchining o'zgaruvchisi o'zgaradimi? Qanday qilib o'zgartirish mumkin?
3. Funksiyani `main` dan pastda yozsam va prototip bo'lmasa nima bo'ladi?
4. `static` funksiya nima uchun kerak?
5. Nega lokal o'zgaruvchining manzilini qaytarish xato?

<details><summary>Javoblar</summary>

1. Hech narsa qaytarmaydi; argument olmaydi.
2. Yo'q — nusxa o'zgaradi. `void f(int *x) { *x = 10; }` va `f(&a)`.
3. `implicit declaration` xatosi (zamonaviy GCC).
4. Faqat shu faylda ko'rinishi uchun: nomlar to'qnashmaydi, interfeys aniq bo'ladi, inline qilish oson.
5. Funksiya qaytgach uning stek kadri yo'qoladi va o'sha joyni keyingi chaqiruvlar egallaydi.
</details>

## 5.14. Mashqlar

- **03** — xato kodini qaytarish va chiqish parametri.
- **06** — manzil orqali o'zgartirish (bu bobning asosiy mashqi).
- **07** — chiqish parametrlari va bufer hajmi.

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
