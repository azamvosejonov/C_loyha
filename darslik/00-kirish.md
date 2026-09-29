# 0-bob. Kirish: C nima va birinchi dastur

> **Bu bobdan keyin:** kompyuteringizda C dasturini yozib, kompilyatsiya qilib, ishga tushira olasiz
> va eng birinchi dasturdagi **har bir belgining** nima qilishini bilasiz.

> **To'liq ishlaydigan misol:** [misollar/00_salom.c](misollar/00_salom.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

Bu bobning asosiy g'oyalari kundalik hayotdagi narsalar orqali. Avval shuni o'qing — keyingi
bo'limlardagi texnik tafsilotlar ancha oson tushuniladi.

**Kompyuter — ish stolidagi xodim (0.1).** Tasavvur qiling: bir xodim stol ustida ishlaydi.
- **Protsessor (CPU)** — xodimning o'zi. U juda tez, lekin juda oddiy ishlarni qila oladi: "ikki sonni qo'sh",
  "bu sonni u yerga yoz", "agar nol bo'lsa, 5-qadamga o't".
- **Operativ xotira (RAM)** — stol usti. Hozir ishlayotgan qog'ozlar shu yerda. Tez qo'l yetadi, lekin
  chiroq o'chsa (kompyuter o'chsa) stol tozalanadi.
- **Disk** — arxiv shkafi. Hamma narsa saqlanib qoladi, lekin undan qog'oz olib kelish sekin.
- **Dastur** — xodimga berilgan ko'rsatmalar ro'yxati. Xodim ro'yxatni tepadan pastga, qadamma-qadam bajaradi.

**Kompilyator — kitob tarjimoni (0.2, 0.3).** Python — **sinxron tarjimon**: siz har safar dasturni
ishga tushirganingizda, u har bir qatorni o'sha zahoti tarjima qiladi. Qulay, lekin sekin. C esa
**kitob tarjimoni**: kitob (dastur) bir marta to'liq tarjima qilinadi (`gcc`) va keyin istalgancha
tez o'qiladi (`./dastur`). Tarjima paytida xato topilsa, kitob umuman chiqmaydi — shuning uchun C
xatolarni ishga tushirishdan **oldin** aytadi.

**`#include <stdio.h>` — ma'lumotnomani stolga qo'yish (0.5).** Siz `printf` ni o'zingiz yozmaysiz.
U tayyor, faqat kompilyatorga "printf degan narsa bor, u shunday ishlatiladi" deb aytish kerak. Bu —
kerakli ma'lumotnomani ish boshlashdan oldin stolga qo'yib qo'yish.

**`main` — uyning kirish eshigi (0.5).** Uyda o'nlab xona bo'lishi mumkin, lekin mehmon doim eshikdan
kiradi. Dasturda ham funksiyalar ko'p bo'lishi mumkin, lekin bajarilish doim `main` dan boshlanadi.

**`return 0;` — ishni topshirish (0.5).** Xodim ishni tugatib, rahbarga "hammasi joyida" (0) yoki "muammo
bo'ldi" (0 emas) deb xabar beradi. Terminal bu xabarni `echo $?` bilan ko'rsatadi.

**`;` — gap oxiridagi nuqta.** O'zbek tilida gap nuqta bilan tugaydi. C'da har bir buyruq `;` bilan
tugaydi. Nuqtani unutsangiz, kompilyator ikki gapni bitta deb o'qiydi va tushunmaydi.

### To'liq dastur: yoshni hisoblash

Birinchi dasturdan keyingi qadam: o'zgaruvchilar, hisoblash va `printf` ning turli formatlari.

```c
/* yosh.c - tug'ilgan yildan yoshni hisoblash */
#include <stdio.h>

int main(void)
{
    int joriy_yil = 2026;
    int tugilgan_yil = 2001;
    int yosh = joriy_yil - tugilgan_yil;

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

**Sinab ko'ring:** `tugilgan_yil` ni o'zingiznikiga o'zgartiring. `return 0;` ni `return 3;` qilib,
`echo $?` nima ko'rsatishini tekshiring. `;` lardan birini o'chirib, kompilyator xatosini o'qing.

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

## 0.9. Kitobdagi kod parchalarini qanday sinash kerak

Keyingi boblarda ko'pincha **parcha** ko'rasiz — to'liq dastur emas, faqat g'oya:

```c
#define N 10
int massiv[N];
```

Bu parchani o'zi kompilyatsiya qilib bo'lmaydi: unda `main` yo'q. Uni sinash uchun doim bitta
**shablonga** qo'yasiz (`sinov.c` deb saqlang):

```c
#include <stdio.h>

/* 1) #define, struct, funksiyalar - shu yerga, main dan TASHQARIGA */
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

```bash
gcc -Wall -Wextra -g sinov.c -o sinov && ./sinov
```

Endi **o'zgartiring**: `N` ni 5 qiling, yana yig'ing — faqat bitta qatorni o'zgartirdingiz, lekin massiv
ham, ikkala sikl ham moslashdi. Aynan shunday tajribalar bilan o'rganasiz: har safar natijani **avval
taxmin qiling**, keyin tekshiring. Taxmin noto'g'ri chiqqan joy — siz hali tushunmagan joy.

Kompilyator `implicit declaration of function 'strlen'` desa — kerakli `#include` yetishmayapti
(`strlen` → `<string.h>`, `malloc` → `<stdlib.h>`). Qaysi biri kerakligini `man 3 strlen` aytadi.

Bundan tashqari, **har bir bob uchun tayyor to'liq dastur** bor: [misollar/](misollar/README.md). Har
birining boshida — ishga tushirish buyrug'i, kutilgan natija va "Sinab ko'ring" topshiriqlari.

## 0.10. Savol-javob

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

## 0.11. O'zingizni tekshiring

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

## 0.12. Mashq

- `salom.c` ni o'zgartiring: ismingiz va yoshingizni `printf` bilan chiqaring.
- `;` ni ataylab o'chirib, xato xabarini o'qing. `}` ni o'chirib ko'ring. `#include` ni o'chirib ko'ring.
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
