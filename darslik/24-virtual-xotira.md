# 24-bob. Virtual xotira nazariyasi

> **Bu bobdan keyin:** manzil maydoni g'oyasini, base/bounds va segmentatsiyadan sahifalashgacha
> bo'lgan yo'lni, ko'p darajali sahifa jadvallarini, page fault'ni qayta ishlashni, talab bo'yicha
> sahifalash (demand paging), almashtirish algoritmlarini (OPT, FIFO, LRU, Clock), thrashing'ni,
> copy-on-write va `mmap` ni bilasiz. (OSTEP virtualizatsiya qismi + CS:APP 9-bob.) Mashqlar: 31, 43.

> **To'liq ishlaydigan misol:** [misollar/24_virtual_xotira.c](misollar/24_virtual_xotira.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Virtual manzil — mehmonxona xona raqami (24.1).** Kalitingizda "305" deb yozilgan. Xona binoning qaysi
qanotida, qaysi qavatda ekanini bilishingiz shart emas — qabulxona biladi. Boshqa mehmonxonada ham
"305" xona bo'lishi mumkin — ular bir-biriga xalaqit bermaydi. Har bir jarayonning manzillari ham
shunday: ikkala dasturda `0x400000` manzil bor, lekin ular RAM'ning **turli** joylariga to'g'ri keladi.
Bir jarayon boshqasining xotirasiga umuman yeta olmaydi — bu himoya.

**Sahifa jadvali — qabulxona jurnali (24.3).** Jurnalda yozilgan: "305-xona → Sharqiy qanot, 3-qavat,
12-eshik". Protsessor har bir manzilni sahifa jadvali orqali tarjima qiladi. Xotira 4 KB lik
**sahifalarga** bo'lingan — xuddi mehmonxona xonalarga bo'lingandek. Manzilning yuqori qismi —
xona raqami (sahifa), pastki qismi — xona ichidagi joy (siljish).

**Page fault — kutubxonada kitob javonda yo'q (24.4).** Kutubxonachidan kitob so'radingiz — javonda yo'q.
Bu xato emas: kutubxonachi omborga borib, kitobni olib keladi va sizga beradi. Siz faqat biroz kutasiz.
Page fault ham shunday: sahifa hali xotirada yo'q — yadro uni tayyorlaydi (nol bilan to'ldiradi yoki
diskdan o'qiydi) va dastur hech narsani sezmay davom etadi. Faqat haqiqatan ruxsat etilmagan manzil
bo'lsa — Segmentation fault.

**Talab bo'yicha sahifalash — mebelni kerak bo'lganda olib kelish (24.5).** Yangi uyga ko'chdingiz
va 100 xonali saroy "ijaraga oldingiz" (`mmap` 100 MB). Hamma xonaga birdaniga mebel olib kelinmaydi —
qaysi xonaga birinchi marta kirsangiz, o'shanga olib kelinadi. Kirmagan xonalar hech narsaga tushmaydi.

**Swap — garaj (24.5).** Uyda joy qolmasa, kam ishlatiladigan narsalarni garajga olib chiqasiz. Kerak
bo'lsa — qaytib olib kelasiz (sekin). Garajga borib-kelish juda ko'payib ketsa, ish umuman oldinga
siljimaydi — bu **thrashing** (24.7).

**Copy-on-write — umumiy darslik (24.8).** Aka-uka bitta darslikdan o'qiydi — nusxa shart emas. Uka
kitobga nimadir **yozmoqchi** bo'lsa, faqat o'sha sahifaning nusxasini oladi va o'z nusxasiga yozadi.
`fork` aynan shunday: bola otaning barcha sahifalarini bo'lishadi, faqat yozilgan sahifa nusxalanadi.
Shuning uchun 1 GB xotirali jarayonni `fork` qilish bir zumda bo'ladi.

### To'liq dastur: manzil tarjimasi simulyatori

