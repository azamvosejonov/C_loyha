# 16-bob. Bitlar, baytlar tartibi va apparat bilan gaplashish

> **Bu bobda nima o'rganasiz:** ko'p baytli son xotirada qaysi tartibda yozilishini (endianness); `volatile` ning haqiqiy ma'nosini; qurilma registrlarini (MMIO va port I/O) bit maskalar
> bilan o'qish/yozishni; bitmap'larni. Bular — drayver yozish uchun asosiy bilimlar.
> **Oldindan nima kerak:** 2-, 3-, 7-, 9-boblar (ayniqsa 3.4 bitlar).   **Vaqt:** 6–7 soat.
> Mashqlar: 04, 21, 22.

> **To'liq ishlaydigan misol:** [misollar/16_bitlar_apparat.c](misollar/16_bitlar_apparat.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Hozirgacha dasturlarimiz faqat xotira bilan ishladi. Haqiqiy kompyuterda esa **apparat** bor: klaviatura, disk, tarmoq kartasi. Ular bilan gaplashish — **registrlar** orqali:
qurilma ichida "tugmalar va lampochkalar" (bitlar) bor, siz ularni xotiradagi manzillar yoki portlar orqali bosasiz/o'qiysiz. Buning uchun uchta narsani bilish kerak:
(1) baytlar qaysi tartibda saqlanadi; (2) kompilyatorga "bu xotira o'zi o'zgarishi mumkin" deyish (`volatile`); (3) bitlar bilan aniq ishlash.

**Hayotdan misol: elektr shchiti.** Uyning elektr shchitida bir qator avtomatlar bor: biri oshxona, biri konditsioner... Qurilma registri ham shunday — bitta 32 bitli son, har bir bit yoki bitlar guruhi — alohida
sozlama. Bittasini o'zgartirish uchun: avval hozirgi holatni o'qiysiz, kerakli avtomatni o'zgartirasiz, qolganlariga tegmasdan qaytarib yozasiz (o'qi-o'zgartir-yoz).

## 16.1. Bayt tartibi (endianness)

**Hayotdan misol: sanani yozish tartibi.** Bir sanani ikki xil yozish mumkin: `27.09.2026` (kun birinchi) va `2026-09-27` (yil birinchi). Sana bir xil, tartib boshqa. Agar yozuvchi va o'quvchi tartibni
kelishmasa — 9-oyning 27-kuni o'rniga 27-oy chiqadi. Kompyuterda ham: x86 son baytlarini **kichigidan** boshlab yozadi (little-endian), tarmoq protokollari esa **kattasidan** (big-endian).

Ko'p baytli son xotirada qaysi tartibda yoziladi?

```c
uint32_t x = 0x11223344;
```

```text
                manzil:  +0   +1   +2   +3
little-endian (x86, ARM): 44   33   22   11     <- kichik bayt OLDIN
big-endian (tarmoq):      11   22   33   44     <- katta bayt oldin
```

```c
/* endian.c - bayt tartibi */
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Baytlardan big-endian sonni qo'lda yig'ish: har qanday CPU'da to'g'ri */
static uint32_t oqi_be32(const uint8_t *p)
{
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3];
}

int main(void)
{
    uint32_t x = 0x11223344;
    uint8_t b[4];
    memcpy(b, &x, 4);                       /* x ning baytlarini xotiradagi tartibda olish */
    printf("0x%08X xotirada: %02X %02X %02X %02X\n", x, b[0], b[1], b[2], b[3]);
    printf("bu CPU: %s\n", b[0] == 0x44 ? "little-endian" : "big-endian");

    uint32_t t = htonl(x);                  /* host -> network (big-endian) */
    memcpy(b, &t, 4);
    printf("htonl dan keyin:    %02X %02X %02X %02X\n", b[0], b[1], b[2], b[3]);

    uint8_t paket[4] = { 0x00, 0x00, 0x1F, 0x90 };      /* tarmoqdan kelgan 4 bayt */
    printf("paketdan o'qilgan son: %u (port %u)\n", oqi_be32(paket), oqi_be32(paket));
    return 0;
}
```

```console
$ gcc -Wall -Wextra endian.c -o endian
$ ./endian
0x11223344 xotirada: 44 33 22 11
bu CPU: little-endian
htonl dan keyin:    11 22 33 44
paketdan o'qilgan son: 8080 (port 8080)
```

**Bu dastur nima qiladi (umumiy):** bir sonning xotiradagi baytlarini ko'rsatadi (CPU qaysi tartibda yozishini aniqlaydi), uni tarmoq tartibiga aylantiradi (`htonl`) va tarmoq paketidan 4 baytni qo'lda son qilib yig'adi.

**Qismlar:**

| Qism | Vazifasi | Tafsilot |
|---|---|---|
| `memcpy(b, &x, 4)` | `x` ning baytlarini **xotiradagi tartibda** nusxalash | `b[0]` — birinchi (eng past manzildagi) bayt |
| `b[0] == 0x44` | CPU little-endian ekanini aniqlash | kichik bayt (`44`) birinchi turibdi |
| `htonl(x)` | **h**ost **to** **n**etwork **l**ong: tarmoq tartibiga aylantirish | little-endian'da baytlarni teskari aylantiradi; ntohl/htons/ntohs — shunga o'xshash |
| `oqi_be32(p)` | 4 baytni **big-endian** deb o'qib, son yasash | `p[0] << 24 \| p[1] << 16 \| p[2] << 8 \| p[3]` — har qanday CPU'da to'g'ri (surishlar — 3.4.4) |

