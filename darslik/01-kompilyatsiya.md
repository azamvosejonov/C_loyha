# 1-bob. Kompilyatsiya: koddan dasturgacha

> **Bu bobdan keyin:** `gcc` ichida nima bo'layotganini, `.c`, `.h`, `.o` fayllar farqini,
> "undefined reference" va "implicit declaration" xatolari nimadan kelib chiqishini bilasiz.
> Yadro yozishda bu bilim **majburiy**: yadro Makefile'i va linker skripti aynan shu bosqichlarni boshqaradi.

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

Keyingi bob: [2-bob. O'zgaruvchilar va turlar](02-turlar.md)
