# 0-bob. Kirish: C nima va birinchi dastur

> **Bu bobdan keyin:** kompyuteringizda C dasturini yozib, kompilyatsiya qilib, ishga tushira olasiz
> va eng birinchi dasturdagi **har bir belgining** nima qilishini bilasiz.

## 0.1. Kompyuter aslida nima qiladi

Kompyuterni juda soddalashtirib tasavvur qilsak, unda ikkita asosiy qism bor:

- **Protsessor (CPU)** — buyruqlarni bajaradi. U juda oddiy buyruqlarni tushunadi: "shu
  manzildagi sonni o'qi", "ikki sonni qo'sh", "natijani shu manzilga yoz", "agar nol bo'lsa,
  boshqa joyga sakra". Sekundiga milliardlab.
- **Xotira (RAM)** — juda uzun baytlar qatori. Har bir baytning **manzili** (raqami) bor:
  0, 1, 2, ... 8 GB RAM'da taxminan 8 milliard bayt.

```text
manzil:  1000  1001  1002  1003  1004  1005 ...
qiymat: [ 72 ][ 105][  0 ][ 255][ 17 ][ 3  ]...
```

Hamma narsa — sonlar, harflar, rasmlar, dasturning o'zi — shu baytlarda turadi. CPU tushunadigan
buyruqlar ham xotiradagi baytlar (**mashina kodi**).

## 0.2. C tili qayerda turadi

```text
Python     ← "yuqori daraja": xotira, turlar, manzillar yashirilgan
  C        ← xotira va manzillar ko'rinib turadi, lekin yozish qulay
Assembly   ← har bir CPU buyrug'i alohida yoziladi
Mashina kodi ← CPU bajaradigan baytlar
```

C — **"ko'chma assembly"**. C'da `a = b + c;` deb yozasiz, kompilyator uni 2–3 ta CPU buyrug'iga
aylantiradi. Siz nimani yozsangiz, CPU deyarli aynan shuni bajaradi. Yashirin ish yo'q: garbage
collector yo'q, avtomatik tekshiruvlar yo'q.

**Nega yadrolar C'da yoziladi (Linux, Windows yadrosi, macOS'ning XNU'si, MyOS):**

1. **Xotirani to'liq boshqarish.** Yadro "`0xB8000` manzilga shu baytni yoz" deya olishi kerak.
   Python buni qila olmaydi.
2. **Yashirin ish yo'q.** Python'da `a = [1, 2]` qatori ortida yuzlab C qatorlari ishlaydi. Yadroda
   har bir amal hisobda bo'lishi kerak: uzilish (interrupt) ichida kutilmagan xotira ajratish
   tizimni qulatadi.
3. **Hech narsaga tayanmaydi.** Python dasturini ishga tushirish uchun Python interpretatori kerak,
   interpretatorga esa operatsion tizim kerak. C kodi esa "yalang'och" apparatda ishlay oladi —
   yadro aynan shunday bo'lishi shart.
4. **Tezlik** — CPU'ga eng yaqin til (assembly'dan tashqari).

## 0.3. Python va C: asosiy farqlar

| Python | C | Nima uchun C'da shunday |
|---|---|---|
| `x = 5` — tur yozilmaydi | `int x = 5;` — tur **majburiy** | Kompilyator o'zgaruvchi uchun necha bayt ajratishni bilishi kerak |
| Son cheksiz o'sadi | `int` — 32 bit, **toshadi** | CPU registri qat'iy o'lchamda |
| Satr — obyekt, uzunligini biladi | Satr — `'\0'` bilan tugaydigan baytlar | Eng oddiy tasvir, qo'shimcha xotirasiz |
| `list` o'zi o'sadi | Massiv o'lchami **qat'iy** | Dinamik o'sishni o'zingiz yozasiz (13-mashq) |
| Xotira avtomatik tozalanadi | `malloc` / `free` — **qo'lda** | Garbage collector yo'q |
| Xato → tushunarli exception | Xato → dastur qulaydi yoki **jim** buziladi | Tekshiruvlar yo'q (tezlik uchun) |
| Bloklar — chekinish (indent) bilan | Bloklar — `{ }` bilan | Bo'shliq va yangi qator C uchun ahamiyatsiz |
| Qator oxiri — buyruq oxiri | `;` — buyruq oxiri | (pastda batafsil) |
| Interpretator qatorma-qator bajaradi | Kompilyator avval **hammasini** mashina kodiga aylantiradi | Tez ishlaydi, xatolarning bir qismi ishga tushirishdan oldin topiladi |

## 0.4. O'rnatish

**Linux (Ubuntu/Debian):**

