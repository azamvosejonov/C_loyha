# 24-bob. Virtual xotira nazariyasi

> **Bu bobda nima o'rganasiz:** manzil maydoni g'oyasini; base/bounds va segmentatsiyadan sahifalashgacha bo'lgan yo'lni; ko'p darajali sahifa jadvallarini; page fault'ni qayta ishlashni; talab bo'yicha sahifalash
> (demand paging); almashtirish algoritmlarini (OPT, FIFO, LRU, Clock); thrashing'ni; copy-on-write va `mmap` ni. (OSTEP virtualizatsiya qismi + CS:APP 9-bob.)
> **Oldindan nima kerak:** 7-, 8-, 14-, 20-, 21-boblar.   **Vaqt:** 7–9 soat.
> Mashqlar: 31, 43.

> **To'liq ishlaydigan misol:** [misollar/24_virtual_xotira.c](misollar/24_virtual_xotira.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

7-bobda aytdik: dasturdagi manzillar "virtual" — haqiqiy RAM manzillari emas. Bu bobda shu "virtual"ning **hammasini** ochamiz: kim, qanday va nega virtual manzilni fizik manzilga aylantiradi, `malloc(1 GB)` nega
darhol 1 GB egallamaydi, `fork` nega bir zumda bajariladi.

**Hayotdan misol: mehmonxona xona raqami.** Kalitingizda "305" deb yozilgan. Xona binoning qaysi qanotida, qaysi qavatda ekanini bilishingiz shart emas — qabulxona biladi. Boshqa mehmonxonada ham "305" xona
bo'lishi mumkin — ular bir-biriga xalaqit bermaydi. Har bir jarayonning manzillari ham shunday: ikkala dasturda `0x400000` manzil bor, lekin ular RAM'ning **turli** joylariga to'g'ri keladi.
Bir jarayon boshqasining xotirasiga umuman yeta olmaydi — bu himoya.

| Mehmonxonada | Kompyuterda |
|---|---|
| xona raqami "305" (kalit) | **virtual manzil** |
| xonaning haqiqiy joyi (qanot, qavat) | **fizik manzil** (RAM) |
| qabulxona jurnali | **sahifa jadvali** (yadro boshqaradi) |
| qabulxona xodimi | **MMU** (CPU ichida: tarjima qiladi) |
| boshqa mehmonxonada ham "305" | boshqa jarayonda ham `0x400000` |

## 24.1. Nega virtual xotira

Agar har bir dastur fizik xotirani to'g'ridan-to'g'ri ishlatsa:

1. **Himoya yo'q** — bir dastur boshqasining (yoki yadroning) xotirasini buzadi;
2. **Joylashtirish qiyin** — har bir dastur qayerga yuklanishini oldindan bilishi kerak;
3. **Xotira yetmasa** — hech narsa qilib bo'lmaydi.

Yechim: har bir jarayon o'zining **virtual manzil maydonini** ko'radi (0 dan 2⁴⁷ gacha), CPU'dagi **MMU** har bir murojaatda virtual manzilni fizikka aylantiradi, tarjima jadvalini esa **yadro** boshqaradi.
Jarayon boshqalarning xotirasini hatto "ko'ra" olmaydi — uning manzillari boshqa joyga tarjima qilinadi.

Buni ikki jarayonda ko'ramiz — **bir xil manzil, turli ma'lumot**:

```c
/* ikki_manzil.c - bir xil virtual manzil, turli fizik xotira */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int son = 100;                                  /* global o'zgaruvchi */

int main(void)
{
    printf("fork dan oldin: son manzili %p, qiymati %d\n", (void *)&son, son);
    fflush(stdout);

    if (fork() == 0) {                          /* BOLA */
        son = 999;                              /* faqat bolaning nusxasi o'zgaradi */
        printf("bola: son manzili %p, qiymati %d\n", (void *)&son, son);
        return 0;
    }
    wait(NULL);                                 /* ota bolani kutadi */
    printf("ota:  son manzili %p, qiymati %d\n", (void *)&son, son);
    return 0;
}
```

```console
$ gcc -Wall -Wextra ikki_manzil.c -o ikki_manzil
$ ./ikki_manzil | sed -E 's/0x[0-9a-f]+/0xMANZIL/'
fork dan oldin: son manzili 0xMANZIL, qiymati 100
bola: son manzili 0xMANZIL, qiymati 999
ota:  son manzili 0xMANZIL, qiymati 100
$ ./ikki_manzil | grep -o '0x[0-9a-f]*' | sort -u | wc -l
1
```

