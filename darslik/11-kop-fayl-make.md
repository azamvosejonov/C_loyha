# 11-bob. Ko'p faylli dasturlar, bog'lanish (linkage) va Make

> **Bu bobdan keyin:** katta dasturni fayllarga bo'lishni, `extern`/`static` ning bog'lanishdagi
> ma'nosini, kutubxonalar (`.a`) ni va `Makefile` o'qish/yozishni bilasiz. MyOS'ning `Makefile`'i
> endi siz uchun ochiq kitob bo'ladi.

> **To'liq ishlaydigan misol:** [misollar/11_kop_fayl/](misollar/11_kop_fayl/main.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Loyihani fayllarga bo'lish — qurilish brigadalari (11.1).** Uy qurilishida elektriklar, santexniklar va
suvoqchilar alohida ishlaydi. Har bir brigada faqat o'z ishini biladi va boshqalar bilan **chizma**
(`.h` fayl) orqali kelishadi: "rozetka shu devorda, 30 sm balandlikda". Bitta brigada ishini qayta
qilsa, qolganlari qaytadan ishlamaydi.

**`extern` — "u boshqa bo'limda ishlaydi" (11.2).** Katta tashkilotda: "Hisob-kitob bo'yicha Karimova
opaga murojaat qiling, u 3-qavatda". Siz uni ko'rmagansiz, lekin mavjudligini bilasiz. `extern int soni;`
— "soni degan o'zgaruvchi bor, lekin boshqa faylda yashaydi". Uni topib ulash — linkerning ishi.

**`static` (fayl darajasida) — oilaviy ish (11.2).** Oila ichidagi gaplar ko'chaga chiqmaydi. `static`
funksiya faqat o'z faylida ko'rinadi — boshqa fayllar uni chaqira olmaydi va tasodifan bir xil nomli
funksiya yozib qo'ysa ham, to'qnashuv bo'lmaydi.

**Kutubxonalar — asboblar (11.3).**
- **Statik kutubxona (`.a`)** — asbobni **sotib olish**: u sizning ustaxonangizda qoladi. Dastur
  kattaroq bo'ladi, lekin hech kimga bog'liq emas.
- **Dinamik kutubxona (`.so`)** — mahalladagi **umumiy ijaraxona**: hamma bitta asbobdan
  foydalanadi. Dastur kichik, lekin ishga tushganda ijaraxona (kutubxona) joyida bo'lishi shart.
  Asbob yangilansa — hamma birdaniga yangisini oladi.

**Make — aqlli prorab (11.4).** Prorab har kuni ertalab tekshiradi: "Chizma o'zgardimi? O'zgargan bo'lsa,
unga bog'liq devorlarni qayta qur. O'zgarmagan bo'lsa — tegma". Make ham fayllarning **o'zgartirilgan
vaqtini** solishtiradi: `.c` fayl `.o` dan yangiroq bo'lsa — qayta kompilyatsiya qiladi, aks holda
o'tkazib yuboradi. Linux'dagi 30 000 fayldan bittasini o'zgartirsangiz, bir necha soniyada yig'iladi.

### To'liq dastur: ob-havo stantsiyasi (3 fayl + Makefile)

1-bobdagi restoran misolidan farqli, bu yerda `static` va `extern` ham bor.

```c
/* harorat.h - chizma: tashqi dunyo uchun nima bor */
#pragma once

double selsiydan_farengeytga(double c);
double ortacha(const double *qiymatlar, int soni);
extern int olchovlar_soni;                      /* e'lon: o'zgaruvchi harorat.c da yashaydi */
```

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

Ikkinchi `make` hech narsa qilmadi — hech narsa o'zgarmagan. `touch harorat.c` (faylni "o'zgartirilgan"
deb belgilash) dan keyin esa faqat `harorat.o` qayta yig'ildi va qayta bog'landi — `stantsiya.o` ga
tegilmadi. `nm` da: `T` — tashqariga ochiq funksiya, `t` (kichik harf) — `static`, `D` — ta'rif
qilingan global o'zgaruvchi.

**Sinab ko'ring:** `stantsiya.c` da `yaxlitla(1.26)` ni chaqiring — linker nima deydi va nega?
`harorat.c` dagi `int olchovlar_soni = 0;` ni o'chiring — xato qaysi bosqichda chiqadi?

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
