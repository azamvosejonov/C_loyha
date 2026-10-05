# 7-bob. Virtual xotira: Sv32, TLB va tekis bo'lmagan murojaat

> **Bu bobda nima o'rganasiz:** MMU apparat sifatida nima qiladi; Sv32 virtual manzil va PTE formati;
> ikki darajali sahifa jadvali bo'ylab yurish; ruxsat qoidalari (U, SUM, MXR, MPRV); A va D bitlari; TLB
> va `sfence.vma`; 4 MB "katta sahifa"; tekis bo'lmagan (misaligned) murojaatni baytlarga bo'lish.
> **Oldindan nima kerak:** darslik 24-bob (virtual xotira nazariyasi) — **albatta**. 1, 4, 6-boblar.
> **Mashqlar:** E5 (`jadval_yurish`, [emu/mmu.c](../emu/mmu.c)), E8a/E8b ([emu/shina.c](../emu/shina.c)).
> **Vaqt:** 5–6 soat.

## Bu bob nima haqida?

Darslikning 24-bobida virtual xotirani **yadro tomondan** o'rgandingiz: jadvalni yadro tuzadi, sahifa
xatosini yadro hal qiladi. Bu bobda **apparat tomoni**: protsessor har bir `lw`, `sw` va har bir buyruq
o'qishda (!) virtual manzilni qanday fizikka aylantiradi. Bu — emulyatorning eng ko'p ishlaydigan qismi:
Linux yuklanishida `mmu_tarjima` taxminan **64 million** marta chaqiriladi.

**Hayotdan misol: mehmonxona (darslik 24-bobdan).** Kalitdagi "305" (virtual manzil) → qabulxona jurnali
(sahifa jadvali) → haqiqiy xona (fizik manzil). Bu bobda biz **qabulxona xodimi** (MMU) bo'lamiz: jurnalni
o'qish qoidalari, "bu mehmon bu qavatga kira oladimi" tekshiruvi va eng ko'p so'raladigan xonalarni yodda
saqlash (TLB).

## 7.1. Sv32 — formatlar

```text
Virtual manzil (32 bit):
 31          22 21          12 11              0
+--------------+--------------+-----------------+
|   VPN[1]     |   VPN[0]     |     siljish     |
+--------------+--------------+-----------------+
   10 bit: 1-darajali   10 bit: 2-darajali   12 bit: sahifa ichida (4 KB)
   jadvalda indeks      jadvalda indeks

PTE — sahifa jadvali yozuvi (32 bit):
 31                   10  9  8   7   6   5   4   3   2   1   0
+-----------------------+-----+---+---+---+---+---+---+---+---+
|      PPN (22 bit)      | RSW | D | A | G | U | X | W | R | V |
+-----------------------+-----+---+---+---+---+---+---+---+---+

satp: | MODE (1) | ASID (9) | PPN (22) |     MODE = 1 — Sv32 yoqilgan
```

| bit | ma'nosi |
|---|---|
| V | yozuv yaroqli. 0 bo'lsa — qolgan bitlar ahamiyatsiz, sahifa xatosi |
| R, W, X | o'qish, yozish, bajarish ruxsati. **R=W=X=0** — bu barg emas: PPN keyingi jadvalga ishora |
| U | U rejim kira oladi |
| G | global (barcha manzil maydonlarida bir xil — yadro sahifalari) |
| A | murojaat bo'lgan (accessed) |
| D | yozilgan (dirty) |

Nega PPN 22 bit, manzil esa 32 bit? `PPN << 12` = **34 bitli** fizik manzil. Sv32 32 bitli protsessorga
16 GB gacha fizik xotirani ko'rish imkonini beradi (har jarayon baribir 4 GB virtual ko'radi). Bizning shina
32 bitli, shuning uchun 4 GB dan yuqori fizik manzil — kirish xatosi. Kodda `uint64_t` shu sababli.

**Taqiqlangan kombinatsiya:** `W = 1, R = 0` ("faqat yozish") — reserved. Bunday PTE — sahifa xatosi.

