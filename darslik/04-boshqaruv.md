# 4-bob. Boshqaruv oqimi: `if`, sikllar, `switch`, `goto`

> **Bu bobdan keyin:** C'dagi barcha shart va sikl ko'rinishlarini, `{ }` qo'yilmasa nima bo'lishini,
> `switch` dagi "tushib ketish"ni va yadroda nega `goto` ishlatilishini bilasiz. Mashqlar: 01, 02, 05.

## 4.1. `if` / `else`

```c
if (ball >= 86) {
    printf("a'lo\n");
} else if (ball >= 71) {
    printf("yaxshi\n");
} else {
    printf("qoniqarli emas\n");
}
```

- Shart **qavs ichida** bo'lishi shart: `if (x > 0)`. Python'dagi `if x > 0:` C'da xato.
- Shart — istalgan son: 0 — yolg'on, qolgani — rost (3-bob).
- `elif` yo'q — `else if` yoziladi (aslida bu `else` + ichida yangi `if`).

### `{ }` qo'yilmasa

`if` dan keyin `{ }` bo'lmasa, **faqat bitta keyingi buyruq** shartga tegishli bo'ladi:

```c
if (xato)
    printf("xato!\n");
    return -1;              /* DIQQAT: bu if ga TEGISHLI EMAS - doim bajariladi */
```

Chekinish sizni aldaydi — kompilyator uchun u yo'q. 2014-yilda Apple'ning SSL kodidagi mashhur
"goto fail" xatosi aynan shunday bo'lgan: bitta ortiqcha qator butun sertifikat tekshiruvini o'chirib
qo'ygan. GCC `-Wall` (`-Wmisleading-indentation`) buni ogohlantiradi.

**Qoida:** ko'p qatorli blokda doim `{ }`. Linux uslubida bitta qatorli `if` uchun `{ }` qo'yilmaydi —
lekin ikkinchi qator qo'shganda albatta qo'ying.

### `;` tuzog'i

```c
if (x > 0);                 /* ; - BO'SH BUYRUQ. if shu bilan tugadi */
{
    printf("musbat\n");     /* bu blok if ga tegishli emas - doim bajariladi */
}
```

## 4.2. `while`

```c
int n = 10;
while (n > 0) {
    printf("%d\n", n);
    n--;
}
```

Shart **avval** tekshiriladi: boshida yolg'on bo'lsa, tana bir marta ham bajarilmaydi.

## 4.3. `do ... while`

```c
do {
    c = oqi();
} while (c == ' ');         /* ; SHART - do-while oxirida */
```

Tana **kamida bir marta** bajariladi, shart oxirida tekshiriladi. Oxiridagi `;` — ko'p unutiladi.

## 4.4. `for`

```c
for (int i = 0; i < 10; i++) {
    printf("%d\n", i);
}
```

Uch qism, `;` bilan ajratilgan:

```text
for ( BOSHLASH ; SHART ; QADAM )  TANA
      bir marta   har aylanishdan   har aylanishdan
                  OLDIN             KEYIN
```

Bu aynan shu `while` ga teng:

```c
{
    int i = 0;
    while (i < 10) {
        printf("%d\n", i);
        i++;
    }
}
```

Python'dagi `for i in range(10)` → `for (int i = 0; i < 10; i++)`.
`range(a, b, qadam)` → `for (int i = a; i < b; i += qadam)`.
Teskari: `for (int i = n - 1; i >= 0; i--)` — lekin `i` `size_t` bo'lsa, `i >= 0` doim rost (2-bob)!
Ishorasiz tur uchun to'g'ri teskari sikl: `for (size_t i = n; i-- > 0; )`.

Istalgan qismini tashlab ketish mumkin: `for (;;)` — **cheksiz sikl** (yadrodagi scheduler va `init`
jarayoni shunday: `kernel/proc/process.c` dagi `scheduler_loop`, `user/bin/init.c`).

## 4.5. `break` va `continue`

```c
for (int i = 0; i < n; i++) {
    if (a[i] < 0)
        continue;           /* bu elementni o'tkazib, keyingisiga */
    if (a[i] == x)
        break;              /* sikldan butunlay chiqish */
    ...
}
```

`break` faqat **eng ichki** sikldan (yoki `switch` dan) chiqaradi. Ichma-ich sikllardan birdaniga
chiqish uchun: flag o'zgaruvchi, funksiyaga ajratib `return`, yoki `goto`.

## 4.6. `switch`

```c
switch (belgi) {
case '+':
    r = a + b;
    break;
case '-':
    r = a - b;
    break;
case 'q':
case 'Q':                   /* ikki case bitta kodga - ataylab "tushib ketish" */
    chiqish = 1;
    break;
default:                    /* hech biri mos kelmasa */
    printf("noma'lum: %c\n", belgi);
    break;
}
```

