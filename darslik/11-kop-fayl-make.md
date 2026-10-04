# 11-bob. Ko'p faylli dasturlar, bog'lanish (linkage) va Make

> **Bu bobda nima o'rganasiz:** katta dasturni fayllarga qanday bo'lishni; `extern` va `static` ning bog'lanishdagi ma'nosini; kutubxonalar (`.a`) ni yasashni;
> `Makefile` o'qish va yozishni. MyOS'ning `Makefile`'i endi siz uchun ochiq kitob bo'ladi.
> **Oldindan nima kerak:** 1-bob (e'lon/ta'rif, `.h`/`.c`/`.o`), 5-bob (`static` funksiya), 10-bob (`#include`, `#pragma once`).   **Vaqt:** 5–6 soat.

> **To'liq ishlaydigan misol:** [misollar/11_kop_fayl/](misollar/11_kop_fayl/main.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Kichik dastur bitta faylda sig'adi. Katta dastur (Linux — 30 000 dan ortiq `.c` fayl) — yo'q. Dasturni **modullarga** bo'lamiz, har modul o'z fayllari bilan. Lekin ko'p fayl
yangi savollar tug'diradi: bir fayldagi o'zgaruvchini boshqa fayl qanday ko'radi? Ikki faylda bir xil nom bo'lsa-chi? 30 ta faylni har safar qo'lda kompilyatsiya qilamizmi?

**Hayotdan misol: qurilish brigadalari.** Uy qurilishida elektriklar, santexniklar va suvoqchilar alohida ishlaydi. Har bir brigada faqat o'z ishini biladi va boshqalar bilan
**chizma** (`.h` fayl) orqali kelishadi: "rozetka shu devorda, 30 sm balandlikda". Bitta brigada ishini qayta qilsa, qolganlari qaytadan ishlamaydi.

| Qurilishda | C dasturida |
|---|---|
| brigada | modul (`.c` fayl) |
| chizma (kelishuv) | `.h` fayl |
| brigadaning bajargan ishi | `.o` fayl |
| uyni yig'ish | linker |
| prorab ("nima o'zgardi?") | **Make** |

## 11.1. Loyihani fayllarga bo'lish

Qoidalar (MyOS va Linux shunday tuzilgan):

1. Har bir **modul** — bir juft fayl: `pipe.h` (interfeys) + `pipe.c` (amalga oshirish).
2. `.h` da faqat boshqalarga kerak bo'lgan narsa: e'lonlar, tuzilmalar, o'zgarmaslar, `static inline`.
3. `.c` dagi ichki funksiyalar va o'zgaruvchilar — `static`.
4. Har bir `.c` o'zining `.h` ini **birinchi** bo'lib `#include` qiladi — shunda `.h` o'zi yetarli (kerakli include'lari bilan) ekani tekshiriladi.

## 11.2. Bog'lanish (linkage): `extern` va `static`

Global nom (funksiya yoki o'zgaruvchi) ikki xil bo'ladi:

| | Tashqi bog'lanish (external) | Ichki bog'lanish (internal) |
|---|---|---|
| Yozuv | `int hisob;` / `void f(void) {}` | `static int hisob;` / `static void f(void) {}` |
| Ko'rinishi | barcha fayllarda (linker ulaydi) | faqat shu faylda |

**Hayotdan misol: `extern` — "u boshqa bo'limda ishlaydi".** Katta tashkilotda: "Hisob-kitob bo'yicha Karimova opaga murojaat qiling, u 3-qavatda". Siz uni ko'rmagansiz, lekin mavjudligini bilasiz.
`extern int soni;` — "`soni` degan o'zgaruvchi bor, lekin boshqa faylda yashaydi". Uni topib ulash — linkerning ishi.

**Hayotdan misol: `static` — oilaviy ish.** Oila ichidagi gaplar ko'chaga chiqmaydi. `static` funksiya faqat o'z faylida ko'rinadi — boshqa fayllar uni chaqira olmaydi va tasodifan bir xil nomli
funksiya yozib qo'ysa ham, to'qnashuv bo'lmaydi.

Uch fayl bilan amalda ko'ramiz (taymer: tashqariga `tiklar` o'zgaruvchisi va funksiyalar, ichkarida `static` narsalar):

```c
/* taymer.h - chizma: tashqi dunyo uchun nima bor */
#pragma once
#include <stdint.h>

extern uint64_t tiklar;                 /* E'LON: "boshqa joyda bor" - xotira ajratilmaydi */
void taymer_tik(void);
uint64_t taymer_tiklar(void);
```

