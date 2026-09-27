# 24-bob. Virtual xotira nazariyasi

> **Bu bobdan keyin:** manzil maydoni g'oyasini, base/bounds va segmentatsiyadan sahifalashgacha
> bo'lgan yo'lni, ko'p darajali sahifa jadvallarini, page fault'ni qayta ishlashni, talab bo'yicha
> sahifalash (demand paging), almashtirish algoritmlarini (OPT, FIFO, LRU, Clock), thrashing'ni,
> copy-on-write va `mmap` ni bilasiz. (OSTEP virtualizatsiya qismi + CS:APP 9-bob.) Mashqlar: 31, 43.

> **To'liq ishlaydigan misol:** [misollar/24_virtual_xotira.c](misollar/24_virtual_xotira.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

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

Keyingi bob: [25-bob. Dinamik xotira ajratish](25-xotira-ajratish.md)