- Faqat **butun son** qiymatlar (`int`, `char`, `enum`) bo'yicha ishlaydi — satrlar bo'yicha emas.
- `case` qiymatlari — **o'zgarmas** (literal yoki `#define`/`enum`).
- **`break` bo'lmasa — keyingi `case` ga "tushib ketadi" (fallthrough)**. Ba'zan ataylab (yuqoridagi
  `'q'`/`'Q'`), ko'pincha — xato. Ataylab qilinganda izoh yozing: `/* fallthrough */` (GCC
  `-Wimplicit-fallthrough` ogohlantirishini ham o'chiradi).
- Kompilyator ko'p `case` li `switch` ni **sakrash jadvaliga** aylantirishi mumkin — `if` zanjiridan tezroq.

Yadroda qayerda: syscall raqami bo'yicha tarqatish (`kernel/sys/syscall.c`: `switch (nr)`),
klaviatura skankodlari, terminal escape-ketma-ketliklari (`kernel/drivers/vt.c`).

## 4.7. `goto` — yadroda nega ishlatiladi

"`goto` yomon" degan gapni eshitgan bo'lsangiz — umuman olganda to'g'ri: tartibsiz sakrashlar
kodni tushunib bo'lmas qiladi. Lekin C'da bitta joyda u eng toza yechim — **xatodan keyin tozalash**:

```c
int nusxala(const char *a, const char *b)
{
    int rc = -1;
    int in = open(a, O_RDONLY);
    if (in < 0)
        goto chiq;
    int out = open(b, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out < 0)
        goto yop_in;
    char *buf = malloc(4096);
    if (!buf)
        goto yop_out;

    /* ... asosiy ish ... */
    rc = 0;

    free(buf);
yop_out:
    close(out);
yop_in:
    close(in);
chiq:
    return rc;
}
```

Har bir xato o'z "yorlig'iga" sakraydi va o'shandan pastdagi hamma tozalash bajariladi — resurslar
**teskari tartibda** bo'shatiladi. `goto` siz buni qilish uchun ichma-ich `if` lar yoki takrorlangan
`close` lar kerak bo'lardi — ular xato qilishga ko'proq joy beradi. Linux yadrosida bu uslub
minglab joyda uchraydi. (C++ va Rust'da buning o'rniga destruktorlar bor; C'da — `goto`.)

**Qoida:** `goto` faqat **pastga** va faqat **tozalash** uchun.

## 4.8. Bloklar va ko'rinish sohasi

```c
for (int i = 0; i < 3; i++) {
    int kvadrat = i * i;    /* har aylanishda yangi */
}
/* i va kvadrat bu yerda YO'Q */
```

`for (int i = ...)` dagi `i` faqat sikl ichida yashaydi (C99'dan). Bu yaxshi: sikldan keyin tasodifan
eski `i` ni ishlatib qo'ymaysiz.

## 4.9. Savol-javob

**`while (1)` va `for (;;)` farqi bormi?**
Yo'q, ikkalasi ham cheksiz sikl. Linux uslubi — `for (;;)`.

**Nega `else if` alohida kalit so'z emas?**
`else` dan keyin istalgan **bitta** buyruq kelishi mumkin — `if` ham buyruq. Shuning uchun
`else if (...)` aslida `else { if (...) ... }`.

**Qaysi biri tezroq: `switch` yoki `if/else`?**
Ko'p `case` bo'lsa — odatda `switch` (sakrash jadvali). Lekin avval to'g'ri va tushunarli yozing;
tezlikni o'lchab ko'rmasdan optimallashtirmang.

## 4.10. O'zingizni tekshiring

1. `for (int i = 0; i < 3; i++);` `printf("%d", i);` — nima bo'ladi?
2. `switch` da `break` unutilsa nima bo'ladi?
3. `do { } while (0)` necha marta bajariladi? (10-bobda bu nimaga kerakligini ko'rasiz.)
4. `size_t` bilan teskari sikl qanday yoziladi?
5. `goto` yadroda nima uchun ishlatiladi?

<details><summary>Javoblar</summary>

1. Kompilyatsiya xatosi: `;` sikl tanasini bo'sh qildi, `i` esa sikldan tashqarida mavjud emas.
2. Keyingi `case` ning kodi ham bajariladi (fallthrough).
3. Bir marta.
4. `for (size_t i = n; i-- > 0; )` — shart tekshirilganda `i` kamayadi, tana `n-1 .. 0` ni ko'radi.
5. Xatodan keyin resurslarni teskari tartibda tozalash uchun.
</details>

## 4.11. Mashqlar

- **01**, **02** — sikllar.
- **05** (massivlar) — teskari sikl va `size_t` tuzog'i.
- Qo'shimcha: 1 dan 100 gacha FizzBuzz; ko'paytirish jadvali (ichma-ich `for`); kiritilgan sonning
  raqamlari yig'indisi (`while` + `% 10` va `/ 10`).

Keyingi bob: [5-bob. Funksiyalar](05-funksiyalar.md)