Paketdagi `00 00 1F 90` = `0x00001F90` = 8080 (`1F90` o'n oltilikda: 1·4096 + 15·256 + 9·16 = 8080). Tarmoq drayveri (YAKUNIY.md, 26.11) yozganda birinchi xato odatda bayt tartibi.

x86 — **little-endian**. Tarmoq protokollari (IP, TCP) — **big-endian** ("network byte order"). Diskdagi formatlar — o'zicha: ext2 little-endian, ba'zilari big-endian.

> **Eslab qoling:** x86'da `0x11223344` xotirada `44 33 22 11`. Tarmoqdan kelgan sonni `ntohl`/qo'lda yig'ish bilan o'qing.

## 16.2. Bit maskalar bilan registrlar

Qurilma registrlari — har bir bit yoki bit guruhi alohida ma'noga ega son. Masalan, AHCI port buyruq registri (MyOS: `kernel/drivers/ahci.c`):

```c
#define CMD_ST    (1u << 0)             /* buyruqlarni bajarishni boshlash */
#define CMD_FRE   (1u << 4)             /* FIS qabul qilishni yoqish */

wr(p, PX_CMD, rd(p, PX_CMD) | CMD_FRE);     /* o'qish -> bitni yoqish -> yozish */
wr(p, PX_CMD, rd(p, PX_CMD) & ~CMD_ST);     /* o'qish -> bitni o'chirish -> yozish */
```

`rd`/`wr` — registrni `volatile` ko'rsatkich orqali o'qiydigan/yozadigan kichik yordamchi funksiyalar. "O'qi — o'zgartir — yoz" (read-modify-write) — boshqa bitlarni buzmaslik uchun.

Haqiqiy qurilmasiz ham mashq qilamiz: "qurilma registri" o'rnini oddiy `uint32_t` o'zgaruvchi bosadi.

```c
/* registr.c - o'qi-o'zgartir-yoz va bit maydonlar */
#include <stdint.h>
#include <stdio.h>

#define CMD_ST      (1u << 0)           /* 0-bit: start */
#define CMD_FRE     (1u << 4)           /* 4-bit: FIS qabul qilish */
#define MAYDON_SHIFT 8                   /* 8..11-bitlardagi 4 bitli maydon */
#define MAYDON_MASK  (0xFu << MAYDON_SHIFT)

static uint32_t qurilma_cmd = 0;        /* "qurilma registri" (haqiqiy apparatda - MMIO manzil) */

static uint32_t rd(void) { return qurilma_cmd; }
static void wr(uint32_t qiymat) { qurilma_cmd = qiymat; }

static void korsat(const char *nom)
{
    printf("%-22s 0x%08X\n", nom, qurilma_cmd);
}

int main(void)
{
    korsat("boshida:");

    wr(rd() | CMD_FRE);                 /* FRE ni yoqish */
    korsat("FRE yoqildi:");

    wr(rd() | CMD_ST);                  /* ST ni yoqish (FRE ga tegmasdan) */
    korsat("ST ham yoqildi:");

    wr(rd() & ~CMD_ST);                 /* ST ni o'chirish */
    korsat("ST o'chirildi:");

    /* 8..11-bitlardagi maydonga 0xA yozish */
    uint32_t yangi = 0xA;
    wr((rd() & ~MAYDON_MASK) | ((yangi << MAYDON_SHIFT) & MAYDON_MASK));
    korsat("maydon = 0xA:");

    uint32_t o = (rd() & MAYDON_MASK) >> MAYDON_SHIFT;
    printf("maydon qiymati o'qildi: 0x%X\n", o);
    return 0;
}
```

```console
$ gcc -Wall -Wextra registr.c -o registr
$ ./registr
boshida:               0x00000000
FRE yoqildi:           0x00000010
ST ham yoqildi:        0x00000011
ST o'chirildi:         0x00000010
maydon = 0xA:          0x00000A10
maydon qiymati o'qildi: 0xA
```

**Bu dastur nima qiladi (umumiy):** "qurilma registri" (32 kalitli panel) ustida 3-bobdagi hunarlarni qo'llaydi: bitni yoqish, o'chirish va 4 bitli **maydon** (bir nechta bit birgalikda bitta qiymat) o'qish/yozish.

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `rd()`, `wr(q)` | registrni o'qish / yozish (haqiqiy apparatda `volatile` ko'rsatkich orqali) |
| `wr(rd() \| CMD_FRE)` | **o'qi-o'zgartir-yoz**: avval joriy qiymatni o'qiymiz, bitni yoqamiz (`\|`), butun qiymatni qaytarib yozamiz — boshqa bitlar **buzilmaydi** |
| `wr(rd() & ~CMD_ST)` | bitni o'chirish (3.4.6) |
| `MAYDON_MASK = 0xF << 8` | 8–11-bitlar tanlangan niqob: `0x00000F00` |
| yozish: `(reg & ~MASK) \| ((yangi << SHIFT) & MASK)` | avval maydonni **tozalaymiz**, keyin yangi qiymatni o'z o'rniga **suramiz** va qo'shamiz |
| o'qish: `(reg & MASK) >> SHIFT` | maydonni ajratamiz va pastga suramiz |

