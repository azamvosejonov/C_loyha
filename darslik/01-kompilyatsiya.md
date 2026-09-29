# 1-bob. Kompilyatsiya: koddan dasturgacha

> **Bu bobdan keyin:** `gcc` ichida nima bo'layotganini, `.c`, `.h`, `.o` fayllar farqini,
> "undefined reference" va "implicit declaration" xatolari nimadan kelib chiqishini bilasiz.
> Yadro yozishda bu bilim **majburiy**: yadro Makefile'i va linker skripti aynan shu bosqichlarni boshqaradi.

> **To'liq ishlaydigan misol:** [misollar/01_kompilyatsiya.sh](misollar/01_kompilyatsiya.sh) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**To'rt bosqich — kitob nashriyoti (1.1).** Muallif qo'lyozma yozdi (`salom.c`). Endi u kitob bo'lishi kerak:
1. **Muharrir** (preprotsessor, `gcc -E`) — qo'lyozmadagi "bu yerga 3-ilovani qo'ying" degan joylarga
   ilovalarni ko'chirib qo'yadi (`#include`), qisqartmalarni to'liq yozadi (`#define`). Natija — hali ham
   oddiy matn (`.i`).
2. **Tarjimon** (kompilyator, `gcc -S`) — matnni protsessor tushunadigan tilga tarjima qiladi. Natija —
   assembly, bu ham matn, lekin boshqa tilda (`.s`).
3. **Matbaa** (assembler, `gcc -c`) — tarjimani mashina kodiga "bosib chiqaradi". Natija — alohida bosilgan
   varaqlar (`.o`). Ularni o'qib bo'lmaydi — ular endi matn emas.
4. **Muqovachi** (linker) — barcha varaqlarni (sizning `.o` laringiz va tayyor kutubxonalarni) bitta kitobga
   tikadi. Faqat shundan keyin kitob tayyor: `./dastur`.

Shuning uchun `./salom.i` ishlamaydi: bu hali muharrir qo'lidagi qo'lyozma, kitob emas.

**E'lon va ta'rif — telefon daftari va odamning o'zi (1.2).** Telefon daftarida "Ali — +998 90 ..." deb
yozilgan (e'lon). Qo'ng'iroq qilish uchun shu yetarli — Alining uyiga borish shart emas. Lekin Ali
haqiqatan mavjud bo'lishi kerak (ta'rif). Daftarda yozilgan-u, bunday odam yo'q bo'lsa — `undefined
reference`. Ikkita har xil Ali bir raqamda bo'lsa — `multiple definition`.

**`.h`, `.c`, `.o` — restoran (1.3).** Batafsil — pastdagi 1.3-bo'limda: menyu, oshxona va mijoz misoli.

**`-Wall -Wextra` — imlo tekshiruvchi (1.5).** Word'dagi qizil to'lqinli chiziq kabi: matn baribir
chop etiladi, lekin "bu yerda xato bo'lishi mumkin" deb ogohlantiradi. Uni o'chirib qo'yish — imloni
tekshirmasdan kitob chiqarish bilan barobar.

**`-g` va `gdb` — futboldagi VAR (1.6).** Hakam o'yinni to'xtatib, lahzani sekinlashtirib, kadrma-kadr
ko'radi. `gdb` ham dasturni istalgan qatorda to'xtatadi (`break`), bir qadam yuradi (`next`) va shu
paytdagi har bir o'zgaruvchining qiymatini ko'rsatadi (`print`). `-g` — kameralarni o'rnatish: usiz
VAR'da ko'rish uchun yozuv bo'lmaydi.

### To'liq dastur: gdb bilan kadrma-kadr

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