**Nima ko'rdik:** bola `son = 999` qildi, ota esa `100` ni ko'rdi — **xuddi shu `&son` manzilida**. Oxirgi buyruq uchala satrdagi manzilni solishtiradi: **bitta** noyob manzil (`1`). Demak,
bir xil virtual manzil ikki jarayonda **turli fizik xotiraga** (turli ma'lumotga) tarjima qilinadi.

## 24.2. Tarixiy yo'l: base/bounds → segmentatsiya → sahifalash

**Base and bounds:** har bir jarayonga bitta uzluksiz fizik hudud. `fizik = base + virtual`, agar `virtual < bounds` bo'lsa. Oddiy, lekin stek va heap orasidagi bo'sh joy ham fizik xotira egallaydi.

**Segmentatsiya:** kod, heap, stek — alohida segmentlar, har birining o'z base/bounds'i. Isrof kamayadi, lekin fizik xotira **turli o'lchamdagi** bo'laklarga bo'linib ketadi — **tashqi fragmentatsiya**
(bo'sh joy ko'p, lekin katta uzluksiz bo'lak yo'q). x86 32 bitda segmentlar bor edi; 64 bitda ular deyarli o'chirilgan (faqat FS/GS qoldi — per-CPU va TLS uchun, MyOS `percpu.c`).

**Sahifalash (paging):** xotira **bir xil o'lchamdagi** kichik bo'laklarga — sahifalarga (4 KB) bo'linadi. Istalgan virtual sahifa istalgan fizik sahifaga (freym) tushishi mumkin. Tashqi fragmentatsiya yo'q
(hamma bo'lak bir xil). Narxi: tarjima jadvali kerak va oxirgi sahifadagi ichki isrof.

## 24.3. Sahifa jadvali va uning o'lchami muammosi

**Hayotdan misol: qabulxona jurnali.** Jurnalda yozilgan: "305-xona → Sharqiy qanot, 3-qavat, 12-eshik". Protsessor har bir manzilni sahifa jadvali orqali tarjima qiladi. Xotira 4 KB lik **sahifalarga**
bo'lingan — xuddi mehmonxona xonalarga bo'lingandek. Manzilning yuqori qismi — xona raqami (sahifa), pastki qismi — xona ichidagi joy (siljish).

Virtual manzil = sahifa raqami + sahifa ichidagi siljish:

```text
48 bitli manzil:  [ 36 bit - virtual sahifa raqami (VPN) | 12 bit - siljish ]
```

Kichik o'yinchoq kompyuterda tarjimani qo'lda bajaramiz: 16 bitli virtual manzil, 256 baytlik sahifalar. Protsessor har bir manzil uchun aynan shu hisobni bajaradi — faqat apparat ichida va 4 KB lik sahifalar bilan.

```c
/* tarjima.c - virtual manzil -> fizik manzil: sahifa jadvali va page fault */
#include <stdint.h>
#include <stdio.h>

#define SAHIFA_HAJMI 256                        /* 8 bit - sahifa ichidagi siljish */
#define SAHIFALAR 256                           /* 16 bitli manzil: 256 sahifa */

struct yozuv {
    int bor;                                    /* sahifa xotirada bormi (present) */
    int ramka;                                  /* fizik xotiradagi ramka raqami */
    int yozish_mumkin;
};

static struct yozuv jadval[SAHIFALAR];
static int keyingi_ramka = 7;                   /* bo'sh ramkalar 7 dan boshlanadi */

static void murojaat(uint16_t vmanzil, int yozish)
{
    unsigned sahifa = vmanzil / SAHIFA_HAJMI;   /* yuqori 8 bit */
    unsigned siljish = vmanzil % SAHIFA_HAJMI;  /* pastki 8 bit */
    printf("0x%04X (%s): sahifa %3u, siljish %3u -> ", vmanzil, yozish ? "yozish" : "o'qish",
           sahifa, siljish);

    if (!jadval[sahifa].bor) {
        printf("PAGE FAULT! yadro ramka %d ajratdi -> ", keyingi_ramka);
        jadval[sahifa] = (struct yozuv){ 1, keyingi_ramka++, 1 };
    }
    if (yozish && !jadval[sahifa].yozish_mumkin) {
        printf("HIMOYA XATOSI (faqat o'qish uchun) -> Segmentation fault\n");
        return;
    }
    unsigned fizik = (unsigned)jadval[sahifa].ramka * SAHIFA_HAJMI + siljish;
    printf("fizik 0x%05X\n", fizik);
}

int main(void)
{
    jadval[0x12] = (struct yozuv){ 1, 3, 1 };   /* 0x12-sahifa allaqachon 3-ramkada */
    jadval[0x40] = (struct yozuv){ 1, 5, 0 };   /* kod sahifasi: faqat o'qish */

    murojaat(0x1234, 0);                        /* mavjud sahifa */
    murojaat(0x12FF, 1);                        /* o'sha sahifa, oxirgi bayti */
    murojaat(0x8000, 1);                        /* yangi sahifa - page fault */
    murojaat(0x8010, 0);                        /* endi xotirada - fault yo'q */
    murojaat(0x4004, 0);                        /* kod sahifasini o'qish - mumkin */
    murojaat(0x4004, 1);                        /* kod sahifasiga yozish - taqiqlangan */
    return 0;
}
```

```console
$ gcc -Wall -Wextra tarjima.c -o tarjima
$ ./tarjima
0x1234 (o'qish): sahifa  18, siljish  52 -> fizik 0x00334
0x12FF (yozish): sahifa  18, siljish 255 -> fizik 0x003FF
0x8000 (yozish): sahifa 128, siljish   0 -> PAGE FAULT! yadro ramka 7 ajratdi -> fizik 0x00700
0x8010 (o'qish): sahifa 128, siljish  16 -> fizik 0x00710
0x4004 (o'qish): sahifa  64, siljish   4 -> fizik 0x00504
0x4004 (yozish): sahifa  64, siljish   4 -> HIMOYA XATOSI (faqat o'qish uchun) -> Segmentation fault
```

**Bu dastur nima qiladi (umumiy):** o'yinchoq "MMU": 6 ta murojaat uchun virtual manzilni fizikka aylantiradi, kerak bo'lsa page fault qayta ishlaydi va ruxsatni tekshiradi.

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `manzil / 256`, `manzil % 256` | manzilni **sahifa raqami** (yuqori 8 bit) va **siljish** (pastki 8 bit) ga bo'lish |
| `struct yozuv` | sahifa jadvali yozuvi: `bor` (present), `ramka` (fizik joy), `yozish_mumkin` (ruxsat) |
| `!jadval[sahifa].bor` | sahifa xotirada yo'q → **PAGE FAULT**: yadro bo'sh ramka ajratadi (hozir 7-ramka) |
| `ramka * 256 + siljish` | **fizik manzil** = ramka boshi + siljish |
| `yozish && !yozish_mumkin` | himoya buzildi → Segmentation fault |

E'tibor bering: `0x1234` va `0x12FF` — bitta sahifa (yuqori bayt `0x12`), shuning uchun bitta ramkaga tushdi. `0x8000` ga birinchi murojaatda page fault bo'ldi, ikkinchisida — yo'q.

### Sahifa jadvalining o'lchami

Oddiy (bir darajali) jadval: har bir VPN uchun bitta yozuv → 2³⁶ yozuv × 8 bayt = **512 GB** har bir jarayon uchun! Mumkin emas. Lekin jarayonlarning manzil maydoni asosan **bo'sh** (kod pastda, stek yuqorida, o'rtasi bo'sh).

**Ko'p darajali jadval** — jadval uchun ham "sahifalash": 4 daraja (PML4 → PDPT → PD → PT), har biri 512 yozuv (9 bit). Bo'sh hudud uchun quyi darajadagi jadvallar umuman **yaratilmaydi** — yuqori darajadagi
yozuvda "yo'q" (present = 0) turadi. Kichik dastur uchun ~4–5 ta jadval (20 KB) yetadi. Narxi — tarjima uchun 4 ta xotira murojaati (TLB buni yashiradi — 21-bob). 31-mashqda aynan shu tuzilmani yozdingiz.

**Yozuv bitlari (x86-64):** P (bor), R/W (yozish), U/S (user), A (accessed — CPU o'zi qo'yadi), D (dirty — yozilgan), PS (katta sahifa), NX (63-bit: bajarib bo'lmaydi), 12..51 — fizik manzil.
A va D bitlari almashtirish algoritmlari uchun juda muhim (24.6).

> **Eslab qoling:** virtual manzil = sahifa raqami | siljish. Fizik = ramka boshi + siljish. Sahifa jadvali ko'p darajali (bo'sh hududlar uchun jadval yaratilmaydi).

O'z jarayoningizning virtual manzil xaritasini ko'rish:

```console
$ cat /proc/self/maps | awk '{print $2, $6}' | grep -E '\[(heap|stack)\]'
rw-p [heap]
rw-p [stack]
```

`/proc/self/maps` — jarayonning hududlari (VMA lar); `r`/`w`/`x`/`p` — ruxsatlar. `[heap]` va `[stack]` — 8.1 dagi xaritaning haqiqiy hududlari.

## 24.4. Page fault — "sahifa yo'q" istisnosi

**Hayotdan misol: kutubxonada kitob javonda yo'q.** Kutubxonachidan kitob so'radingiz — javonda yo'q. Bu xato emas: kutubxonachi omborga borib, kitobni olib keladi va sizga beradi. Siz faqat biroz kutasiz.
Page fault ham shunday: sahifa hali xotirada yo'q — yadro uni tayyorlaydi (nol bilan to'ldiradi yoki diskdan o'qiydi) va dastur hech narsani sezmay davom etadi. Faqat haqiqatan ruxsat etilmagan manzil bo'lsa — Segmentation fault.

MMU tarjima qila olmasa (P=0) yoki ruxsat buzilsa (faqat o'qiladigan sahifaga yozish, user rejimidan yadro sahifasiga) — CPU **#PF** istisnosini chaqiradi: xato manzili `CR2` registrida, sababi xato kodida.
Yadroning ishlovchisi hal qiladi:

```text
page_fault(manzil, sabab):
  hudud = shu manzil jarayonning qaysi VMA'siga tegishli?     (MyOS: struct vm_area, kernel/mm/mm.c)
  yo'q                          -> SIGSEGV (dastur xatosi: NULL, chegaradan tashqari)
  bor, lekin ruxsat yo'q        -> COW bo'lsa: nusxalash (24.8); aks holda SIGSEGV
  bor, sahifa hali yo'q         -> TALAB BO'YICHA: yangi sahifa ajratib (nollangan yoki fayldan),
                                   xaritalab, buyruqni QAYTA bajarish
  sahifa diskka chiqarilgan     -> diskdan o'qib, xaritalash (swap)
```

Muhim: page fault — har doim xato emas, ko'pincha **normal ish rejimi**. MyOS'da `fault_page` lab'i aynan shu.

## 24.5. Talab bo'yicha sahifalash (demand paging) va swap

**Hayotdan misol: mebelni kerak bo'lganda olib kelish.** Yangi uyga ko'chdingiz va 100 xonali saroy "ijaraga oldingiz" (`mmap` 100 MB). Hamma xonaga birdaniga mebel olib kelinmaydi — qaysi xonaga birinchi marta
kirsangiz, o'shanga olib kelinadi. Kirmagan xonalar hech narsaga tushmaydi.

`malloc(1 GB)` darhol 1 GB fizik xotira olmaydi — faqat VMA (virtual hudud) yaratiladi. Sahifa birinchi marta tegilganda page fault orqali ajratiladi. Shuning uchun `exec` tez (faqat kerakli sahifalar yuklanadi),
dasturlar ishlatmagan xotira uchun "to'lamaydi". Buni o'lchaymiz:

```c
/* talab_bilan.c - malloc darhol xotira bermaydi: page fault'lar */
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>

#define MB (1024L * 1024)
#define HAJM (200 * MB)

static long rss_kb(void)                        /* jarayon haqiqatan egallagan fizik xotira */
{
    long sahifalar = 0, jami = 0;
    FILE *f = fopen("/proc/self/statm", "r");
    if (f) {
        if (fscanf(f, "%ld %ld", &jami, &sahifalar) != 2)
            sahifalar = 0;
        fclose(f);
    }
    return sahifalar * sysconf(_SC_PAGESIZE) / 1024;
}

static long xatolar(void)                       /* "yengil" page fault'lar soni */
{
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
    return r.ru_minflt;
}

int main(void)
{
    long rss0 = rss_kb(), pf0 = xatolar();
    char *p = malloc(HAJM);                     /* 200 MB so'raldi */
    long rss1 = rss_kb(), pf1 = xatolar();
    printf("malloc(200 MB) dan keyin:  RSS +%ld MB, page fault +%ld\n", (rss1 - rss0) / 1024, pf1 - pf0);

    for (long i = 0; i < HAJM; i += 4096)       /* har bir sahifaga bittadan tegamiz */
        p[i] = 1;
    long rss2 = rss_kb(), pf2 = xatolar();
    printf("hamma sahifaga tegilgach:  RSS +%ld MB, page fault +%ld\n", (rss2 - rss0) / 1024, pf2 - pf0);
    printf("kutilgan fault: 200 MB / 4 KB = %ld\n", HAJM / 4096);
    free(p);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O1 talab_bilan.c -o talab_bilan
$ ./talab_bilan
malloc(200 MB) dan keyin:  RSS +0 MB, page fault +1
hamma sahifaga tegilgach:  RSS +200 MB, page fault +51201
kutilgan fault: 200 MB / 4 KB = 51200
```

**Bu dastur nima qiladi (umumiy):** 200 MB so'raydi va ikki joyda o'lchaydi: (1) `malloc` dan **keyin**, hali tegmasdan; (2) har bir sahifaga bitta bayt yozgandan keyin. Ko'rsatkichlar: **RSS** — jarayon haqiqatan
egallagan fizik xotira; **page fault** soni.

**Nima ko'rdik:** `malloc` dan keyin RSS deyarli **o'zgarmadi** (0 MB), page fault'lar ham juda kam — faqat virtual hudud (VMA) yaratildi. Sahifalarga tegilgach, RSS ~200 MB ga o'sdi va page fault'lar
~51 200 ta bo'ldi (200 MB / 4 KB) — **har bir birinchi tegish** bitta page fault. Bu — talab bo'yicha sahifalash.

> **Eslab qoling:** `malloc` — faqat **va'da**; haqiqiy fizik xotira sahifaga **birinchi tegilganda** page fault orqali beriladi.

Fizik xotira tugasa — kam ishlatilgan sahifalar **diskka** (swap) chiqariladi, yozuvda P=0 qilinadi. Keyin kerak bo'lsa — page fault → diskdan o'qish. **Hayotdan misol: swap — garaj.** Uyda joy qolmasa, kam
ishlatiladigan narsalarni garajga olib chiqasiz. Kerak bo'lsa — qaytib olib kelasiz (sekin). Savol: **qaysi** sahifani chiqarish kerak?

## 24.6. Sahifa almashtirish algoritmlari

Fizik xotira to'lganda, yangi sahifa uchun joy ochish kerak. **Qaysi** sahifa "qurbon" bo'ladi? Algoritmlar:

- **OPT (Belady'ning optimal algoritmi):** kelajakda **eng uzoq** vaqt ishlatilmaydiganini chiqarish. Eng kam page fault — lekin kelajakni bilish kerak, shuning uchun faqat solishtirish uchun o'lchov.
- **FIFO:** eng birinchi kelganini chiqarish. Oddiy, lekin ko'p ishlatiladigan sahifani ham chiqarib yuborishi mumkin. **Belady anomaliyasi:** FIFO'da freymlar ko'paysa, xatolar **ko'payishi** mumkin!
- **LRU (eng uzoq vaqt ishlatilmagan):** o'tmish kelajakning yaxshi bashoratchisi (lokallik — 21-bob). OPT'ga yaqin natija beradi, anomaliyasi yo'q. Lekin aniq LRU uchun **har bir** murojaatda vaqtni yangilash
  kerak — apparatda qimmat.
- **Clock (ikkinchi imkoniyat):** LRU'ning arzon yaqinlashuvi — apparatning A (accessed) bitidan foydalanadi:

```text
freymlar aylana bo'ylab, "soat mili" bitta freymga ko'rsatadi
chiqarish kerak bo'lsa:
    milning ostidagi sahifaning A = 1 bo'lsa -> A = 0 qilib, milni suramiz ("ikkinchi imkoniyat")
    A = 0 bo'lsa -> shuni chiqaramiz
```

Yaqinda ishlatilgan sahifa (A=1) bir aylanish davomida saqlanadi. Takomillashtirilgani D (dirty) bitini ham hisobga oladi: o'zgarmagan sahifani chiqarish arzonroq (diskka yozish shart emas).
Linux'ning "faol/nofaol ro'yxatlari" — shu oilaning murakkab varianti.

Hammasini simulyator bilan solishtiramiz:

```c
/* almashtirish.c - FIFO, LRU, OPT va Clock: sahifa xatolari soni */
#include <stdio.h>
#include <string.h>

#define MAXF 8

static int fifo(const int *s, int n, int k)
{
    int fr[MAXF], bor = 0, sh = 0, xato = 0;
    for (int i = 0; i < n; i++) {
        int topildi = 0;
        for (int j = 0; j < bor; j++)
            if (fr[j] == s[i])
                topildi = 1;
        if (topildi)
            continue;
        xato++;
        if (bor < k)
            fr[bor++] = s[i];
        else {
            fr[sh] = s[i];                      /* eng eskisini almashtiramiz */
            sh = (sh + 1) % k;
        }
    }
    return xato;
}

static int lru(const int *s, int n, int k)
{
    int fr[MAXF], oxirgi[MAXF], bor = 0, xato = 0;
    for (int i = 0; i < n; i++) {
        int j;
        for (j = 0; j < bor; j++)
            if (fr[j] == s[i])
                break;
        if (j < bor) {
            oxirgi[j] = i;                      /* ishlatildi: vaqtni yangilaymiz */
            continue;
        }
        xato++;
        if (bor < k) {
            fr[bor] = s[i];
            oxirgi[bor++] = i;
        } else {
            int eski = 0;                       /* eng uzoq ishlatilmaganini topamiz */
            for (j = 1; j < k; j++)
                if (oxirgi[j] < oxirgi[eski])
                    eski = j;
            fr[eski] = s[i];
            oxirgi[eski] = i;
        }
    }
    return xato;
}

static int opt(const int *s, int n, int k)
{
    int fr[MAXF], bor = 0, xato = 0;
    for (int i = 0; i < n; i++) {
        int j;
        for (j = 0; j < bor; j++)
            if (fr[j] == s[i])
                break;
        if (j < bor)
            continue;
        xato++;
        if (bor < k) {
            fr[bor++] = s[i];
            continue;
        }
        int qurbon = 0, uzoq = -1;
        for (j = 0; j < k; j++) {               /* kelajakda eng kech kerak bo'ladigani */
            int t = i + 1;
            while (t < n && s[t] != fr[j])
                t++;
            if (t > uzoq) {
                uzoq = t;
                qurbon = j;
            }
        }
        fr[qurbon] = s[i];
    }
    return xato;
}

static int soat(const int *s, int n, int k)
{
    int fr[MAXF], a[MAXF], bor = 0, mil = 0, xato = 0;
    for (int i = 0; i < n; i++) {
        int j;
        for (j = 0; j < bor; j++)
            if (fr[j] == s[i])
                break;
        if (j < bor) {
            a[j] = 1;                           /* A biti: yaqinda ishlatildi */
            continue;
        }
        xato++;
        if (bor < k) {
            fr[bor] = s[i];
            a[bor++] = 1;
            continue;
        }
        while (a[mil]) {                        /* A = 1: ikkinchi imkoniyat */
            a[mil] = 0;
            mil = (mil + 1) % k;
        }
        fr[mil] = s[i];                         /* A = 0: shuni chiqaramiz */
        a[mil] = 1;
        mil = (mil + 1) % k;
    }
    return xato;
}

static void sinov(const char *nom, const int *s, int n, int k)
{
    printf("%-26s FIFO %2d  LRU %2d  OPT %2d  Clock %2d\n", nom, fifo(s, n, k), lru(s, n, k),
           opt(s, n, k), soat(s, n, k));
}

int main(void)
{
    int a[] = { 7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2 };
    int b[] = { 1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5 };
    printf("(%d ta murojaat; ustun - xatolar soni)\n", 13);
    sinov("7 0 1 2 0 3 ... , 3 freym", a, 13, 3);
    sinov("Belady ketma-ketligi, 3 fr.", b, 12, 3);
    sinov("Belady ketma-ketligi, 4 fr.", b, 12, 4);
    return 0;
}
```

```console
$ gcc -Wall -Wextra almashtirish.c -o almashtirish
$ ./almashtirish
(13 ta murojaat; ustun - xatolar soni)
7 0 1 2 0 3 ... , 3 freym  FIFO 10  LRU  9  OPT  7  Clock  9
Belady ketma-ketligi, 3 fr. FIFO  9  LRU 10  OPT  7  Clock  9
Belady ketma-ketligi, 4 fr. FIFO 10  LRU  8  OPT  6  Clock 10
```

**Bu dastur nima qiladi (umumiy):** bir xil murojaatlar ketma-ketligini to'rt algoritm bilan o'tkazib, **page fault** (sahifa xatosi) sonini sanaydi. Kamroq xato — yaxshiroq.

**Nima ko'rdik:**

- **OPT** eng kam xato qildi (eng yaxshi, lekin kelajakni bilishni talab qiladi). Birinchi ketma-ketlikda **LRU** (9) va Clock (9) FIFO'dan (10) yaxshi. Lekin aniq ketma-ketlikka bog'liq: ikkinchisida 3 freymda FIFO (9) LRU'dan (10) yaxshi chiqdi — "doim yaxshi" algoritm yo'q, faqat o'rtacha lokallikka tayanadi.
- **Belady anomaliyasi:** ikkinchi ketma-ketlikda FIFO 3 freymda **9** xato, 4 freymda **10** xato qildi — xotira **ko'paysa ham xatolar ko'paydi**! LRU va OPT'da bunday emas.

43-mashqda FIFO, LRU, OPT va Clock'ni o'zingiz simulyatsiya qilasiz.

> **Eslab qoling:** OPT — o'lchov (kelajakni bilish kerak), LRU — yaxshi, lekin qimmat; Clock — arzon yaqinlashuvi (A biti); FIFO'da Belady anomaliyasi bor.

## 24.7. Thrashing va ishchi to'plam

Jarayonlarning **ishchi to'plami** (yaqin vaqtda faol ishlatayotgan sahifalari) jami fizik xotiradan oshsa — tizim vaqtining ko'pini sahifalarni disk va xotira orasida ko'chirishga sarflaydi ("thrashing"):
disk chirog'i yonib turadi, hech narsa ishlamaydi. **Hayotdan misol:** garajga borib-kelish juda ko'payib ketsa, uyda ish umuman oldinga siljimaydi. Yechimlar: ba'zi jarayonlarni to'xtatish, Linux'da — OOM killer
(xotira tugaganda bitta jarayonni o'ldirish).

## 24.8. Copy-on-write (COW) va `fork`

**Hayotdan misol: umumiy darslik.** Aka-uka bitta darslikdan o'qiydi — nusxa shart emas. Uka kitobga nimadir **yozmoqchi** bo'lsa, faqat o'sha sahifaning nusxasini oladi va o'z nusxasiga yozadi.
`fork` aynan shunday: bola otaning barcha sahifalarini bo'lishadi, faqat yozilgan sahifa nusxalanadi. Shuning uchun 1 GB xotirali jarayonni `fork` qilish bir zumda bo'ladi.

`fork` jarayonning butun xotirasini nusxalashi kerak — lekin ko'pincha bola darhol `exec` qiladi va nusxa behuda. COW:

1. `fork`da xotira **nusxalanmaydi** — ota va bola bir xil fizik sahifalarni ko'radi, ikkalasida ham sahifalar **faqat o'qiladigan** qilib belgilanadi (sahifaning havola sanog'i oshiriladi).
2. Kimdir yozmoqchi bo'lsa — page fault (ruxsat yo'q) → yadro: "bu COW sahifa" → nusxa yaratib, yozuvchiga yoziladigan qilib beradi.

Natija: `fork` + `exec` deyarli bepul. Buni o'lchaymiz — bola 40 MB ni avval **o'qiydi**, keyin **yozadi**:

```c
/* cow_olchov.c - fork: o'qish bepul, yozish nusxalaydi */
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#define HAJM (40L * 1024 * 1024)
#define SAHIFALAR (HAJM / 4096)

static long xatolar(void)
{
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
    return r.ru_minflt;
}

int main(void)
{
    char *p = malloc(HAJM);
    for (long i = 0; i < HAJM; i += 4096)       /* ota hamma sahifani egallaydi */
        p[i] = 1;

    if (fork() == 0) {                          /* BOLA */
        long f0 = xatolar();
        volatile char s = 0;
        for (long i = 0; i < HAJM; i += 4096)
            s += p[i];                          /* faqat O'QISH */
        long f1 = xatolar();
        for (long i = 0; i < HAJM; i += 4096)
            p[i] = 2;                           /* YOZISH: har sahifa COW */
        long f2 = xatolar();
        printf("sahifalar soni: %ld\n", SAHIFALAR);
        printf("bola o'qidi:  page fault +%ld\n", f1 - f0);
        printf("bola yozdi:   page fault +%ld (har bir sahifa nusxalandi)\n", f2 - f1);
        fflush(stdout);                         /* _exit buferni yubormaydi (12.3) */
        _exit(0);
    }
    wait(NULL);
    printf("ota: p[0] = %d (bola 2 yozdi, ota 1 ni ko'radi)\n", p[0]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O1 cow_olchov.c -o cow_olchov
$ ./cow_olchov
sahifalar soni: 10240
bola o'qidi:  page fault +0
bola yozdi:   page fault +10240 (har bir sahifa nusxalandi)
ota: p[0] = 1 (bola 2 yozdi, ota 1 ni ko'radi)
```

**Nima ko'rdik:** bola 10 240 sahifani **o'qiganda** deyarli **hech qanday** page fault bo'lmadi (sahifalar ota bilan bo'lishilgan, yadro ularni allaqachon xaritalagan). **Yozganda** — har bir sahifada fault
(COW: yadro nusxa yaratdi) — ~10 240 ta. Ota o'z nusxasini `1` deb ko'rdi — himoya buzilmadi.

MyOS: `docs/11-fork-cow.md`, `kernel/mm/mm.c`.

## 24.9. `mmap` — faylni xotira sifatida

**Bu nima?** `mmap` fayl hududini manzil maydoniga xaritalaydi: fayl baytlariga oddiy massiv kabi murojaat qilasiz. **Asosiy ishi:** fayl bilan `read`/`write` siz ishlash; sahifalar talab bo'yicha
(page fault orqali) fayldan o'qiladi.

```c
/* mmap_misol.c - faylni massiv sifatida o'qish */
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define HAJM (1024 * 1024)

int main(void)
{
    int fd = open("katta.bin", O_RDWR | O_CREAT | O_TRUNC, 0644);   /* 1 MB fayl yaratamiz */
    if (fd < 0)
        return 1;
    if (ftruncate(fd, HAJM) < 0)
        return 1;
    uint8_t satr[4096];
    for (int b = 0; b < HAJM / 4096; b++) {
        for (int i = 0; i < 4096; i++)
            satr[i] = (uint8_t)((b * 4096 + i) % 251);          /* ma'lum qonuniyat */
        if (write(fd, satr, 4096) != 4096)
            return 1;
    }

    uint8_t *p = mmap(NULL, HAJM, PROT_READ, MAP_PRIVATE, fd, 0);   /* faylni xaritalaymiz */
    if (p == MAP_FAILED)
        return 1;
    printf("p[123456] = %d  (kutilgan: 123456 %% 251 = %d)\n", p[123456], 123456 % 251);
    munmap(p, HAJM);
    close(fd);
    return 0;
}
```

```console
$ gcc -Wall -Wextra mmap_misol.c -o mmap_misol
$ ./mmap_misol
p[123456] = 215  (kutilgan: 123456 % 251 = 215)
```

**Qismlar:** `ftruncate(fd, HAJM)` — fayl uzunligini belgilaydi; `mmap(NULL, HAJM, PROT_READ, MAP_PRIVATE, fd, 0)` — "butun faylni o'qish uchun xaritala"; `p[123456]` — fayl baytiga **oddiy massiv kabi** murojaat
(birinchi tegishda page fault → fayldan sahifa o'qiladi). `MAP_ANONYMOUS` — fayl emas, nollangan xotira (katta `malloc` lar shunday olinadi). Dinamik kutubxonalar ham `mmap` bilan yuklanadi va jarayonlar orasida
bo'lishiladi (bir xil fizik sahifalar).

## 24.10. x86-64 da yadro va user manzil maydonlari

```text
0xFFFFFFFFFFFFFFFF ┌──────────────────────┐
                   │ yadro (hamma          │ yuqori yarmi: har bir jarayonda BIR XIL
                   │ jarayonlarda umumiy)  │ (MyOS: direct map, vmalloc, kernel.elf)
0xFFFF800000000000 ├──────────────────────┤
                   │ kanonik bo'lmagan     │ (ishlatib bo'lmaydi - #GP)
0x00007FFFFFFFFFFF ├──────────────────────┤
                   │ user: stek, mmap,     │ har jarayonda BOSHQA
                   │ heap, kod             │
0x0000000000000000 └──────────────────────┘
```

Yadro sahifalarida U/S = 0 — user rejimi ularga tega olmaydi, lekin syscall paytida yadro darhol ishlay oladi (CR3 almashishi shart emas). MyOS xotira xaritasi: README → "Xotira xaritasi".

## Hayotdan misol va to'liq dastur

**Manzil tarjimasi simulyatori.** Bobning asosiy to'liq dasturi — 24.3 dagi `tarjima.c`: o'yinchoq MMU, u virtual manzilni sahifa jadvali orqali fizikka aylantiradi, page fault'ni qayta ishlaydi va himoyani tekshiradi.
Yuqorida ko'rgan real o'lchovlar (talab bo'yicha sahifalash, COW, `mmap`, almashtirish algoritmlari) shu mexanizmning turli yuzlari.

**Sinab ko'ring:** `SAHIFA_HAJMI` ni 4096 qiling (haqiqiy x86) va `SAHIFALAR` ni 16 — manzillar qanday bo'linadi? Sahifa jadvalining o'lchami nega muammo ekanini hisoblang: 48 bitli manzil va 4 KB sahifada nechta yozuv kerak (24.3)?

<!-- katta:boshi -->
## Katta loyiha: sahifa xatolari laboratoriyasi (demand paging, COW)

**Umumiy fikr.** `malloc(1 MB)` yoki `mmap(1 MB)` qilganingizda yadro **darrov 1 MB RAM bermaydi**. U shunchaki "bu manzillar sizniki" deb yozib qo'yadi (*virtual xotira*). Haqiqiy RAM sahifasi **birinchi tegilganda** beriladi: protsessor "bu manzilga RAM ulanmagan" deb **page fault** (sahifa xatosi) hosil qiladi, yadro sahifani ulaydi va dastur **bilmay** davom etadi. 24-bob shu haqda. Bu bosqichda page faultlarni **o'zimiz sanaymiz** — `getrusage()` yadroning haqiqiy hisoblagichini beradi.

**Hayotiy o'xshatish:** mehmonxona. Siz 256 xonali **bron** qildingiz (mmap), lekin xonalar **kirib borganingizda** tayyorlanadi: birinchi kirishda ma'mur kalitni beradi (page fault), keyingi kirishlarda kalit allaqachon sizda.

**Muhim:** page fault ≠ xato. Ko'pchiligi **oddiy ish** (*minor fault* — diskka murojaatsiz). Faqat noto'g'ri manzilga murojaat `SIGSEGV` bilan tugaydi.

### Dastur

Uchta tajriba: (1) demand paging, (2) `madvise` bilan sahifa qaytarish, (3) `fork` + **copy-on-write**:

```c
/* fault_lab.c - sahifa xatolarini (page fault) HAQIQIY yadro bilan o'lchash: demand paging, takroriy murojaat, fork va COW, madvise */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#define SAHIFALAR 256                           /* 256 x 4 KB = 1 MB (2 MB li "katta sahifa"ga yetmaydi: oddiy 4 KB sahifalar) */

static long xatolar(void)                       /* shu jarayonning "kichik" page fault'lari soni (diskka murojaatsiz) */
{
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
    return r.ru_minflt;
}

static long sahifa_hajm;

/* bo'lakning har sahifasiga bittadan bayt yozadi */
static void tegib_chiq(char *p, int sahifalar)
{
    for (int i = 0; i < sahifalar; i++)
        p[(long)i * sahifa_hajm] = 1;
}

static void chiqar(const char *nom, long xato, int sahifalar)
{
    double nisbat = (double)xato / sahifalar;
    const char *baho = nisbat < 0.05 ? "deyarli yo'q" : (nisbat > 0.9 && nisbat < 1.2 ? "~1 sahifaga 1 ta" : "boshqacha");
    printf("  %-44s %4ld xato  (%.2f / sahifa: %s)\n", nom, xato, nisbat, baho);
}

int main(void)
{
    sahifa_hajm = sysconf(_SC_PAGESIZE);
    printf("sahifa hajmi: %ld bayt, bo'lak: %d sahifa\n\n", sahifa_hajm, SAHIFALAR);

    printf("1) Demand paging: mmap xotirani 'va'da qiladi', sahifa haqiqatda tegilganda beriladi\n");
    long a0 = xatolar();
    char *p = mmap(NULL, (size_t)SAHIFALAR * (size_t)sahifa_hajm, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    chiqar("mmap qilingandan keyin (hali tegilmadi)", xatolar() - a0, SAHIFALAR);

    long a1 = xatolar();
    tegib_chiq(p, SAHIFALAR);
    chiqar("1-marta har sahifaga yozdik", xatolar() - a1, SAHIFALAR);

    long a2 = xatolar();
    tegib_chiq(p, SAHIFALAR);
    chiqar("2-marta (sahifalar allaqachon xotirada)", xatolar() - a2, SAHIFALAR);

    printf("\n2) madvise(DONTNEED): sahifalarni yadroga qaytaramiz\n");
    madvise(p, (size_t)SAHIFALAR * (size_t)sahifa_hajm, MADV_DONTNEED);
    long a3 = xatolar();
    tegib_chiq(p, SAHIFALAR);
    chiqar("qaytargandan keyin yana yozdik", xatolar() - a3, SAHIFALAR);

    printf("\n3) fork + copy-on-write: bola ota xotirasini 'baham ko'radi', yozganda nusxalaydi\n");
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        long b0 = xatolar();
        long yig = 0;
        for (int i = 0; i < SAHIFALAR; i++)
            yig += p[(long)i * sahifa_hajm];    /* faqat O'QIYMIZ */
        chiqar("bola: faqat o'qidi (ota bilan umumiy sahifalar)", xatolar() - b0, SAHIFALAR);
        long b1 = xatolar();
        tegib_chiq(p, SAHIFALAR);               /* endi YOZAMIZ: har sahifa uchun nusxa kerak */
        chiqar("bola: har sahifaga yozdi (COW nusxalari)", xatolar() - b1, SAHIFALAR);
        fflush(stdout);
        _exit(yig == SAHIFALAR ? 0 : 1);
    }
    int holat;
    waitpid(pid, &holat, 0);
    printf("  bola tugadi, ota esa sahifalarini o'z holicha saqladi: p[0] = %d\n", p[0]);

    munmap(p, (size_t)SAHIFALAR * (size_t)sahifa_hajm);
    return 0;
}
```

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `getrusage(RUSAGE_SELF, &r)` → `r.ru_minflt` | yadro hisobi: shu jarayonning **minor** page fault soni |
| `mmap(NULL, hajm, ..., MAP_PRIVATE \| MAP_ANONYMOUS, -1, 0)` | fayl bilan bog'lanmagan **xususiy** xotira so'raydi |
| `tegib_chiq(p, n)` | har sahifaga **bittadan bayt** yozadi (har sahifaning boshiga tegamiz — shu yetarli) |
| `madvise(..., MADV_DONTNEED)` | "bu sahifalar kerak emas" — yadro RAM ni qaytarib oladi |
| `fork()` | bola jarayon yaratadi; xotira **nusxalanmaydi**, ota-bola **bir xil sahifalarni** baham ko'radi |

```console
$ cd katta_loyiha/tizim/24_fault_lab
$ gcc -Wall -Wextra -O1 -g fault_lab.c -o fault_lab
$ ./fault_lab
sahifa hajmi: 4096 bayt, bo'lak: 256 sahifa

1) Demand paging: mmap xotirani 'va'da qiladi', sahifa haqiqatda tegilganda beriladi
  mmap qilingandan keyin (hali tegilmadi)         0 xato  (0.00 / sahifa: deyarli yo'q)
  1-marta har sahifaga yozdik                   256 xato  (1.00 / sahifa: ~1 sahifaga 1 ta)
  2-marta (sahifalar allaqachon xotirada)         0 xato  (0.00 / sahifa: deyarli yo'q)

2) madvise(DONTNEED): sahifalarni yadroga qaytaramiz
  qaytargandan keyin yana yozdik                256 xato  (1.00 / sahifa: ~1 sahifaga 1 ta)

3) fork + copy-on-write: bola ota xotirasini 'baham ko'radi', yozganda nusxalaydi
  bola: faqat o'qidi (ota bilan umumiy sahifalar)    0 xato  (0.00 / sahifa: deyarli yo'q)
  bola: har sahifaga yozdi (COW nusxalari)      256 xato  (1.00 / sahifa: ~1 sahifaga 1 ta)
  bola tugadi, ota esa sahifalarini o'z holicha saqladi: p[0] = 1
```

**Nima ko'rdik:**

1. **Demand paging.** `mmap` dan keyin — **0 xato**: hali hech narsa tegilmadi. 1-marta yozganda — **256 xato = har sahifaga 1 ta**. 2-marta — yana **0**: sahifalar allaqachon ulangan. Demak, RAM **faqat haqiqatda ishlatilgan** sahifalar uchun sarflanadi.
2. **`madvise(DONTNEED)`.** Sahifalarni qaytargach, yana yozsak — **yana 256 xato**: yadro ularni qaytadan (nollangan holda) ulashi kerak. Shuning uchun `malloc`/`free` kutubxonalari xotirani **yadroga qaytarishda ehtiyot bo'ladi**.
3. **fork + COW.** Bola faqat **o'qiganda** — **0 xato**: ota bilan **bir xil fizik sahifalar**, nusxa yo'q. Bola **yozganda** — har sahifa uchun **1 xato**: yadro sahifani **nusxalaydi** (copy-on-write). Ota `p[0] = 1` ni saqlab qoldi — bola o'z nusxasini o'zgartirgan, ota sahifasi esa daxlsiz.

**Nega bu muhim?** `fork` 1 GB li jarayonni **mikrosekundlarda** nusxalaydi (hech narsa ko'chirilmaydi). Shell har buyruqda `fork` + `exec` qiladi — COW bo'lmasa, har buyruq GB larni nusxalardi.

> **Eslab qoling:** xotira **virtual**: `mmap`/`malloc` — va'da, haqiqiy sahifa **birinchi tegilganda** beriladi (page fault). Page fault — odatiy hodisa, **xato emas**. `fork` — **copy-on-write**: sahifa **yozilganda** nusxalanadi. Dastur xotirasini **o'lchash** uchun `getrusage`, `/proc/self/status`, `perf stat -e page-faults` dan foydalaning.

**O'zingiz qo'shing (yechimsiz):**

1. `tegib_chiq` ni **faqat har ikkinchi** sahifaga tegadigan qilib o'zgartiring. Nechta xato kutasiz? Tekshiring.
2. `mmap` o'rniga `calloc(SAHIFALAR, sahifa_hajm)` ishlating (katta bo'lak) — natija qanday? (Maslahat: `calloc` katta bo'laklar uchun `mmap` ishlatadi va **nollash shart emas**.)
3. Bola jarayon yozgandan keyin **ota** `p[0]` ni o'qisin: xato sonini o'lchang. Ota-bola sahifalaridan qaysi biri **nusxalandi**?
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. **Virtual xotira:** har jarayon o'z manzil maydonini ko'radi; MMU virtual → fizik tarjima qiladi, jadvalni yadro boshqaradi → **himoya**, qulay joylashtirish, "xotiradan ko'p" ishlatish.
2. Virtual manzil = **sahifa raqami | siljish**; fizik = ramka boshi + siljish. Jadval ko'p darajali (PML4→PDPT→PD→PT) — bo'sh hududlar uchun jadval yaratilmaydi.
3. **Page fault** — har doim xato emas: talab bo'yicha sahifalash, COW, swap — normal ish; faqat VMA'siz/ruxsatsiz murojaat → SIGSEGV. `malloc` — faqat va'da, sahifa tegilganda beriladi.
4. Almashtirish: OPT (o'lchov), FIFO (Belady anomaliyasi), LRU (yaxshi, qimmat), Clock (A biti, arzon); xotira yetmasa — **thrashing**.
5. **COW:** `fork` xotirani nusxalamaydi, yozishda nusxalaydi; **`mmap`** fayl yoki anonim xotirani manzil maydoniga xaritalaydi.

## Savol-javob

**Savol:** Ikki jarayon bir xil virtual manzildan foydalansa, nega bir-birini buzmaydi?
**Javob:** Har jarayonning o'z sahifa jadvali bor: bir xil virtual manzil turli fizik sahifaga tarjima qilinadi (24.1, 24.3). Jarayon boshqasining jadvaliga murojaat qila olmaydi.

**Savol:** `fork` katta jarayonni qanday qilib tez nusxalaydi?
**Javob:** Copy-on-write: xotira darhol nusxalanmaydi, ota va bola bir xil fizik sahifalarni **faqat o'qish** rejimida bo'lishadi. Kimdir yozmoqchi bo'lganda faqat o'sha sahifa nusxalanadi (24.8).

**Savol:** Page fault doim xatomi?
**Javob:** Yo'q. Ko'pi oddiy ishchi hodisa: sahifa hali xotiraga olinmagan (demand paging), swap'dan qaytarish kerak yoki COW nusxasi kerak. Faqat ruxsatsiz manzilga murojaat bo'lsa, jarayon `SIGSEGV` bilan o'ldiriladi (24.4, 24.5).

## O'zingizni tekshiring

1. Nega bir darajali sahifa jadvali amalda ishlatilmaydi?
2. Page fault qachon xato emas? Uchta misol.
3. 3 freym, `1 2 3 4 1 2 5 1 2 3 4 5` — FIFO nechta xato beradi?
4. Clock algoritmi A bitidan qanday foydalanadi?
5. COW'da yozish paytida nima bo'ladi?

<details><summary>Javoblar</summary>

1. 2³⁶ yozuv — har jarayonga 512 GB; manzil maydoni asosan bo'sh, ko'p darajali jadval bo'sh hududlarni yaratmaydi.
2. Talab bo'yicha birinchi murojaat, COW sahifaga yozish, swap qilingan sahifani qaytarish.
3. 9.
4. A=1 bo'lsa — 0 qilib o'tkazib yuboradi (ikkinchi imkoniyat), A=0 bo'lganini chiqaradi.
5. Page fault → yadro sahifani nusxalaydi, yozuvchining jadvaliga yangi yoziladigan nusxani qo'yadi, buyruq qayta bajariladi.
</details>

## Mashq

- **31** (sahifa jadvali) — agar hali qilmagan bo'lsangiz.
- **43** (sahifa almashtirish algoritmlari).
- MyOS: `docs/04-virtual-xotira.md`, `docs/11-fork-cow.md`; `crash` dasturi bilan turli page fault'larni keltirib chiqarib, yadro xabarlarini o'qing.

<!-- loyiha:boshi -->
## Loyiha: ikki darajali sahifa jadvali

**Maqsad:** virtual manzilning **haqiqiy** tarjimasini o'z qo'lingiz bilan bajarish. Protsessorning MMU'si aynan shu ishni apparatda qiladi;
yadro esa shu jadvallarni to'ldiradi (24.3–24.4).
**Bobdan ishlatiladi:** sahifa jadvali, katalog + jadval (2 daraja), sahifa bayroqlari (mavjud/yozish), page fault.

**Talab:** 32 bitli virtual manzil bo'linadi: `[katalog: 10 bit | jadval: 10 bit | siljish: 12 bit]` (sahifa = 4 KB).
- `xarita(va, pa, bayroq)` — virtual sahifani fizik sahifaga bog'laydi. Kerakli jadval yo'q bo'lsa **shu payt yaratadi** (kerak bo'lganda ajratish).
- `tarjima(va, yozish, &pa)` — manzilni tarjima qiladi yoki **page fault** sababini qaytaradi:
  sahifa yo'q (`FAULT_YOQ`) yoki yozish taqiqlangan sahifaga yozishga urinish (`FAULT_HIMOYA`).

**Nega 2 daraja?** Tekis jadval: 2²⁰ yozuv × 4 bayt = **4 MB** — har bir jarayon uchun! 2 darajada faqat **ishlatilgan** hududlar uchun
4 KB lik jadval ajratiladi: ko'pchilik jarayon bir necha MB dan foydalanadi, xolos.

```c
/* sahifa.c - 2 darajali sahifa jadvali */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define P_MAVJUD 1u
#define P_YOZISH 2u

static uint32_t *katalog[1024];                 /* har biri: 1024 yozuvli jadval yoki NULL */
static int jadval_soni;

enum natija { OK, FAULT_YOQ, FAULT_HIMOYA };

static int xarita(uint32_t va, uint32_t pa, unsigned bayroq)
{
    uint32_t d = va >> 22, j = (va >> 12) & 0x3FF;
    if (!katalog[d]) {
        katalog[d] = calloc(1024, sizeof(uint32_t));    /* nollar: "mavjud" biti 0 */
        if (!katalog[d])
            return -1;
        jadval_soni++;
    }
    katalog[d][j] = (pa & ~0xFFFu) | bayroq | P_MAVJUD;
    return 0;
}

static enum natija tarjima(uint32_t va, int yozish, uint32_t *pa)
{
    uint32_t d = va >> 22, j = (va >> 12) & 0x3FF, siljish = va & 0xFFF;
    if (!katalog[d])
        return FAULT_YOQ;
    uint32_t yozuv = katalog[d][j];
    if (!(yozuv & P_MAVJUD))
        return FAULT_YOQ;
    if (yozish && !(yozuv & P_YOZISH))
        return FAULT_HIMOYA;
    *pa = (yozuv & ~0xFFFu) | siljish;
    return OK;
}

static void sinov(uint32_t va, int yozish)
{
    uint32_t pa = 0;
    enum natija n = tarjima(va, yozish, &pa);
    printf("  %s 0x%08X -> ", yozish ? "yozish" : "o'qish", va);
    if (n == OK)
        printf("fizik 0x%08X\n", pa);
    else
        printf("PAGE FAULT (%s)\n", n == FAULT_YOQ ? "sahifa yo'q" : "yozish taqiqlangan");
}

int main(void)
{
    xarita(0x00400000, 0x00200000, P_YOZISH);   /* ma'lumot: o'qish + yozish */
    xarita(0x00401000, 0x00203000, 0);          /* kod: faqat o'qish */
    xarita(0xBFFFF000, 0x00A00000, P_YOZISH);   /* stek (yuqori manzillar) */

    printf("Tarjimalar:\n");
    sinov(0x00400123, 0);
    sinov(0x00400123, 1);
    sinov(0x00401ABC, 0);
    sinov(0x00401ABC, 1);
    sinov(0x00402000, 0);
    sinov(0xBFFFFFF0, 1);
    sinov(0x80000000, 0);

    printf("Ajratilgan jadvallar: %d (%d KB). Tekis jadval 4096 KB bo'lardi.\n", jadval_soni, jadval_soni * 4);
    for (int i = 0; i < 1024; i++)
        free(katalog[i]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined sahifa.c -o sahifa
$ ./sahifa
Tarjimalar:
  o'qish 0x00400123 -> fizik 0x00200123
  yozish 0x00400123 -> fizik 0x00200123
  o'qish 0x00401ABC -> fizik 0x00203ABC
  yozish 0x00401ABC -> PAGE FAULT (yozish taqiqlangan)
  o'qish 0x00402000 -> PAGE FAULT (sahifa yo'q)
  yozish 0xBFFFFFF0 -> fizik 0x00A00FF0
  o'qish 0x80000000 -> PAGE FAULT (sahifa yo'q)
Ajratilgan jadvallar: 2 (8 KB). Tekis jadval 4096 KB bo'lardi.
```

`0x00400123` va `0x00401ABC` **bir xil katalog yozuvi** (`0x00400000` hududi), shuning uchun **bitta** jadval; `0xBFFFF000` boshqa hudud — ikkinchi jadval.
Uchta sahifa uchun atigi 8 KB, tekis jadval esa 4 MB. `0x00402000` xaritalanmagan — jadval bor, lekin yozuv "mavjud emas": page fault.

**Kengaytiring:** `P_USER` bayrog'ini (user rejimi kirishi) qo'shing va `tarjima` ga `user` argumentini bering. Katalog yozuvining o'ziga ham bayroqlar bo'lishi kerakmi?

## Mustaqil loyiha: VMA ro'yxati — `mmap`, `munmap`, `find_vma` ★★★

**Vazifa:** yadro jarayonning manzil maydonini **VMA** (virtual memory area) — "shu diapazon ishlatilmoqda" — larning tartiblangan ro'yxati
sifatida saqlaydi. Sizning vazifangiz shu ro'yxatni boshqarish. Fayl: `vma.c`.

**Manzil maydoni:** `[0x1000, 0x10000)`. Sahifa = `0x1000`. Har VMA — `[boshi, oxiri)`; ro'yxat **manzil bo'yicha tartiblangan**, VMA lar kesishmaydi
(yonma-yon turishi mumkin, birlashtirilmaydi).

**Funksiyalar:**
- `unsigned long vma_mmap(unsigned long uzunlik)` — uzunlikni sahifaga **yuqoriga** yaxlitlaydi va manzil maydonidagi **birinchi (eng past) yetarli
  bo'sh oraliq**ni (first-fit) topib, yangi VMA yaratadi. Manzilni qaytaradi; joy bo'lmasa `0`.
- `int vma_munmap(unsigned long boshi, unsigned long uzunlik)` — `[boshi, boshi+uzunlik)` oralig'ini VMA lardan olib tashlaydi (`boshi` va uzunlik sahifaga
  tekis deb oling). Oraliq VMA ning: **butunini** yopsa — VMA yo'qoladi; **boshini/oxirini** yopsa — qisqaradi; **o'rtasini** yopsa —
  VMA **ikkiga bo'linadi**. Bir nechta VMA ni qamrab olishi ham mumkin. Har doim `0` qaytaradi.
- `const struct vma *vma_top(unsigned long manzil)` — `manzil` qaysi VMA ga tegishli bo'lsa, o'shani; bo'lmasa `NULL`.

**Chiqish shakli (aniq):**
- amal qatori: `mmap(0x3000) = 0x1000` (joy bo'lmasa `mmap(0x20000) = 0x0 (joy yo'q)`), `munmap(0x4000, 0x2000)`,
  `find_vma(0x2500) = yo'q` yoki `find_vma(0x3800) = [0x3000, 0x4000)`;
- `mmap` va `munmap` dan keyin `holat:` qatori, undan keyin ro'yxat qatori: **ikki probel**, so'ng VMA lar `[0x1000, 0x4000)` shaklida
  bir probel bilan ajratilgan; ro'yxat bo'sh bo'lsa `  (bo'sh)`. `find_vma` dan keyin holat chiqarilmaydi.

**Amallar (tartib bilan):**
1. `mmap(0x3000)` &nbsp; 2. `mmap(0x2000)` &nbsp; 3. `mmap(0x1000)` &nbsp; 4. `munmap(0x4000, 0x2000)` &nbsp; 5. `mmap(0x1000)` &nbsp;
6. `munmap(0x2000, 0x1000)` &nbsp; 7. `find_vma(0x2500)` va `find_vma(0x3800)` &nbsp; 8. `mmap(0x20000)` &nbsp; 9. `munmap(0x1000, 0xF000)`

**Kutilgan natija** (`darslik/loyihalar/24_vma/kutilgan.txt`):

```text
mmap(0x3000) = 0x1000
holat:
  [0x1000, 0x4000)
mmap(0x2000) = 0x4000
holat:
  [0x1000, 0x4000) [0x4000, 0x6000)
mmap(0x1000) = 0x6000
holat:
  [0x1000, 0x4000) [0x4000, 0x6000) [0x6000, 0x7000)
munmap(0x4000, 0x2000)
holat:
  [0x1000, 0x4000) [0x6000, 0x7000)
mmap(0x1000) = 0x4000
holat:
  [0x1000, 0x4000) [0x4000, 0x5000) [0x6000, 0x7000)
munmap(0x2000, 0x1000)
holat:
  [0x1000, 0x2000) [0x3000, 0x4000) [0x4000, 0x5000) [0x6000, 0x7000)
find_vma(0x2500) = yo'q
find_vma(0x3800) = [0x3000, 0x4000)
mmap(0x20000) = 0x0 (joy yo'q)
munmap(0x1000, 0xF000)
holat:
  (bo'sh)
```

**Maslahat** (yechim emas):
- Ro'yxat: `struct vma { unsigned long boshi, oxiri; }` massivi (masalan 64 tagacha) + `soni`. Tartiblangan — qo'shish/o'chirishda siljitish kerak.
- `mmap` bo'sh oraliqlarni ko'rib chiqadi: `[0x1000, birinchi.boshi)`, `[i.oxiri, (i+1).boshi)`, `[oxirgi.oxiri, 0x10000)`. Birinchisi yetarli bo'lsa — o'sha.
- `munmap` ni har VMA uchun ko'ring: kesishma bormi? bo'lsa 4 holatdan qaysi biri (butun / chap qism / o'ng qism / o'rta)?
- O'rta holat ro'yxatga **yangi element** qo'shadi (bo'linish).
- Bu — Linux `mm/mmap.c` dagi `find_vma`, `__split_vma`, `unmap_region` ning kichik nusxasi.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined vma.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/24_vma/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [25-bob. Dinamik xotira ajratish](25-xotira-ajratish.md)