Manba — qurilma **spetsifikatsiyasi** (datasheet): unda har bir registrning har bir biti tasvirlangan. Drayver yozish — spetsifikatsiyani bitma-bit C'ga ko'chirish. MyOS: `kernel/drivers/ahci.c`, `kernel/arch/apic.c`, `kernel/drivers/pci.c`.

## 16.3. `volatile` — nima uchun va nima uchun EMAS

**Hayotdan misol: pochta qutisi.** Kecha pochta qutingiz bo'sh edi. Bugun "kecha bo'sh edi — demak bugun ham bo'sh" deb qaramasangiz, xatni o'tkazib yuborasiz. Kompilyator ham "bu o'zgaruvchini hech kim o'zgartirmadi"
deb eski qiymatni ishlatishi mumkin. Qurilma registrini esa **qurilma o'zi** o'zgartiradi. `volatile` — "har safar borib, qutiga qarab chiq".

```c
volatile uint32_t *status = (volatile uint32_t *)(bar + 0x10);
while (!(*status & TAYYOR))
    ;
```

`volatile` kompilyatorga: **"bu xotiraga har bir murojaatni aynan yozilgandek bajar — keshlamang, o'chirmang, birlashtirmang, tartibini o'zgartirmang"**. Farqni assembly'da ko'ramiz:

```c
/* vol.c - volatile bor va yo'q sikl */
#include <stdint.h>

void bekor(uint32_t *s)                 /* volatile YO'Q */
{
    while (!(*s & 1))
        ;
}

void bor(volatile uint32_t *s)          /* volatile BOR */
{
    while (!(*s & 1))
        ;
}
```

```console
$ gcc -O2 -S -o - vol.c | grep -vE '^\s+\.' | grep -E '^(bekor|bor|\.L[0-9]+|\s+(movl|testb|jne|je|jmp|ret))'
bekor:
	testb	$1, (%rdi)
	jne	.L1
.L3:
	jmp	.L3
.L1:
	ret
bor:
.L6:
	movl	(%rdi), %eax
	testb	$1, %al
	je	.L6
	ret
```

**Nima ko'rdik:**

- `bekor` (volatile siz): `testb $1, (%rdi)` bir marta bajariladi; shart yolg'on bo'lsa `jmp .L3` — **o'ziga sakrab abadiy aylanadi**, xotiraga **qayta qaramaydi** (kompilyator: "`*s` sikl ichida o'zgarmaydi").
  Qurilma bitni yoqsa ham, dastur buni ko'rmaydi.
- `bor` (volatile): sikl ichida **har safar** `movl (%rdi), %eax` — xotiradan qayta o'qiydi.

**`volatile` kerak bo'lgan joylar:**
1. Qurilma registrlari (MMIO).
2. Signal handler o'zgartiradigan bayroq (`volatile sig_atomic_t`, 14-bob).
3. `setjmp`/`longjmp` atrofidagi lokal o'zgaruvchilar.

**`volatile` QILMAYDIGAN narsalar:** atomiklik, CPU'lar orasidagi tartib, qulf. Ko'p oqimli kod uchun atomiklar va qulflar (15-bob). Linux yadrosida `volatile` o'zgaruvchilar deyarli taqiqlangan —
`READ_ONCE`/`WRITE_ONCE` va to'siqlar ishlatiladi.

> **Eslab qoling:** `volatile` = "kompilyator, bu xotira o'zi o'zgaradi — har gal o'qi". Qurilma registri uchun shart; ko'p oqimlilikni hal qilmaydi.

## 16.4. MMIO va port I/O

x86'da qurilmalar bilan gaplashishning ikki yo'li bor:

**1) MMIO (memory-mapped I/O).** **Hayotdan misol: pult tugmalari xotira ko'rinishida.** Qurilma o'z registrlarini xotira manzillari sifatida ko'rsatadi. Siz oddiy xotiraga yozgandek yozasiz,
lekin aslida qurilmaning tugmasini bosasiz. Qurilma registrlari fizik manzillar maydonida ko'rinadi. Ularni virtual manzilga xaritalab (keshlanmaydigan qilib), oddiy ko'rsatkich bilan o'qiysiz:

```text
volatile uint32_t *lapic = ioremap(0xFEE00000, 4096);   /* Local APIC registrlari */
uint32_t id = lapic[0x20 / 4] >> 24;                     /* APIC ID registri */
```

(MyOS: `kernel/mm/vmalloc.c` → `ioremap`, `kernel/arch/apic.c`.) Zamonaviy qurilmalarning (AHCI, NVMe, xHCI, tarmoq kartalari) deyarli hammasi MMIO.

