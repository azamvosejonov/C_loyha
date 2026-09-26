# 16-bob. Bitlar, baytlar tartibi va apparat bilan gaplashish

> **Bu bobdan keyin:** endianness'ni, `volatile` ning haqiqiy ma'nosini, MMIO va port I/O'ni,
> qurilma registrlarini bit maskalar bilan o'qish/yozishni va bitmap'larni bilasiz — drayver yozish
> uchun asosiy bilimlar. Mashqlar: 04, 21, 22.

## 16.1. Bayt tartibi (endianness)

Ko'p baytli son xotirada qaysi tartibda yoziladi?

```c
uint32_t x = 0x11223344;
```

```text
                manzil:  +0   +1   +2   +3
little-endian (x86, ARM): 44   33   22   11     <- kichik bayt OLDIN
big-endian (tarmoq):      11   22   33   44     <- katta bayt oldin
```

x86 — **little-endian**. Tarmoq protokollari (IP, TCP) — **big-endian** ("network byte order").
Diskdagi formatlar — o'zicha: ext2 little-endian, ba'zilari big-endian.

```c
#include <arpa/inet.h>
uint16_t port_tarmoq = htons(8080);      /* host -> network (short) */
uint32_t ip = ntohl(paket->manba_ip);    /* network -> host (long) */
```

Yadroda yoki formatni qo'lda o'qishda:

```c
uint32_t oqi_be32(const uint8_t *p)
{
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3];
}
```

Bu kod har qanday CPU'da to'g'ri ishlaydi — baytlarni aniq tartibda yig'adi. Tarmoq drayveri
(YAKUNIY.md, 26.11) yozganda birinchi xato odatda shu.

## 16.2. Bit maskalar bilan registrlar

Qurilma registrlari — har bir bit yoki bit guruhi alohida ma'noga ega son. Masalan, AHCI port buyruq
registri (MyOS: `kernel/drivers/ahci.c`):

```c
#define CMD_ST    (1u << 0)             /* buyruqlarni bajarishni boshlash */
#define CMD_FRE   (1u << 4)             /* FIS qabul qilishni yoqish */

wr(p, PX_CMD, rd(p, PX_CMD) | CMD_FRE);     /* o'qish -> bitni yoqish -> yozish */
wr(p, PX_CMD, rd(p, PX_CMD) & ~CMD_ST);     /* o'qish -> bitni o'chirish -> yozish */
```

`rd`/`wr` — registrni `volatile` ko'rsatkich orqali o'qiydigan/yozadigan kichik yordamchi funksiyalar.
"O'qi — o'zgartir — yoz" (read-modify-write) — boshqa bitlarni buzmaslik uchun.

Bit guruhi (maydon) o'qish/yozish:

```c
/* 4..7-bitlardagi maydon */
#define MAYDON_SHIFT 4
#define MAYDON_MASK  (0xFu << MAYDON_SHIFT)

uint32_t qiymat = (reg & MAYDON_MASK) >> MAYDON_SHIFT;          /* o'qish */
reg = (reg & ~MAYDON_MASK) | ((yangi << MAYDON_SHIFT) & MAYDON_MASK);  /* yozish */
```

Manba — qurilma **spetsifikatsiyasi** (datasheet): unda har bir registrning har bir biti tasvirlangan.
Drayver yozish — spetsifikatsiyani bitma-bit C'ga ko'chirish. MyOS: `kernel/drivers/ahci.c`,
`kernel/arch/apic.c`, `kernel/drivers/pci.c`.

## 16.3. `volatile` — nima uchun va nima uchun EMAS

```c
volatile uint32_t *status = (volatile uint32_t *)(bar + 0x10);
while (!(*status & TAYYOR))
    ;
```

`volatile` kompilyatorga: **"bu xotiraga har bir murojaatni aynan yozilgandek bajar — keshlamang,
o'chirmang, birlashtirmang, tartibini o'zgartirmang"**. `volatile` siz:

```c
uint32_t *status = ...;
while (!(*status & TAYYOR))    /* kompilyator: "*status sikl ichida o'zgarmaydi" */
    ;                          /* -> bir marta o'qib, cheksiz siklga aylantiradi */
```

Qurilma registri kompilyatorga ko'rinmasdan o'zgaradi — shuning uchun `volatile` shart.

**`volatile` kerak bo'lgan joylar:**
1. Qurilma registrlari (MMIO).
2. Signal handler o'zgartiradigan bayroq (`volatile sig_atomic_t`, 14-bob).
3. `setjmp`/`longjmp` atrofidagi lokal o'zgaruvchilar.

**`volatile` QILMAYDIGAN narsalar:** atomiklik, CPU'lar orasidagi tartib, qulf. Ko'p oqimli kod uchun
atomiklar va qulflar (15-bob). Linux yadrosida `volatile` o'zgaruvchilar deyarli taqiqlangan —
`READ_ONCE`/`WRITE_ONCE` va to'siqlar ishlatiladi.

## 16.4. MMIO va port I/O

x86'da qurilmalar bilan gaplashishning ikki yo'li bor:

**1) MMIO (memory-mapped I/O)** — qurilma registrlari fizik manzillar maydonida ko'rinadi. Ularni
virtual manzilga xaritalab (keshlanmaydigan qilib), oddiy ko'rsatkich bilan o'qiysiz:

```c
volatile uint32_t *lapic = ioremap(0xFEE00000, 4096);   /* Local APIC registrlari */
uint32_t id = lapic[0x20 / 4] >> 24;                     /* APIC ID registri */
```

(MyOS: `kernel/mm/vmalloc.c` → `ioremap`, `kernel/arch/apic.c`.) Zamonaviy qurilmalarning (AHCI, NVMe,
xHCI, tarmoq kartalari) deyarli hammasi MMIO.

**2) Port I/O** — alohida 64 KB "port" maydoni, maxsus `in`/`out` buyruqlari bilan:

```c
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

outb(0x3F8, 'A');               /* COM1 serial portiga 'A' */
uint8_t sc = inb(0x60);         /* PS/2 klaviatura skankodi */
```

Eski qurilmalar (PS/2 klaviatura, serial port, PIT taymer, PIC) port I/O ishlatadi. MyOS:
`kernel/arch/io.h`, `kernel/drivers/serial.c`, `keyboard.c`. (Inline assembly — 17-bob.)

## 16.5. Apparatni kutish — doim vaqt chegarasi bilan

```c
/* XATO - haqiqiy apparatda abadiy qotishi mumkin */
while (!(inb(0x64) & 1))
    ;

/* TO'G'RI */
for (int i = 0; i < 100000; i++) {
    if (inb(0x64) & 1)
        return 0;
    cpu_pause();
}
return -ETIMEDOUT;
```

Qurilma yo'q bo'lishi, buzilgan bo'lishi yoki kutilgandan sekin bo'lishi mumkin. Mavjud bo'lmagan
port ko'pincha `0xFF` qaytaradi (hamma bit 1!). MyOS'da aynan shunday xato topilib tuzatilgan:
PS/2 kontrolleri yo'q kompyuterda `keyboard_init` abadiy aylanardi (`kernel/drivers/keyboard.c`,
YAKUNIY.md 27-bo'lim).

## 16.6. Bitmap — 1 bit bitta obyekt uchun

```c
#define BITLAR_SOZDA 64
static uint64_t bm[1024];                   /* 65536 ta obyekt uchun 8 KB */

void yoq(size_t i)   { bm[i / 64] |=  (1ull << (i % 64)); }
void ochir(size_t i) { bm[i / 64] &= ~(1ull << (i % 64)); }
bool bormi(size_t i) { return (bm[i / 64] >> (i % 64)) & 1; }
```

Tezlashtirish: to'liq band so'zni (`== ~0ull`) butunlay o'tkazib yuborish, bo'sh bitni `__builtin_ctzll`
("trailing zeros" — eng pastki 1-bitning o'rni, x86: `tzcnt`/`bsf`) bilan bitta buyruqda topish.

Qayerda: ext2 blok va inode bitmap'lari (diskda!), boot paytidagi fizik xotira, PID ajratish, IRQ
vektorlari. 21-mashq.

## 16.7. Foydali bit hunarlari (to'plam)

```c
x & (x - 1)             /* eng pastki 1-bitni o'chirish; 0 bo'lsa - x ikkining darajasi (x != 0 da) */
x & -x                  /* faqat eng pastki 1-bitni qoldirish (ishorasiz x uchun) */
(x + a - 1) & ~(a - 1)  /* a ga (ikkining darajasi) yuqoriga tekislash */
x & ~(a - 1)            /* pastga tekislash */
x & (a - 1)             /* x % a (a ikkining darajasi bo'lsa) - tezroq */
__builtin_popcount(x)   /* 1-bitlar soni */
__builtin_ctz(x)        /* pastdagi nollar soni (x != 0) */
__builtin_clz(x)        /* yuqoridagi nollar soni (x != 0) -> log2 = 31 - clz */
```

Buddy allocator'da: blokning "jufti" — `indeks ^ (1 << tartib)`; kerakli tartib — `log2` ni yuqoriga
yaxlitlash. MyOS: `kernel/mm/pmm.c`.

## 16.8. Tekislash va DMA

Qurilmalar ko'pincha **tekislangan** va **fizik jihatdan ketma-ket** xotirani talab qiladi (AHCI
buyruq ro'yxati — 1 KB ga tekislangan, sahifa jadvallari — 4 KB). DMA (Direct Memory Access) — qurilma
xotiraga **CPU'siz**, **fizik** manzil bo'yicha yozadi. Shuning uchun drayver:
1. fizik sahifa ajratadi (`alloc_pages`);
2. qurilmaga **fizik** manzilni beradi (virtual emas!);
3. CPU uni virtual manzil orqali o'qiydi.

## 16.9. O'zingizni tekshiring

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

## 16.10. Mashqlar

- **04** (bitlar), **21** (bitmap), **22** (halqa bufer — qurilma navbatlarining asosi).
- MyOS'da: `kernel/drivers/serial.c` ni o'qing — har bir `outb` qaysi registrga nima yozayotganini
  izohlardan tushuning. Keyin `kernel/drivers/ahci.c` dagi `#define` larni AHCI spetsifikatsiyasi bilan
  solishtiring (spetsifikatsiya internetda bepul: "Serial ATA AHCI 1.3.1").

Keyingi bob: [17-bob. Assembly va C](17-assembly.md)
