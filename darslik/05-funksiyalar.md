# 5-bob. Funksiyalar

> **Bu bobdan keyin:** funksiya e'loni va ta'rifini, `void` ning barcha ma'nolarini, argumentlar
> nega **nusxa** sifatida uzatilishini, `static` funksiyalarni, rekursiya va stek nima ekanini bilasiz.
> Mashqlar: 03, 06, 07.

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

Keyingi bob: [6-bob. Massivlar va satrlar](06-massivlar-satrlar.md)