**2) Port I/O** — alohida 64 KB "port" maydoni, maxsus `in`/`out` buyruqlari bilan. Quyidagi funksiyalarni kompilyatsiya qila olamiz, lekin oddiy dasturda **ishga tushira olmaymiz**
(ruxsat yo'q — faqat yadro/drayver):

```c
/* port_io.c - in/out buyruqlari (faqat yadroda ishlaydi) */
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t v;
    __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

void serial_yoz(char c)
{
    outb(0x3F8, (uint8_t)c);            /* COM1 serial portiga belgi */
}

uint8_t klaviatura_skankod(void)
{
    return inb(0x60);                   /* PS/2 klaviatura skankodi */
}
```

```console
$ gcc -Wall -Wextra -c port_io.c -o port_io.o
$ objdump -d port_io.o | grep -E 'out|in ' | head -4
0000000000000000 <outb>:
  17:	ee                   	out    %al,(%dx)
  2b:	ec                   	in     (%dx),%al
  54:	e8 a7 ff ff ff       	call   0 <outb>
```

`objdump` ko'rsatadi: funksiyalar haqiqatan `out`/`in` mashina buyruqlariga aylandi. Eski qurilmalar (PS/2 klaviatura, serial port, PIT taymer, PIC) port I/O ishlatadi.
MyOS: `kernel/arch/io.h`, `kernel/drivers/serial.c`, `keyboard.c`. (Inline assembly — 17-bob.)

## 16.5. Apparatni kutish — doim vaqt chegarasi bilan

**Hayotdan misol: kuryer.** Kuryerni kutyapsiz, lekin **cheksiz** emas: "30 daqiqada kelmasa — qo'ng'iroq qilaman". Qurilma buzilgan bo'lishi mumkin — `while (!tayyor)` cheksiz sikl butun tizimni qotiradi.
Doim hisoblagich qo'ying.

```c
/* kutish.c - vaqt chegarasi bilan kutish */
#include <errno.h>
#include <stdio.h>

static int soat = 0;                    /* "qurilma" ichidagi vaqt hisobi */

/* qurilma 'tayyor_vaqt' dan keyin tayyor bo'ladi (agar umuman bo'lsa) */
static int qurilma_tayyormi(int tayyor_vaqt)
{
    soat++;
    return tayyor_vaqt >= 0 && soat >= tayyor_vaqt;
}

static int kut(int tayyor_vaqt)
{
    soat = 0;
    for (int i = 0; i < 1000; i++) {            /* chegara: 1000 urinish */
        if (qurilma_tayyormi(tayyor_vaqt))
            return 0;
    }
    return -ETIMEDOUT;                          /* qurilma javob bermadi */
}

int main(void)
{
    printf("sog'lom qurilma (50 urinishda tayyor): %d\n", kut(50));
    printf("buzilgan qurilma (hech qachon tayyor emas): %d (ETIMEDOUT = %d)\n", kut(-1), -ETIMEDOUT);
    return 0;
}
```

```console
$ gcc -Wall -Wextra kutish.c -o kutish
$ ./kutish
sog'lom qurilma (50 urinishda tayyor): 0
buzilgan qurilma (hech qachon tayyor emas): -110 (ETIMEDOUT = -110)
```

**Qismlar:** `kut()` ko'pi bilan 1000 marta so'raydi; qurilma tayyor bo'lsa `0`, bo'lmasa `-ETIMEDOUT` (manfiy xato kodi, 5.5). Sikl **chegarasiz** bo'lsa (`while (!tayyor)`), "buzilgan qurilma" dasturni abadiy qotirardi.

```text
/* XATO - haqiqiy apparatda abadiy qotishi mumkin */
while (!(inb(0x64) & 1))
    ;
```

Qurilma yo'q bo'lishi, buzilgan bo'lishi yoki kutilgandan sekin bo'lishi mumkin. Mavjud bo'lmagan port ko'pincha `0xFF` qaytaradi (hamma bit 1!). MyOS'da aynan shunday xato topilib tuzatilgan:
PS/2 kontrolleri yo'q kompyuterda `keyboard_init` abadiy aylanardi (`kernel/drivers/keyboard.c`, YAKUNIY.md 27-bo'lim).

## 16.6. Bitmap — 1 bit bitta obyekt uchun

**Hayotdan misol: kinoteatr o'rindiqlari sxemasi.** Kassa ekranida har bir o'rindiq — bitta katakcha: band yoki bo'sh. 64 o'rindiqli zal uchun bitta 64 bitli son yetadi! Yadroda xuddi shunday: qaysi xotira sahifalari bo'sh,
qaysi disk bloklari band, qaysi jarayon raqamlari ishlatilgan.