**Bu dastur nima qiladi (umumiy):** `taymer` brigadasining ichki ishi: global hisoblagich (ta'rif shu yerda), `static` o'zgaruvchi va funksiya hamda tashqariga ochiq funksiyalar.

```c
/* taymer.c - brigadaning ichki ishi */
#include "taymer.h"

uint64_t tiklar = 0;                    /* TA'RIF: xotira shu yerda ajratiladi */
static int ichki_hisob = 0;             /* static o'zgaruvchi: faqat shu faylda */

static void ichki(void)                 /* static funksiya: faqat shu faylda */
{
    ichki_hisob++;
}

void taymer_tik(void)
{
    ichki();
    tiklar++;
}

uint64_t taymer_tiklar(void)
{
    return tiklar;
}
```

**Bu dastur nima qiladi (umumiy):** taymerdan foydalanuvchi dastur: hisoblagichni ham funksiya orqali, ham `extern` bilan to'g'ridan-to'g'ri o'qiydi.

```c
/* taymer_main.c - foydalanuvchi */
#include <inttypes.h>
#include <stdio.h>
#include "taymer.h"

int main(void)
{
    taymer_tik();
    taymer_tik();
    taymer_tik();
    printf("funksiya orqali: %" PRIu64 "\n", taymer_tiklar());
    printf("to'g'ridan-to'g'ri (extern): %" PRIu64 "\n", tiklar);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -c taymer.c
$ gcc -Wall -Wextra -c taymer_main.c
$ gcc taymer.o taymer_main.o -o taymer_dastur
$ ./taymer_dastur
funksiya orqali: 3
to'g'ridan-to'g'ri (extern): 3
$ nm taymer.o
0000000000000000 t ichki
0000000000000008 b ichki_hisob
000000000000001a T taymer_tik
000000000000003c T taymer_tiklar
0000000000000000 B tiklar
```

**Kodda nimalar bor:**

| Qism | Nima qiladi | Nega |
|---|---|---|
| `extern uint64_t tiklar;` (`.h` da) | e'lon: "`tiklar` bor, boshqa faylda" | `taymer_main.c` uni ishlata olishi uchun; **xotira ajratmaydi** |
| `uint64_t tiklar = 0;` (`.c` da) | ta'rif: xotira **shu yerda** | butun dasturda ta'rif **aynan bitta** bo'lishi shart |
| `static int ichki_hisob` | faqat `taymer.c` ko'radi | boshqa fayllar tegib ketmasin |
| `static void ichki(void)` | faqat `taymer.c` chaqira oladi | ichki tafsilot |
| `PRIu64` | `uint64_t` ni chop etish formati (`<inttypes.h>`) | `%lu` har tizimda to'g'ri bo'lmasligi mumkin |

**`nm` natijasi** (har qator: manzil, **harf**, nom):

| Harf | Ma'nosi |
|---|---|
| `T` | tashqariga ochiq **funksiya** (katta harf = tashqi bog'lanish) |
| `t` | **static** funksiya (kichik harf = ichki; boshqa fayllar ko'rmaydi) |
| `B` | tashqariga ochiq global o'zgaruvchi (boshlang'ich 0 — `.bss`) |
| `b` | **static** global o'zgaruvchi |

Endi uchta xatoni ataylab qilib ko'ramiz — bularni linker xatolari sifatida taniysiz:

**a) `static` funksiyani boshqa fayldan chaqirish:**

**Bu dastur nima qiladi (umumiy):** boshqa fayldagi `static` funksiyani chaqirishga urinadi — linker xatosi (ataylab xatoli).

```c
/* static_xato.c - boshqa faylning static funksiyasi */
void ichki(void);                       /* e'lonni o'zimiz yozdik */

int main(void)
{
    ichki();
    return 0;
}
```

```console
$ gcc -Wall -Wextra -c static_xato.c
$ gcc static_xato.o taymer.o -o static_xato # xato kutiladi
/usr/bin/ld: static_xato.o: in function `main':
static_xato.c:(.text+0x9): undefined reference to `ichki'
collect2: error: ld returned 1 exit status
```

E'lon yozish kompilyatorni qanoatlantirdi, lekin `taymer.o` da `ichki` — `t` (kichik harf): **ko'rinmaydi**. Linker: `undefined reference to 'ichki'`.

**b) `extern` e'lon bor, ta'rif yo'q:**

**Bu dastur nima qiladi (umumiy):** `extern` bilan e'lon qilingan, lekin hech joyda ta'riflanmagan o'zgaruvchini ishlatadi — linker xatosi (ataylab xatoli).

```c
/* extern_xato.c - ta'rifsiz extern */
extern int yoq_bunaqasi;

int main(void)
{
    return yoq_bunaqasi;
}
```

```console
$ gcc -Wall -Wextra -c extern_xato.c
$ gcc extern_xato.o -o extern_xato # xato kutiladi
/usr/bin/ld: extern_xato.o: warning: relocation against `yoq_bunaqasi' in read-only section `.text'
/usr/bin/ld: extern_xato.o: in function `main':
extern_xato.c:(.text+0xa): undefined reference to `yoq_bunaqasi'
/usr/bin/ld: warning: creating DT_TEXTREL in a PIE
collect2: error: ld returned 1 exit status
```

Asosiy qator — `undefined reference to 'yoq_bunaqasi'` (qolgan linker ogohlantirishlari ikkinchi darajali).

**c) Ta'rif ikki joyda (`.h` ga o'zgaruvchi ta'rifini yozish):**

```c
/* sanagich.h - YOMON: ta'rif .h da */
#pragma once
int sanagich;
```

```c
/* s1.c */
#include "sanagich.h"

int main(void)
{
    return sanagich;
}
```

```c
/* s2.c */
#include "sanagich.h"