Kichik o'yinchoq kompyuter: 16 bitli virtual manzil, 256 baytlik sahifalar. Protsessor har bir manzil
uchun aynan shu hisobni bajaradi — faqat apparat ichida va 4 KB lik sahifalar bilan.

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

E'tibor bering: `0x1234` va `0x12FF` — bitta sahifa (yuqori bayt `0x12`), shuning uchun bitta ramkaga
tushdi. `0x8000` ga birinchi murojaatda page fault bo'ldi, ikkinchisida — yo'q.

**Sinab ko'ring:** `SAHIFA_HAJMI` ni 4096 qiling (haqiqiy x86) va `SAHIFALAR` ni 16 — manzillar qanday
bo'linadi? Sahifa jadvalining o'lchami nega muammo ekanini hisoblang: 48 bitli manzil va 4 KB sahifada
nechta yozuv kerak (24.3)?

## 24.1. Nega virtual xotira

Agar har bir dastur fizik xotirani to'g'ridan-to'g'ri ishlatsa:
1. **Himoya yo'q** — bir dastur boshqasining (yoki yadroning) xotirasini buzadi;
2. **Joylashtirish qiyin** — har bir dastur qayerga yuklanishini oldindan bilishi kerak;
3. **Xotira yetmasa** — hech narsa qilib bo'lmaydi.

Yechim: har bir jarayon o'zining **virtual manzil maydonini** ko'radi (0 dan 2⁴⁷ gacha), CPU'dagi
**MMU** har bir murojaatda virtual manzilni fizikka aylantiradi, tarjima jadvalini esa **yadro**
boshqaradi. Jarayon boshqalarning xotirasini hatto "ko'ra" olmaydi — uning manzillari boshqa joyga tarjima qilinadi.

## 24.2. Tarixiy yo'l: base/bounds → segmentatsiya → sahifalash

**Base and bounds:** har bir jarayonga bitta uzluksiz fizik hudud. `fizik = base + virtual`, agar
`virtual < bounds` bo'lsa. Oddiy, lekin stek va heap orasidagi bo'sh joy ham fizik xotira egallaydi.

**Segmentatsiya:** kod, heap, stek — alohida segmentlar, har birining o'z base/bounds'i. Isrof
kamayadi, lekin fizik xotira **turli o'lchamdagi** bo'laklarga bo'linib ketadi — **tashqi fragmentatsiya**
(bo'sh joy ko'p, lekin katta uzluksiz bo'lak yo'q). x86 32 bitda segmentlar bor edi; 64 bitda ular deyarli
o'chirilgan (faqat FS/GS qoldi — per-CPU va TLS uchun, MyOS `percpu.c`).

**Sahifalash (paging):** xotira **bir xil o'lchamdagi** kichik bo'laklarga — sahifalarga (4 KB) bo'linadi.
Istalgan virtual sahifa istalgan fizik sahifaga (freym) tushishi mumkin. Tashqi fragmentatsiya yo'q
(hamma bo'lak bir xil). Narxi: tarjima jadvali kerak va oxirgi sahifadagi ichki isrof.

## 24.3. Sahifa jadvali va uning o'lchami muammosi

Virtual manzil = sahifa raqami + sahifa ichidagi siljish:

```text
48 bitli manzil:  [ 36 bit - virtual sahifa raqami (VPN) | 12 bit - siljish ]
```

Oddiy (bir darajali) jadval: har bir VPN uchun bitta yozuv → 2³⁶ yozuv × 8 bayt = **512 GB** har bir
jarayon uchun! Mumkin emas. Lekin jarayonlarning manzil maydoni asosan **bo'sh** (kod pastda, stek
yuqorida, o'rtasi bo'sh).