## 7.2. Jadval bo'ylab yurish (page table walk)

```text
satp.PPN << 12 ──> [1-darajali jadval, 1024 PTE]
                         │ VPN[1] × 4
                         v
                    PTE₁: barg? ──ha──> 4 MB "katta sahifa" (megapage)
                         │ yo'q (R=W=X=0)
                         v
               PTE₁.PPN << 12 ──> [2-darajali jadval]
                                       │ VPN[0] × 4
                                       v
                                  PTE₀: barg? ──ha──> 4 KB sahifa
                                       │ yo'q
                                       v
                                  sahifa xatosi
```

Algoritm spetsifikatsiyada (Privileged, "Virtual Address Translation Process") **raqamlangan qadamlar**
sifatida berilgan. E5 — shuni kodga aylantirish. Funksiya imzosi:

```c
static int jadval_yurish(struct mashina *m, uint32_t va, enum kirish tur, uint32_t *pte_chiq, int *daraja_chiq,
                         uint32_t *pte_manzil_chiq);
/* 0 — barg topildi (chiqish parametrlari to'ldirilgan), aks holda istisno sababi */
```

Tayyor yordamchilar: `sahifa_xatosi(tur)` (12, 13 yoki 15), `kirish_xatosi(tur)` (1, 5 yoki 7) — murojaat
turiga mos sabab. PTE ni o'qish uchun `shina_oqi` (**fizik** manzil — jadval fizik xotirada!).

### Katta sahifa (megapage) va uning tekisligi

1-darajadagi barg — 4 MB sahifa. Linux yadroni shunday xaritalaydi (kamroq TLB yozuvi). Bunda fizik manzil:

```c
if (daraja == 1)                            /* 4 MB sahifa: VPN[0] va siljish (22 bit) virtual manzildan olinadi */
    return ((ppn >> 10) << 22) | (va & 0x3FFFFF);
```

PPN ning past 10 biti (`PPN[0]`) bu yerda **ishlatilmaydi** — virtual manzildagi VPN[0] uning o'rnini egallaydi.
Spetsifikatsiya: `PPN[0] != 0` bo'lsa — bu "noto'g'ri tekislangan katta sahifa", **sahifa xatosi**. Test buni
tekshiradi (`katta sahifa 4MB ga tekis emas`).

### Qo'lda bitta misol

Test qurgan jadval: `satp.PPN` → `j1 = 0x80010000`; `j1[0x100]` → `j0 = 0x80011000`;
`j0[1]` → sahifa `0x80020000` (V R). VA = `0x40001234`:

```text
VA = 0x40001234 = 0100 0000 0000 0000 0001 0010 0011 0100
VPN[1] = 31..22 = 0b0100000000 = 0x100
VPN[0] = 21..12 = 0b0000000001 = 1
siljish = 0x234

PTE₁ manzili = 0x80010000 + 0x100 × 4 = 0x80010400  -> PTE₁ = PTE(j0, V)  (R=W=X=0: barg emas)
PTE₀ manzili = 0x80011000 + 1 × 4     = 0x80011004  -> PTE₀ = PTE(0x80020000, V|R)  (barg)
fizik = 0x80020000 + 0x234 = 0x80020234   ✓
```

## 7.3. Ruxsatlar — tayyor kod

Barg topilgach, `mmu_tarjima` `ruxsat_bormi()` ni chaqiradi. Uni o'qing — qoidalar zich, lekin mantiqiy:

```c
static int ruxsat_bormi(const struct cpu *c, enum rejim rejim, uint32_t pte, enum kirish tur)
{
    if (rejim == REJIM_U && !(pte & PTE_U))
        return 0;
    if (rejim == REJIM_S && (pte & PTE_U)) {
        if (tur == KIRISH_BAJARISH)
            return 0;
        if (!(c->mstatus & MSTATUS_SUM))
            return 0;
    }
    switch (tur) {
    case KIRISH_OQISH:
        return (pte & PTE_R) || ((c->mstatus & MSTATUS_MXR) && (pte & PTE_X));
    case KIRISH_YOZISH:
        return (pte & PTE_W) != 0;
    case KIRISH_BAJARISH:
        return (pte & PTE_X) != 0;
    }
    return 0;
}
```