int boshqa(void)
{
    return sanagich;
}
```

```console
$ gcc -Wall -Wextra -c s1.c
$ gcc -Wall -Wextra -c s2.c
$ gcc s1.o s2.o -o s12 # xato kutiladi
/usr/bin/ld: s2.o:(.bss+0x0): multiple definition of `sanagich'; s1.o:(.bss+0x0): first defined here
collect2: error: ld returned 1 exit status
```

`.h` ikki `.c` ga kirdi → `sanagich` ikki joyda ta'riflandi → `multiple definition`. **Yechim:** `.h` da `extern int sanagich;`, bitta `.c` da `int sanagich;` (yuqoridagi `tiklar` kabi).

**Umumiy qoida:** global o'zgaruvchilardan iloji boricha qoching. Kerak bo'lsa — `static` qilib, tashqariga funksiyalar orqali bering (`uint64_t taymer_tiklar(void)`). Shunda kim o'zgartirishini
nazorat qilasiz (ko'p yadroli tizimda bu hayotiy). MyOS'da: `extern uint64_t tsc_khz;` (`kernel/arch/tsc.h`), `extern const struct amal amallar[];` (20-mashq).

> **Eslab qoling:** `extern` = "boshqa faylda bor" (e'lon, `.h` da). Ta'rif — aynan bitta `.c` da. `static` = "faqat shu fayl" (nom to'qnashmaydi).

## 11.3. Kutubxonalar

**Hayotdan misol: asboblar.**

- **Statik kutubxona (`.a`)** — asbobni **sotib olish**: u sizning ustaxonangizda qoladi. Dastur kattaroq bo'ladi, lekin hech kimga bog'liq emas.
- **Dinamik kutubxona (`.so`)** — mahalladagi **umumiy ijaraxona**: hamma bitta asbobdan foydalanadi. Dastur kichik, lekin ishga tushganda ijaraxona (kutubxona) joyida bo'lishi shart.
  Asbob yangilansa — hamma birdaniga yangisini oladi.

**Statik kutubxona** (`.a`) — `.o` fayllar arxivi. Kichik namuna:

```c
/* kutub.h */
#pragma once

int qoshish(int a, int b);
int ayirish(int a, int b);
```

```c
/* kutub_a.c */
#include "kutub.h"

int qoshish(int a, int b) { return a + b; }
```

```c
/* kutub_b.c */
#include "kutub.h"

int ayirish(int a, int b) { return a - b; }
```

```c
/* kutub_main.c */
#include <stdio.h>
#include "kutub.h"

int main(void)
{
    printf("qoshish(7, 3) = %d, ayirish(7, 3) = %d\n", qoshish(7, 3), ayirish(7, 3));
    return 0;
}
```

```console
$ gcc -Wall -Wextra -c kutub_a.c kutub_b.c
$ ar rcs libmening.a kutub_a.o kutub_b.o
$ ar t libmening.a
kutub_a.o
kutub_b.o
$ gcc -Wall -Wextra kutub_main.c -L. -lmening -o kutub_dastur
$ ./kutub_dastur
qoshish(7, 3) = 10, ayirish(7, 3) = 4
```

**Buyruqlar:**

| Buyruq | Nima qiladi |
|---|---|
| `ar rcs libmening.a a.o b.o` | `.o` fayllarni `libmening.a` arxiviga joylaydi (`r` — qo'sh, `c` — yarat, `s` — indeks tuz) |
| `ar t libmening.a` | arxiv ichidagi fayllar ro'yxati |
| `-L.` | kutubxonani **qayerdan** qidirish (`.` — joriy papka) |
| `-lmening` | `libmening.a` ni ulash (`-l` + nom; `lib` va `.a` yozilmaydi) |

Linker arxivdan faqat **kerakli** `.o` larni oladi. **Tartib muhim:** kutubxona undan foydalanadigan fayllardan **keyin** yozilishi kerak (linker chapdan o'ngga yuradi,
faqat o'sha paytgacha "yetishmayotgan" nomlarni arxivdan oladi):

```console
$ gcc -L. -lmening kutub_main.c -o kutub_xato 2>&1 | sed 's#/tmp/cc[A-Za-z0-9]*[.]o#kutub_main.o#' # xato kutiladi
/usr/bin/ld: kutub_main.o: in function `main':
kutub_main.c:(.text+0x18): undefined reference to `ayirish'
/usr/bin/ld: kutub_main.c:(.text+0x29): undefined reference to `qoshish'
collect2: error: ld returned 1 exit status
```

Kutubxona oldinda turdi — linker u yerda hech narsa kerak emas deb o'tib ketdi, keyin `kutub_main.c` ning `qoshish` ga ehtiyoji qolib ketdi. MyOS Makefile'ida shu haqida izoh bor:
`crt0.o + dastur.o + libc.a` — `libc.a` eng oxirida.

**Dinamik kutubxona** (`.so`) — dastur ishga tushganda yuklanadi (Linux'dagi `libc.so.6`). MyOS'da hali yo'q (YAKUNIY.md, 26.12-loyiha).

## 11.4. Make — nima uchun

**Hayotdan misol: aqlli prorab.** Prorab har kuni ertalab tekshiradi: "Chizma o'zgardimi? O'zgargan bo'lsa, unga bog'liq devorlarni qayta qur. O'zgarmagan bo'lsa — tegma".
Make ham fayllarning **o'zgartirilgan vaqtini** solishtiradi: `.c` fayl `.o` dan yangiroq bo'lsa — qayta kompilyatsiya qiladi, aks holda o'tkazib yuboradi. Linux'dagi 30 000 fayldan bittasini
o'zgartirsangiz, bir necha soniyada yig'iladi.

Qo'lda `gcc a.c b.c c.c ... -o dastur` — har safar hammasini qayta kompilyatsiya qiladi. Make esa **faqat o'zgargan** fayllarni qayta yig'adi: har bir natija fayl uchun "u nimadan yasaladi" va
"qanday yasaladi" qoidasini yozasiz.

### Qoida sintaksisi

```text
NISHON: BOG'LIQLIKLAR
<TAB>BUYRUQ          # TAB belgisi bilan boshlanishi SHART (bo'shliqlar bilan - xato!)
```

- **Nishon** (target) — yasalishi kerak bo'lgan fayl nomi.
- **Bog'liqliklar** — nishon nimaga tayanadi. Ulardan biri nishondan **yangi** bo'lsa, nishon qayta yasaladi.
- **Buyruq** — yasash usuli.

```make
# Makefile.oddiy - birinchi Makefile (nom: Makefile.oddiy)
kutub_dastur2: kutub_main.o kutub_a.o kutub_b.o
	gcc kutub_main.o kutub_a.o kutub_b.o -o kutub_dastur2

kutub_main.o: kutub_main.c kutub.h
	gcc -Wall -c kutub_main.c

kutub_a.o: kutub_a.c kutub.h
	gcc -Wall -c kutub_a.c

kutub_b.o: kutub_b.c kutub.h
	gcc -Wall -c kutub_b.c

clean:
	rm -f *.o kutub_dastur2
```

```console
$ make -f Makefile.oddiy
gcc -Wall -c kutub_main.c
gcc kutub_main.o kutub_a.o kutub_b.o -o kutub_dastur2
$ make -f Makefile.oddiy
make: 'kutub_dastur2' is up to date.
$ ./kutub_dastur2
qoshish(7, 3) = 10, ayirish(7, 3) = 4
```

`make -f Makefile.oddiy` — birinchi nishonni (`kutub_dastur2`) yig'adi. U `kutub_main.o`, `kutub_a.o`, `kutub_b.o` ga tayanadi. Biz `kutub_a.o` va `kutub_b.o` ni yuqorida allaqachon yig'gan edik
(yangi, shuning uchun tegilmadi), `kutub_main.o` esa yo'q edi — **faqat shu** kompilyatsiya qilindi, keyin hammasi bog'landi. **Ikkinchi** `make` hech narsa qilmadi ("up to date") — o'zgarish yo'q.

**TAB tuzog'i:** buyruq qatori **TAB** bilan boshlanishi shart.

```make
# Makefile.xato - bo'shliq bilan yozilgan buyruq
qoida:
    echo salom
```

```console
$ make -f Makefile.xato # xato kutiladi
Makefile.xato:3: *** missing separator.  Stop.
```

`missing separator` — TAB o'rniga bo'shliq yozilgan. Tarixiy qaror (1976): buyruq qatorlarini ajratish uchun.

### O'zgaruvchilar va naqsh qoidalari

```make
# Makefile.avto - o'zgaruvchilar va naqsh qoidalari
CC      := gcc
CFLAGS  := -Wall -Wextra -g
SRC     := kutub_main.c kutub_a.c kutub_b.c
OBJ     := $(SRC:.c=.o)                 # kutub_main.c -> kutub_main.o

kutub_dastur3: $(OBJ)
	$(CC) $^ -o $@

%.o: %.c kutub.h                        # NAQSH: har qanday X.o X.c dan
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ) kutub_dastur3
```

```console
$ rm -f kutub_main.o kutub_a.o kutub_b.o
$ make -f Makefile.avto
gcc -Wall -Wextra -g -c kutub_main.c -o kutub_main.o
gcc -Wall -Wextra -g -c kutub_a.c -o kutub_a.o
gcc -Wall -Wextra -g -c kutub_b.c -o kutub_b.o
gcc kutub_main.o kutub_a.o kutub_b.o -o kutub_dastur3
$ ./kutub_dastur3
qoshish(7, 3) = 10, ayirish(7, 3) = 4
```

| Belgi | Ma'nosi | Misolda |
|---|---|---|
| `$@` | nishon (natija fayl) | `kutub_dastur3` yoki `kutub_a.o` |
| `$<` | birinchi bog'liqlik | `kutub_a.c` |
| `$^` | hamma bog'liqliklar | `kutub_main.o kutub_a.o kutub_b.o` |
| `%` | naqsh: "istalgan nom" | `%.o: %.c` — har qanday `X.o` `X.c` dan yasaladi |
| `:=` | darhol hisoblanadigan o'zgaruvchi | `CC := gcc` |
| `?=` | "tashqaridan berilmagan bo'lsa" | `make QEMU_MEM=512M` |
| `$(SRC:.c=.o)` | ro'yxatda `.c` ni `.o` ga almashtirish | |
| `$(wildcard *.c)`, `$(patsubst ...)` | fayllar ro'yxati bilan ishlash funksiyalari | |
| `.PHONY` | "bu nishon fayl emas, buyruq" | `clean` |

**Nega naqsh qoidasi kerak?** 3 ta fayl uchun 3 ta qoida yozish mumkin, 300 ta fayl uchun — yo'q. `%.o: %.c` — bitta qoida hammasiga.

### Sarlavha bog'liqliklarini avtomatik kuzatish

`.h` o'zgarsa, uni `#include` qilgan hamma `.c` qayta yig'ilishi kerak. Qo'lda yozish — mashaqqat (biz `kutub.h` ni qo'lda yozdik). GCC buni o'zi yozadi:

```make
CFLAGS += -MMD -MP          # har bir .o yonida .d fayl: "main.o: main.c matematika.h"
-include $(OBJ:.o=.d)       # o'sha qoidalarni qo'shish
```

```console
$ gcc -MMD -MP -c kutub_main.c
$ cat kutub_main.d
kutub_main.o: kutub_main.c kutub.h
kutub.h:
```

`.d` fayl — GCC yozgan qoida: "`kutub_main.o` `kutub_main.c` va `kutub.h` ga bog'liq" (`-MMD` tizim sarlavhalarini — `stdio.h` — kiritmaydi; `-MD` hammasini kiritadi). `kutub.h:` qatori (`-MP`) — bo'sh qoida:
`.h` keyin o'chirilsa, Make xato bermasligi uchun. Make'ga `-include` qilsangiz, ro'yxatni siz yozmaysiz.

## 11.5. MyOS Makefile'ini o'qish

Endi `Makefile` ni oching — u to'liq izohlangan. Asosiy qismlari:

| Qism | Nima qiladi |
|---|---|
| `KERNEL_CFLAGS` | Yadro uchun bayroqlar: `-ffreestanding`, `-mno-red-zone`, `-mcmodel=kernel`... Har biri izohlangan (18-bob) |
| `$(BUILD)/kernel/%.c.o: kernel/%.c` | Har bir yadro `.c` → `.o` |
| `$(BUILD)/kernel.elf` | Hamma `.o` → `linker.ld` bo'yicha bitta yadro |
| `$(LIBC_A)` | `user/libc/*.c` → `libc.a` |
| `$(BUILD)/user/bin/%.elf` | Har bir dastur: `crt0.o` + `dastur.o` + `libc.a` |
| `initrd.tar`, `myos.iso` | Root fayl tizimi va yuklanadigan disk |
| `FLAGS_STAMP` | Bayroqlar o'zgarsa hammasini qayta yig'ish hiylasi |

## Hayotdan misol va to'liq dastur

**Ob-havo stantsiyasi (3 fayl + Makefile).** Harorat birliklari: bir modul (`harorat.c`) tashqariga ikkita funksiya va bitta o'zgaruvchi beradi, ichkarida yaxlitlash `static`. Uni asosiy dastur
(`stantsiya.c`) ishlatadi, Make esa yig'adi.

```c
/* harorat.h - chizma: tashqi dunyo uchun nima bor */
#pragma once

double selsiydan_farengeytga(double c);
double ortacha(const double *qiymatlar, int soni);
extern int olchovlar_soni;                      /* e'lon: o'zgaruvchi harorat.c da yashaydi */
```

**Bu dastur nima qiladi (umumiy):** harorat kutubxonasining ichki ishi: o'lchovlar sanagichi (ta'rif shu yerda), `static` yordamchi `yaxlitla` va tashqariga ochiq funksiyalar (Selsiy → Farengeyt, o'rtacha).

```c
/* harorat.c - brigadaning ichki ishi */
#include "harorat.h"

int olchovlar_soni = 0;                         /* ta'rif: xotira shu yerda ajratiladi */

static double yaxlitla(double x)                /* static: faqat shu fayl uchun */
{
    return (double)(long)(x * 10 + (x >= 0 ? 0.5 : -0.5)) / 10;
}

double selsiydan_farengeytga(double c)
{
    olchovlar_soni++;
    return yaxlitla(c * 9 / 5 + 32);
}

double ortacha(const double *q, int n)
{
    double s = 0;
    for (int i = 0; i < n; i++)
        s += q[i];
    return yaxlitla(s / n);
}
```

**Bu dastur nima qiladi (umumiy):** ob-havo stansiyasining asosiy dasturi: bir necha kunlik haroratni Farengeytga o'giradi, o'rtachani va konvertatsiyalar sonini chiqaradi.

```c
/* stantsiya.c - asosiy dastur */
#include <stdio.h>
#include "harorat.h"

int main(void)
{
    double kun[] = { 18.5, 22.0, 27.3, 24.1 };
    for (int i = 0; i < 4; i++)
        printf("%5.1f C = %5.1f F\n", kun[i], selsiydan_farengeytga(kun[i]));
    printf("O'rtacha: %.1f C\n", ortacha(kun, 4));
    printf("Konvertatsiyalar soni: %d\n", olchovlar_soni);
    return 0;
}
```

```make
# Makefile - prorab uchun reja
CFLAGS := -Wall -Wextra

stantsiya: stantsiya.o harorat.o
	$(CC) $^ -o $@

stantsiya.o: stantsiya.c harorat.h
harorat.o: harorat.c harorat.h

clean:
	rm -f stantsiya *.o
```

```console
$ make
cc -Wall -Wextra   -c -o stantsiya.o stantsiya.c
cc -Wall -Wextra   -c -o harorat.o harorat.c
cc stantsiya.o harorat.o -o stantsiya
$ ./stantsiya
 18.5 C =  65.3 F
 22.0 C =  71.6 F
 27.3 C =  81.1 F
 24.1 C =  75.4 F
O'rtacha: 23.0 C
Konvertatsiyalar soni: 4
$ make
make: 'stantsiya' is up to date.
$ touch harorat.c && make
cc -Wall -Wextra   -c -o harorat.o harorat.c
cc stantsiya.o harorat.o -o stantsiya
$ nm harorat.o | grep -i "yaxlitla\|olchovlar\|selsiy"
0000000000000000 B olchovlar_soni
000000000000005f T selsiydan_farengeytga
0000000000000000 t yaxlitla
```

**Kodda nimalar bor:**

| Qism | Nima qiladi |
|---|---|
| `harorat.h` | chizma: ikki funksiya e'loni va `extern int olchovlar_soni` |
| `harorat.c` | ta'riflar; `olchovlar_soni` xotirasi shu yerda; `yaxlitla` — `static` (ichki yordamchi) |
| `stantsiya.c` | `harorat.h` ni qo'shadi va ishlatadi; `yaxlitla` ni **ko'rmaydi** (chaqirsa — linker xatosi, yuqorida ko'rgansiz) |
| `Makefile` | `stantsiya` ← `stantsiya.o` + `harorat.o`; `.o` lar `.c` va `harorat.h` ga bog'liq. `.o` ni qanday yasashni **yozmadik** — Make o'zi biladi (o'rnatilgan qoida: `$(CC) $(CFLAGS) -c`) |

**Qadamlar:**

1. Birinchi `make`: `.o` lar yo'q → ikkalasi kompilyatsiya qilinadi → keyin `stantsiya` bog'lanadi.
2. Ikkinchi `make`: hech narsa o'zgarmagan → **"up to date"** (hech narsa qilinmaydi).
3. `touch harorat.c` ("faylni o'zgartirilgan deb belgilash") dan keyin faqat `harorat.o` qayta yig'ildi va qayta bog'landi — `stantsiya.o` ga **tegilmadi**.
4. `nm`: `T selsiydan_farengeytga` (ochiq funksiya), `t yaxlitla` (**static** — kichik harf), `B olchovlar_soni` (global o'zgaruvchi).

**Hisob tekshiruvi:** 18.5 °C = 18.5 × 9/5 + 32 = 65.3 °F ✓. O'rtacha (18.5 + 22 + 27.3 + 24.1) / 4 = 23.0 ✓.

**Sinab ko'ring:** `stantsiya.c` da `yaxlitla(1.26)` ni chaqiring — linker nima deydi va nega? `harorat.c` dagi `int olchovlar_soni = 0;` ni o'chiring — xato qaysi bosqichda chiqadi?

## Bob xulosasi (yodlash uchun)

1. Katta dasturni modullarga bo'ling: har modul = `.h` (e'lon, chizma) + `.c` (ta'rif); har `.c` o'z `.h` ini birinchi `#include` qiladi.
2. `extern` — "boshqa faylda bor" (`.h` da); ta'rif — aynan **bitta** `.c` da. `static` — faqat shu fayl (nom to'qnashmaydi).
3. Linker xatolari: `undefined reference` (ta'rif yo'q yoki `static`), `multiple definition` (ta'rif ikki joyda, masalan `.h` da).
4. Statik kutubxona: `ar rcs libX.a a.o b.o`, ulash `-L. -lX` — kutubxona **oxirida** yozilsin.
5. Make: `nishon: bog'liqliklar` + **TAB** + buyruq; faqat o'zgarganini qayta yig'adi; `$@`, `$<`, `$^`, `%`.

## Savol-javob

**Make'da nega TAB shart?**
Tarixiy qaror (1976): buyruq qatorlarini ajratish uchun. `Makefile:5: *** missing separator` xatosi — deyarli har doim TAB o'rniga bo'shliq.

**CMake, Meson nima?**
Makefile'larni avtomatik yaratadigan yuqori darajadagi vositalar. Yadrolar (Linux, MyOS) odatda oddiy Make ishlatadi — to'liq nazorat uchun.

**`multiple definition of 'x'` — global o'zgaruvchi bilan?**
`.h` da `int x;` yozilgan va ikki `.c` uni qo'shgan. `.h` da `extern int x;`, bitta `.c` da `int x;`.

**`.o` fayl o'zgarmasa ham Make nega qayta yig'adi?**
Bog'liqlikda `.h` ham bo'lsa va u yangilangan bo'lsa. Shu uchun `.h` bog'liqliklarini to'g'ri yozish (yoki `-MMD`) muhim.

## O'zingizni tekshiring

1. `static int n;` fayl darajasida nimani bildiradi?
2. `extern` qachon kerak?
3. Makefile'da `$@`, `$<`, `$^`?
4. Nega kutubxona buyruq qatorida oxirida turishi kerak?
5. `nm` da `t` va `T` farqi nima?

<details><summary>Javoblar</summary>

1. n faqat shu faylda ko'rinadi (ichki bog'lanish), statik xotirada, boshlang'ich qiymati 0.
2. Boshqa faylda ta'riflangan global o'zgaruvchini e'lon qilish uchun.
3. Nishon, birinchi bog'liqlik, hamma bog'liqliklar.
4. Linker chapdan o'ngga yuradi va arxivdan faqat o'sha paytgacha "yetishmayotgan" nomlarni oladi.
5. `T` — tashqariga ochiq funksiya, `t` — `static` (faqat shu fayl).
</details>

## Mashq

- 11.2 va 11.3 dagi fayllarni o'zingiz yozib, har bir xatoni (a, b, c) ko'ring. Har xabarni so'zlab tushuntiring.
- `Makefile.avto` ga `-MMD -MP` qo'shing va `.h` ni o'zgartirib, faqat kerakli `.c` lar qayta yig'ilishini tekshiring.
- MyOS `Makefile` ini oching va 11.5 jadvalidagi har bir qismni toping.

<!-- loyiha:boshi -->
## Loyiha: bank moduli (3 fayl + Makefile)

**Maqsad:** loyihani modullarga bo'lish: kim nimani **ko'radi** va nimani **yashiradi**.
**Bobdan ishlatiladi:** `.h`/`.c`/`main.c` taqsimoti, `static` (fayl ichida yashirish), Makefile, `-MMD`.

**Talab:** hisob ochish, o'tkazma, hisobot. Hisoblar ro'yxati **modul ichida yashirin** — tashqaridan faqat
raqam (`id`) va funksiyalar orqali murojaat qilinadi. Shunda ichki tuzilishni keyin o'zgartirish mumkin
(masalan, massivni bog'langan ro'yxatga almashtirish) va `main.c` ga tegish shart bo'lmaydi.

**Qaysi fayl nima biladi:**

| Fayl | Ko'radi | Yashiradi |
|---|---|---|
| `bank.h` | funksiya e'lonlari, `BANK_MAX` | hech narsa (menyu) |
| `bank.c` | hisoblar massivi (`static`), yordamchi `togri_id` (`static`) | tashqaridan ko'rinmaydi |
| `main.c` | faqat `bank.h` | `bank.c` ichini bilmaydi |

```c
/* bank.h - menyu */
#pragma once

#define BANK_MAX 8

int bank_och(const char *ism, long boshlangich);    /* hisob raqami (id) yoki -1: joy yo'q */
int bank_otkazma(int dan, int ga, long summa);      /* 0 - bajarildi, -1 - rad etildi */
long bank_balans(int id);
void bank_hisobot(void);
```

```c
/* bank.c - oshxona */
#include "bank.h"

#include <stdio.h>
#include <string.h>

struct hisob {                                  /* main.c bu tuzilmani KO'RMAYDI */
    char ism[16];
    long balans;
};
static struct hisob hisoblar[BANK_MAX];         /* static: faqat shu faylda ko'rinadi */
static int soni;

static int togri_id(int id)
{
    return id >= 0 && id < soni;
}

int bank_och(const char *ism, long boshlangich)
{
    if (soni == BANK_MAX)
        return -1;
    snprintf(hisoblar[soni].ism, sizeof(hisoblar[soni].ism), "%s", ism);
    hisoblar[soni].balans = boshlangich;
    return soni++;
}

int bank_otkazma(int dan, int ga, long summa)
{
    if (!togri_id(dan) || !togri_id(ga) || summa <= 0 || hisoblar[dan].balans < summa)
        return -1;
    hisoblar[dan].balans -= summa;
    hisoblar[ga].balans += summa;
    return 0;
}

long bank_balans(int id)
{
    return togri_id(id) ? hisoblar[id].balans : -1;
}

void bank_hisobot(void)
{
    printf("Bank hisoboti:\n");
    for (int i = 0; i < soni; i++)
        printf("  #%d %-10s %8ld\n", i, hisoblar[i].ism, hisoblar[i].balans);
    printf("Jami %d ta hisob\n", soni);
}
```

```c
/* main.c - mijoz */
#include <stdio.h>

#include "bank.h"

int main(void)
{
    int ali = bank_och("Ali", 100000);
    int vali = bank_och("Vali", 50000);
    printf("Ali (#%d) va Vali (#%d) hisoblari ochildi\n", ali, vali);

    printf("O'tkazma 30000: %d\n", bank_otkazma(ali, vali, 30000));
    printf("O'tkazma 90000 (yetmaydi): %d\n", bank_otkazma(ali, vali, 90000));
    printf("Noto'g'ri raqam: %d\n", bank_otkazma(ali, 7, 10));
    bank_hisobot();
    return 0;
}
```

```make
# Makefile
CC     := gcc
CFLAGS := -Wall -Wextra -g -MMD -MP
OBJ    := main.o bank.o

dastur: $(OBJ)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJ:.o=.d)

.PHONY: clean
clean:
	rm -f dastur *.o *.d
```

```console
$ make
gcc -Wall -Wextra -g -MMD -MP -c main.c -o main.o
gcc -Wall -Wextra -g -MMD -MP -c bank.c -o bank.o
gcc main.o bank.o -o dastur
$ ./dastur
Ali (#0) va Vali (#1) hisoblari ochildi
O'tkazma 30000: 0
O'tkazma 90000 (yetmaydi): -1
Noto'g'ri raqam: -1
Bank hisoboti:
  #0 Ali           70000
  #1 Vali          80000
Jami 2 ta hisob
$ make
make: 'dastur' is up to date.
$ touch bank.c && make
gcc -Wall -Wextra -g -MMD -MP -c bank.c -o bank.o
gcc main.o bank.o -o dastur
$ touch bank.h && make
gcc -Wall -Wextra -g -MMD -MP -c main.c -o main.o
gcc -Wall -Wextra -g -MMD -MP -c bank.c -o bank.o
gcc main.o bank.o -o dastur
$ nm bank.o | grep -i "hisoblar\|togri\|bank_"
00000000000001ce T bank_balans
0000000000000217 T bank_hisobot
000000000000002a T bank_och
00000000000000ca T bank_otkazma
0000000000000000 b hisoblar
0000000000000000 t togri_id
```

Kuzating: uchinchi `make` hech narsa qilmadi; `bank.c` "o'zgargach" faqat `bank.o` qayta yig'ildi; `bank.h` o'zgargach
**ikkala** `.o` qayta yig'ildi (`-MMD` yaratgan `.d` fayllar sarlavha bog'liqligini biladi). `nm` da `hisoblar`
va `togri_id` kichik harf (`b`, `t`) — `static`, tashqariga chiqmagan.

**Kengaytiring:** `bank_yopish(id)` qo'shing. `main.c` da `hisoblar[0].balans = 1000000;` yozib ko'ring — nega kompilyator
"`hisoblar` undeclared" deydi? (Yashirin!)

## Mustaqil loyiha: kitob moduli ★★☆

**Vazifa:** o'qiyotgan kitoblaringizni kuzatuvchi modul. Uch fayl (`kitob.h`, `kitob.c`, `main.c`) va `Makefile`.
Bu — oldingi suhbatdagi mashq, endi to'liq talab bilan.

**Talab.** `kitob.h` da aynan shular (boshqa hech narsa, `#pragma once` dan tashqari):

```text
struct kitob { const char *nom; int jami; int oqilgan; };
void kitob_och(struct kitob *k, const char *nom, int sahifa);   // oqilgan = 0
void kitob_oqi(struct kitob *k, int n);          // n ta bet o'qidi
int  kitob_foiz(const struct kitob *k);          // 0..100, butun bo'lish bilan
void kitob_chiqar(const struct kitob *k);        // "C tili: 150/320 bet (46%)" + yangi qator
int  kitob_soni(void);                           // hozirgacha nechta kitob ochilgan
```

**Qoidalar (`kitob.c` ichida):**
- `kitob_oqi` da o'qilgan betlar soni **jami dan oshib ketmasin**; `n` manfiy yoki nol bo'lsa hech narsa o'zgarmasin.
- Bu tekshiruvni **`static` yordamchi funksiya** bajarsin (masalan `chegarala`) — u `.h` da yo'q.
- Nechta kitob ochilgani **`static` o'zgaruvchi**da saqlansin (`.h` da e'lon qilinmaydi!); tashqaridan faqat `kitob_soni()` orqali.

**Makefile:** `dastur` nishoni, naqsh qoidasi `%.o: %.c`, `-MMD -MP`, `clean`.

**`main.c` sinovi:** "C tili" (320 bet) va "Yadro" (450 bet) ochiladi; birinchisidan 100, keyin 50 bet o'qiladi
(orasida `-20` — hisobga olinmaydi); ikkinchisidan 500 bet o'qiladi (450 dan oshmaydi); ikkalasi chiqariladi; oxirida kitoblar soni.

**Kutilgan natija** (`darslik/loyihalar/11_kitob_moduli/kutilgan.txt`):

```text
C tili: 150/320 bet (46%)
Yadro: 450/450 bet (100%)
Ochilgan kitoblar: 2
```

**Maslahat** (yechim emas):
- Avval qog'ozda 5 ta funksiyaning imzosini yozing — bu tayyor `.h`.
- `nm kitob.o`: 5 ta `T` (kitob_*), bitta kichik `t` (yordamchi), bitta kichik `b`/`d` (hisoblagich).
- `touch kitob.h && make` — ikkala `.o` qayta yig'ilganini tekshiring; `touch main.c && make` — faqat `main.o`.
- `kitob_foiz` da `jami == 0` bo'lsa nima bo'ladi? (Nolga bo'lish — UB! Qanday himoya qilasiz?)

**Tekshirish:**

```bash
make && ./dastur | diff - ~/C_loyha/darslik/loyihalar/11_kitob_moduli/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [12-bob. Standart kutubxona](12-standart-kutubxona.md)
