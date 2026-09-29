# 4-bob. Boshqaruv oqimi: `if`, sikllar, `switch`, `goto`

> **Bu bobdan keyin:** C'dagi barcha shart va sikl ko'rinishlarini, `{ }` qo'yilmasa nima bo'lishini,
> `switch` dagi "tushib ketish"ni va yadroda nega `goto` ishlatilishini bilasiz. Mashqlar: 01, 02, 05.

> **To'liq ishlaydigan misol:** [misollar/04_boshqaruv.c](misollar/04_boshqaruv.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**`if` / `else` — eshikdagi qo'riqchi (4.1).** "Chiptangiz bormi? Bo'lsa — kiring, bo'lmasa — kassaga
boring." Bitta shart, ikki yo'l. `else if` zanjiri — ko'p eshikli yo'lak: birinchi mos kelgan eshikdan
kirasiz, qolganlariga qaramaysiz.

**`while` — choynak qaynashini kutish (4.2).** "Suv qaynamaguncha — kut." Avval tekshirasiz, keyin
kutasiz. Agar suv allaqachon qaynagan bo'lsa, umuman kutmaysiz — `while` tanasi bir marta ham
bajarilmasligi mumkin.

**`do ... while` — ovqatni tatib ko'rish (4.3).** Tuz yetarlimi — bilish uchun **avval tatib ko'rasiz**,
keyin qaror qilasiz: "yetmasa — tuz qo'shib, yana tatib ko'r". Tana kamida bir marta bajariladi.
PIN kod so'rash ham shunday: kamida bir marta so'raladi.

**`for` — zinapoyadan chiqish (4.4).** "1-qavatdan boshla; 9-qavatgacha; har safar bitta yuqoriga."
Qayerdan boshlash, qachon to'xtash va qanday qadam — hammasi bitta qatorda.

**`break` — kalitni topdingiz (4.5).** Kalitni cho'ntaklardan qidiryapsiz. Ikkinchi cho'ntakda topdingiz —
qolganlarini tekshirmaysiz: `break`.

**`continue` — chirigan olmani o'tkazib yuborish (4.5).** Savatdagi olmalarni saralaysiz. Chirigani
chiqsa — uni tashlab, **keyingisiga** o'tasiz. Sikl to'xtamaydi, faqat shu qadam qolganini tashlab ketadi.

**`switch` — liftning tugmalari (4.6).** Qaysi tugma bosilsa, lift o'sha qavatga boradi. `case` —
tugmalar, `default` — "bunday qavat yo'q". `break` ni unutish — lift kerakli qavatda to'xtamay, keyingisiga
ham chiqib ketgani kabi.

**`goto` yadroda — uydan chiqishdagi tartib (4.7).** Uydan chiqishda: avval gazni o'chirasiz, keyin
chiroqni, oxirida eshikni qulflaysiz. Agar gazni o'chirayotganda muammo chiqsa ham, chiroq va eshikni
baribir yopish kerak. Yadrodagi `goto xato_chiqish;` aynan shu: qayerda xato bo'lmasin, olingan
resurslar teskari tartibda qaytariladi.

### To'liq dastur: bankomat

Foydalanuvchi o'rniga buyruqlar oldindan massivda yozilgan — dastur har safar bir xil ishlaydi.

