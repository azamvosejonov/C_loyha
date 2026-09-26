# 11-bob. Ko'p faylli dasturlar, bog'lanish (linkage) va Make

> **Bu bobdan keyin:** katta dasturni fayllarga bo'lishni, `extern`/`static` ning bog'lanishdagi
> ma'nosini, kutubxonalar (`.a`) ni va `Makefile` o'qish/yozishni bilasiz. MyOS'ning `Makefile`'i
> endi siz uchun ochiq kitob bo'ladi.

## 11.1. Loyihani fayllarga bo'lish

Qoidalar (MyOS va Linux shunday tuzilgan):

1. Har bir **modul** — bir juft fayl: `pipe.h` (interfeys) + `pipe.c` (amalga oshirish).
2. `.h` da faqat boshqalarga kerak bo'lgan narsa: e'lonlar, tuzilmalar, o'zgarmaslar, `static inline`.
3. `.c` dagi ichki funksiyalar va o'zgaruvchilar — `static`.
4. Har bir `.c` o'zining `.h` ini **birinchi** bo'lib `#include` qiladi — shunda `.h` o'zi yetarli
   (kerakli include'lari bilan) ekani tekshiriladi.

## 11.2. Bog'lanish (linkage): `extern` va `static`

Global nom (funksiya yoki o'zgaruvchi) ikki xil bo'ladi:

| | Tashqi bog'lanish (external) | Ichki bog'lanish (internal) |
|---|---|---|
| Yozuv | `int hisob;` / `void f(void) {}` | `static int hisob;` / `static void f(void) {}` |
| Ko'rinishi | barcha fayllarda (linker ulaydi) | faqat shu faylda |

Boshqa fayldagi **o'zgaruvchidan** foydalanish:

```c
/* taymer.c */
uint64_t tiklar = 0;                /* TA'RIF: xotira shu yerda */

/* taymer.h */
extern uint64_t tiklar;             /* E'LON: "boshqa joyda bor" - xotira ajratilmaydi */
```

`extern` — "bu nom boshqa faylda ta'riflangan". Funksiya e'lonlari avtomatik `extern` (so'zni yozish
shart emas). MyOS'da: `extern uint64_t tsc_khz;` (`kernel/arch/tsc.h`), `extern const struct amal amallar[];` (20-mashq).

**Umumiy qoida:** global o'zgaruvchilardan iloji boricha qoching. Kerak bo'lsa — `static` qilib,
tashqariga funksiyalar orqali bering (`uint64_t timer_ticks(void)`). Shunda kim o'zgartirishini
nazorat qilasiz (ko'p yadroli tizimda bu hayotiy).

## 11.3. Kutubxonalar

**Statik kutubxona** (`.a`) — `.o` fayllar arxivi:

```bash
gcc -c string.c stdio.c malloc.c        # -> string.o stdio.o malloc.o
ar rcs libmening.a string.o stdio.o malloc.o
gcc main.c -L. -lmening -o dastur       # -lmening -> libmening.a
```

Linker arxivdan faqat **kerakli** `.o` larni oladi. Tartib muhim: kutubxona undan foydalanadigan
fayllardan **keyin** yozilishi kerak (linker chapdan o'ngga yuradi). MyOS Makefile'ida shu haqida izoh
bor: `crt0.o + dastur.o + libc.a` — `libc.a` eng oxirida.

**Dinamik kutubxona** (`.so`) — dastur ishga tushganda yuklanadi (Linux'dagi `libc.so.6`). MyOS'da hali
yo'q (YAKUNIY.md, 26.12-loyiha).

## 11.4. Make — nima uchun

Qo'lda `gcc a.c b.c c.c ... -o dastur` — har safar hammasini qayta kompilyatsiya qiladi. Make esa
**faqat o'zgargan** fayllarni qayta yig'adi: har bir natija fayl uchun "u nimadan yasaladi" va
"qanday yasaladi" qoidasini yozasiz, Make fayllarning o'zgartirilgan vaqtini solishtiradi.

### Qoida sintaksisi

```make
NISHON: BOG'LIQLIKLAR
	BUYRUQ          # TAB bilan boshlanishi SHART (bo'shliqlar bilan - xato!)
```

```make
dastur: main.o matematika.o
	gcc main.o matematika.o -o dastur

main.o: main.c matematika.h
	gcc -Wall -c main.c

matematika.o: matematika.c matematika.h
	gcc -Wall -c matematika.c

clean:
	rm -f *.o dastur
```

`make` — birinchi nishonni (`dastur`) yig'adi. `matematika.c` ni o'zgartirsangiz, faqat `matematika.o`
va `dastur` qayta yig'iladi.

### O'zgaruvchilar va naqsh qoidalari

```make
CC      := gcc
CFLAGS  := -Wall -Wextra -g
SRC     := $(wildcard *.c)              # hamma .c fayllar
OBJ     := $(SRC:.c=.o)                 # main.c -> main.o

dastur: $(OBJ)
	$(CC) $^ -o $@

%.o: %.c                                # NAQSH: har qanday X.o X.c dan
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean                           # "clean - fayl emas, buyruq"
clean:
	rm -f $(OBJ) dastur
```

| Belgi | Ma'nosi |
|---|---|
| `$@` | nishon (natija fayl) |
| `$<` | birinchi bog'liqlik |
| `$^` | hamma bog'liqliklar |
| `%` | naqsh: "istalgan nom" |
| `:=` | darhol hisoblanadigan o'zgaruvchi |
| `?=` | "tashqaridan berilmagan bo'lsa" (`make QEMU_MEM=512M`) |
| `$(wildcard ...)`, `$(patsubst ...)` | fayllar ro'yxati bilan ishlash funksiyalari |

### Sarlavha bog'liqliklarini avtomatik kuzatish

`.h` o'zgarsa, uni `#include` qilgan hamma `.c` qayta yig'ilishi kerak. Qo'lda yozish — mashaqqat.
GCC buni o'zi yozadi:

```make
CFLAGS += -MMD -MP          # har bir .o yonida .d fayl: "main.o: main.c matematika.h"
-include $(OBJ:.o=.d)       # o'sha qoidalarni qo'shish
```

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

## 11.6. Savol-javob

**Make'da nega TAB shart?**
Tarixiy qaror (1976): buyruq qatorlarini ajratish uchun. `Makefile:5: *** missing separator` xatosi —
deyarli har doim TAB o'rniga bo'shliq.

**CMake, Meson nima?**
Makefile'larni avtomatik yaratadigan yuqori darajadagi vositalar. Yadrolar (Linux, MyOS) odatda
oddiy Make ishlatadi — to'liq nazorat uchun.

**`multiple definition of 'x'` — global o'zgaruvchi bilan?**
`.h` da `int x;` yozilgan va ikki `.c` uni qo'shgan. `.h` da `extern int x;`, bitta `.c` da `int x;`.

## 11.7. O'zingizni tekshiring

1. `static int n;` fayl darajasida nimani bildiradi?
2. `extern` qachon kerak?
3. Makefile'da `$@`, `$<`, `$^`?
4. Nega kutubxona buyruq qatorida oxirida turishi kerak?

<details><summary>Javoblar</summary>

1. n faqat shu faylda ko'rinadi (ichki bog'lanish), statik xotirada, boshlang'ich qiymati 0.
2. Boshqa faylda ta'riflangan global o'zgaruvchini e'lon qilish uchun.
3. Nishon, birinchi bog'liqlik, hamma bog'liqliklar.
4. Linker chapdan o'ngga yuradi va arxivdan faqat o'sha paytgacha "yetishmayotgan" nomlarni oladi.
</details>

## 11.8. Mashq

- 1-bobdagi uch faylli dastur uchun naqsh qoidali `Makefile` yozing; `matematika.h` ni o'zgartirib,
  qaysi fayllar qayta yig'ilishini kuzating (`-MMD -MP` bilan va usiz).
- MyOS `Makefile`'ini boshidan oxirigacha o'qing va `make V=1` bilan haqiqiy buyruqlarni ko'ring.

Keyingi bob: [12-bob. Standart kutubxona](12-standart-kutubxona.md)