**Ko'p darajali jadval** — jadval uchun ham "sahifalash": 4 daraja (PML4 → PDPT → PD → PT), har biri
512 yozuv (9 bit). Bo'sh hudud uchun quyi darajadagi jadvallar umuman **yaratilmaydi** — yuqori
darajadagi yozuvda "yo'q" (present = 0) turadi. Kichik dastur uchun ~4–5 ta jadval (20 KB) yetadi.
Narxi — tarjima uchun 4 ta xotira murojaati (TLB buni yashiradi — 21-bob). 31-mashqda aynan shu tuzilmani yozdingiz.

**Yozuv bitlari (x86-64):** P (bor), R/W (yozish), U/S (user), A (accessed — CPU o'zi qo'yadi),
D (dirty — yozilgan), PS (katta sahifa), NX (63-bit: bajarib bo'lmaydi), 12..51 — fizik manzil.
A va D bitlari almashtirish algoritmlari uchun juda muhim (24.6).

## 24.4. Page fault — "sahifa yo'q" istisnosi

MMU tarjima qila olmasa (P=0) yoki ruxsat buzilsa (faqat o'qiladigan sahifaga yozish, user rejimidan
yadro sahifasiga) — CPU **#PF** istisnosini chaqiradi: xato manzili `CR2` registrida, sababi xato kodida.
Yadroning ishlovchisi hal qiladi:

```text
page_fault(manzil, sabab):
  hudud = shu manzil jarayonning qaysi VMA'siga tegishli?     (MyOS: struct vm_area, kernel/mm/mm.c)
  yo'q                          -> SIGSEGV (dastur xatosi: NULL, chegaradan tashqari)
  bor, lekin ruxsat yo'q        -> COW bo'lsa: nusxalash (24.7); aks holda SIGSEGV
  bor, sahifa hali yo'q         -> TALAB BO'YICHA: yangi sahifa ajratib (nollangan yoki fayldan),
                                   xaritalab, buyruqni QAYTA bajarish
  sahifa diskka chiqarilgan     -> diskdan o'qib, xaritalash (swap)
```

Muhim: page fault — har doim xato emas, ko'pincha **normal ish rejimi**. MyOS'da `fault_page` lab'i aynan shu.

## 24.5. Talab bo'yicha sahifalash (demand paging) va swap

`malloc(1 GB)` darhol 1 GB fizik xotira olmaydi — faqat VMA (virtual hudud) yaratiladi. Sahifa birinchi
marta tegilganda page fault orqali ajratiladi. Shuning uchun `exec` tez (faqat kerakli sahifalar
yuklanadi), dasturlar ishlatmagan xotira uchun "to'lamaydi".

Fizik xotira tugasa — kam ishlatilgan sahifalar **diskka** (swap) chiqariladi, yozuvda P=0 qilinadi.
Keyin kerak bo'lsa — page fault → diskdan o'qish. Savol: **qaysi** sahifani chiqarish kerak?

## 24.6. Sahifa almashtirish algoritmlari

Misol: 3 ta freym, murojaatlar ketma-ketligi `7 0 1 2 0 3 0 4 2 3 0 3 2`.

**OPT (Belady'ning optimal algoritmi):** kelajakda **eng uzoq** vaqt ishlatilmaydiganini chiqarish.
Eng kam page fault — lekin kelajakni bilish kerak, shuning uchun faqat solishtirish uchun o'lchov.

**FIFO:** eng birinchi kelganini chiqarish. Oddiy, lekin ko'p ishlatiladigan sahifani ham chiqarib
yuborishi mumkin. **Belady anomaliyasi:** FIFO'da freymlar ko'paysa, xatolar **ko'payishi** mumkin!
(`1 2 3 4 1 2 5 1 2 3 4 5` — 3 freymda 9 xato, 4 freymda 10 xato.)

**LRU (eng uzoq vaqt ishlatilmagan):** o'tmish kelajakning yaxshi bashoratchisi (lokallik — 21-bob).
OPT'ga yaqin natija beradi, anomaliyasi yo'q. Lekin aniq LRU uchun **har bir** murojaatda vaqtni yangilash
kerak — apparatda qimmat.

**Clock (ikkinchi imkoniyat):** LRU'ning arzon yaqinlashuvi — apparatning A (accessed) bitidan foydalanadi:

```text
freymlar aylana bo'ylab, "soat mili" bitta freymga ko'rsatadi
chiqarish kerak bo'lsa:
    milning ostidagi sahifaning A = 1 bo'lsa -> A = 0 qilib, milni suramiz ("ikkinchi imkoniyat")
    A = 0 bo'lsa -> shuni chiqaramiz
```

Yaqinda ishlatilgan sahifa (A=1) bir aylanish davomida saqlanadi. Takomillashtirilgani D (dirty) bitini
ham hisobga oladi: o'zgarmagan sahifani chiqarish arzonroq (diskka yozish shart emas). Linux'ning
"faol/nofaol ro'yxatlari" — shu oilaning murakkab varianti.

43-mashqda FIFO, LRU, OPT va Clock'ni simulyatsiya qilib, xatolar sonini solishtirasiz.

## 24.7. Thrashing va ishchi to'plam

Jarayonlarning **ishchi to'plami** (yaqin vaqtda faol ishlatayotgan sahifalari) jami fizik xotiradan
oshsa — tizim vaqtining ko'pini sahifalarni disk va xotira orasida ko'chirishga sarflaydi ("thrashing"):
disk chirog'i yonib turadi, hech narsa ishlamaydi. Yechimlar: ba'zi jarayonlarni to'xtatish, Linux'da —
OOM killer (xotira tugaganda bitta jarayonni o'ldirish).

## 24.8. Copy-on-write (COW) va `fork`

`fork` jarayonning butun xotirasini nusxalashi kerak — lekin ko'pincha bola darhol `exec` qiladi va
nusxa behuda. COW:
1. `fork`da xotira **nusxalanmaydi** — ota va bola bir xil fizik sahifalarni ko'radi, ikkalasida ham
   sahifalar **faqat o'qiladigan** qilib belgilanadi (sahifaning havola sanog'i oshiriladi).
2. Kimdir yozmoqchi bo'lsa — page fault (ruxsat yo'q) → yadro: "bu COW sahifa" → nusxa yaratib,
   yozuvchiga yoziladigan qilib beradi.

Natija: `fork` + `exec` deyarli bepul. MyOS: `docs/11-fork-cow.md`, `kernel/mm/mm.c`.

## 24.9. `mmap` — faylni xotira sifatida

```c
int fd = open("katta.bin", O_RDONLY);
uint8_t *p = mmap(NULL, hajm, PROT_READ, MAP_PRIVATE, fd, 0);
printf("%d\n", p[123456]);          /* fayl baytini oddiy massivdek o'qish */
```

`mmap` fayl hududini manzil maydoniga xaritalaydi; sahifalar talab bo'yicha (page fault orqali) fayldan
o'qiladi. `MAP_ANONYMOUS` — fayl emas, nollangan xotira (katta `malloc` lar shunday olinadi). Dinamik
kutubxonalar ham `mmap` bilan yuklanadi va jarayonlar orasida bo'lishiladi (bir xil fizik sahifalar).

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

Yadro sahifalarida U/S = 0 — user rejimi ularga tega olmaydi, lekin syscall paytida yadro darhol
ishlay oladi (CR3 almashishi shart emas). MyOS xotira xaritasi: README → "Xotira xaritasi".

## 24.11. O'zingizni tekshiring

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

## 24.12. Mashqlar

- **31** (sahifa jadvali) — agar hali qilmagan bo'lsangiz.
- **43** (sahifa almashtirish algoritmlari).
- MyOS: `docs/04-virtual-xotira.md`, `docs/11-fork-cow.md`; `crash` dasturi bilan turli page fault'larni
  keltirib chiqarib, yadro xabarlarini o'qing.

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