```c
/* bankomat.c - if, while, do-while, for, switch, break, continue */
#include <stdio.h>

int main(void)
{
    const int togri_pin = 1234;
    int urinishlar[] = { 1111, 4321, 1234 };      /* foydalanuvchi kiritgan PIN'lar */
    int urinish = 0, pin;

    do {                                          /* kamida bir marta so'raladi */
        pin = urinishlar[urinish++];
        if (pin != togri_pin)
            printf("PIN %d noto'g'ri\n", pin);
    } while (pin != togri_pin && urinish < 3);

    if (pin != togri_pin) {
        printf("Karta bloklandi\n");
        return 1;
    }
    printf("PIN to'g'ri (%d-urinishda)\n\n", urinish);

    long balans = 1000000;
    char buyruqlar[] = { 'b', 'y', 'y', 'x', 'b', 'q' };   /* b-balans, y-yechish, x-noma'lum, q-chiqish */

    for (int i = 0; i < 6; i++) {
        char b = buyruqlar[i];
        if (b == 'x') {
            printf("'%c': noma'lum tugma, o'tkazib yuborildi\n", b);
            continue;                             /* keyingi buyruqqa */
        }
        switch (b) {
        case 'b':
            printf("Balans: %ld so'm\n", balans);
            break;
        case 'y':
            if (balans >= 700000) {
                balans -= 700000;
                printf("700 000 so'm berildi\n");
            } else {
                printf("Mablag' yetarli emas\n");
            }
            break;
        case 'q':
            printf("Kartangizni oling. Xayr!\n");
            break;
        }
        if (b == 'q')
            break;                                /* sikldan butunlay chiqish */
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra bankomat.c -o bankomat
$ ./bankomat
PIN 1111 noto'g'ri
PIN 4321 noto'g'ri
PIN to'g'ri (3-urinishda)

Balans: 1000000 so'm
700 000 so'm berildi
Mablag' yetarli emas
'x': noma'lum tugma, o'tkazib yuborildi
Balans: 300000 so'm
Kartangizni oling. Xayr!
```

**Sinab ko'ring:** `urinishlar` massivini `{ 1, 2, 3 }` qiling — karta bloklanadimi? `case 'b':` dagi
`break;` ni o'chiring — balansni so'raganda nima bo'ladi va nega?

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

<!-- loyiha:boshi -->
## Loyiha: taxmin o'yini (ikkilik qidiruv)

**Maqsad:** `while`, `if/else if/else`, `break` va ichma-ich `for` bilan kichik "aql" yaratish: kompyuter
1 dan 100 gacha maxfiy sonni eng kam urinishda topadi.
**Bobdan ishlatiladi:** `while`, `for`, `if`, `else if`, `break`.

**Talab:** maxfiy son berilgan. Kompyuter har safar oraliqning o'rtasini aytadi. "Kattaroq" javobi
kelsa — pastki yarmini tashlaydi, "kichikroq" bo'lsa — yuqori yarmini. Har urinishda variantlar **ikki
barobar** kamayadi.
**Ma'lumotlar:** `maxfiy`, oraliq chegaralari `past`, `yuqori`, `qadam` hisoblagichi.
**Qadamlar:** o'rta = (past + yuqori) / 2 → solishtirish → chegarani siljitish → takrorlash.

```c
/* taxmin.c - ikkiga bo'lib qidirish */
#include <stdio.h>

int main(void)
{
    int maxfiy = 73;
    int past = 1, yuqori = 100, qadam = 0;

    printf("Maxfiy son 1..100 orasida. Kompyuter oraliqni ikkiga bo'lib qidiradi:\n");
    while (past <= yuqori) {
        int orta = (past + yuqori) / 2;
        qadam++;
        if (orta == maxfiy) {
            printf("  %d-qadam: %d -> TOPILDI!\n", qadam, orta);
            break;
        } else if (orta < maxfiy) {
            printf("  %d-qadam: %d -> maxfiy kattaroq\n", qadam, orta);
            past = orta + 1;
        } else {
            printf("  %d-qadam: %d -> maxfiy kichikroq\n", qadam, orta);
            yuqori = orta - 1;
        }
    }

    /* Hamma maxfiy sonlar uchun eng yomon holatni topamiz */
    int eng_kop = 0, eng_kop_son = 0;
    for (int s = 1; s <= 100; s++) {
        int p = 1, y = 100, q = 0;
        while (p <= y) {
            int o = (p + y) / 2;
            q++;
            if (o == s)
                break;
            if (o < s)
                p = o + 1;
            else
                y = o - 1;
        }
        if (q > eng_kop) {
            eng_kop = q;
            eng_kop_son = s;
        }
    }
    printf("1..100 ichida eng ko'p qadam: %d (birinchi marta maxfiy = %d da)\n", eng_kop, eng_kop_son);
    return 0;
}
```