**Bu nima?** **Bitmap** — massiv, unda har bir bit bitta obyektning holati (0 — bo'sh, 1 — band). **Asosiy ishi:** juda ko'p obyektning holatini juda kam xotirada saqlash (1 bit obyektga).

```c
/* bitmap.c - katta bitmap: 3 ta amal */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static uint64_t bm[1024];                       /* 65536 ta obyekt uchun 8 KB */

static void yoq(size_t i)   { bm[i / 64] |=  (1ull << (i % 64)); }      /* band qilish */
static void ochir(size_t i) { bm[i / 64] &= ~(1ull << (i % 64)); }      /* bo'shatish */
static bool bormi(size_t i) { return (bm[i / 64] >> (i % 64)) & 1; }    /* band-mi? */

int main(void)
{
    yoq(5);
    yoq(70);
    yoq(65535);
    printf("5: %d, 6: %d, 70: %d, 65535: %d\n", bormi(5), bormi(6), bormi(70), bormi(65535));
    ochir(70);
    printf("ochirgandan keyin 70: %d\n", bormi(70));
    printf("obyekt 70: bm[%d] ning %d-biti\n", 70 / 64, 70 % 64);
    return 0;
}
```

```console
$ gcc -Wall -Wextra bitmap.c -o bitmap
$ ./bitmap
5: 1, 6: 0, 70: 1, 65535: 1
ochirgandan keyin 70: 0
obyekt 70: bm[1] ning 6-biti
```

**Qismlar:** `i`-obyektning o'rni: **qaysi so'z** — `i / 64`, **so'zdagi qaysi bit** — `i % 64`. Keyin 3.4.6 dagi hunarlar: `|=` (yoqish), `&= ~` (o'chirish), `(x >> n) & 1` (o'qish).
70-obyekt: so'z `70 / 64 = 1`, bit `70 % 64 = 6`.

**Tezlashtirish:** to'liq band so'zni (`== ~0ull`) butunlay o'tkazib yuborish, bo'sh bitni `__builtin_ctzll` ("trailing zeros" — eng pastki 1-bitning o'rni, x86: `tzcnt`/`bsf`) bilan bitta buyruqda topish.
Qayerda: ext2 blok va inode bitmap'lari (diskda!), boot paytidagi fizik xotira, PID ajratish, IRQ vektorlari. 21-mashq.

## 16.7. Foydali bit hunarlari (to'plam)

```c
/* bit_hunarlar.c - yadroda ko'p uchraydigan hunarlar */
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    uint32_t x = 0x58;                          /* 0101 1000 */
    uint32_t a = 4096;

    printf("x                  = 0x%02X (0101 1000)\n", x);
    printf("x & (x - 1)        = 0x%02X  eng pastki 1-bitni o'chirish\n", x & (x - 1));
    printf("x & -x             = 0x%02X  faqat eng pastki 1-bit qoladi\n", x & -x);
    printf("popcount(x)        = %d     1-bitlar soni\n", __builtin_popcount(x));
    printf("ctz(x)             = %d     pastdagi nollar soni\n", __builtin_ctz(x));
    printf("31 - clz(x)        = %d     log2(x) (butun qismi)\n", 31 - __builtin_clz(x));

    uint32_t manzil = 5000;
    printf("yuqoriga tekislash : %u -> %u\n", manzil, (manzil + a - 1) & ~(a - 1));
    printf("pastga tekislash   : %u -> %u\n", manzil, manzil & ~(a - 1));
    printf("manzil %% 4096      : %u (tezkor: manzil & 4095 = %u)\n", manzil % a, manzil & (a - 1));
    return 0;
}
```

```console
$ gcc -Wall -Wextra bit_hunarlar.c -o bit_hunarlar
$ ./bit_hunarlar
x                  = 0x58 (0101 1000)
x & (x - 1)        = 0x50  eng pastki 1-bitni o'chirish
x & -x             = 0x08  faqat eng pastki 1-bit qoladi
popcount(x)        = 3     1-bitlar soni
ctz(x)             = 3     pastdagi nollar soni
31 - clz(x)        = 6     log2(x) (butun qismi)
yuqoriga tekislash : 5000 -> 8192
pastga tekislash   : 5000 -> 4096
manzil % 4096      : 904 (tezkor: manzil & 4095 = 904)
```

| Hunar | Nima beradi | Ishlatiladi |
|---|---|---|
| `x & (x - 1)` | eng pastki 1-bitni o'chiradi; natija 0 bo'lsa — `x` ikkining darajasi (`x != 0` da) | bitlarni sanash |
| `x & -x` | faqat eng pastki 1-bitni qoldiradi (ishorasiz `x` uchun) | Fenwick daraxti, planlashtirish |
| `(x + a - 1) & ~(a - 1)` | `a` ga (ikkining darajasi) **yuqoriga** tekislash | xotira ajratish |
| `x & ~(a - 1)` | **pastga** tekislash | sahifa boshi |
| `x & (a - 1)` | `x % a` (`a` ikkining darajasi bo'lsa) — tezroq | |
| `__builtin_popcount(x)` | 1-bitlar soni | |
| `__builtin_ctz(x)` | pastdagi nollar soni (`x != 0`) | bo'sh bit topish |
| `__builtin_clz(x)` | yuqoridagi nollar soni (`x != 0`) → `log2 = 31 - clz` | |

Buddy allocator'da: blokning "jufti" — `indeks ^ (1 << tartib)`; kerakli tartib — `log2` ni yuqoriga yaxlitlash. MyOS: `kernel/mm/pmm.c`.

## 16.8. Tekislash va DMA

Qurilmalar ko'pincha **tekislangan** va **fizik jihatdan ketma-ket** xotirani talab qiladi (AHCI buyruq ro'yxati — 1 KB ga tekislangan, sahifa jadvallari — 4 KB). **DMA** (Direct Memory Access) — qurilma
xotiraga **CPU'siz**, **fizik** manzil bo'yicha yozadi. Shuning uchun drayver:

1. fizik sahifa ajratadi (`alloc_pages`);
2. qurilmaga **fizik** manzilni beradi (virtual emas!);
3. CPU uni virtual manzil orqali o'qiydi.

## Hayotdan misol va to'liq dastur

**Kinoteatr kassasi.** 64 o'rindiqli zal — bitta `uint64_t`. Dasturda: bitmap (band/bo'sh), birinchi bo'sh o'rindiqni topish (`ctz`), bitlarni sanash (`popcount`), shuningdek sanani bitta sonda saqlash va baytlar tartibi.

```c
/* kinoteatr.c - bitmap, bit maskalar va bayt tartibi */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ORINLAR 64
static uint64_t zal = 0;                        /* 1 bit = 1 o'rindiq: 1 - band */

static int band_qil(int orin)
{
    if (zal & (1ull << orin))
        return -1;                              /* allaqachon band */
    zal |= 1ull << orin;
    return 0;
}

static int birinchi_bosh(void)
{
    if (zal == UINT64_MAX)
        return -1;                              /* zal to'la */
    return __builtin_ctzll(~zal);               /* eng kichik 0 bitning raqami */
}

static void sxema(void)
{
    for (int qator = 0; qator < 8; qator++) {
        printf("  %d-qator: ", qator + 1);
        for (int o = 0; o < 8; o++)
            putchar((zal >> (qator * 8 + o)) & 1 ? 'X' : '.');
        putchar('\n');
    }
}

int main(void)
{
    int sotildi[] = { 0, 1, 2, 5, 8, 9, 27, 28 };
    for (int i = 0; i < 8; i++)
        band_qil(sotildi[i]);
    printf("5-o'rindiqni qayta sotish: %s\n", band_qil(5) == 0 ? "sotildi" : "RAD - band");

    printf("Zal sxemasi (X - band):\n");
    sxema();
    printf("Band: %d ta, bo'sh: %d ta, birinchi bo'sh o'rindiq: %d\n",
           __builtin_popcountll(zal), ORINLAR - __builtin_popcountll(zal), birinchi_bosh());

    /* Sana baytlari: 2026-09-27 ni bitta sonda saqlash */
    uint32_t sana = 0x07EA091B;                 /* 0x07EA = 2026, 0x09 = 9, 0x1B = 27 */
    uint8_t b[4];
    memcpy(b, &sana, 4);
    printf("\nSana soni 0x%08X xotirada: %02X %02X %02X %02X (kichik bayt birinchi)\n",
           sana, b[0], b[1], b[2], b[3]);
    printf("Maskalar bilan ajratish: yil %u, oy %u, kun %u\n",
           sana >> 16, (sana >> 8) & 0xFF, sana & 0xFF);
    return 0;
}
```

```console
$ gcc -Wall -Wextra kinoteatr.c -o kinoteatr
$ ./kinoteatr
5-o'rindiqni qayta sotish: RAD - band
Zal sxemasi (X - band):
  1-qator: XXX..X..
  2-qator: XX......
  3-qator: ........
  4-qator: ...XX...
  5-qator: ........
  6-qator: ........
  7-qator: ........
  8-qator: ........
Band: 8 ta, bo'sh: 56 ta, birinchi bo'sh o'rindiq: 3

Sana soni 0x07EA091B xotirada: 1B 09 EA 07 (kichik bayt birinchi)
Maskalar bilan ajratish: yil 2026, oy 9, kun 27
```

**Bu dastur nima qiladi (umumiy):** zalni 64 bitli sonda saqlaydi; 8 ta o'rindiqni sotadi, band o'rindiqni qayta sotishni rad etadi, sxemani chizadi, band/bo'sh sonini va birinchi bo'sh o'rindiqni topadi; oxirida sanani bir sonda saqlab, bayt tartibini va maskalar bilan ajratishni ko'rsatadi.

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `uint64_t zal = 0` | 64 ta kalit (o'rindiq), boshida hammasi **0** (bo'sh) |
| `band_qil(orin)` | `zal & (1ull << orin)` — band-mi tekshiradi (rad); aks holda `\|=` bilan yoqadi |
| `birinchi_bosh()` | `~zal` da (invert) eng pastki **1** = asl `zal` dagi eng pastki **0** → `ctzll` uning raqamini beradi |
| `sxema()` | `(zal >> (qator*8 + o)) & 1` — har bitni o'qib `X` yoki `.` chizadi |
| `popcountll(zal)` | band o'rindiqlar soni |
| `sana = 0x07EA091B` | yil (16 bit) + oy (8 bit) + kun (8 bit) — **bitta sonda** (`0x07EA` = 2026, `0x09`, `0x1B` = 27) |
| `sana >> 16`, `(sana >> 8) & 0xFF`, `sana & 0xFF` | maydonlarni ajratish (16.2 dagi usul) |

**Sinab ko'ring:** 3-o'rindiqni sotib, `birinchi_bosh()` qanday o'zgarishini kuzating. Barcha 64 o'rindiqni sotuvchi sikl yozing — `birinchi_bosh()` endi nima qaytaradi?

## Bob xulosasi (yodlash uchun)

1. x86 — **little-endian**: `0x11223344` xotirada `44 33 22 11`; tarmoq — big-endian (`htonl`/`ntohl`); formatni qo'lda o'qishda baytlarni aniq tartibda yig'ing.
2. Qurilma registri — bitlar to'plami: **o'qi → o'zgartir → yoz** (`reg | BIT`, `reg & ~BIT`); maydon: `(reg & MASK) >> SHIFT`.
3. `volatile` — "har gal xotiradan o'qi/yoz" (kompilyator keshlamasin); qurilma registrlari uchun shart; atomik emas.
4. MMIO — registrlar xotira manzillari sifatida; port I/O — `in`/`out`. Apparatni kutishda **doim vaqt chegarasi**.
5. Bitmap — 1 bit = 1 obyekt (`i/64` so'z, `i%64` bit); `ctz`, `popcount` bilan tez ishlaydi.

## O'zingizni tekshiring

1. `uint32_t x = 1;` x86'da xotiradagi birinchi bayt nechaga teng?
2. `volatile` nimani kafolatlaydi va nimani kafolatlamaydi?
3. MMIO va port I/O farqi?
4. Nega mavjud bo'lmagan qurilmani chegarasiz kutish xavfli?
5. 4096 ga tekislangan-mi — qanday tekshirasiz?

<details><summary>Javoblar</summary>

1. 1 (kichik bayt oldin).
2. Har bir murojaat haqiqatan bajarilishini (kompilyator o'chirmaydi/keshlamaydi); atomiklik va CPU'lar orasidagi tartibni kafolatlamaydi.
3. MMIO — oddiy xotira manzillari orqali (ko'rsatkich), port I/O — alohida maydon, `in`/`out` buyruqlari.
4. Yo'q port ko'pincha 0xFF qaytaradi — shart hech qachon o'zgarmasligi mumkin va tizim qotadi.
5. `(x & 4095) == 0`.
</details>

## Mashq

- **04** (bitlar), **21** (bitmap), **22** (halqa bufer).
- Qo'shimcha: MyOS `kernel/drivers/ahci.c` dagi `rd`/`wr` va maskalarni toping; har bir `#define` bitini qurilma spetsifikatsiyasi bilan solishtiring.

<!-- loyiha:boshi -->
## Loyiha: virtual UART va uning drayveri

**Maqsad:** qurilma bilan **registrlar orqali** gaplashishni o'rganish: holat bitlarini tekshirish, sozlash registrini
"o'qi–o'zgartir–yoz" bilan o'zgartirish, va qurilma javob bermasa — **vaqt chegarasi** bilan chiqib ketish.
Bu yadrodagi har qanday drayverning tuzilishi (16.2–16.5).
**Bobdan ishlatiladi:** bit maskalar, `volatile`, registr tuzilmasi, kutish sikli.

**Talab:** UART — ketma-ket port. Uch registr: `data` (yuboriladigan belgi), `status` (holat: `TX_BOSH` — yuborishga tayyor,
`XATO`), `ctrl` (sozlash: `YOQ` bit + baud bo'luvchisi 8..15-bitlarda).
Haqiqiy apparat yo'q, shuning uchun uni **kichik simulyator** qildik (kodning yuqori qismi). Drayver qismi esa haqiqiy drayverdek
faqat registrlar bilan ishlaydi.

```c
/* uart.c - virtual UART va drayver */
#include <stdint.h>
#include <stdio.h>

#define ST_TX_BOSH      (1u << 0)               /* yuborishga tayyor */
#define ST_XATO         (1u << 7)
#define CTL_YOQ         (1u << 0)
#define CTL_BAUD_SHIFT  8
#define CTL_BAUD_MASK   (0xFFu << CTL_BAUD_SHIFT)

struct uart_regs {
    volatile uint32_t data;
    volatile uint32_t status;
    volatile uint32_t ctrl;
};

/* ============ "APPARAT" (haqiqiy tizimda bu qism chipning ichida) ============ */
static struct uart_regs qurilma = { 0, ST_TX_BOSH, 0 };
static char sim_chiqish[64];
static int sim_n, band_tick;

static void qurilma_tick(void)                  /* vaqt o'tadi: yuborish tugaydi */
{
    if (band_tick > 0 && --band_tick == 0)
        qurilma.status |= ST_TX_BOSH;
}

static void qurilma_data_yozildi(void)          /* DATA ga yozilganda qurilma "sezadi" */
{
    if (!(qurilma.ctrl & CTL_YOQ)) {
        qurilma.status |= ST_XATO;
        return;
    }
    if (sim_n < 63)
        sim_chiqish[sim_n++] = (char)qurilma.data;
    qurilma.status &= ~ST_TX_BOSH;              /* band */
    band_tick = 3;                              /* 3 tickdan keyin bo'shaydi */
}

/* ================================ DRAYVER ================================ */
static void uart_init(struct uart_regs *u, unsigned baud)
{
    uint32_t c = u->ctrl;                                           /* 1) o'qi */
    c = (c & ~CTL_BAUD_MASK) | ((baud << CTL_BAUD_SHIFT) & CTL_BAUD_MASK);   /* 2) o'zgartir */
    u->ctrl = c | CTL_YOQ;                                          /* 3) yoz */
}

static int uart_putc(struct uart_regs *u, char c, int *kutildi)
{
    int aylanish = 0;
    while (!(u->status & ST_TX_BOSH)) {         /* qurilma tayyor bo'lguncha kutamiz... */
        qurilma_tick();                         /* (simulyatsiya: vaqt o'tadi) */
        if (++aylanish > 100)
            return -1;                          /* ...lekin CHEKSIZ emas (16.5) */
    }
    *kutildi += aylanish;
    u->data = (uint32_t)(unsigned char)c;
    qurilma_data_yozildi();                     /* (simulyatsiya: qurilma javob beradi) */
    return 0;
}

int main(void)
{
    struct uart_regs *u = &qurilma;
    uart_init(u, 26);
    printf("ctrl = 0x%08X (baud = %u, yoqilgan = %u)\n", u->ctrl,
           (u->ctrl & CTL_BAUD_MASK) >> CTL_BAUD_SHIFT, u->ctrl & CTL_YOQ);

    const char *matn = "SALOM";
    int kutildi = 0;
    for (const char *p = matn; *p; p++)
        if (uart_putc(u, *p, &kutildi) != 0)
            printf("belgi '%c' yuborilmadi!\n", *p);
    printf("simda yuborilgan: \"%s\", jami kutish: %d aylanish\n", sim_chiqish, kutildi);

    u->status &= ~ST_TX_BOSH;                   /* qurilma "qotdi": hech qachon tayyor bo'lmaydi */
    band_tick = 100000;
    if (uart_putc(u, '!', &kutildi) != 0)
        printf("qurilma javob bermadi -> vaqt chegarasi ishladi (dastur qotib qolmadi)\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined uart.c -o uart
$ ./uart
ctrl = 0x00001A01 (baud = 26, yoqilgan = 1)
simda yuborilgan: "SALOM", jami kutish: 12 aylanish
qurilma javob bermadi -> vaqt chegarasi ishladi (dastur qotib qolmadi)
```

Tuzilish yadro drayveriga aynan o'xshaydi: registrlar tuzilmasi, `volatile`, maskalar, RMW, vaqt chegarali kutish.
Faqat `qurilma_*` funksiyalari o'rniga haqiqiy chip bor.

**Kengaytiring:** `uart_puts(u, s)` yozing. `CTL_YOQ` ni o'chirib `uart_putc` chaqiring — `ST_XATO` qanday o'rnatiladi? Drayver xatoni sezishi uchun nima qo'shasiz?

## Mustaqil loyiha: ma'lumot yaxlitligi — parity, CRC-8, Gray ★★★

**Vazifa:** aloqa va xotirada xatolarni aniqlash uchun ishlatiladigan to'rtta bit algoritmini o'zingiz yozing.
Hammasi **bitli amallar** bilan; `__builtin_*` funksiyalari **taqiqlangan**. Fayl: `yaxlitlik.c`.

1. `int parity(uint8_t b)` — birlar soni toq bo'lsa `1`, juft bo'lsa `0`.
2. `uint8_t bit_teskari(uint8_t b)` — bitlar tartibini teskari aylantiradi (`0x01` → `0x80`).
3. `uint8_t gray(uint8_t n)` va `uint8_t gray_teskari(uint8_t g)` — Gray kodi: qo'shni sonlar **bitta bit**ga farq qiladi.
   `gray(n) = n ^ (n >> 1)`. Teskarisini o'zingiz toping (Maslahat pastda).
4. `uint8_t crc8(const uint8_t *data, size_t n)` — CRC-8, polinom `0x07` (x⁸+x²+x+1), boshlang'ich qiymat 0, aks ettirishsiz:

```text
crc = 0
har bir bayt uchun:
    crc ^= bayt
    8 marta:  agar crc ning eng katta biti 1 bo'lsa:  crc = (crc << 1) ^ 0x07
              aks holda:                              crc = crc << 1        (8 bitda qoladi)
```

Standart tekshiruv: `"123456789"` uchun CRC-8 = `0xF4`.

**Chiqish shakli aniq:**

**Kutilgan natija** (`darslik/loyihalar/16_crc_gray/kutilgan.txt`):

```text
Parity (1 - toq sondagi birlar):
  0x00 -> 0
  0x01 -> 1
  0xFF -> 0
  0xB7 -> 0
Bitlarni teskari aylantirish:
  0x01 -> 0x80
  0x0F -> 0xF0
  0xA5 -> 0xA5
  0x1D -> 0xB8
Gray kodi (n -> gray -> orqaga):
  0 -> 0 -> 0
  1 -> 1 -> 1
  2 -> 3 -> 2
  3 -> 2 -> 3
  4 -> 6 -> 4
  5 -> 7 -> 5
  6 -> 5 -> 6
  7 -> 4 -> 7
CRC-8 (polinom 0x07):
  "" -> 0x00
  "A" -> 0xC0
  "123456789" -> 0xF4
  "Salom, dunyo!" -> 0x8D
  "Samom, dunyo!" (3-bayt buzilgan) -> 0x92
```

Sinov qiymatlari: parity — `0x00, 0x01, 0xFF, 0xB7`; teskari — `0x01, 0x0F, 0xA5, 0x1D`; Gray — `n = 0..7`;
CRC — `""` (bo'sh), `"A"`, `"123456789"`, `"Salom, dunyo!"`, va oxirgisining **3-baytini** `0x01` bilan XOR qilib buzilgan varianti.

**Maslahat** (yechim emas):
- `parity`: baytni o'z-o'ziga siljitib XOR qilish: `b ^= b >> 4; b ^= b >> 2; b ^= b >> 1; return b & 1;` — nega ishlaydi?
- `bit_teskari`: 8 marta: natijani 1 ga chapga suring, `b` ning eng past bitini qo'shing, `b` ni 1 ga o'ngga suring.
- `gray_teskari`: `n = g; n ^= n >> 1; n ^= n >> 2; n ^= n >> 4;` (prefiks XOR). Qog'ozda `g = 0b110` uchun tekshiring.
- CRC ichida `crc << 1` `int` da hisoblanadi (integer promotion, 2.11) — `(uint8_t)` cast bilan 8 bitda qoldiring.
- Bitta bit buzilganda CRC **doim** o'zgaradi (bitta bit xatosini CRC-8 kafolatli ushlaydi). Natijada ko'rasiz.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined yaxlitlik.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/16_crc_gray/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [17-bob. Assembly va C](17-assembly.md)