Ikki xavfsizlik g'oyasi:

- **Yadro U-sahifadan kod bajara olmaydi**, hech qachon. Hujumchi o'z xotirasiga kod qo'yib, yadrodagi xato
  orqali unga sakratsa — protsessor to'xtatadi (x86 da bu SMEP deb ataladi).
- **Yadro U-sahifani faqat ataylab o'qiydi** (`SUM = 1`). Linux `copy_from_user()` boshida `SUM` ni yoqib,
  oxirida o'chiradi. Tasodifiy ko'rsatkich xatosi foydalanuvchi ma'lumotini o'qib yubormaydi (x86: SMAP).

### MPRV — firmware yadro nomidan

M rejimda tarjima yo'q. Lekin SBI chaqiruvida yadro **virtual** manzilni bersa-chi? `MPRV = 1` bo'lsa,
M rejimning load/store'lari `MPP` dagi rejim nomidan, **tarjima bilan** bajariladi:

```c
static enum rejim samarali_rejim(const struct cpu *c, enum kirish tur)
{
    if (tur != KIRISH_BAJARISH && c->rejim == REJIM_M && (c->mstatus & MSTATUS_MPRV))
        return (enum rejim)((c->mstatus & MSTATUS_MPP) >> MSTATUS_MPP_SILJISH);
    return c->rejim;
}
```

(Buyruq o'qish bunga kirmaydi — firmware kodi o'z manzilida qoladi.)

## 7.4. A va D bitlari

Barg topildi, ruxsat bor. Endi apparat PTE ga **yozadi**: `A = 1` (murojaat bo'ldi), yozish bo'lsa `D = 1`.

```c
uint32_t yangi = pte | PTE_A | (tur == KIRISH_YOZISH ? PTE_D : 0);
if (yangi != pte) {
    shina_yoz(m, pte_manzil, 4, yangi);
    pte = yangi;
}
```

Yadro ular orqali biladi: qaysi sahifa yaqinda ishlatilgan (A — sahifani almashtirish algoritmi uchun,
darslik 24-bob "Clock") va qaysi biri o'zgargan (D — diskka yozish kerakmi). Spetsifikatsiya ikki yo'lga
ruxsat beradi: apparat o'zi qo'yadi (biz) yoki sahifa xatosi beradi va yadro qo'yadi. Linux ikkalasini ham
qo'llaydi.

Muhim tartib: A/D **ruxsat tekshiruvidan keyin**. Yozish rad etilgan sahifada D qo'yilmasligi kerak
(test: `W=0 sahifa: o'qilgan (A), yozish rad etildi (D yo'q)`).

## 7.5. TLB — tarjimalar keshi

Har murojaatda 2 ta qo'shimcha xotira o'qish — juda sekin. Shuning uchun oxirgi tarjimalar TLB'da:
bizda 64 yozuvli, to'g'ridan-to'g'ri xaritalangan (darslik 21-bob kesh turlari):

```c
uint32_t vpn = va >> 12, asid = BITLAR(c->satp, 30, 22);
struct tlb_yozuv *t = &c->tlb[vpn % TLB_HAJM];
/* 4 MB sahifa TLB ga 4 KB qismlari bo'yicha yoziladi: kalit sifatida doim to'liq VPN ishlatiladi */
if (t->bor && t->vpn == vpn && ((t->pte & PTE_G) || t->asid == asid) &&
    (tur != KIRISH_YOZISH || (t->pte & PTE_D))) {
    c->tlb_topildi++;
    ...
```

Shartda uchta tekshiruv bor:

1. `t->vpn == vpn` — aynan shu sahifa (bir katakka 64 ta sahifadan biri tushadi).
2. **ASID** — yozuv shu **jarayonniki**mi (7.6 ga qarang). `G` (global) bitli sahifalar — yadro — hamma
   jarayonda bir xil, ular uchun ASID ahamiyatsiz.
3. **yozish** uchun TLB yozuvidan faqat `D = 1` bo'lsa foydalanamiz. Aks holda birinchi yozish D bitini
   qo'ymay o'tib ketardi — yadro sahifa o'zgarganini bilmay qolardi.

Linux yuklanishidagi haqiqiy statistika (`-s`):

```text
statistika: 56999744 ta buyruq bajarildi; TLB: 63337120 topildi, 767362 topilmadi (98.8%); ...
```

98.8% — har 100 tarjimadan atigi 1 tasi jadval bo'ylab yurishni talab qildi.

### `sfence.vma` — "jadval o'zgardi"

TLB jadvalning **nusxasi**. Yadro jadvalni o'zgartirsa (sahifani o'chirsa, ruxsatni olib tashlasa), TLB eski
tarjimani ishlataveradi — **xavfsizlik teshigi**. Yadro `sfence.vma` buyrug'i bilan TLB'ni tozalashi SHART.
Bizda u `tlb_tozala()` ni chaqiradi. Diqqat: `satp` ga yozish TLB'ni tozalamaydi — haqiqiy protsessordagidek:

```c
case CSR_SATP:
    ...
    /* DIQQAT: satp ni yozish TLB ni TOZALAMAYDI — haqiqiy protsessordagi kabi, yadro sfence.vma qiladi */
    c->satp = q;
```

Nega bunday "noqulay" qilamiz? Chunki emulyator yadro xatolarini **yashirmasligi** kerak. Agar biz satp yozishda
TLB'ni tozalasak, `sfence.vma` ni unutgan yadro bizda ishlaydi, haqiqiy apparatda esa — yo'q. Yaxshi emulyator
haqiqiy apparat kabi **qattiqqo'l**.

## 7.6. ASID — va kitob yozilayotganda topilgan xato

`satp` ning 30..22-bitlari — **ASID** (address space ID, "manzil maydoni raqami"). Har jarayonga yadro
alohida ASID beradi. Nega? Jarayon almashganda (`satp` ga boshqa jadval yoziladi) TLB'ni butunlay tozalash
qimmat — yangi jarayon har sahifasi uchun jadval bo'ylab yuradi. ASID bilan TLB yozuvlari "qaysi jarayonniki"
ekanini eslab qoladi va tozalash kerak bo'lmaydi.

Linux ASID borligini boot paytida tekshiradi (`satp` ning ASID maydoniga hamma birlarni yozib, qaytarib
o'qiydi). Bizning `satp` hamma bitlarni saqlaydi, shuning uchun:

```text
[    0.039001] ASID allocator using 9 bits (512 entries)
```

Va shundan keyin Linux jarayon almashganda **`sfence.vma` qilmaydi**. Emulyatorning birinchi versiyasida TLB
ASID'ni **saqlamas** edi — ya'ni yangi jarayon eski jarayonning tarjimasini olishi mumkin edi! Linux
testlari baribir o'tardi: kichik (64 yozuvli) TLB yadro ishlari orasida tez "yuvilib" ketadi, xato kamdan-kam
holatda ko'rinadi. Bu xato kitobning aynan shu bo'limini yozayotganda, "Linux ASID bilan nima qiladi?" degan
savolga javob izlaganda topildi. Tuzatish tartibi — har qanday jiddiy xato uchun namuna:

1. **Avval xatoni ko'rsatuvchi test** (birlik testi, E5 guruhida): ASID 1 bilan sahifani TLB ga
   tushiramiz, keyin `satp` ga boshqa jadval va ASID 2 yozamiz, `sfence.vma` siz. Kutilgan: yangi jadvaldagi
   sahifa. Eski kod: `olindi 0x80020000, kutilgan 0x80030000` — **xato tasdiqlandi**.
2. **Tuzatish:** TLB yozuviga `asid` maydoni, qidiruvda ASID (yoki G) tekshiruvi.
3. **Hamma testlar qayta**: birlik, assembly, firmware va Linux.

Saboq: "testlar o'tyapti" — "xato yo'q" degani emas. Spetsifikatsiyadagi har bir maydon uchun o'zingizdan
so'rang: "kim buni ishlatadi va qanday?"

## 7.7. Tekis bo'lmagan murojaat (E8)

`lw` manzili 4 ga karrali bo'lmasa (masalan `0x1002`) — nima qilish kerak? Spetsifikatsiya ruxsat beradi:
(a) apparat o'zi bajaradi yoki (b) istisno (4 yoki 6) va firmware dasturiy bajaradi (OpenSBI shunday — sekin).
Biz (a) ni tanladik — Linux buni boot paytida o'lchaydi:

```text
[    0.120905] cpu0: Ratio of byte access time to unaligned word access is 3.99, unaligned accesses are fast
```

(Linux bayt-bayt nusxalash va tekis bo'lmagan so'z nusxalashni solishtirib, tezrog'ini tanlaydi.)

Usul — murojaatni **baytlarga bo'lish**, har baytni **alohida tarjima** qilish:

```c
int xotira_oqi(struct mashina *m, uint32_t va, int hajm, uint32_t *qiymat)
{
    if (!tekis(va, hajm))
        return tekis_emas_oqi(m, va, hajm, qiymat);
    ...
```

Nega har baytni alohida tarjima? `0x1FFE` dagi 4 bayt: ikkitasi bir sahifada (`0x1xxx`), ikkitasi keyingisida
(`0x2xxx`) — ular butunlay boshqa fizik joylarda bo'lishi mumkin.

**E8a — o'qish:** har bayt uchun tarjima → `shina_oqi(..., 1, ...)` → little-endian yig'ish (1.7).

**E8b — yozish — nozikroq:** avval **hamma** baytlarning tarjimasini tekshiring, keyin yozing. Nega? Agar
2 bayt yozilib, 3-baytda sahifa xatosi chiqsa — xotira "yarim" o'zgaradi. Yadro sahifani yuklab, buyruqni
qayta bajaradi (4.2) — natija to'g'ri chiqadi, lekin oraliqda **boshqa** oqim yarim yozilgan qiymatni ko'rishi
mumkin edi. "Istisno — hech narsa bo'lmagandek" qoidasi xotiraga ham tegishli.

Faqat atomik buyruqlar (lr/sc/amo) tekis bo'lishi **shart** — ular uchun istisno (`atomik()` da tekshiriladi).

**Tekshirish:**

```console
$ make test 2>&1 | grep -E "E5|E8|mmu|trap"
  [ OK ] E5 jadval_yurish: Sv32 (mmu.c): 13/13
  [ OK ] E8 tekis bo'lmagan murojaat (shina.c): 8/8
  [ OK ] trap
  [ OK ] mmu
```

`mmu.S` testi butun mexanizmni haqiqiy buyruqlar bilan sinaydi: jadval quradi, `satp` ni yozadi, U rejimga
o'tadi, ruxsatsiz sahifaga murojaat qiladi va sahifa xatosi kelishini kutadi.

## Savol-javob

**Savol:** Nega `mmu_tarjima` M rejimda `*fiz = va` qiladi?
**Javob:** M rejim (firmware) har doim **fizik** manzillar bilan ishlaydi — u apparatning "egasi" va undan
himoyalanadigan narsa yo'q. Haqiqiy apparatda M rejim uchun ham cheklov bor — PMP (physical memory
protection). Bizda PMP registrlari faqat saqlanadi (Linux ularni o'qiydi), ta'siri yo'q — mustaqil mashq.

**Savol:** `sfence.vma` ning argumentlari bor (manzil, ASID). Bizda nega hammasi tozalanadi?
**Javob:** `sfence.vma rs1, rs2` — "faqat shu manzil va/yoki shu ASID uchun tozala". Hammasini tozalash
**ko'proq** ish, lekin har doim to'g'ri (spetsifikatsiya bunga ruxsat beradi). Faqat kerakli yozuvlarni
tozalash — yaxshi optimizatsiya mashqi: TLB "topildi" foizi qanchaga o'zgarishini `-s` bilan o'lchang.