```console
$ gcc -Wall -Wextra taxmin.c -o taxmin
$ ./taxmin
Maxfiy son 1..100 orasida. Kompyuter oraliqni ikkiga bo'lib qidiradi:
  1-qadam: 50 -> maxfiy kattaroq
  2-qadam: 75 -> maxfiy kichikroq
  3-qadam: 62 -> maxfiy kattaroq
  4-qadam: 68 -> maxfiy kattaroq
  5-qadam: 71 -> maxfiy kattaroq
  6-qadam: 73 -> TOPILDI!
1..100 ichida eng ko'p qadam: 7 (birinchi marta maxfiy = 2 da)
```

100 ta variantni 7 qadamda topdi: 2⁷ = 128 > 100. Oddiy ketma-ket sanash (1, 2, 3...) esa 100 gacha qadam talab qilardi.
Bir milliard variant bo'lsa ham 30 qadam yetadi — algoritm shu bilan kuchli (28-bob).

**Kengaytiring:** `maxfiy` ni 1, 50, 100 qilib ko'ring. Sikl sharti `past <= yuqori` ni `<` ga almashtirsangiz, qaysi
sonlarda dastur xato qiladi?

## Mustaqil loyiha: oy kalendari ★★☆

**Vazifa:** `cal` buyrug'i kabi oy kalendarini chiqaring. Massiv va funksiya **kerak emas** — faqat sikl,
`if` va `%`. Fayl: `kalendar.c`.

**Ma'lumotlar** (kod boshida):
- `birinchi` — oyning 1-kuni haftaning qaysi kuni (0 = Dushanba, 1 = Seshanba, ..., 6 = Yakshanba);
- `kunlar` — oyda nechta kun.

**Talab:**
- Birinchi qator: ` Du Se Ch Pa Ju Sh Ya` (har ustun 3 belgi kenglikda).
- Har kun `%3d` bilan chiqadi. 1-kundan oldingi bo'sh kataklar — 3 tadan probel.
- Har Yakshanbadan keyin yangi qator. Oxirgi qatordan keyin **bitta** yangi qator (bo'sh qator bo'lmasin;
  agar oy aynan Yakshanba bilan tugasa, ikkita yangi qator chiqmasin!).
- Qatorlar oxirida ortiqcha probel bo'lmasin.

**Kutilgan natija** — `birinchi = 2` (Chorshanba), `kunlar = 30` (`darslik/loyihalar/04_kalendar/kutilgan.txt`):

```text
 Du Se Ch Pa Ju Sh Ya
        1  2  3  4  5
  6  7  8  9 10 11 12
 13 14 15 16 17 18 19
 20 21 22 23 24 25 26
 27 28 29 30
```

**Qo'shimcha sinovlar.** `birinchi = 6`, `kunlar = 31` (Yakshanbadan boshlanadigan):

```text
 Du Se Ch Pa Ju Sh Ya
                    1
  2  3  4  5  6  7  8
  9 10 11 12 13 14 15
 16 17 18 19 20 21 22
 23 24 25 26 27 28 29
 30 31
```

`birinchi = 0`, `kunlar = 28` (mukammal to'rt hafta):

```text
 Du Se Ch Pa Ju Sh Ya
  1  2  3  4  5  6  7
  8  9 10 11 12 13 14
 15 16 17 18 19 20 21
 22 23 24 25 26 27 28
```

**Maslahat** (yechim emas):
- Avval qog'ozda 30 kunlik oyni chizing va "hafta kuni" hisoblagichini yuritish qoidasini toping:
  har kundan keyin u 1 ga oshadi, 7 ga yetsa 0 ga qaytadi (`%` yoki `if`).
- Birinchi qatordagi bo'sh kataklar soni = `birinchi`.
- Oxirgi qator to'liq bo'lmasa, uni yangi qator bilan yopish kerak. To'liq bo'lsa — allaqachon yopilgan.
  Bu ikki holatni bitta shart bilan ajrating.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g kalendar.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/04_kalendar/kutilgan.txt && echo "TO'G'RI"
```

Ikkinchi va uchinchi sinov uchun `birinchi`, `kunlar` ni o'zgartirib, `kutilgan_2.txt`, `kutilgan_3.txt` bilan solishtiring.
Uchala sinovdan o'tsangiz — chegaraviy holatlar (oy Yakshanba bilan tugash, hafta Dushanbadan boshlanish) to'g'ri.
<!-- loyiha:oxiri -->

Keyingi bob: [5-bob. Funksiyalar](05-funksiyalar.md)
