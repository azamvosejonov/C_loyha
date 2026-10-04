# 1-bob. Kompilyatsiya: koddan dasturgacha

> **Bu bobda nima o'rganasiz:** `gcc salom.c -o salom` deganingizda **ichkarida nima bo'lishini**; `.c`, `.h`, `.o`
> fayllar nima ekanini; nega dastur bir nechta faylga bo'linishini; va `implicit declaration`, `undefined reference`
> kabi xatolar **nimadan** chiqishini.
> **Oldindan nima kerak:** 0-bob (birinchi dastur).   **Vaqt:** 3–4 soat.
> Yadro yozishda bu bilim **majburiy**: yadro Makefile'i va linker skripti aynan shu bosqichlarni boshqaradi.

> **To'liq ishlaydigan misol:** [misollar/01_kompilyatsiya.sh](misollar/01_kompilyatsiya.sh) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

0-bobda siz `gcc salom.c -o salom` deb yozdingiz va dastur tayyor bo'ldi. Go'yo bitta sehrli tugma bosildi.
Aslida bu tugma ostida **to'rtta alohida dastur** ketma-ket ishlaydi. Bu bobda shu sehrni **qismlarga ajratamiz**.

**Nega bu kerak?** Katta dasturlarda (Linux yadrosida 30 000 dan ortiq `.c` fayl bor) xatolar turli bosqichlarda chiqadi.
Qaysi bosqichda ekanini bilmasangiz, xato xabarini tushunmaysiz. Bilsangiz — xatoning yarmi o'zi hal bo'ladi.

**Hayotdan misol: kitob nashriyoti.** Muallif qo'lyozma yozdi (`salom.c`). Endi u kitob bo'lishi kerak:

| Bosqich | Hayotda | Kompyuterda | Natija fayl |
|---|---|---|---|
| 1 | **Muharrir**: "bu yerga 3-ilovani qo'ying" degan joylarga ilovalarni ko'chirib qo'yadi, qisqartmalarni to'liq yozadi | **Preprotsessor** (`#include`, `#define`) | `salom.i` — hali ham oddiy matn |
| 2 | **Tarjimon**: matnni boshqa tilga tarjima qiladi | **Kompilyator**: C → assembly | `salom.s` — matn, lekin boshqa tilda |
| 3 | **Matbaa**: tarjimani "bosib chiqaradi" — alohida varaqlar | **Assembler**: assembly → mashina kodi | `salom.o` — endi matn emas, baytlar |
| 4 | **Muqovachi**: hamma varaqlarni bitta kitobga tikadi | **Linker** (bog'lovchi) | `salom` — tayyor dastur |

Shuning uchun `./salom.i` ishlamaydi: bu hali muharrir qo'lidagi qo'lyozma, kitob emas.

## 1.1. To'rt bosqichni o'z ko'zingiz bilan ko'ring

```text
salom.c ──[1. preprotsessor]──> salom.i ──[2. kompilyator]──> salom.s
        ──[3. assembler]──> salom.o ──[4. linker]──> salom (bajariladigan fayl)
```

Avval tajriba uchun dastur yozamiz (0-bobdagi o'sha `salom.c`):

**Bu dastur nima qiladi (umumiy):** `Salom, dunyo!` ni chiqaradigan dastur — kompilyatsiyaning to'rt bosqichini (preprotsessor, kompilyator, assembler, linker) birma-bir kuzatish uchun ishlatiladi.

```c
/* salom.c - to'rt bosqichni kuzatish uchun */
#include <stdio.h>

int main(void)
{
    printf("Salom, dunyo!\n");
    return 0;
}
```

Har bir bosqichni **alohida** ishga tushirish mumkin. `gcc` ga "qayerda to'xta" deb bayroq beramiz:

| Buyruq | Nima qiladi | Qayerda to'xtaydi |
|---|---|---|
| `gcc -E salom.c -o salom.i` | faqat preprotsessor | 1-bosqichdan keyin |
| `gcc -S salom.c -o salom.s` | + kompilyator | 2-bosqichdan keyin |
| `gcc -c salom.c -o salom.o` | + assembler | 3-bosqichdan keyin |
| `gcc salom.o -o salom` | linker (bog'lash) | 4-bosqich — tayyor |

(`-E`, `-S`, `-c` — "Expand", "Source (assembly)", "Compile only" so'zlaridan. Yodlash shart emas, jadval doim shu yerda.)

### 1-bosqich: preprotsessor — matn almashtirgich

U C'ni **tushunmaydi**, faqat `#` bilan boshlangan qatorlarni bajaradi:

- `#include <stdio.h>` → shu faylning **butun mazmunini** shu joyga ko'chiradi;
- `#define N 10` → keyingi hamma `N` so'zini `10` bilan almashtiradi;
- izohlarni (`/* */`, `//`) olib tashlaydi.

```console
$ gcc -E salom.c -o salom.i
$ wc -l salom.i
821 salom.i
$ tail -6 salom.i
# 4 "salom.c"
int main(void)
{
    printf("Salom, dunyo!\n");
    return 0;
}
```

**Nima ko'rdik:** `salom.i` yuzlab qator bo'lib ketdi (ko'chirilgan `stdio.h` hisobiga), lekin eng oxirida
sizning 6 qatoringiz turibdi: izoh yo'q, `#include` yo'q. `#include` o'rnida **butun fayl** turibdi.

### 2-bosqich: kompilyator — C dan assembly'ga

Bu **eng muhim** tarjimon: C kodini **assembly**ga aylantiradi. Assembly — CPU buyruqlarining **odam o'qiy oladigan**
yozuvi (har qator — bitta CPU buyrug'i). Sintaksis xatolari (`expected ';'`), tur xatolari, ogohlantirishlar —
hammasi shu yerda chiqadi. Optimallashtirish ham (`-O1`, `-O2` — "kodni tezroq qil") shu yerda ishlaydi.

```console
$ gcc -S -O1 salom.c -o salom.s
$ sed -n '/^main:/,/ret/p' salom.s
main:
.LFB23:
	.cfi_startproc
	endbr64
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	leaq	.LC0(%rip), %rdi
	call	puts@PLT
	movl	$0, %eax
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	ret
```

Har qatorning ma'nosi (hozircha tushunmasangiz ham bo'ladi, 17-bobda to'liq o'rganamiz):

| Assembly qatori | Ma'nosi |
|---|---|
| `endbr64` | xavfsizlik belgisi — hozircha e'tibor bermang |
| `subq $8, %rsp` | stekni ajratish (17-bob) |
| `leaq .LC0(%rip), %rdi` | matn `"Salom, dunyo!"` ning **manzilini** 1-argument sifatida tayyorlash |
| `call puts@PLT` | `puts` funksiyasini **chaqirish** |
| `movl $0, %eax` | `return 0` — natija `eax` registriga |
| `ret` | `main` dan qaytish |

**Qiziq tomoni:** biz `printf` yozdik, lekin kompilyator `puts` ni chaqirdi! Sabab: matnda `%` belgisi yo'q va
oxirida `\n` bor — shunday matn uchun `puts` tezroq. Kompilyator siz yozgan **so'zlarni** emas, **ma'noni** bajaradi.
Buni 13-bobda (kutilmagan xatti-harakatlar) eslaysiz.

### 3-bosqich: assembler — matndan baytlarga

Assembly matnini **mashina kodi** (sonlar) ga aylantiradi. Natija — **obyekt fayl** (`.o`). Uni matn sifatida o'qib bo'lmaydi.

Muhim narsa: `.o` fayl **hali tayyor emas**. `puts` funksiyasining kodi bu faylda **yo'q**; faylda faqat
"bu yerga `puts` ning manzili kerak" degan **belgi (bo'sh joy)** turadi.

```console
$ gcc -c salom.c -o salom.o
$ nm salom.o
0000000000000000 T main
                 U puts
```

`nm` — fayldagi **nomlar ro'yxati**. Har qatorda harf bor:

| Harf | Ma'nosi | Hayotdan misol |
|---|---|---|
| `T` | "bu nom **shu yerda** aniqlangan" (Text — kod bo'limida) | "taom shu oshxonada pishiriladi" |
| `U` | "bu nom **kerak, lekin bu yerda yo'q**" (Undefined) | "bu taom kerak, boshqa joydan olinadi" |

`T main` — `main` shu faylda bor. `U puts` — `puts` kerak, lekin boshqa joyda.

### 4-bosqich: linker — hamma bo'shliqlarni to'ldiradi

Linker **bir nechta** `.o` fayl va kutubxonalarni **bitta** bajariladigan faylga birlashtiradi va har bir `U` nomni
qayerdandir topib ulaydi. `puts` — standart C kutubxonasida (libc), linker uni o'zi topadi.

```console
$ gcc salom.o -o salom
$ ./salom
Salom, dunyo!
```

> **Eslab qoling:** **4 bosqich:** preprotsessor (matn) → kompilyator (assembly) → assembler (`.o`) → linker (tayyor dastur).
> Birinchi uchtasi **har bir `.c` faylni alohida** qayta ishlaydi, faqat oxirgisi **hammasini birlashtiradi**.

**Tez-tez xato:** `./salom.i` yoki `./salom.o` ni ishga tushirishga urinish. Bular oraliq natijalar, dastur emas.

## 1.2. E'lon va ta'rif — eng muhim farq

Bu bo'lim butun bobning **kaliti**. Ikkita so'zni ajrating:

- **E'lon (declaration)** — "shunday funksiya **bor**" deyish: nomi, nima olishi, nima qaytarishi. Kod **yo'q**.
- **Ta'rif (definition)** — funksiyaning **o'zi**: kodi bilan.

**Hayotdan misol: telefon daftari.** Daftarda "Ali — +998 90 123 45 67" deb yozilgan (**e'lon**). Qo'ng'iroq qilish
uchun shu yetarli: Alining uyiga borish shart emas. Lekin Ali **haqiqatan yashashi kerak** (**ta'rif**). Daftarda yozilgan,
lekin bunday odam yo'q bo'lsa — qo'ng'iroq o'tmaydi. Ikkita har xil Ali bir raqamda bo'lsa — chalkashlik.

```c
/* elon_tarif.c - e'lon va ta'rif yonma-yon */
#include <stdio.h>

int kvadrat(int x);            /* E'LON: "kvadrat degan funksiya bor, int oladi, int qaytaradi" */

int main(void)
{
    printf("%d\n", kvadrat(7));    /* bu yerda kvadrat ni CHAQIRYAPMIZ */
    return 0;
}

int kvadrat(int x)             /* TA'RIF: funksiyaning o'zi */
{
    return x * x;
}
```

```console
$ gcc -Wall -Wextra elon_tarif.c -o elon_tarif
$ ./elon_tarif
49
```

**Qatorma-qator:**

| Qator | Nima qiladi | Nega kerak |
|---|---|---|
| `int kvadrat(int x);` | **e'lon**: oxirida `;` — tana yo'q | `main` ichida `kvadrat(7)` yozilganda kompilyator **allaqachon** `kvadrat` nimaligini bilishi kerak |
| `printf("%d\n", kvadrat(7));` | `kvadrat(7)` ni chaqiradi, natijani `%d` ga qo'yadi | `7` — argument; `kvadrat` `49` qaytaradi |
| `int kvadrat(int x) { ... }` | **ta'rif**: tana `{ }` bor | funksiya **aslida** nima qilishini yozadi |
| `return x * x;` | `x` ni `x` ga ko'paytirib, natijani qaytaradi | chaqirgan joyga javob berish |

**Nega e'lon kerak?** Kompilyator kodni **tepadan pastga** o'qiydi. `main` da `kvadrat(7)` ni uchratganda u
`kvadrat` ni hali ko'rmagan. E'lon bo'lmasa, "bu nima? nechta narsa oladi? nima qaytaradi?" deb ushlanib qoladi.
Agar `kvadrat` ta'rifini `main` dan **tepaga** yozsangiz, alohida e'lon kerak bo'lmaydi (ta'rifning o'zi ham e'lon).

Endi muhim fikr: **ikki xil tekshiruvchi** bor, ularga turli narsa kerak:

- **Kompilyator** bir vaqtda faqat **bitta** `.c` faylni ko'radi. Unga funksiyani chaqirish uchun **e'lon** yetarli.
- **Linker** **hamma** `.o` fayllarni ko'radi. Unga **ta'rif** kerak: kodning o'zi, **aynan bitta** joyda.

Shuning uchun uch xil xato bor:

| Xato | Kim chiqaradi | Ma'nosi | Yechim |
|---|---|---|---|
| `implicit declaration of function 'f'` | kompilyator | **E'lon yo'q**: "`f` haqida hech narsa bilmayman" | Kerakli `.h` ni `#include` qiling yoki e'lon yozing |
| `undefined reference to 'f'` | linker | E'lon bor, lekin **ta'rif hech qayerda yo'q** | `f` yozilgan `.c` ni kompilyatsiyaga qo'shing yoki kutubxonani ulang (`-lm`, `-pthread`) |
| `multiple definition of 'f'` | linker | Ta'rif **ikki joyda** bor | Ta'rifni `.h` ga yozmang — faqat e'lonni |

> **Eslab qoling:** **e'lon** = "bor" (kompilyator uchun, bir necha marta yozsa bo'ladi). **Ta'rif** = "mana kodi"
> (linker uchun, **aynan bir marta**).

## 1.3. Sarlavha (`.h`) fayllari: nega kerak va qanday ishlaydi

Katta dasturda funksiya **bitta** faylda yoziladi, **o'nlab** fayllarda chaqiriladi. Har bir faylga e'lonni qo'lda
yozish — xatolarga yo'l (bittasini noto'g'ri yozdingiz — dastur jim buziladi). Yechim: e'lonlar **bitta `.h` faylda**,
hamma uni `#include` qiladi.

### Bu `class` emasmi?

Yo'q. C'da `class` umuman yo'q. Bu fayllar Python'dagi **modulga** o'xshaydi:

```python
# --- matematika.py ---
def kvadrat(x):
    return x * x

# --- main.py ---
from matematika import kvadrat
print(kvadrat(7))
```

Python'da bitta `matematika.py` yetadi. C'da esa u **ikkiga** bo'linadi:

| Python | C | Ichida nima bor |
|---|---|---|
| `def kvadrat(x):` qatori | `matematika.h` | Faqat **e'lon**: "shunday funksiya bor" |
| funksiya tanasi | `matematika.c` | **Ta'rif**: funksiya aslida nima qiladi |
| `from matematika import kvadrat` | `#include "matematika.h"` | Boshqa fayldagi funksiyadan foydalanishga ruxsat |
| `python main.py` (hammasi o'zi) | 3 ta `gcc` buyrug'i | Tarjima va birlashtirishni o'zingiz buyurasiz |

### Uch fayl: har biri nima uchun

```c
/* matematika.h */
#pragma once                   /* bu fayl bir kompilyatsiyada ikki marta qo'shilmasin */
int kvadrat(int x);
```

```c
/* matematika.c */
#include "matematika.h"        /* o'z e'loni bilan mos kelishini kompilyator tekshiradi */

int kvadrat(int x)
{
    return x * x;
}
```

```c
/* main.c */
#include <stdio.h>
#include "matematika.h"

int main(void)
{
    printf("%d\n", kvadrat(7));
    return 0;
}
```

**Kodda nimalar bor:**

| Fayl | Qator | Nima qiladi | Nega kerak |
|---|---|---|---|
| `matematika.h` | `#pragma once` | "bu faylni bitta kompilyatsiyada **faqat bir marta** qo'sh" | Katta dasturda bitta `.h` bir necha yo'l bilan qayta-qayta kirib qolishi mumkin; ikki marta kirsa — xato |
| | `int kvadrat(int x);` | **e'lon** | Boshqa fayllar `kvadrat` ni chaqira olishi uchun |
| `matematika.c` | `#include "matematika.h"` | o'z menyusini o'qish | E'lon va ta'rif **mos kelmasa**, kompilyator xato beradi (pastda ko'rasiz) |
| | `int kvadrat(int x) {...}` | **ta'rif** | Linker aynan shu kodni topadi |
| `main.c` | `#include <stdio.h>` | `printf` e'loni | `< >` — tizim fayli |
| | `#include "matematika.h"` | `kvadrat` e'loni | `" "` — **o'zingizning** fayl (shu papkadan qidiriladi) |
| | `kvadrat(7)` | funksiyani chaqirish | `main.c` `kvadrat` ning kodini **ko'rmaydi**, faqat e'lonini |

**Hayotdan misol: restoran.**

- **`matematika.h` — menyu.** "Osh — 30 000 so'm". Oshning qanday pishirilishi menyuda yo'q: faqat **nima bor** va
  **nima berib, nima olasiz**. `int kvadrat(int x);` ham shunday: "`kvadrat` degan taom bor, unga `int` berasiz, `int` olasiz".
- **`matematika.c` — oshxona.** Oshpaz oshni qanday pishirishni biladi: `return x * x;`. Oshxona o'z menyusini ham
  o'qiydi (`#include "matematika.h"`), aks holda menyuda "Osh" yozilgan-u, oshxona "Lag'mon" pishirib qo'yishi mumkin.
- **`main.c` — mijoz.** Menyuni o'qiydi va buyurtma beradi: `kvadrat(7)`. Osh qanday pishishini bilishi shart emas.
- **`gcc -c matematika.c`** — oshxona taomni idishga soldi (`matematika.o`), lekin hech kimga bermadi.
- **`gcc -c main.c`** — ofitsiant buyurtmani yozdi (`main.o`): "kvadrat kerak" — lekin taom qayerdaligi hali noma'lum.
- **`gcc main.o matematika.o -o dastur`** (linker) — ofitsiant buyurtmani oshxonadagi taomga ulaydi. Restoran ishladi.

`printf` ham aynan shunday: `stdio.h` — menyu, `printf` ning o'zi libc kutubxonasida (oshxona). Linker uni o'zi topadi.
Qaysi `#include` nima uchun kerakligi — [sarlavhalar.md](sarlavhalar.md) sahifasida.

### Qadamma-qadam o'zingiz bajaring

**1-qadam.** Oshxona tayyorlandi:

```console
$ gcc -Wall -Wextra -c matematika.c
$ ls matematika.*
matematika.c
matematika.h
matematika.o
$ nm matematika.o
0000000000000000 T kvadrat
```

`matematika.o` paydo bo'ldi. `T kvadrat` — "`kvadrat` **shu yerda bor**".

**2-qadam.** Buyurtma yozildi:

```console
$ gcc -Wall -Wextra -c main.c
$ nm main.o
                 U kvadrat
0000000000000000 T main
                 U printf
```

`U kvadrat` — "`kvadrat` **kerak, bu yerda yo'q**" (bo'sh joy). `U printf` — `printf` ham kerak, uni libc beradi.
`.o` fayllar hali dastur emas: `./main.o` ishlamaydi.

**3-qadam.** Linker bo'sh joylarni to'ldiradi: `U kvadrat` ↔ `T kvadrat`.

```console
$ gcc main.o matematika.o -o dastur
$ ./dastur
49
```

### Ataylab buzib ko'ring — har bir xato nimani anglatadi

Bu xatolarni hozir **bir marta o'z ko'zingiz bilan** ko'rsangiz, katta loyihada darhol taniysiz.

**a) Oshxonani unutish.** Bog'lashda `matematika.o` ni yozmang:

```console
$ gcc main.o -o dastur_a # xato kutiladi
/usr/bin/ld: main.o: in function `main':
main.c:(.text+0xe): undefined reference to `kvadrat'
collect2: error: ld returned 1 exit status
```

`ld` — linker. "Buyurtmada `kvadrat` bor, lekin hech bir oshxonada bu taom yo'q" (`undefined reference`).

**b) Menyusiz buyurtma.** `#include "matematika.h"` siz yozilgan `main` (`menyusiz.c`):

**Bu dastur nima qiladi (umumiy):** `kvadrat` funksiyasini e'lon qilmasdan chaqiradi — ataylab xatoli.

```c
/* menyusiz.c - e'lon yo'q */
#include <stdio.h>

int main(void)
{
    printf("%d\n", kvadrat(7));
    return 0;
}
```

```console
$ gcc -Wall -Wextra -c menyusiz.c # xato kutiladi
menyusiz.c: In function ‘main’:
menyusiz.c:6:20: warning: implicit declaration of function ‘kvadrat’ [-Wimplicit-function-declaration]
    6 |     printf("%d\n", kvadrat(7));
      |                    ^~~~~~~
```

Kompilyator: "`kvadrat` haqida hech narsa bilmayman." Eski kompilyatorlar faqat ogohlantirib, **taxmin** qilib davom etadi:
taxmin noto'g'ri bo'lsa dastur jim turib noto'g'ri ishlaydi. Yangi GCC (14+) buni to'g'ridan-to'g'ri **xato** deydi.
Doim shunday ogohlantirishni xato deb hisoblang.

**c) Menyu va oshxona mos emas.** `matematika_xato.c` da `int` o'rniga `long`:

**Bu dastur nima qiladi (umumiy):** `.h` faylida `int` deb e'lon qilingan funksiyani `.c` faylida `long` deb ta'riflaydi — e'lon va ta'rif mos emasligi xatosi.

```c
/* matematika_xato.c - e'lon bilan ta'rif mos emas */
#include "matematika.h"

long kvadrat(int x)
{
    return x * x;
}
```

```console
$ gcc -Wall -Wextra -c matematika_xato.c # xato kutiladi
matematika_xato.c:4:6: error: conflicting types for ‘kvadrat’; have ‘long int(int)’
    4 | long kvadrat(int x)
      |      ^~~~~~~
In file included from matematika_xato.c:2:
matematika.h:3:5: note: previous declaration of ‘kvadrat’ with type ‘int(int)’
    3 | int kvadrat(int x);
      |     ^~~~~~~
```

Oshxona o'z menyusini o'qigani (`#include "matematika.h"`) aynan shu nomuvofiqlikni ushladi (`conflicting types`).
Agar `#include` bo'lmaganda, bu xato **sezilmasdi** — shuning uchun `.c` o'z `.h` ini doim qo'shadi.

**d) Ikki oshxonada bir xil taom.** `boshqa.c` da ham `kvadrat` ning ta'rifi bor:

**Bu dastur nima qiladi (umumiy):** `kvadrat` funksiyasining ikkinchi ta'rifi — bir funksiya ikki faylda ta'riflansa, linker xato beradi.

```c
/* boshqa.c - ikkinchi ta'rif */
int kvadrat(int x)
{
    return x * x * 1;
}
```

```console
$ gcc -Wall -Wextra -c boshqa.c
$ gcc main.o matematika.o boshqa.o -o dastur_d # xato kutiladi
/usr/bin/ld: boshqa.o: in function `kvadrat':
boshqa.c:(.text+0x0): multiple definition of `kvadrat'; matematika.o:matematika.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
```

Linker qaysi oshxonadan olishni bilmaydi (`multiple definition`). Ta'rif butun dasturda **aynan bitta** bo'lishi shart.
Shuning uchun ta'rifni `.h` ga yozmang: `.h` ko'p `.c` ga kirib, ta'rif har birida paydo bo'ladi.

> **Eslab qoling:** `.h` = **e'lonlar** (menyu), `.c` = **ta'riflar** (oshxona). Har `.c` o'z `.h` ini `#include` qiladi.
> `main.c` kerakli `.h` larni `#include` qiladi. Bog'lashda barcha `.o` larni **birga** beramiz.

**Mashqlardagi tuzilma aynan shunday:** `mashq.h` — e'lonlar, `yechim.c` — siz yozadigan ta'riflar, `test.c` —
ularni chaqiradigan kod. Tekshiruvchi `gcc yechim.c test.c` qiladi.

**MyOS'da:** har bir `kernel/xxx/yyy.c` ning yonida `yyy.h` bor. Masalan `kernel/fs/pipe.h` pipe'ning "tashqi dunyo"
uchun funksiyalarini e'lon qiladi, `pipe.c` esa ularni amalga oshiradi. Makefile har bir `.c` ni alohida `.o` ga
kompilyatsiya qilib, `kernel/linker.ld` bo'yicha bitta `kernel.elf` ga bog'laydi.

To'liqroq namuna (`static`, `extern`, Makefile bilan): [misollar/11_kop_fayl/](misollar/11_kop_fayl/main.c).

## 1.4. Nega har bir `.c` alohida kompilyatsiya qilinadi

**Hayotdan misol:** 1000 betlik kitobda bitta bobni tuzatdingiz. Butun kitobni qayta bosmaysiz — faqat shu bobni.

1. **Tezlik.** Linux yadrosida ~30 000 ta `.c` bor. Bitta faylni o'zgartirsangiz, faqat o'sha `.c` qayta
   kompilyatsiya qilinadi (`.c` → `.o`), keyin tez bog'lanadi. `make` aynan shuni kuzatadi (11-bob).
2. **Ajratish.** `static` bilan belgilangan funksiya faqat o'z faylida ko'rinadi — boshqa fayllar unga tasodifan
   tegib keta olmaydi (5-bob).

## 1.5. Kompilyatsiya bayroqlari: nega har biri kerak

**Bayroq** — `gcc` ga beriladigan qo'shimcha ko'rsatma (`-` bilan boshlanadi).

| Bayroq | Nima qiladi | Nega kerak |
|---|---|---|
| `-Wall -Wextra` | Deyarli barcha ogohlantirishlarni yoqadi | Xatolarni dastur ishga tushmasdan topish. **Hech qachon o'chirmang** |
| `-Werror` | Ogohlantirish = xato | "Keyin tuzataman" deb qoldirmaslik. MyOS va mashqlarda yoqilgan |
| `-g` | Debug ma'lumoti | `gdb` qator raqamlari va o'zgaruvchi nomlarini ko'rsatadi |
| `-O0` / `-O2` | Optimallashtirish darajasi | `-O0` — debug uchun (kod siz yozgandek), `-O2` — tez kod |
| `-std=c11` / `-std=gnu11` | Til standarti | `gnu11` — C11 + GNU kengaytmalari (inline asm). MyOS shuni ishlatadi |
| `-I papka` | `#include` qidiriladigan papka | `-Ikernel` bilan `#include "fs/vfs.h"` ishlaydi |
| `-c` | Faqat `.o` gacha | Katta loyihalar uchun |
| `-fsanitize=address,undefined` | Ish vaqtida xatolarni ushlash | O'rganish va testlar uchun |
| `-ffreestanding` | "OS yo'q" rejimi | **Yadro uchun**: standart kutubxona yo'q deb hisoblash (18-bob) |

**Hayotdan misol: `-Wall -Wextra` — imlo tekshiruvchi.** Word'dagi qizil to'lqinli chiziq kabi: matn baribir
chop etiladi, lekin "bu yerda xato bo'lishi mumkin" deydi. Uni o'chirish — imloni tekshirmasdan kitob chiqarish bilan barobar.

## 1.6. `gdb` — dastur ichiga qarash

**Hayotdan misol: futboldagi VAR.** Hakam o'yinni to'xtatib, lahzani sekinlashtirib, kadrma-kadr ko'radi.
`gdb` ham dasturni istalgan qatorda to'xtatadi, bir qadam yuradi va **har bir o'zgaruvchining qiymatini** ko'rsatadi.
`-g` — kameralarni o'rnatish: usiz VAR'da ko'rish uchun yozuv bo'lmaydi.

```bash
gcc -g -O0 dastur.c -o dastur
gdb ./dastur
```

| gdb buyrug'i | Nima qiladi |
|---|---|
| `break main` | `main` da to'xta (yoki `break 11` — 11-qatorda) |
| `run` | ishga tushir |
| `next` | keyingi qator (funksiyaga kirmasdan) |
| `step` | keyingi qator (funksiyaga **kirib**) |
| `print x` | o'zgaruvchi qiymati |
| `print *p` | ko'rsatkich ko'rsatgan qiymat |
| `bt` | chaqiruvlar zanjiri (backtrace): qayerdan kelib qoldik |
| `info locals` | hamma lokal o'zgaruvchilar |
| `continue` | keyingi to'xtash nuqtasigacha |
| `quit` | chiqish |

Dastur qulaganda (`Segmentation fault`) — `gdb ./dastur`, `run`, keyin `bt`: qaysi qatorda qulagani darhol ko'rinadi.
**MyOS'ni ham aynan shu gdb bilan debug qilasiz** (`make debug`, docs/08).

## Hayotdan misol va to'liq dastur

**Jamg'arma.** Har oy jamg'armaga pul qo'shamiz va har oy qo'shiladigan summani 100 ming ko'paytiramiz. gdb bilan kadrma-kadr ko'ramiz.

```c
/* jamgarma.c - har oy jamg'arma: gdb bilan qadamma-qadam ko'rish uchun */
#include <stdio.h>

int main(void)
{
    int jamgarma = 0;
    int oylik_qoshish = 500000;

    for (int oy = 1; oy <= 3; oy++) {
        jamgarma = jamgarma + oylik_qoshish;
        oylik_qoshish = oylik_qoshish + 100000;     /* har oy 100 ming ko'proq */
    }
    printf("3 oyda jamg'arildi: %d so'm\n", jamgarma);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g jamgarma.c -o jamgarma
$ ./jamgarma
3 oyda jamg'arildi: 1800000 so'm
$ gdb -q -batch -ex 'break 11' -ex run -ex 'print oy' -ex 'print jamgarma' -ex continue -ex 'print oy' -ex 'print jamgarma' ./jamgarma 2>&1 | grep '^\$'
$1 = 1
$2 = 500000
$3 = 2
$4 = 1100000
```

**Kodda nimalar bor:**

| Nom | Turi | Boshlang'ich qiymat | Nima uchun |
|---|---|---|---|
| `jamgarma` | `int` (32 bit) | `0` | yig'ilgan jami pul |
| `oylik_qoshish` | `int` | `500000` | shu oy qo'shiladigan summa |
| `oy` | `int` (faqat sikl ichida yashaydi) | `1` | nechanchi oy |

**Qiymatlar qanday o'zgaradi** (`for` ning har aylanishi):

| Aylanish | `oy` | 1-qator: `jamgarma = jamgarma + oylik_qoshish` | 2-qator: `oylik_qoshish = oylik_qoshish + 100000` |
|---|---|---|---|
| boshlanishi | — | `jamgarma = 0` | `oylik_qoshish = 500000` |
| 1 | 1 | 0 + 500000 = **500000** | 500000 + 100000 = **600000** |
| 2 | 2 | 500000 + 600000 = **1100000** | 600000 + 100000 = **700000** |
| 3 | 3 | 1100000 + 700000 = **1800000** | 700000 + 100000 = 800000 |

`gdb` buyrug'i 11-qatorda (`oylik_qoshish = ...` qatori) to'xtaydi: 1-marta `oy = 1`, `jamgarma = 500000` (jadvaldagi 1-aylanish);
`continue` dan keyin `oy = 2`, `jamgarma = 1100000`. Jadval bilan solishtiring — mos keladi.

O'zingiz qo'lda qiling: `gdb ./jamgarma`, keyin `break 11`, `run`, `print jamgarma`, `next`, `print oylik_qoshish`, `continue`.
Har qadamda qiymatni **avval taxmin qiling**, keyin `print` bilan tekshiring.

## Bob xulosasi (yodlash uchun)

1. `gcc` — to'rt bosqich: **preprotsessor → kompilyator → assembler → linker**.
2. `.c` har biri alohida `.o` ga aylanadi; linker hamma `.o` ni bitta dasturga bog'laydi.
3. **E'lon** = "bor" (`.h` da), **ta'rif** = "mana kodi" (`.c` da, aynan bir marta).
4. Xatolar: `implicit declaration` (e'lon yo'q), `undefined reference` (ta'rif yo'q), `multiple definition` (ta'rif ikki marta).
5. `-Wall -Wextra -g` doim yoqilgan bo'lsin; xato qidirish uchun — `gdb`.

## Savol-javob

**`.o` va bajariladigan fayl farqi nima?**
`.o` — "yarim tayyor": tashqi manzillar hali to'ldirilmagan va `main` dan boshlash haqida ma'lumot yo'q. Bajariladigan
fayl (Linux'da **ELF** formati) — to'liq bog'langan, OS uni yuklab ishga tushira oladi. MyOS'ning `kernel/sys/elf.c`
fayli aynan shu formatni o'qib, dasturni yuklaydi.

**`-lm` nima?**
`-l` — kutubxonani ulash: `-lm` → `libm.so` (matematika: `sqrt`, `sin`). Kutubxona — `.o` fayllar to'plami.
MyOS'ning `libc.a` si ham shunday (`Makefile` → `ar rcs`).

**`#pragma once` nima uchun?**
`a.h` va `b.h` ikkalasi ham `c.h` ni qo'shsa, `main.c` ikkalasini qo'shganda `c.h` ikki marta kiradi → `struct` ikki marta
ta'riflanadi → xato. `#pragma once` "bu faylni faqat bir marta qo'sh" deydi. Eski usul — "include guard" (10-bob).

**Nega `main.o` ni `./main.o` deb ishga tushirib bo'lmaydi?**
Chunki unda `kvadrat` uchun bo'sh joy bor — kod to'liq emas. Faqat linker hammasini to'ldirgandan keyin dastur bo'ladi.

## O'zingizni tekshiring

1. `undefined reference to 'kvadrat'` — kompilyator xatosimi yoki linker xatosimi? Qanday tuzatiladi?
2. Nega funksiya ta'rifini `.h` faylga yozish xato?
3. `gcc -E` nima ko'rsatadi?
4. Qaysi bayroq bilan mashina kodini ko'rish mumkin?

<details><summary>Javoblar</summary>

1. Linker xatosi: e'lon bor, ta'rif yo'q. `kvadrat` yozilgan `.c`/`.o` faylni buyruqqa qo'shing.
2. `.h` bir nechta `.c` ga qo'shiladi — ta'rif har birida paydo bo'ladi → "multiple definition".
3. Preprotsessordan keyingi kod: include'lar ochilgan, makrolar almashtirilgan, izohlar yo'q.
4. `gcc -S` (assembly matni) yoki `gcc -c` + `objdump -d x.o`.
</details>

## Mashq

- 1.3-bo'limdagi uch faylli dasturni yarating. Keyin `matematika.o` ni bog'lash buyrug'idan olib tashlab, linker xatosini
  o'qing. `#include "matematika.h"` ni olib tashlab, kompilyator xatosini o'qing.
- `gcc -S -O0` va `gcc -S -O2` bilan kichik funksiyaning assembly'sini solishtiring.
- `mashqlar/01_kvadratlar` ni oching: `mashq.h`, `yechim.c`, `test.c` qanday bog'langanini tushuning. Hozircha yechmasangiz ham bo'ladi.

<!-- loyiha:boshi -->
## Loyiha: geometriya moduli

**Maqsad:** bitta dasturni uch faylga bo'lish: `.h` (menyu), `.c` (oshxona), `main.c` (mijoz).
**Bobdan ishlatiladi:** e'lon va ta'rif (1.2), sarlavha fayli (1.3), alohida kompilyatsiya va bog'lash.

**Talab:** to'g'ri to'rtburchak va kvadrat uchun yuza, perimetr, o'rta qiymat hisoblovchi modul.
**Ma'lumotlar:** faqat `int` sonlar — modulning o'z holati yo'q.
**Funksiyalar:** `yuza(a, b)`, `perimetr(a, b)`, `kvadrat(a)`, `kub(a)`, `yarim_yigindi(a, b)`.
Qaysi biri `.h` ga? Hammasi (main ishlatadi). Qaysi biri `.c` da qoladi? Ichki yordamchilar (hozircha yo'q).

```c
/* geometriya.h - menyu */
#pragma once

int yuza(int a, int b);
int perimetr(int a, int b);
int kvadrat(int a);
int kub(int a);
int yarim_yigindi(int a, int b);
```

**Bu dastur nima qiladi (umumiy):** geometriya kutubxonasining ta'riflari: `yuza`, `perimetr`, `kvadrat`, `kub` va `yarim_yigindi` funksiyalari.

```c
/* geometriya.c - oshxona */
#include "geometriya.h"

int yuza(int a, int b) { return a * b; }
int perimetr(int a, int b) { return 2 * (a + b); }
int kvadrat(int a) { return a * a; }
int kub(int a) { return a * a * a; }
int yarim_yigindi(int a, int b) { return (a + b) / 2; }
```

**Bu dastur nima qiladi (umumiy):** geometriya kutubxonasidan foydalanuvchi dastur (mijoz): funksiyalarni chaqirib, natijalarni chiqaradi.

```c
/* main.c - mijoz */
#include <stdio.h>
#include "geometriya.h"

int main(void)
{
    printf("4 x 7 to'rtburchak: yuza = %d, perimetr = %d\n", yuza(4, 7), perimetr(4, 7));
    printf("kvadrat(9) = %d, kub(3) = %d\n", kvadrat(9), kub(3));
    printf("yarim_yigindi(10, 15) = %d\n", yarim_yigindi(10, 15));
    return 0;
}
```

```console
$ gcc -Wall -Wextra -c geometriya.c
$ gcc -Wall -Wextra -c main.c
$ gcc main.o geometriya.o -o dastur
$ ./dastur
4 x 7 to'rtburchak: yuza = 28, perimetr = 22
kvadrat(9) = 81, kub(3) = 27
yarim_yigindi(10, 15) = 12
$ nm geometriya.o
0000000000000044 T kub
0000000000000031 T kvadrat
0000000000000017 T perimetr
000000000000005b T yarim_yigindi
0000000000000000 T yuza
$ nm main.o
                 U kub
                 U kvadrat
0000000000000000 T main
                 U perimetr
                 U printf
                 U yarim_yigindi
                 U yuza
```

`nm` da `T` — "bu yerda aniqlangan", `U` — "boshqa joydan kerak". `main.o` dagi `U yuza` linker
`geometriya.o` dagi `T yuza` bilan ulaganda to'ldiriladi (1.3-bo'lim).

**Kengaytiring:** `diagonal_kvadrati(a, b)` (= a² + b²) funksiyasini qo'shing — nechta faylga tegdingiz?
(Uchta: `.h` ga e'lon, `.c` ga tana, `main.c` da chaqiruv.) Faqat `geometriya.c` ni o'zgartirsangiz, `main.c` ni
qayta kompilyatsiya qilish shartmi?

## Mustaqil loyiha: vaqt moduli ★☆☆

**Vazifa:** sutka 1440 daqiqadan iborat. "09:45 + 02:30" kabi hisoblarni bajaradigan modul yozing.
Uch fayl: `vaqt.h`, `vaqt.c`, `main.c`.

**Talab.** Vaqt bitta `int` da — sutka boshidan o'tgan daqiqalar (0..1439). Modul aynan shu **4 ta**
funksiyani `vaqt.h` orqali beradi:

| Funksiya | Nima qiladi |
|---|---|
| `int vaqt_yasa(int soat, int daqiqa)` | soat×60 + daqiqa (sutkadan oshsa aylanadi) |
| `int vaqt_qosh(int a, int b)` | a + b, sutkadan oshsa boshidan boshlanadi |
| `int vaqt_farq(int a, int b)` | a dan b gacha necha daqiqa oldinga (kechasi ham o'tadi) |
| `void vaqt_chiqar(int t)` | `HH:MM` ko'rinishida chiqaradi, **yangi qatorsiz** |

`vaqt.c` da bitta **`static`** yordamchi funksiya bo'lsin: har qanday (manfiy ham) daqiqani
0..1439 ga keltiradigan. Uni boshqa funksiyalar ishlatsin.

**Sinov** (`main.c` shuni bajarsin) — kutilgan natija (`darslik/loyihalar/01_geometriya/kutilgan.txt`):

```text
09:45 + 02:30 = 12:15
23:50 + 00:25 = 00:15
08:30 dan 17:05 gacha: 08:35
22:00 dan 06:30 gacha: 08:30
```

Sinovlar: `09:45 + 02:30`; `23:50 + 00:25`; `08:30` dan `17:05` gacha farq; `22:00` dan `06:30` gacha farq.

**Maslahat** (yechim emas):
- Manfiy son bilan `%` C da manfiy qoldiq beradi: `-10 % 1440` nechaga teng? Shuning uchun static yordamchi kerak.
- `%02d` — ikki xonali, oldiga nol qo'yiladi.
- `.h` ga faqat 4 ta e'lon (va `#pragma once`) yoziladi, yordamchi funksiya yozilmaydi.

**O'zingizni tekshiring:** `nm vaqt.o` da 4 ta `T` va bitta kichik `t` (static) bo'lishi kerak.

**Tekshirish:**

```bash
gcc -Wall -Wextra -c vaqt.c && gcc -Wall -Wextra -c main.c && gcc vaqt.o main.o -o dastur \
  && ./dastur | diff - ~/C_loyha/darslik/loyihalar/01_geometriya/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [2-bob. O'zgaruvchilar va turlar](02-turlar.md)
