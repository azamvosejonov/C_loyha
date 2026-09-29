# 16-bob. Bitlar, baytlar tartibi va apparat bilan gaplashish

> **Bu bobdan keyin:** endianness'ni, `volatile` ning haqiqiy ma'nosini, MMIO va port I/O'ni,
> qurilma registrlarini bit maskalar bilan o'qish/yozishni va bitmap'larni bilasiz — drayver yozish
> uchun asosiy bilimlar. Mashqlar: 04, 21, 22.

> **To'liq ishlaydigan misol:** [misollar/16_bitlar_apparat.c](misollar/16_bitlar_apparat.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Endianness — sanani yozish tartibi (16.1).** Bir sanani ikki xil yozish mumkin: `27.09.2026` (kun
birinchi) va `2026-09-27` (yil birinchi). Sana bir xil, tartib boshqa. Agar yozuvchi va o'quvchi
tartibni kelishmasa — 9-oyning 27-kuni o'rniga 27-oy chiqadi. Kompyuterda ham: x86 son baytlarini
**kichigidan** boshlab yozadi (little-endian), tarmoq protokollari esa **kattasidan** (big-endian).
Tarmoqdan kelgan sonni o'qishda tartibni aylantirish kerak.

**Registr va bit maskalar — elektr shchiti (16.2).** Uyning elektr shchitida bir qator avtomatlar bor:
biri oshxona, biri konditsioner... Qurilma registri ham shunday — bitta 32 bitli son, har bir bit
yoki bitlar guruhi — alohida sozlama. Bittasini o'zgartirish uchun: avval hozirgi holatni o'qiysiz,
kerakli avtomatni o'zgartirasiz, qolganlariga tegmasdan qaytarib yozasiz (o'qi-o'zgartir-yoz).

**`volatile` — pochta qutisi (16.3).** Kecha pochta qutingiz bo'sh edi. Bugun "kecha bo'sh edi — demak
bugun ham bo'sh" deb qaramasangiz, xatni o'tkazib yuborasiz. Kompilyator ham "bu o'zgaruvchini hech kim
o'zgartirmadi" deb eski qiymatni ishlatishi mumkin. Qurilma registrini esa **qurilma o'zi** o'zgartiradi.
`volatile` — "har safar borib, qutiga qarab chiq".

**MMIO — pult tugmalari xotira ko'rinishida (16.4).** Qurilma o'z registrlarini xotira manzillari
sifatida ko'rsatadi. Siz oddiy xotiraga yozgandek yozasiz, lekin aslida qurilmaning tugmasini bosasiz.

**Vaqt chegarasi bilan kutish — kuryer (16.5).** Kuryerni kutyapsiz, lekin **cheksiz** emas: "30
daqiqada kelmasa — qo'ng'iroq qilaman". Qurilma buzilgan bo'lishi mumkin — `while (!tayyor)` cheksiz
sikl butun tizimni qotiradi. Doim hisoblagich qo'ying.

**Bitmap — kinoteatr o'rindiqlari sxemasi (16.6).** Kassa ekranida har bir o'rindiq — bitta katakcha:
band yoki bo'sh. 64 o'rindiqli zal uchun bitta 64 bitli son yetadi! Yadroda xuddi shunday: qaysi xotira
sahifalari bo'sh, qaysi disk bloklari band, qaysi jarayon raqamlari ishlatilgan.

### To'liq dastur: kinoteatr kassasi

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

**Sinab ko'ring:** 3-o'rindiqni sotib, `birinchi_bosh()` qanday o'zgarishini kuzating. Barcha 64
o'rindiqni sotuvchi sikl yozing — `birinchi_bosh()` endi nima qaytaradi?

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
