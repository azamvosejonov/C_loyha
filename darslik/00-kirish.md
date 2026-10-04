# 0-bob. Kirish: C nima va birinchi dastur

> **Bu bobda nima o'rganasiz:** kompyuter ichida nima bo'lishini, C tili nimaligini va nega yadrolar
> aynan C'da yozilishini bilib olasiz. Birinchi dasturingizni yozib, ishga tushirasiz va u **har bir
> belgisi nima qilishini** tushunasiz.
> **Oldindan nima kerak:** hech narsa. Faqat Python'dagi oddiy tajriba kifoya.
> **Vaqt:** 2–3 soat (dastur yozish bilan).

> **To'liq ishlaydigan misol:** [misollar/00_salom.c](misollar/00_salom.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Kompyuter o'zi "aqlli" emas. U **juda tez** ishlaydigan, lekin faqat **juda oddiy** ko'rsatmalarni tushunadigan
mashina. Dasturchining ishi — murakkab ishni shunday oddiy ko'rsatmalarga bo'lib berish.

**Hayotdan misol.** Tasavvur qiling, siz stolda ishlayotgan xodimsiz:

| Hayotda | Kompyuterda | Nima qiladi |
|---|---|---|
| **Xodim** (juda tez, lekin oddiy ishni biladi) | **Protsessor (CPU)** | Ko'rsatmalarni bajaradi |
| **Stol usti** (ishlayotgan qog'ozlar shu yerda) | **Xotira (RAM)** | Hozir kerak bo'lgan narsa turadi. Tez, lekin o'chsa — yo'qoladi |
| **Arxiv shkafi** (hammasi saqlanadi, lekin olish sekin) | **Disk** | Fayllar doimiy turadi |
| **Ko'rsatmalar ro'yxati** | **Dastur** | Xodim qadam-baqadam bajaradi |

C tilida dastur yozish — shu ro'yxatni tuzish. Bu bobda esa birinchi ro'yxatni (eng kichigini) tuzamiz
va uning har bir so'zini tushunamiz.

## 0.1. Kompyuter aslida nima qiladi

### Ikkita asosiy qism

- **Protsessor (CPU)** — buyruqlarni bajaradi. U faqat oddiy buyruqlarni tushunadi: "shu joydagi sonni
  o'qi", "ikki sonni qo'sh", "natijani shu joyga yoz", "agar nol bo'lsa, boshqa joyga o't". Lekin sekundiga
  **milliardlab** marta.
- **Xotira (RAM)** — juda uzun **qutilar qatori**. Har bir qutining raqami bor — bu uning **manzili**.
  8 GB xotirada taxminan 8 milliard quti.

### Bit va bayt: kompyuterning "harf"lari

Har bir quti ichida **son** turadi. Kompyuter son qanday saqlaydi? **Kalitlar** bilan.

- **Bit** — bitta kalit: yoqilgan (`1`) yoki o'chirilgan (`0`). Eng kichik ma'lumot.
- **Bayt** — **8 ta** kalitdan iborat panel. Har kalit 2 holatda bo'lgani uchun, 8 ta kalit
  2 × 2 × 2 × 2 × 2 × 2 × 2 × 2 = **256** xil holatni ko'rsata oladi. Ya'ni bir bayt — `0` dan `255` gacha son.

```text
Bitta bayt (8 ta kalit):     0 1 0 0 1 0 0 0      <- yoqilgan kalitlar "1", o'chganlar "0"
Qiymati:                     = 72                  (bu qanday hisoblanishi 3-bobda)
```

Quti (bayt) ichida 72 son turibdi. Shu 72 ni biz **harf `H`** deb o'qishimiz ham mumkin, **rang** deb ham, **buyruq** deb ham.
Kompyuter uchun hammasi — shunchaki sonlar.

```text
manzil:   1000   1001   1002   1003   1004   1005 ...
qiymat:  [ 72 ] [105 ] [  0 ] [255 ] [ 17 ] [  3 ] ...
```

> **Eslab qoling:** xotira — raqamlangan baytlar qatori. **Hamma narsa** (sonlar, matn, rasm, hatto dasturning o'zi)
> shu baytlarda turadi. Dasturning o'zi ham — CPU tushunadigan sonlar ketma-ketligi (**mashina kodi**).

Qancha bayt ko'p: 1 KB ≈ ming bayt, 1 MB ≈ million, 1 GB ≈ milliard bayt.

## 0.2. C tili qayerda turadi

Mashina kodini to'g'ridan-to'g'ri yozish juda qiyin (faqat sonlar!). Shuning uchun **dasturlash tillari** yaratilgan:
ular odam tushunadigan matnni mashina kodiga aylantiradi. Tillar "qavatlarga" bo'linadi:

```text
Python       ← yuqori qavat: xotira, turlar, manzillar yashirilgan. Yozish oson, lekin nima bo'layotganini ko'rmaysiz
  C          ← o'rta qavat: xotira va manzillar KO'RINIB turadi, lekin yozish hali qulay
Assembly     ← har bir CPU buyrug'i alohida yoziladi
Mashina kodi ← CPU bajaradigan baytlar
```

**C — "ko'chma assembly"** deb ataladi. `a = b + c;` deb yozasiz, kompilyator buni 2–3 ta CPU buyrug'iga aylantiradi.
Siz nima yozsangiz, CPU deyarli aynan shuni bajaradi. **Yashirin ish yo'q**: o'zi xotirani tozalaydigan
"axlatchi" (garbage collector) yo'q, har qadamda xatoni tekshiradigan nazoratchi yo'q.

### Nega yadrolar C'da yoziladi? (Linux, Windows yadrosi, MyOS)

**Yadro** — operatsion tizimning eng muhim qismi: u kompyuterning hamma resurslarini (xotira, disk, ekran)
boshqaradi. Uni yozish uchun nima kerak?

1. **Xotirani to'liq boshqarish.** Yadro "`0xB8000` manzildagi qutiga shu sonni yoz" deya olishi kerak, chunki shu
   manzilda ekran xotirasi turadi. Python buni qila olmaydi.
2. **Yashirin ish bo'lmasligi.** Python'da `a = [1, 2]` qatori ortida yuzlab C qatori ishlaydi. Yadroda esa har
   bir amal hisobda bo'lishi kerak: kutilmagan xotira ajratish tizimni qulatishi mumkin.
3. **Hech narsaga tayanmaslik.** Python dasturi ishlashi uchun Python interpretatori kerak, interpretatorga esa
   operatsion tizim kerak. C kodi esa "yalang'och" kompyuterda ham ishlaydi. Yadro aynan shunday bo'lishi shart —
   undan keyin hech narsa yo'q-ku!
4. **Tezlik.** C (assembly'dan tashqari) CPU'ga eng yaqin til.

## 0.3. Python va C: asosiy farqlar

Python bilasiz — shuning uchun avval **nima farq qilishini** ko'rib oling. Bu jadvalga ko'p qaytasiz:

| Python | C | Nega C'da shunday? |
|---|---|---|
| `x = 5` (tur yozilmaydi) | `int x = 5;` (tur **majburiy**) | Kompilyator qancha quti (bayt) ajratishni bilishi kerak |
| Son cheksiz kattalashadi | `int` — chegaralangan, **toshishi** mumkin | Son qat'iy o'lchamdagi qutiga joylanadi (2-bob) |
| Satr — obyekt, uzunligini o'zi biladi | Satr — oxirida maxsus belgi turgan baytlar | Eng oddiy tuzilma, ortiqcha xotira yo'q (6-bob) |
| `list` o'zi o'sadi | Massiv o'lchami **qat'iy** | O'sishni o'zingiz yozasiz (8-bob) |
| Xotira o'zi tozalanadi | `malloc`/`free` — **qo'lda** | Axlatchi yo'q (8-bob) |
| Xato bo'lsa — tushunarli `Exception` | Xato bo'lsa — dastur qulaydi yoki **jim** buziladi | Tekshiruvlar tezlikni oladi |
| Blok — chekinish (indent) bilan | Blok — `{ }` bilan | C uchun bo'shliq va yangi qator ahamiyatsiz |
| Qator oxiri — buyruq oxiri | `;` — buyruq oxiri | (0.5 da batafsil) |

### Interpretator va kompilyator: ikki xil tarjimon

**Hayotdan misol.** Chet tilidagi nutq:
- **Sinxron tarjimon** (Python — *interpretator*): nutq bo'layotgan paytda **gapma-gap** tarjima qiladi. Qulay,
  lekin har gal tarjima qilish kerak, shuning uchun sekin.
- **Kitob tarjimoni** (C — *kompilyator*): kitobni **bir marta, to'liq** tarjima qiladi. Keyin tarjima qilingan
  kitobni istalgancha tez o'qiysiz. Tarjima paytida xato topilsa — kitob umuman chiqmaydi, ya'ni xatolarning bir qismi
  dastur **ishga tushmasdan oldin** topiladi.

C'da shuning uchun ikki bosqich bor: **1) kompilyatsiya** (matn → mashina kodi), **2) ishga tushirish**.

## 0.4. O'rnatish

**Linux (Ubuntu/Debian):**

```bash
sudo apt install build-essential gdb python3
gcc --version        # gcc (Ubuntu ...) 13.x chiqsa - hammasi tayyor
```

- `build-essential` — kompilyator (`gcc`), `make` va boshqa kerakli vositalar to'plami.
- `gdb` — dasturni qadamma-qadam ko'rsatuvchi vosita (keyinroq kerak).

**Windows:** WSL o'rnating (PowerShell'da administrator sifatida: `wsl --install`), kompyuterni qayta yoqing, Ubuntu'ni
oching va yuqoridagi buyruqni bajaring. Kodni Windows muharririda ham yozish mumkin (VS Code + "WSL" kengaytmasi).

**Muharrir:** VS Code (C/C++ kengaytmasi bilan) yoki terminalda `nano`. Muhimi muharrir emas, **har kuni yozish**.

## 0.5. Birinchi dastur

`salom.c` faylini yarating (`nano salom.c`) va quyidagini yozing:

```c
/* salom.c - birinchi dastur */
#include <stdio.h>

int main(void)
{
    printf("Salom, dunyo!\n");
    return 0;
}
```

Kompilyatsiya qilib, ishga tushiring:

```console
$ gcc -Wall -Wextra -g salom.c -o salom
$ ./salom
Salom, dunyo!
```

Dasturingiz ishladi. Endi **har bir belgini** ko'rib chiqamiz — bu 7 qatorda C'ning deyarli hamma asosiy g'oyalari bor.

**Avval umumiy xarita** — har qator nima qiladi (keyin har birini alohida ochamiz):

| Qator | Nima qiladi | Nega kerak |
|---|---|---|
| `/* salom.c - ... */` | izoh | faqat odam uchun eslatma |
| `#include <stdio.h>` | `printf` haqidagi ma'lumotni olib keladi | kompilyator `printf` nimaligini bilishi uchun |
| `int main(void)` | dastur shu yerdan boshlanadi | OS dasturni aynan `main` dan ishga tushiradi |
| `{` ... `}` | `main` ning tanasi | qaysi buyruqlar `main` ga tegishli ekanini ko'rsatadi |
| `printf("Salom, dunyo!\n");` | ekranga yozadi | dasturning asosiy ishi |
| `return 0;` | "hammasi joyida" deb tugaydi | OS natijani bilishi uchun |

**Dastur qanday ishlaydi (vaqt bo'yicha):** 1) OS dasturni xotiraga yuklaydi → 2) `main` ni chaqiradi →
3) `printf` matnni ekranga chiqaradi → 4) `return 0` — `main` tugaydi, `0` OS'ga qaytadi → 5) dastur o'ladi.

### Birinchi qator: `/* ... */` — izoh

`/* salom.c - birinchi dastur */` — **izoh** (comment). Kompilyator uni o'qimaydi, u faqat **odam uchun** yozilgan
eslatma. Python'dagi `#` ga o'xshaydi. Ikki xil yoziladi: `/* ... */` (bir necha qator bo'lishi mumkin) va `// ...` (qator oxirigacha).

### `#include <stdio.h>` — "bu yerga ko'chirib qo'y"

- `#` bilan boshlangan qator — C buyrug'i emas, **preprotsessor** buyrug'i: kompilyatordan **oldin** ishlaydigan "matn
  almashtirgich" (10-bob).
- `include` — "shu faylning mazmunini **shu joyga ko'chirib qo'y**" degani.
- `<stdio.h>` — "standard input/output header": `printf` kabi funksiyalar **haqida ma'lumot** turgan fayl.
  `.h` — "header" (sarlavha) fayl.
- `< >` — "tizim papkalaridan qidir". O'zingizning faylingiz uchun `" "` yoziladi: `#include "mening.h"`.
- Oxirida **`;` yo'q** — chunki bu C buyrug'i emas.

**Nega kerak? Python'da `print` uchun hech narsa yozmasdim-ku.** C'da kompilyator har bir funksiyani ishlatishdan
oldin uning **qanday ekanini** bilishi kerak: nechta narsa oladi, nima qaytaradi. Bu ma'lumot `stdio.h` ichida
turadi, taxminan shunday qatorda: `int printf(const char *format, ...);`. Bu — **e'lon** ("shunday funksiya bor").
Funksiyaning **kodi** esa boshqa joyda (tayyor kutubxonada) turadi va uni keyin **linker** ulaydi (1-bob).

**Qo'ymasam nima bo'ladi?**

```c
/* include_yoq.c - sarlavhasiz printf */
int main(void)
{
    printf("Salom\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra include_yoq.c -o include_yoq # xato kutiladi
include_yoq.c: In function ‘main’:
include_yoq.c:4:5: warning: implicit declaration of function ‘printf’ [-Wimplicit-function-declaration]
    4 |     printf("Salom\n");
      |     ^~~~~~
include_yoq.c:1:1: note: include ‘<stdio.h>’ or provide a declaration of ‘printf’
  +++ |+#include <stdio.h>
    1 | /* include_yoq.c - sarlavhasiz printf */
include_yoq.c:4:5: warning: incompatible implicit declaration of built-in function ‘printf’ [-Wbuiltin-declaration-mismatch]
    4 |     printf("Salom\n");
      |     ^~~~~~
include_yoq.c:4:5: note: include ‘<stdio.h>’ or provide a declaration of ‘printf’
```

`implicit declaration of function 'printf'` — "`printf` ning e'loni yo'q" degani. Eski C da bu faqat ogohlantirish
edi, yangi kompilyatorlarda (GCC 14+) — **xato**.

> **Qo'shimcha:** qaysi ish uchun qaysi `#include` kerakligi, hamda `<stdint.h>`, `<stdlib.h>`, `<string.h>` nima
> ekanligi — [sarlavhalar.md](sarlavhalar.md) sahifasida. Notanish **belgi** uchrasa — [belgilar.md](belgilar.md).

### `int main(void)` — dasturning kirish eshigi

- **`main`** — dasturning **boshlanish nuqtasi**. Dastur ishga tushganda operatsion tizim aynan shu funksiyani chaqiradi.
  Nomi aynan `main` bo'lishi shart.
- **`int`** — funksiya **butun son qaytaradi**. Bu son dasturning **chiqish kodi** (pastda).
- **`(void)`** — "bu funksiya **hech narsa olmaydi**".

**`void` nima?** `void` — "hech narsa". U uch joyda uchraydi va har birida ma'nosi biroz boshqa:

| Yozuv | Ma'nosi |
|---|---|
| `int f(void)` | `f` **hech narsa olmaydi** |
| `void f(int x)` | `f` **hech narsa qaytarmaydi** (Python'da `return` siz funksiya kabi) |
| `void *p` | `p` — **turi noma'lum** xotiraga ko'rsatkich (7-bob). `malloc` shuni qaytaradi |
| `(void)x;` | "`x` ning qiymatini ataylab tashlab yuboryapman" (ishlatilmagan o'zgaruvchi ogohlantirishini o'chirish) |

**`int main()` bilan `int main(void)` farqi nima?** C'da bo'sh qavs `()` "argumentlar haqida hech narsa demayman" degani —
kompilyator istalgan argument bilan chaqirishga ruxsat beradi va **tekshirmaydi**. `(void)` esa aniq "argument yo'q"
degani va noto'g'ri chaqiruvni xato qiladi. **Doim `(void)` yozing.** (C23 standartida `()` ham "yo'q" ma'nosini oldi,
lekin eski kodda farq bor.)

**Buyruq qatori argumentlari kerak bo'lsa?** `int main(int argc, char **argv)`. `argc` — argumentlar soni,
`argv` — ularning matnlari. `./salom a b` da `argc = 3`, `argv[0]` = `"./salom"`, `argv[1]` = `"a"`, `argv[2]` = `"b"` (7-bob).

### `{` va `}` — funksiya tanasi

`{` va `}` funksiyaning **boshi va oxirini** belgilaydi. Ular orasidagi buyruqlar funksiya ishga tushganda bajariladi.
Python'da blokni **chekinish** belgilaydi, C'da esa **faqat `{ }`**. Chekinish C uchun hech narsa anglatmaydi, u
**faqat odam o'qishi uchun**. (Butun dasturni bitta qatorga yozish ham mumkin, lekin yozmang!)

### `printf("Salom, dunyo!\n");` — ekranga chiqarish

- `printf` — "print formatted" (formatlab chiqar). Ekranga matn chiqaradigan tayyor funksiya.
- `( ... )` — funksiyani **chaqirish**; qavs ichida — unga beriladigan narsalar (argumentlar).
- `"Salom, dunyo!\n"` — **satr** (matn). Qo'shtirnoq ichida yoziladi.
- **`\n`** — **yangi qator** belgisi. `\` — "keyingi belgi maxsus ma'noda" (escape). Python'dagi `print` yangi qatorni
  **o'zi qo'shadi**, `printf` esa **qo'shmaydi** — `\n` ni siz yozasiz.
- **`;`** — "bu buyruq tugadi".

Mana `\n` ning farqini ko'ring:

```c
/* yangi_qator.c - \n bor va yo'q */
#include <stdio.h>

int main(void)
{
    printf("a");
    printf("b");
    printf("c\n");
    printf("d\n");
    printf("e\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra yangi_qator.c -o yangi_qator
$ ./yangi_qator
abc
d
e
```

`a`, `b`, `c` bir qatorda chiqdi — chunki ularning orasida `\n` yo'q edi.

**Qatorma-qator:** `printf("a")` — `a` ni chiqardi, kursor **shu qatorda** qoldi (chunki `\n` yo'q).
`printf("b")` — kursor turgan joyga `b` qo'shildi. `printf("c\n")` — `c` chiqdi va **kursor yangi qatorga** tushdi.
Keyingi `d\n` va `e\n` alohida qatorlarga yozildi. Demak: **`\n` = "kursorni keyingi qatorga tushir"**,
u o'zi ekranda ko'rinmaydi.

**Nega `;` kerak? Python'da yo'q-ku.** C kompilyatori uchun yangi qator (Enter) **hech narsa anglatmaydi** — u oddiy
bo'shliq bilan bir xil. Buyruq **qayerda tugashini** kompilyator faqat `;` dan biladi. Shuning uchun bitta buyruqni
bir necha qatorga bo'lish mumkin:

```c
printf("%d %d %d\n",
       birinchi_uzun_ozgaruvchi,
       ikkinchi_uzun_ozgaruvchi,
       uchinchisi);          /* hammasi BITTA buyruq: ; faqat oxirida */
```

Ataylab `;` ni olib tashlab ko'ring:

```c
/* xato1.c - ; unutilgan */
#include <stdio.h>

int main(void)
{
    printf("Salom\n")
    return 0;
}
```

```console
$ gcc -Wall -Wextra xato1.c -o xato1 # xato kutiladi
xato1.c: In function ‘main’:
xato1.c:6:22: error: expected ‘;’ before ‘return’
    6 |     printf("Salom\n")
      |                      ^
      |                      ;
    7 |     return 0;
      |     ~~~~~~            
```

E'tibor bering: kompilyator xatoni **keyingi** qatorda (`return` oldida) ko'rsatyapti. Sabab: u buyruq tugamaganini
faqat keyingi so'zni ko'rganda tushunadi.

> **Eslab qoling:** `expected ';' before ...` xatosini ko'rsangiz — **bir qator yuqoriga** qarang, o'sha yerda `;` unutilgan.

**`;` qo'yilmaydigan joylar:** `#include`/`#define` oxiriga, funksiya tanasining `}` idan keyin, `if`, `for`, `while`
bloklarining `}` idan keyin. **Kutilmagan joyda qo'yiladi:** `struct` ta'rifidagi `}` dan keyin (`struct a { int x; };`)
— nega ekanini 9-bobda ko'rasiz.

### `return 0;` — "hammasi joyida" deb tugatish

`return` — funksiyadan **chiqish** va qiymat qaytarish. `main` uchun bu qiymat — dasturning **chiqish kodi**:
`0` — "hammasi yaxshi", boshqa son — "xato bo'ldi". Terminalda uni `echo $?` ko'rsatadi:

```console
$ ./salom
Salom, dunyo!
$ echo $?
0
```

`main` da `return` yozmasangiz, C99 dan boshlab avtomatik `return 0` bo'ladi. Boshqa funksiyalarda esa `int` qaytarishi
kerak bo'lgan funksiyadan qiymatsiz chiqish — xato.

> **Eslab qoling:** 7 qatorli dasturning har bir belgisi nima uchun borligini bilasiz. Birinchi dasturdan keyin
> ko'p narsa ravshan bo'ldi: **`#include`** — ma'lumot olib keladi, **`main`** — boshlanish, **`{ }`** — tana,
> **`printf(...)`** — chiqarish, **`;`** — buyruq oxiri, **`return 0`** — "yaxshi" deb tugatish.

## 0.6. Kompilyatsiya buyrug'i

```bash
gcc -Wall -Wextra -g salom.c -o salom
```

| Qism | Nima qiladi |
|---|---|
| `gcc` | GNU C kompilyatori — matnni mashina kodiga aylantiruvchi dastur |
| `-Wall -Wextra` | **Hamma ogohlantirishlarni yoqish.** Ular xatolarning 30–50% ini dastur ishga tushmasdan topadi. HECH QACHON o'chirmang |
| `-g` | Debug ma'lumoti (gdb bilan qadamma-qadam ko'rish uchun) |
| `salom.c` | manba fayl (sizning kodingiz) |
| `-o salom` | natija faylning nomi (bo'lmasa `a.out` bo'ladi) |
| `./salom` | joriy papkadagi `salom` ni ishga tushirish. `./` shart: shell dasturlarni faqat maxsus (`PATH`) papkalardan qidiradi |

Yana ikkita foydali bayroq (keyinroq doim ishlatasiz):

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined salom.c -o salom
```

`-fsanitize=address` — xotira xatolarini (massiv chegarasidan chiqish, `free` dan keyin ishlatish) **ushlaydi**.
`-fsanitize=undefined` — "aniqlanmagan xatti-harakat"ni (13-bob) ushlaydi. Dastur biroz sekinlashadi, lekin o'rganish
davrida bu — eng yaxshi o'qituvchi.

## 0.7. Kompilyator xatolarini o'qish

```text
xato1.c:6:5: error: expected ';' before 'return'
    6 |     return 0;
      |     ^~~~~~
```

Format: `fayl:qator:ustun: turi: xabar`. Oltin qoidalar:

1. **Eng birinchi** xatoni o'qing va tuzating. Qolganlari ko'pincha undan kelib chiqadi (bitta `}` unutilsa, 50 ta xato chiqishi mumkin).
2. `warning` (ogohlantirish) ham xato deb hisoblang. Katta loyihalarda `-Werror` ularni xatoga aylantiradi.
3. Xabarni tushunmasangiz — shu qatorga va **bir qator yuqoriga** qarang.

## 0.8. `printf` — birinchi yordamchingiz

```c
/* formatlar.c - printf formatlari */
#include <stdio.h>

int main(void)
{
    int yosh = 20;
    double boy = 1.75;
    char harf = 'A';
    printf("yosh=%d boy=%.2f harf=%c ism=%s\n", yosh, boy, harf, "Ali");
    return 0;
}
```

```console
$ gcc -Wall -Wextra formatlar.c -o formatlar
$ ./formatlar
yosh=20 boy=1.75 harf=A ism=Ali
```

**Kodda nimalar bor:**
- `int yosh = 20;` — `yosh` nomli quti, ichida **butun son**. `int` odatda **4 bayt = 32 bit** joy oladi. Boshlang'ich qiymati — `20` (o'zimiz berdik).
- `double boy = 1.75;` — **kasrli son** uchun quti (`double` — 8 bayt). Boshlang'ich qiymati — `1.75`.
- `char harf = 'A';` — **bitta belgi** uchun quti (1 bayt). Belgi bir tirnoq `' '` ichida yoziladi. Ichida aslida `A` ning kodi (65) turadi.
- `"Ali"` — qo'shtirnoqdagi **matn**; uni to'g'ridan-to'g'ri `printf` ga beramiz, qutiga solish shart emas.

**`printf` qanday ishlaydi:** birinchi argument — **qolip** (`"yosh=%d boy=%.2f ..."`). Qolipdagi har `%...` — bo'sh joy.
Keyingi argumentlar shu bo'sh joylarga **tartib bilan** qo'yiladi: 1-`%d` ← `yosh`, `%.2f` ← `boy`, `%c` ← `harf`, `%s` ← `"Ali"`.

```text
"yosh=%d boy=%.2f harf=%c ism=%s\n"
       |      |        |       |
       v      v        v       v
      yosh    boy     harf    "Ali"       ->  yosh=20 boy=1.75 harf=A ism=Ali
```

`%.2f` dagi `.2` — "nuqtadan keyin **2 ta** raqam" (1.75 → `1.75`, `%f` yolg'iz bo'lsa `1.750000` chiqardi).

`%` — "**bu yerga keyingi argumentni qo'y**" degan belgi (**format belgisi**). Har `%` dan keyingi harf nimani qo'yishni aytadi:

| Format | Nimani qo'yadi | Misol |
|---|---|---|
| `%d` | butun son (`int`) | `42` |
| `%ld` | uzunroq butun son (`long`) | `9000000000` |
| `%u` | ishorasiz butun | |
| `%zu` | `sizeof` natijasi (`size_t`) | |
| `%x` | o'n oltilik son | `ff` |
| `%c` | bitta belgi | `A` |
| `%s` | matn | `Ali` |
| `%p` | manzil (ko'rsatkich) | `0x7ffd5e8e3a4c` |
| `%f` / `%.2f` | kasr son / 2 xonali kasr | `1.750000` / `1.75` |
| `%%` | ekranda oddiy `%` belgisi (pastda tushuntirilgan) | `%` |

**Maxsus holat — `%%`.** Umumiy qoida: `printf` qolipida `%` belgisi "bu yerga qiymat qo'y" degan **buyruq** hisoblanadi. Shuning uchun ekranda **oddiy `%` harfini**
ko'rsatmoqchi bo'lsangiz, uni **ikkita** yozasiz: `%%`. Masalan, `printf("QQS 12%%\n");` ekranga `QQS 12%` chiqaradi. Bu faqat `printf` qolipi ichida;
hisoblashdagi `%` (qoldiq, 3-bob) — butunlay boshqa narsa.

**Muhim:** format bilan argument **turi mos bo'lishi** kerak (`%d` ga `long` berib bo'lmaydi) — aks holda natija
noto'g'ri (aniqlanmagan xatti-harakat). `-Wall` buni ogohlantiradi. Python'dagi f-string kabi avtomatik moslashuv **yo'q**.
Hamma formatlar: [belgilar.md](belgilar.md).

## 0.9. Kitobdagi kod parchalarini qanday sinash kerak

Keyingi boblarda ko'pincha **parcha** uchraydi — to'liq dastur emas, faqat g'oya:

```text
#define N 10
int massiv[N];
```

Bu parchani o'zicha kompilyatsiya qilib bo'lmaydi: `main` yo'q. Uni sinash uchun doim bitta **shablonga** qo'yasiz
(`sinov.c` deb saqlang):

```c
/* sinov.c - parchalarni sinash shabloni */
#include <stdio.h>

/* 1) #define, struct, funksiyalar - SHU YERGA, main dan TASHQARIGA */
#define N 10

int main(void)
{
    /* 2) o'zgaruvchilar va buyruqlar - main ICHIGA */
    int massiv[N];
    for (int i = 0; i < N; i++)
        massiv[i] = i * i;

    /* 3) natijani ko'rish uchun - printf */
    for (int i = 0; i < N; i++)
        printf("massiv[%d] = %d\n", i, massiv[i]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g sinov.c -o sinov
$ ./sinov
massiv[0] = 0
massiv[1] = 1
massiv[2] = 4
massiv[3] = 9
massiv[4] = 16
massiv[5] = 25
massiv[6] = 36
massiv[7] = 49
massiv[8] = 64
massiv[9] = 81
```

**Kodda nimalar bor va nega shu tartibda:**
- `#define N 10` — "kodda `N` ni ko'rsang, `10` deb o'qi". Bu **o'zgaruvchi emas**, matn almashtirish (10-bob). Massiv o'lchamini **bir joyda** saqlash uchun.
- `int massiv[N];` — 10 ta `int` qutisi qatori. Raqamlash **0 dan**: `massiv[0]` … `massiv[9]`. Boshida ichida **tasodifiy qoldiq** (hali qiymat berilmagan!).
- 1-sikl: `i` 0 dan 9 gacha o'sadi; har safar `massiv[i]` ga `i * i` (kvadrat) yoziladi → `0, 1, 4, 9, ...`.
- 2-sikl: o'sha qutilarni ekranga chiqaradi. `%d` ga ikki marta qiymat beramiz: `i` va `massiv[i]`.
- **Nega ikkita sikl?** Avval to'ldiramiz, keyin o'qiymiz — bu har dasturning odatiy shakli: **tayyorla → hisobla → ko'rsat**.

Qiymatlar qanday o'zgaradi (birinchi sikl):

| `i` | bajariladigan qator | `massiv` ning holati |
|---|---|---|
| — | e'lon | `[?, ?, ?, ?, ?, ?, ?, ?, ?, ?]` (hali bo'sh) |
| 0 | `massiv[0] = 0*0` | `[0, ?, ?, ...]` |
| 1 | `massiv[1] = 1*1` | `[0, 1, ?, ...]` |
| 2 | `massiv[2] = 2*2` | `[0, 1, 4, ?, ...]` |
| … | … | … |
| 9 | `massiv[9] = 9*9` | `[0, 1, 4, 9, 16, 25, 36, 49, 64, 81]` |

Endi **o'zgartiring**: `N` ni 5 qiling va qayta yig'ing. Faqat **bitta** qatorni o'zgartirdingiz, lekin massiv ham, ikkala sikl
ham moslashdi. Shunday tajribalar bilan o'rganasiz: har safar natijani **avval taxmin qiling**, keyin tekshiring.
Taxmin noto'g'ri chiqqan joy — siz hali tushunmagan joy.

Har bobda **tayyor to'liq dastur** ham bor: [misollar/](misollar/README.md). Har birining boshida — ishga tushirish buyrug'i,
kutilgan natija va "Sinab ko'ring" topshiriqlari.

## Hayotdan misol va to'liq dastur

Birinchi dasturdan keyingi qadam — **o'zgaruvchi**dan foydalanish: ma'lumotni nom bilan saqlash (2-bob). Hozircha shuni bilsangiz kifoya:
`int yosh = 25;` — "`yosh` nomli quti yarat va ichiga 25 ni qo'y".

**Hayotdan misol: yoshni hisoblash.** Kassir chek yozmoqda: qiymatlarni bir joyda saqlaydi, keyin ularga qarab hisoblaydi.

```c
/* yosh.c - tug'ilgan yildan yoshni hisoblash */
#include <stdio.h>

int main(void)
{
    int joriy_yil = 2026;
    int tugilgan_yil = 2001;
    int yosh = joriy_yil - tugilgan_yil;       /* qutilardagi sonlarni ayirdik */

    printf("Siz %d yoshdasiz.\n", yosh);
    printf("Bu taxminan %d oy yoki %d kun.\n", yosh * 12, yosh * 365);
    printf("100 yoshga %d yil qoldi.\n", 100 - yosh);
    return 0;
}
```

```console
$ gcc -Wall -Wextra yosh.c -o yosh
$ ./yosh
Siz 25 yoshdasiz.
Bu taxminan 300 oy yoki 9125 kun.
100 yoshga 75 yil qoldi.
$ echo "chiqish kodi: $?"
chiqish kodi: 0
```

**Kodda nimalar bor:** uchta `int` quti. Har biri 4 bayt (32 bit), ichiga butun son sig'adi.

| Quti | Nima uchun | Boshlang'ich qiymati |
|---|---|---|
| `joriy_yil` | hozirgi yil | `2026` (e'londa berdik) |
| `tugilgan_yil` | tug'ilgan yil | `2001` |
| `yosh` | natija | `joriy_yil - tugilgan_yil` = `2026 - 2001` = `25` (e'lon paytida **hisoblab** qo'yamiz) |

Dasturni qadam-baqadam tushuntiramiz:

1. Uchta "quti" yaratildi: `joriy_yil` (2026), `tugilgan_yil` (2001), `yosh` (ikkisining farqi — 25).
2. `printf` har safar `%d` o'rniga keyingi qutidagi (yoki hisoblangan) sonni qo'ydi.
3. `return 0` — "hammasi joyida": `echo $?` buni `0` deb ko'rsatdi.

**Sinab ko'ring:** `tugilgan_yil` ni o'zingiznikiga o'zgartiring. `return 0;` ni `return 3;` qilib, `echo $?` nima
ko'rsatishini tekshiring. Ataylab `;` lardan birini o'chirib, kompilyator xabarini o'qing.

## Bob xulosasi (yodlash uchun)

1. Kompyuter — **protsessor** (tez xodim) + **xotira** (raqamlangan baytlar qatori). Hamma narsa — sonlar.
2. **Bit** — bitta kalit (0/1), **bayt** — 8 ta kalit (0…255).
3. C — kompyuterga yaqin til: xotira ko'rinadi, yashirin ish yo'q. Shuning uchun yadrolar C'da yoziladi.
4. C **kompilyator** bilan ishlaydi: avval butun matn tarjima qilinadi (`gcc`), keyin dastur ishlaydi (`./dastur`).
5. Birinchi dastur: `#include` (ma'lumot olib keladi) + `int main(void)` (kirish eshigi) + `{ }` (tana) + `printf(...);` + `return 0;`.

## Savol-javob

**Nega `main` dan qaytgan son "chiqish kodi" bo'ladi?**
Dastur tugagach, operatsion tizim (shell) uning natijasini so'raydi. `make`, shell skriptlari, `&&` hammasi shu kodga qaraydi:
`gcc x.c && ./a.out` — faqat gcc `0` qaytarsagina dastur ishga tushadi.

**`stdio.h` ni ochib ko'rsam bo'ladimi?**
Ha: `less /usr/include/stdio.h`. Murakkab ko'rinadi, lekin ichidan `printf` e'lonini topa olasiz. MyOS'ning o'z `stdio.h` i
ancha sodda: `user/include/stdio.h`.

**Dastur ekranga qanday yozadi?**
`printf` → matnni vaqtinchalik joyga yig'adi → `write(1, matn, uzunlik)` **tizim chaqiruvi** → yadro → terminal drayveri → ekran.
MyOS'da bu yo'lning har qadamini o'qishingiz mumkin. 14-bobda buni o'zingiz qilasiz.

**Nega xato xabari bir qator pastda ko'rsatiladi?**
Kompilyator `;` yo'qligini faqat keyingi so'zni ko'rganda payqaydi. Shuning uchun xato "keyingi so'zda" deb xabar qilinadi.

## O'zingizni tekshiring

1. `#include` qatori oxirida nega `;` yo'q?
2. `printf("a"); printf("b\n");` nima chiqaradi? `\n` bo'lmasa-chi?
3. `int main(void)` dagi `int` va `void` nimani bildiradi?
4. `error: expected ';' before '}'` — xato qayerda bo'lishi mumkin?
5. `./salom` dagi `./` nega kerak?
6. Bir bayt nechta bit? U qanday sonlarni saqlay oladi?

<details><summary>Javoblar</summary>

1. `#include` — preprotsessor buyrug'i, C buyrug'i emas; u qator oxirigacha davom etadi.
2. `ab` va yangi qator. `\n` bo'lmasa, keyingi chiqish yoki terminal taklifi shu qatorda davom etadi.
3. `int` — qaytish turi (chiqish kodi, butun son), `void` — argument yo'q.
4. `}` dan oldingi buyruqda (odatda bir qator yuqorida) `;` unutilgan.
5. Shell dasturni faqat `PATH` dagi papkalarda qidiradi; joriy papka odatda unda yo'q, shuning uchun `./` bilan aniq yo'l beriladi.
6. 8 bit; 0 dan 255 gacha (256 xil son).
</details>

## Mashq

- `salom.c` ni o'zgartiring: ismingiz va yoshingizni `printf` bilan chiqaring.
- `;` ni ataylab o'chirib, xato xabarini o'qing. `}` ni o'chirib ko'ring. `#include` ni o'chirib ko'ring. Har safar xabarni **o'zingiz tushuntiring**.
- `return 3;` qilib, `./salom; echo $?` bilan chiqish kodini ko'ring.

<!-- loyiha:boshi -->
## Loyiha: do'kon cheki

**Maqsad:** printf va oddiy hisob-kitob bilan chiroyli, ustunlari tekis chek chop etish.
**Bobdan ishlatiladi:** `int`, arifmetika, `printf` formatlari (`%-12s`, `%7d`).

### Loyihalash usuli (har bir loyihada shu 5 qadam)

1. **Talab:** dastur nima chiqarishi kerak? Natijani avval qog'ozga chizib oling.
2. **Ma'lumotlar:** qaysi qiymatlar kerak? Har biriga o'zgaruvchi.
3. **Qadamlar:** qaysi tartibda hisoblanadi? (avval yig'indi, keyin soliq...)
4. **Kod:** qadamlarni koddga aylantiring.
5. **Sinov:** natijani qog'ozdagi bilan solishtiring.

**Talab:** 3 xil mahsulot (nomi, narxi, soni) — jadval, jami summa va 12% QQS.
**Ma'lumotlar:** har mahsulot uchun `narx` va `soni`; `jami`, `qqs`.
**Qadamlar:** jami = narx×soni yig'indisi → qqs = jami × 12 / 100 → chiqarish.

```c
/* chek.c - do'kon cheki */
#include <stdio.h>

int main(void)
{
    int non_narx = 4000, non_soni = 3;
    int sut_narx = 12000, sut_soni = 1;
    int guruch_narx = 18000, guruch_soni = 2;

    int jami = non_narx * non_soni + sut_narx * sut_soni + guruch_narx * guruch_soni;
    int qqs = jami * 12 / 100;

    printf("==================================\n");
    printf("%-12s %7s %4s %8s\n", "Mahsulot", "Narx", "Soni", "Summa");
    printf("----------------------------------\n");
    printf("%-12s %7d %4d %8d\n", "Non", non_narx, non_soni, non_narx * non_soni);
    printf("%-12s %7d %4d %8d\n", "Sut", sut_narx, sut_soni, sut_narx * sut_soni);
    printf("%-12s %7d %4d %8d\n", "Guruch", guruch_narx, guruch_soni, guruch_narx * guruch_soni);
    printf("----------------------------------\n");
    printf("%-25s %8d\n", "Jami:", jami);
    printf("%-25s %8d\n", "shundan QQS (12%):", qqs);
    printf("==================================\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra chek.c -o chek
$ ./chek
==================================
Mahsulot        Narx Soni    Summa
----------------------------------
Non             4000    3    12000
Sut            12000    1    12000
Guruch         18000    2    36000
----------------------------------
Jami:                        60000
shundan QQS (12%):            7200
==================================
```

`%-12s` — matnni 12 belgi kenglikda **chapga** tekislaydi, `%7d` — sonni 7 belgi kenglikda **o'ngga**.
Ustunlar tekis chiqishining sababi shu.

**Kengaytiring:** to'rtinchi mahsulot qo'shing. `qqs` ni 15% qiling. Nega `jami * 12 / 100` ni
`jami / 100 * 12` deb yozsak natija boshqacha chiqishi mumkin? (Butun bo'lish qoldiqni tashlaydi — 3-bob.)

## Mustaqil loyiha: yo'l xarajati kalkulyatori ★☆☆

**Vazifa:** do'stlar bilan sayohatga chiqasiz. Yo'lga qancha pul ketishini va har biringizga qancha
tushishini hisoblaydigan dastur yozing. Fayl nomi: `yol.c`.

**Ma'lumotlar** (kodning boshida o'zgaruvchi qilib yozing):
- masofa — 250 km
- yoqilg'i sarfi — 100 km ga 8 litr
- 1 litr narxi — 9000 so'm
- yo'l to'lovi — 15000 so'm, 2 marta (borishda va qaytishda)
- odamlar soni — 4

**Talab:** dastur hech narsa so'ramaydi, faqat hisoblab quyidagini chiqaradi:

- Yorliq — `%-18s` (chapga tekis), qiymat — `%8d` (o'ngga tekis), keyin birlik.
- Litrlar = masofa × sarf / 100.

**Kutilgan natija** (`darslik/loyihalar/00_chek/kutilgan.txt`):

```text
Masofa:                250 km
Yoqilg'i:               20 litr
Yoqilg'i puli:      180000 so'm
Yo'l to'lovi:        30000 so'm
Jami:               210000 so'm
Har bir kishiga:     52500 so'm
```

**Qo'shimcha sinov.** Ma'lumotlarni o'zgartiring: masofa 400, sarf 6, litr narxi 10000, to'lov 20000
× 3 marta, 5 kishi. Natija:

```text
Masofa:                400 km
Yoqilg'i:               24 litr
Yoqilg'i puli:      240000 so'm
Yo'l to'lovi:        60000 so'm
Jami:               300000 so'm
Har bir kishiga:     60000 so'm
```

**Maslahat** (yechim emas):
- Avval qog'ozda hisoblang: litrlar → yoqilg'i puli → to'lovlar → jami → har kishiga.
- Har bir oraliq natijaga alohida o'zgaruvchi bering — kodni o'qish oson bo'ladi.
- `printf` da `'` belgisi matn ichida oddiy belgi (`"Yo'l"`), maxsus emas.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g yol.c -o yol && ./yol | diff - ~/C_loyha/darslik/loyihalar/00_chek/kutilgan.txt && echo "TO'G'RI"
```

`diff` hech narsa chiqarmasa va "TO'G'RI" yozilsa — natija harfma-harf bir xil. Farq bo'lsa, `diff` qaysi
qator boshqacha ekanini ko'rsatadi (`<` — sizniki, `>` — kutilgani).
<!-- loyiha:oxiri -->

Keyingi bob: [1-bob. Kompilyatsiya: koddan dasturgacha](01-kompilyatsiya.md)