Oxirgi buyruq gdb'ni avtomatik boshqaradi: 11-qatorda to'xtaydi va `oy`, `jamgarma` ni ko'rsatadi,
keyin keyingi aylanishga o'tib yana ko'rsatadi. O'zingiz qo'lda qiling: `gdb ./jamgarma`, keyin
`break 11`, `run`, `print jamgarma`, `next`, `print oylik_qoshish`, `continue`. Har qadamda qiymatni
**avval taxmin qiling**, keyin `print` bilan tekshiring.

## 1.1. To'rt bosqich

`gcc salom.c -o salom` aslida to'rtta dasturni ketma-ket ishga tushiradi:

```text
salom.c ──[1. preprotsessor]──> salom.i ──[2. kompilyator]──> salom.s
        ──[3. assembler]──> salom.o ──[4. linker]──> salom (bajariladigan fayl)
```

Har bir bosqichni alohida ko'rish mumkin:

```bash
gcc -E salom.c -o salom.i     # 1: faqat preprotsessor
gcc -S salom.c -o salom.s     # 1+2: assembly kodigacha
gcc -c salom.c -o salom.o     # 1+2+3: obyekt faylgacha
gcc salom.o -o salom          # 4: bog'lash (link)
```

### 1-bosqich: preprotsessor