```bash
sudo apt install build-essential gdb python3
gcc --version        # gcc (Ubuntu ...) 13.x - ishlayapti
```

**Windows:** WSL o'rnating (PowerShell'da administrator sifatida: `wsl --install`), qayta yuklang,
Ubuntu'ni oching va yuqoridagi buyruqni bajaring. Kodni Windows'dagi muharrirda ham yozishingiz
mumkin (VS Code + "WSL" kengaytmasi).

**Muharrir:** VS Code (C/C++ kengaytmasi bilan) yoki terminalda `nano`, `vim`. Muhim emas —
muhimi har kuni yozish.

## 0.5. Birinchi dastur

`salom.c` faylini yarating:

```c
#include <stdio.h>

int main(void)
{
    printf("Salom, dunyo!\n");
    return 0;
}
```

Kompilyatsiya qiling va ishga tushiring:

```bash
gcc -Wall -Wextra -g salom.c -o salom
./salom
```

Natija: `Salom, dunyo!`

Endi **har bir belgini** ko'rib chiqamiz. Bu 6 qatorda C'ning deyarli hamma asosiy g'oyalari bor.

### `#include <stdio.h>`

- `#` — bu qator **C kodi emas**, balki **preprotsessor** buyrug'i. Preprotsessor kompilyatordan
  OLDIN ishlaydigan "matn almashtirgich" (10-bob).
- `include` — "shu faylning mazmunini shu yerga **ko'chirib qo'y**".
- `<stdio.h>` — "standard input/output header": `printf`, `scanf`, `fopen` kabi funksiyalarning
  **e'lonlari** shu faylda. `.h` — "header" (sarlavha) fayli.
- `< >` — "tizim papkalaridan qidir" (`/usr/include/stdio.h`). O'z fayllaringiz uchun `" "`:
  `#include "mening.h"`.
- **Oxirida `;` yo'q** — chunki bu C buyrug'i emas.

**Savol: nega kerak? Python'da `print` ni hech narsa import qilmasdan ishlataman.**
C'da kompilyator har bir funksiyani ishlatishdan oldin uning **qanday ekanini** bilishi kerak:
qancha argument oladi, qanday turlar, nima qaytaradi. `stdio.h` ichida taxminan shunday qator bor:
`int printf(const char *format, ...);`. Bu **e'lon** (declaration) — "shunday funksiya bor". Uning
kodi esa boshqa joyda (libc kutubxonasida) va uni **linker** ulaydi (1-bob).

**Qo'yilmasa nima bo'ladi:** `error: implicit declaration of function 'printf'`. Eski C
standartlarida bu faqat ogohlantirish edi va kompilyator "u int qaytaradi" deb taxmin qilardi. Bu
ko'p xatolarning manbai bo'lgani uchun zamonaviy GCC'da xato.

### `int main(void)`

- `main` — dasturning **kirish nuqtasi**. Operatsion tizim dasturni ishga tushirganda aynan shu
  funksiyani chaqiradi. Nomi aynan `main` bo'lishi shart.
- `int` — funksiya **butun son qaytaradi**. Bu son dasturning **chiqish kodi**: 0 — "hammasi
  yaxshi", boshqa son — "xato bo'ldi". Shell'da `echo $?` uni ko'rsatadi.
- `(void)` — "bu funksiya **hech qanday argument olmaydi**".

**Savol: `void` nima?**
`void` — "hech narsa" yoki "turi yo'q" degani. U uch joyda keladi va har birida ma'nosi biroz
boshqacha:

| Yozuv | Ma'nosi |
|---|---|
| `int f(void)` | f **argument olmaydi** |
| `void f(int x)` | f **hech narsa qaytarmaydi** (Python'dagi `return` siz funksiya kabi) |
| `void *p` | p — **turi noma'lum** xotiraga ko'rsatkich (7-bob). `malloc` shuni qaytaradi |
| `(void)x;` | "x ning qiymatini ataylab tashlab yuboryapman" (ishlatilmagan o'zgaruvchi ogohlantirishini o'chirish) |

**Savol: `int main()` va `int main(void)` farqi?**
C'da (C++'dan farqli) bo'sh qavslar `()` "argumentlar haqida hech narsa demayman" degani —
kompilyator istalgan argumentlar bilan chaqirishga ruxsat beradi va tekshirmaydi. `(void)` —
aniq "argument yo'q", noto'g'ri chaqiruvda xato beradi. Doim `(void)` yozing. (C23 standartida
`()` ham "argument yo'q" deb o'zgartirildi, lekin eski kodda farq bor.)

**Savol: buyruq qatori argumentlari kerak bo'lsa?**
`int main(int argc, char **argv)` — `argc` — argumentlar soni, `argv` — ular satrlari massivi.
`./salom a b` da `argc = 3`, `argv[0] = "./salom"`, `argv[1] = "a"`, `argv[2] = "b"`. (7-bob.)

### `{` va `}`

- Funksiya **tanasining** boshi va oxiri. Ular orasida — funksiya bajaradigan buyruqlar.
- Python'da chekinish (indent) blokni belgilaydi, C'da — **faqat** `{ }`. Chekinish C uchun hech
  narsa anglatmaydi, faqat odam o'qishi uchun. Butun dasturni bitta qatorga yozish mumkin
  (lekin yozmang!).

### `printf("Salom, dunyo!\n");`

- `printf` — "print formatted": formatlangan chiqarish funksiyasi.
- `( ... )` — funksiyani **chaqirish**; qavs ichida — argumentlar.
- `"Salom, dunyo!\n"` — **satr literali**. Xotirada: `S a l o m , _ d u n y o ! \n \0` —
  oxirida kompilyator avtomatik qo'shadigan **nol bayt** (`'\0'`) bor. U satr qayerda
  tugashini bildiradi (6-bob).
- `\n` — **yangi qator** belgisi. `\` — "maxsus belgi keladi" (escape). Python'dagi `print`
  yangi qatorni o'zi qo'shadi, `printf` esa **qo'shmaydi**: `\n` siz keyingi chiqish shu qatorda
  davom etadi.
- `;` — **buyruq tugadi**.

**Savol: nega `;` kerak? Python'da yo'q-ku.**
C kompilyatori uchun yangi qator (Enter) **hech narsa anglatmaydi** — u bo'shliq bilan bir xil.
Buyruq qayerda tugashini kompilyator faqat `;` dan biladi. Shuning uchun bitta buyruqni bir
necha qatorga bo'lish mumkin:

```c
printf("%d %d %d\n",
       birinchi_uzun_ozgaruvchi,
       ikkinchi_uzun_ozgaruvchi,
       uchinchisi);          /* hammasi BITTA buyruq - ; faqat oxirida */
```

**Qo'yilmasa:** `error: expected ';' before 'return'`. E'tibor bering: xato **keyingi** qatorda
ko'rsatiladi, chunki kompilyator buyruq tugamaganini faqat keyingi so'zni (`return`) ko'rganda
tushunadi. Qoida: "`expected ';'`" xatosini ko'rsangiz — **bir qator yuqoriga** qarang.

**`;` qo'yilMAYdigan joylar:** `#include`/`#define` oxiri, funksiya tanasining `}` idan keyin,
`if (...) { }` / `for (...) { }` / `while (...) { }` bloklaridan keyin.
**Albatta qo'yiladigan kutilmagan joy:** `struct` ta'rifining `}` idan keyin (`struct a { int x; };`)
— 9-bobda nega ekanini ko'rasiz.

### `return 0;`

- Funksiyadan **chiqish** va qiymat qaytarish. `main` uchun — dasturning chiqish kodi.
- `main` da `return` yozilmasa, C99'dan boshlab avtomatik `return 0` bo'ladi. Boshqa
  funksiyalarda esa `int` qaytarishi kerak bo'lgan funksiyadan qiymatsiz chiqish — xato.

## 0.6. Kompilyatsiya buyrug'i

```bash
gcc -Wall -Wextra -g salom.c -o salom
```

| Qism | Ma'nosi |
|---|---|
| `gcc` | GNU C kompilyatori |
| `-Wall -Wextra` | **Hamma ogohlantirishlarni yoqish.** Ular xatolarning 30–50% ini dastur ishga tushmasdan topadi. HECH QACHON o'chirmang |
| `-g` | Debug ma'lumoti (gdb bilan qadamma-qadam ko'rish uchun) |
| `salom.c` | Manba fayl |
| `-o salom` | Natija fayl nomi (bo'lmasa `a.out`) |
| `./salom` | Joriy papkadagi `salom` dasturini ishga tushirish. `./` shart: shell dasturlarni faqat `PATH` dagi papkalardan qidiradi |

Yana ikkita juda foydali bayroq (18-mashqdan boshlab doim ishlatasiz):

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined salom.c -o salom
```

`-fsanitize=address` — xotira xatolarini **ushlaydi** (massiv chegarasidan chiqish, `free` dan
keyin foydalanish). `-fsanitize=undefined` — aniqlanmagan xatti-harakatni (13-bob). Dastur
biroz sekinlashadi, lekin o'rganish davrida bu eng yaxshi o'qituvchi.

## 0.7. Kompilyator xatolarini o'qish

```text
salom.c:5:5: error: expected ';' before 'return'
    5 |     return 0;
      |     ^~~~~~
```

`fayl:qator:ustun: turi: xabar`. Oltin qoidalar:

1. **Eng birinchi xatoni** o'qing va tuzating. Qolganlari ko'pincha undan kelib chiqqan
   (bitta `}` unutilsa, 50 ta xato chiqishi mumkin).
2. `warning` ham xato deb hisoblang. Loyihada `-Werror` shuni majbur qiladi.
3. Xabarni tushunmasangiz — aynan shu qatorni va **bir qator yuqorisini** qarang.

## 0.8. `printf` — birinchi yordamchingiz

```c
int yosh = 20;
double boy = 1.75;
char harf = 'A';
printf("yosh=%d boy=%.2f harf=%c ism=%s\n", yosh, boy, harf, "Ali");
/* yosh=20 boy=1.75 harf=A ism=Ali */
```

`%` — "bu yerga keyingi argumentni qo'y" degan **format belgisi**:

| Format | Tur | Misol |
|---|---|---|
| `%d` | `int` | `42` |
| `%ld` | `long` | `9000000000` |
| `%u` | `unsigned int` | |
| `%zu` | `size_t` | `sizeof` natijasi |
| `%x` | o'n oltilik | `ff` |
| `%c` | bitta belgi (`char`) | `A` |
| `%s` | satr (`char *`) | `Ali` |
| `%p` | ko'rsatkich (manzil) | `0x7ffd5e8e3a4c` |
| `%f` | `double` | `1.750000` |
| `%%` | `%` belgisining o'zi | |

**Muhim:** format va argument turi mos kelmasa — **aniqlanmagan xatti-harakat** (masalan, `%d`
ga `long` berish). `-Wall` buni ogohlantiradi. Python'dagi f-string kabi avtomatik o'giriш yo'q.

## 0.9. Savol-javob

**Nega `main` dan qaytgan son "chiqish kodi" bo'ladi?**
Operatsion tizim (shell) dasturni ishga tushiradi va u tugaganda natijasini so'raydi. `make`,
shell skriptlari, `&&` hammasi shu kodga qaraydi: `gcc x.c && ./a.out` — gcc 0 qaytarsagina
dastur ishga tushadi.

**`stdio.h` ni ochib ko'rsam bo'ladimi?**
Ha: `less /usr/include/stdio.h`. Juda murakkab ko'rinadi, lekin ichida `printf` e'lonini topa
olasiz. MyOS'ning o'z `stdio.h` i ancha sodda: `user/include/stdio.h`.

**Dastur qanday qilib ekranga yozadi?**
`printf` → matnni buferga yig'adi → `write(1, buf, n)` **tizim chaqiruvi** → yadro → terminal
drayveri → ekran. MyOS'da bu yo'lning har bir qadamini o'qishingiz mumkin (`user/bin/hello.c`
ning boshidagi izohga qarang). 14-bobda buni o'zingiz qilasiz.

## 0.10. O'zingizni tekshiring

1. `#include` qatorining oxirida nega `;` yo'q?
2. `printf("a"); printf("b\n");` nima chiqaradi? `\n` bo'lmasa-chi?
3. `int main(void)` dagi `int` va `void` nimani bildiradi?
4. `error: expected ';' before '}'` — xato qayerda bo'lishi mumkin?
5. `./salom` dagi `./` nega kerak?

<details><summary>Javoblar</summary>

1. `#include` — preprotsessor buyrug'i, C buyrug'i emas; preprotsessor qatorni yangi qator bilan tugaydi deb hisoblaydi.
2. `ab` va yangi qator. `\n` bo'lmasa, `ab` va keyingi chiqish (shell taklifi) shu qatorda davom etadi.
3. `int` — qaytish turi (chiqish kodi), `void` — argument yo'q.
4. `}` dan oldingi buyruqda (odatda bir qator yuqorida) `;` unutilgan.
5. Shell dasturni faqat `PATH` papkalarida qidiradi; joriy papka odatda unda yo'q.
</details>

## 0.11. Mashq

- `salom.c` ni o'zgartiring: ismingiz va yoshingizni `printf` bilan chiqaring.
- `;` ni ataylab o'chirib, xato xabarini o'qing. `}` ni o'chirib ko'ring. `#include` ni o'chirib ko'ring.
- `return 3;` qilib, `./salom; echo $?` bilan chiqish kodini ko'ring.

Keyingi bob: [1-bob. Kompilyatsiya: koddan dasturgacha](01-kompilyatsiya.md)