Matn bilan ishlaydi — C'ni **tushunmaydi**:
- `#include <stdio.h>` → o'sha faylning butun mazmunini qo'yadi (`salom.i` ~800 qator bo'lib qoladi!);
- `#define N 10` → keyingi hamma `N` so'zlarini `10` ga almashtiradi;
- izohlarni (`/* */`, `//`) olib tashlaydi.

`gcc -E salom.c | tail -20` qilib ko'ring: oxirida sizning kodingiz, izohlarsiz.

### 2-bosqich: kompilyator

C kodini **assembly**ga aylantiradi. Aynan shu yerda sintaksis xatolari (`expected ';'`), tur
xatolari va ogohlantirishlar chiqadi. Optimallashtirish (`-O2`) ham shu yerda.

```bash
gcc -S -O1 salom.c && cat salom.s
```

```nasm
main:
        endbr64                       # xavfsizlik belgisi (CET) - hozircha e'tibor bermang
        subq    $8, %rsp              # stekni 16 ga tekislash (17-bob)
        leaq    .LC0(%rip), %rdi      # satr manzili -> 1-argument (rdi)
        call    puts@PLT              # printf("...\n") -> puts("...")! (optimallashtirish)
        movl    $0, %eax              # return 0
        addq    $8, %rsp
        ret
```

Qiziq: kompilyator `printf("Salom, dunyo!\n")` ni `puts("Salom, dunyo!")` ga almashtirdi —
formatsiz satr uchun u tezroq. Kompilyator siz yozganingizni emas, **siz yozgan narsaning
ma'nosini** bajaradi. Buni 13-bobda (UB) eslaysiz.

### 3-bosqich: assembler

Assembly matnini **mashina kodi** baytlariga aylantiradi → **obyekt fayl** (`.o`). Unda kod bor,
lekin tashqi nomlar (`puts`) hali **ulanmagan** — "bu yerga puts ning manzili kerak" degan belgi turadi.

```bash
objdump -d salom.o       # mashina kodi va assembly yonma-yon
nm salom.o               # nomlar: T main (shu faylda aniqlangan), U puts (Undefined - tashqarida)
```

### 4-bosqich: linker (bog'lovchi)

Bir nechta `.o` fayl va kutubxonalarni **bitta** bajariladigan faylga birlashtiradi va har bir
"U" (aniqlanmagan) nomni qayerdandir topib ulaydi. `puts` — standart C kutubxonasidan (libc).

## 1.2. E'lon va ta'rif — eng muhim farq

```c
int kvadrat(int x);            /* E'LON (declaration): "shunday funksiya bor" - kompilyator uchun */

int kvadrat(int x)             /* TA'RIF (definition): funksiyaning o'zi - linker uchun */
{
    return x * x;
}
```

- **Kompilyator** faqat bitta `.c` faylni ko'radi. Funksiyani chaqirish uchun unga faqat **e'lon**
  kerak: nomi, argument turlari, qaytish turi.
- **Linker** barcha fayllarni ko'radi. Unga **ta'rif** kerak: kodning o'zi. Aynan bitta.

Shuning uchun ikki xil xato bor:

| Xato | Bosqich | Ma'nosi | Yechim |
|---|---|---|---|
| `implicit declaration of function 'f'` | kompilyator | E'lon yo'q — `f` haqida hech narsa bilmayman | Kerakli `.h` ni `#include` qiling yoki e'lon yozing |
| `undefined reference to 'f'` | linker | E'lon bor, lekin ta'rif hech qayerda yo'q | `f` yozilgan `.c` faylni kompilyatsiyaga qo'shing yoki kutubxonani ulang (`-lm`, `-pthread`) |
| `multiple definition of 'f'` | linker | Ta'rif ikki joyda bor | Ta'rifni `.h` ga yozmang — faqat e'lonni |

## 1.3. Sarlavha (`.h`) fayllari nega kerak

Katta dasturda funksiya bir faylda yoziladi, o'nlab fayllarda chaqiriladi. Har bir faylga e'lonni
qo'lda yozish — xatolarga yo'l. Yechim: e'lonlar **bitta** `.h` faylda, hamma uni `#include` qiladi.

```c
/* matematika.h */
#pragma once                   /* bu fayl bir kompilyatsiyada ikki marta qo'shilmasin */
int kvadrat(int x);
```

```c
/* matematika.c */
#include "matematika.h"        /* o'z e'loni bilan mos kelishini kompilyator tekshiradi */
int kvadrat(int x) { return x * x; }
```

```c
/* main.c */
#include <stdio.h>
#include "matematika.h"
int main(void) { printf("%d\n", kvadrat(7)); return 0; }
```

```bash
gcc -Wall -c matematika.c         # -> matematika.o
gcc -Wall -c main.c               # -> main.o
gcc main.o matematika.o -o dastur # bog'lash
```

### Bu `class` emasmi?

Yo'q. C'da `class` umuman yo'q. Bu uch fayl Python'dagi **modulga** o'xshaydi:

```python
# matematika.py
def kvadrat(x):
    return x * x

# main.py
from matematika import kvadrat
print(kvadrat(7))
```

Python'da bitta `matematika.py` yetadi. C'da esa u **ikkiga** bo'linadi:

| Python | C | Nima bor ichida |
|---|---|---|
| `matematika.py` dagi `def kvadrat(x):` qatori | `matematika.h` | Faqat **e'lon**: "shunday funksiya bor, `int` oladi, `int` qaytaradi" |
| `matematika.py` dagi funksiya tanasi | `matematika.c` | **Ta'rif**: funksiya aslida nima qiladi |
| `from matematika import kvadrat` | `#include "matematika.h"` | Boshqa fayldagi funksiyani ishlatishga ruxsat |
| `python main.py` (hammasi o'zi) | 3 ta `gcc` buyrug'i | Tarjima va birlashtirishni o'zingiz buyurasiz |

### Hayotdagi misol: restoran

- **`matematika.h` — menyu.** Unda "Osh — 30 000 so'm" deb yozilgan. Oshning qanday pishirilishi
  menyuda yo'q. Menyu faqat **nima bor** va **nima berib, nima olishingizni** aytadi.
  `int kvadrat(int x);` ham shunday: "`kvadrat` degan taom bor, unga `int` berasiz, `int` olasiz".
- **`matematika.c` — oshxona.** Oshpaz oshni aynan qanday pishirishni biladi: `return x * x;`.
  Oshxona ham o'zining menyusini o'qiydi (`#include "matematika.h"`). Aks holda menyuda "Osh" deb
  yozilgan-u, oshxona "Lag'mon" pishirib qo'yishi mumkin. Kompilyator shu nomuvofiqlikni ushlaydi.
- **`main.c` — mijoz.** U menyuni o'qiydi (`#include "matematika.h"`) va buyurtma beradi:
  `kvadrat(7)`. Mijoz oshxonaga kirmaydi, osh qanday pishishini bilishi ham shart emas.
- **`#pragma once`** — bitta stolga menyuni ikki marta qo'ymaslik. Katta dasturda bitta `.h` bir
  necha yo'l bilan qayta-qayta `#include` bo'lib qolishi mumkin. Bu qator uni faqat bir marta qo'shadi.
- **`gcc -c matematika.c`** — oshxona taomni tayyorlab, idishga solib qo'ydi (`matematika.o`).
  Lekin bu hali restoran emas — hech kimga berilmagan.
- **`gcc -c main.c`** — ofitsiant buyurtmani yozib oldi (`main.o`). Unda "kvadrat kerak" deb yozilgan,
  lekin taom qayerdaligi hali noma'lum — buyurtma varag'ida bo'sh joy qoldirilgan.
- **`gcc main.o matematika.o -o dastur`** (bog'lash, linker) — ofitsiant buyurtmani oshxonadagi
  taomga olib boradi: bo'sh joylarni to'ldiradi. Natija — ishlaydigan restoran, ya'ni `dastur`.

`printf` ham aynan shunday ishlaydi: `stdio.h` — menyu, `printf` ning o'zi (oshxonasi) esa tizimdagi
tayyor C kutubxonasida (libc) turadi. Linker uni o'zi topib ulaydi, siz sezmaysiz ham.

**Nega bunchalik murakkab?** Kichik dasturda bu ortiqcha tuyuladi. Lekin Linux yadrosida ~30 000 ta
`.c` fayl bor va minglab odam ular ustida ishlaydi. Bitta oshpaz retseptni o'zgartirsa, faqat
o'sha oshxona qaytadan ishlaydi (`matematika.c` → `matematika.o`), qolgan 29 999 tasi tegilmaydi.
Mijozlar esa menyu o'zgarmaguncha hech narsani sezmaydi.

### Qadamma-qadam o'zingiz bajaring

```bash
mkdir ~/matematika && cd ~/matematika
nano matematika.h      # yuqoridagi 3 qatorni yozing, Ctrl+O, Enter, Ctrl+X
nano matematika.c      # 3 qator
nano main.c            # 4 qator
ls                     # main.c  matematika.c  matematika.h
```

**1-qadam.** Oshxona ishlaydi:

```text
$ gcc -Wall -c matematika.c
$ ls
main.c  matematika.c  matematika.h  matematika.o      <- yangi fayl paydo bo'ldi
$ nm matematika.o
0000000000000000 T kvadrat          <- T: "kvadrat SHU YERDA bor (tayyor taom)"
```

**2-qadam.** Buyurtma yoziladi:

```text
$ gcc -Wall -c main.c
$ nm main.o
                 U kvadrat          <- U: "kvadrat KERAK, lekin bu yerda yo'q (bo'sh joy)"
0000000000000000 T main
                 U printf           <- printf ham kerak - uni libc beradi
```

`.o` fayllar hali dastur emas: `./matematika.o` qilsangiz ishlamaydi. Ular — yarim tayyor qismlar.

**3-qadam.** Bog'lash: `U kvadrat` bo'sh joyi `T kvadrat` bilan to'ldiriladi:

```text
$ gcc main.o matematika.o -o dastur
$ ./dastur
49
```

### Ataylab buzib ko'ring — har bir xato nimani anglatadi

Bu xatolarni hozir bir marta o'z ko'zingiz bilan ko'rsangiz, keyin katta loyihada darhol taniysiz.

**a) Oshxonani unutish** — bog'lashda `matematika.o` ni yozmang:

```text
$ gcc main.o -o dastur
main.c:(.text+0xe): undefined reference to `kvadrat'
collect2: error: ld returned 1 exit status
```

`ld` — linker. "Buyurtmada `kvadrat` bor, lekin hech bir oshxonada bu taom yo'q."

**b) Menyusiz buyurtma** — `main.c` dan `#include "matematika.h"` ni o'chiring:

```text
$ gcc -Wall -c main.c
main.c:2:33: warning: implicit declaration of function 'kvadrat'
```

Kompilyator: "`kvadrat` haqida hech narsa bilmayman — nima berib, nima olishini taxmin qilaman."
Taxmin noto'g'ri bo'lsa, dastur jim turib noto'g'ri ishlaydi. Shuning uchun bu ogohlantirishni
doim xato deb hisoblang (yangi GCC 14 uni allaqachon xato deb chiqaradi).

**c) Menyu va oshxona mos emas** — `matematika.c` da `int kvadrat` ni `long kvadrat` qiling:

```text
$ gcc -Wall -c matematika.c
matematika.c:2:6: error: conflicting types for 'kvadrat'; have 'long int(int)'
```

Oshxona o'z menyusini o'qigani (`#include "matematika.h"`) aynan shu xatoni ushladi.

**d) Ikki oshxonada bir xil taom** — yana bir `boshqa.c` fayl yozib, unda ham `int kvadrat(int x)`
ta'rifini qoldiring va uchala `.o` ni bog'lang:

```text
$ gcc main.o matematika.o boshqa.o -o dastur
multiple definition of `kvadrat'; matematika.o:matematika.c:(.text+0x0): first defined here
```

Linker qaysi oshxonadan olishni bilmaydi. Ta'rif butun dasturda **aynan bitta** bo'lishi shart.

To'liqroq namuna (`static`, `extern`, Makefile bilan): [misollar/11_kop_fayl/](misollar/11_kop_fayl/main.c).

**Mashqlardagi tuzilma aynan shunday:** `mashq.h` — e'lonlar, `yechim.c` — siz yozadigan ta'riflar,
`test.c` — ularni chaqiradigan kod. Tekshiruvchi `gcc yechim.c test.c` qiladi.

**MyOS'da:** har bir `kernel/xxx/yyy.c` ning yonida `yyy.h` bor. Masalan `kernel/fs/pipe.h` pipe'ning
"tashqi dunyo" uchun funksiyalarini e'lon qiladi, `pipe.c` esa ularni amalga oshiradi. Makefile har
bir `.c` ni alohida `.o` ga kompilyatsiya qilib, `kernel/linker.ld` bo'yicha bitta `kernel.elf` ga bog'laydi.

## 1.4. Nega har bir `.c` alohida kompilyatsiya qilinadi

1. **Tezlik.** Linux yadrosida ~30 000 ta `.c` fayl bor. Bitta faylni o'zgartirsangiz, faqat o'sha
   qayta kompilyatsiya qilinadi, keyin tez bog'lanadi. `make` aynan shuni kuzatadi (11-bob).
2. **Ajratish.** `static` bilan belgilangan funksiya faqat o'z faylida ko'rinadi — boshqa fayllar
   unga tasodifan tegib keta olmaydi (5-bob).

## 1.5. Kompilyatsiya bayroqlari — nega har biri kerak

| Bayroq | Nima qiladi | Nega |
|---|---|---|
| `-Wall -Wextra` | Deyarli barcha ogohlantirishlar | Xatolarni dastur ishga tushmasdan topish |
| `-Werror` | Ogohlantirish = xato | Ogohlantirishlarni "keyin tuzataman" deb qoldirmaslik. MyOS va mashqlarda yoqilgan |
| `-g` | Debug ma'lumoti | `gdb` qatorlar va o'zgaruvchi nomlarini ko'rsatadi |
| `-O0` / `-O2` | Optimallashtirish darajasi | `-O0` — debug uchun (kod siz yozgandek), `-O2` — tez kod |
| `-std=c11` / `-std=gnu11` | Til standarti | `gnu11` — C11 + GNU kengaytmalari (inline asm). MyOS shuni ishlatadi |
| `-I papka` | `#include` qidiriladigan papka | `-Ikernel` bilan `#include "fs/vfs.h"` ishlaydi |
| `-c` | Faqat `.o` gacha | Katta loyihalar uchun |
| `-fsanitize=address,undefined` | Ish vaqtida xatolarni ushlash | O'rganish va testlar uchun |
| `-ffreestanding` | "OS yo'q" rejimi | **Yadro uchun**: standart kutubxona yo'q deb hisoblash (18-bob) |

## 1.6. `gdb` — dastur ichiga qarash

```bash
gcc -g -O0 dastur.c -o dastur
gdb ./dastur
```

```text
(gdb) break main          # main da to'xta
(gdb) run                 # ishga tushir
(gdb) next                # keyingi qator (funksiyaga kirmasdan)
(gdb) step                # keyingi qator (funksiyaga kirib)
(gdb) print x             # o'zgaruvchi qiymati
(gdb) print *p            # ko'rsatkich ko'rsatgan qiymat
(gdb) bt                  # chaqiruvlar zanjiri (backtrace): qayerdan kelib qoldik
(gdb) info locals         # hamma lokal o'zgaruvchilar
(gdb) continue            # keyingi to'xtash nuqtasigacha
(gdb) quit
```

Dastur qulaganda (`Segmentation fault`) — `gdb ./dastur`, `run`, keyin `bt`: qaysi qatorda qulagani
darhol ko'rinadi. **MyOS'ni ham aynan shu gdb bilan debug qilasiz** (`make debug`, docs/08).

## 1.7. Savol-javob

**`.o` va bajariladigan fayl farqi?**
`.o` — "yarim tayyor": kodda tashqi manzillar hali to'ldirilmagan va `main` dan boshlash haqida
ma'lumot yo'q. Bajariladigan fayl (ELF formati, Linux'da) — to'liq bog'langan, OS uni yuklab
ishga tushira oladi. MyOS'ning `kernel/sys/elf.c` fayli aynan shu formatni o'qib, dasturni yuklaydi.

**`-lm` nima?**
`-l` — kutubxonani ulash: `-lm` → `libm.so` (matematika: `sqrt`, `sin`). Kutubxona — `.o` fayllar
to'plami. MyOS'ning `libc.a` si ham shunday (`Makefile` → `ar rcs`).

**`#pragma once` nima uchun?**
`a.h` va `b.h` ikkalasi ham `c.h` ni qo'shsa, `main.c` ikkalasini qo'shganda `c.h` ikki marta
kiradi → `struct` ikki marta ta'riflanadi → xato. `#pragma once` "bu faylni bitta kompilyatsiyada
faqat bir marta qo'sh" deydi. Eski usul — "include guard" (10-bob).

## 1.8. O'zingizni tekshiring

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

## 1.9. Mashq

- 1.3-bo'limdagi uch faylli dasturni yarating. Keyin `matematika.o` ni bog'lash buyrug'idan
  olib tashlab, linker xatosini o'qing. `#include "matematika.h"` ni olib tashlab, kompilyator
  xatosini o'qing.
- `gcc -S -O0` va `gcc -S -O2` bilan kichik funksiyaning assembly'sini solishtiring.
- `mashqlar/01_kvadratlar` ni oching: `mashq.h`, `yechim.c`, `test.c` qanday bog'langanini tushuning.
  Hozircha yechmasangiz ham bo'ladi.

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

```c
/* geometriya.c - oshxona */
#include "geometriya.h"

int yuza(int a, int b) { return a * b; }
int perimetr(int a, int b) { return 2 * (a + b); }
int kvadrat(int a) { return a * a; }
int kub(int a) { return a * a * a; }
int yarim_yigindi(int a, int b) { return (a + b) / 2; }
```

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
