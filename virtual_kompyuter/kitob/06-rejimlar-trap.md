# 6-bob. Imtiyoz rejimlari, CSR, trap va uzilishlar

> **Bu bobda nima o'rganasiz:** nega protsessorda "rejimlar" bor (U, S, M); CSR registrlari va ularning
> himoya qoidasi; `mstatus` bitlari; trap — istisno va uzilish — paytida apparat **aniq** nima qiladi;
> `mret`/`sret`; delegatsiya (`medeleg`, `mideleg`); uzilish qachon qabul qilinadi va ustuvorlik;
> Linux'ni yuklashdagi eng qiyin xato — `mip.SEIP` hikoyasi.
> **Oldindan nima kerak:** 1–4-boblar. Darslik 14-bob (tizim chaqiruvlari), 23-bob (jarayonlar), 30-bob (yadro).
> **Mashqlar:** E6 (`trap_kirish`), E9 (`uzilish_tekshir`) — [emu/trap.c](../emu/trap.c).   **Vaqt:** 5–6 soat.

## Bu bob nima haqida?

Hozirgacha protsessorimiz "sodda": har buyruq hammaga ruxsat. Bunday protsessorda operatsion tizim
**qura olmaysiz**: istalgan dastur boshqa dasturning xotirasini o'qiydi, taymerni o'chiradi, diskka to'g'ridan-
to'g'ri yozadi. Himoya uchun apparat ikki narsa beradi:

1. **Rejimlar** — protsessor hozir "kim nomidan" ishlayapti: foydalanuvchi dasturimi (U), yadromi (S),
   firmwaremi (M). Har rejimga ruxsat etilgan narsa — boshqa.
2. **Trap** — boshqaruvni **majburan** yuqori rejimga o'tkazish mexanizmi: dastur xato qilganda,
   tizim chaqiruvi qilganda yoki tashqi hodisa (taymer, klaviatura) bo'lganda.

Operatsion tizimning **butun** ishi shu ikkitasi ustiga quriladi. Yadro hech qachon "o'zi" ishlamaydi —
uni faqat trap uyg'otadi.

**Hayotdan misol: bank.** Mijoz (U) kassaga ariza beradi (ecall), lekin seyfga kira olmaydi. Kassir (S)
seyfdan pul oladi, lekin bankning signalizatsiya tizimini o'chira olmaydi. Xavfsizlik xizmati (M) hammasini
boshqaradi. Agar mijoz seyfga kirmoqchi bo'lsa (taqiqlangan buyruq) — signal (istisno) kassirga boradi.
Yong'in signali (uzilish) esa kim nima qilayotganidan qat'i nazar ishni to'xtatadi.

## 6.1. Uchta rejim

| Rejim | Kod | Kim | Nima qila oladi |
|---|---|---|---|
| U (user) | 0 | `/init`, `/bin/salom` | oddiy buyruqlar; o'z sahifalari; `ecall` |
| S (supervisor) | 1 | Linux yadrosi | + S CSR'lar (`satp`, `stvec` ...), `sret`, `sfence.vma`, hamma sahifalar |
| M (machine) | 3 | firmware | hammasi: M CSR'lar, fizik xotira (tarjimasiz), `mret` |

(2 — gipervizor (H) uchun band; bizda yo'q.)

Joriy rejim `struct cpu` dagi `rejim` maydoni. Uni **faqat** trap (yuqoriga) va `mret`/`sret` (pastga)
o'zgartiradi. Dasturning "M rejimga o'tish" buyrug'i yo'q — aks holda himoyaning ma'nosi qolmasdi.

## 6.2. CSR — boshqaruv registrlari

CSR (control and status register) — 12 bitli raqam bilan tanlanadigan maxsus registrlar. Ular bilan
olti buyruq ishlaydi: `csrrw`, `csrrs`, `csrrc` va `...i` variantlari (2-bobdagi SYSTEM opcode).

| Buyruq | Ma'nosi | Maxsus holat |
|---|---|---|
| `csrrw rd, csr, rs1` | `rd ← csr; csr ← rs1` | `rd = x0` — **o'qimaydi** |
| `csrrs rd, csr, rs1` | `rd ← csr; csr ← csr \| rs1` (bitlarni yoqish) | `rs1 = x0` — **yozmaydi** |
| `csrrc rd, csr, rs1` | `rd ← csr; csr ← csr & ~rs1` (bitlarni o'chirish) | `rs1 = x0` — **yozmaydi** |

Assemblerdagi `csrr t0, mstatus` — aslida `csrrs t0, mstatus, x0` (faqat o'qish), `csrw mtvec, t0` —
`csrrw x0, mtvec, t0` (faqat yozish). "Maxsus holatlar" muhim: faqat-o'qish CSR'ni (`cycle`) `csrr` bilan
o'qish **istisno bermasligi** kerak, chunki u yozmaydi.

### Himoya CSR raqamining o'zida

```text
 11 10  9  8   7 ... 0
+-----+-----+---------+
| R/W | rej |  raqam  |
+-----+-----+---------+
  11 — faqat o'qish     00 — U, 01 — S, 11 — M (kamida qaysi rejim kerak)
```

`satp = 0x180 = 0b00_01_1000_0000` → yozish mumkin, kamida S. `mhartid = 0xF14 = 0b11_11_...` → faqat o'qish,
faqat M. Shuning uchun [emu/csr.c](../emu/csr.c) har murojaatni ikki qatordan boshlaydi:

```c
if (BITLAR(raqam, 9, 8) > (uint32_t)c->rejim)
    return SABAB_NOTOGRI_BUYRUQ;            /* rejim yetarli emas */
if (BITLAR(raqam, 11, 10) == 3)
    return SABAB_NOTOGRI_BUYRUQ;            /* faqat o'qiladigan CSR (faqat csr_yoz da) */
```

Bu — dizayndagi go'zallik: ruxsat jadvali kerak emas, **raqamning bitlari** jadvalning o'zi.

### Ko'rinishlar: `sstatus`, `sie`, `sip`

`sstatus` — alohida registr **emas**. U `mstatus` ning S rejimga ruxsat etilgan bitlaridan iborat "deraza".
`sstatus` ga yozish aslida `mstatus` ning o'sha bitlarini o'zgartiradi (1-bobdagi andoza 3):

```c
case CSR_SSTATUS: *q = c->mstatus & SSTATUS_MASKA; return 0;           /* o'qish */
...
case CSR_SSTATUS:
    c->mstatus = maskali(c->mstatus, q, SSTATUS_MASKA);   /* faqat S ga ko'rinadigan bitlar */
```

Xuddi shunday `sie = mie & mideleg` — yadro faqat firmware unga "bergan" uzilishlarni ko'radi.

### `mstatus` — eng muhim bitlar

| bit | nom | ma'nosi |
|---|---|---|
| 1 | SIE | S rejimda uzilishlar yoqilgan |
| 3 | MIE | M rejimda uzilishlar yoqilgan |
| 5 | SPIE | S trap'idan **oldingi** SIE |
| 7 | MPIE | M trap'idan oldingi MIE |
| 8 | SPP | S trap'idan oldingi rejim (1 bit: U yoki S) |
| 12..11 | MPP | M trap'idan oldingi rejim (2 bit: U, S yoki M) |
| 17 | MPRV | M rejimda load/store'ni MPP rejimi nomidan bajarish (7-bob) |
| 18 | SUM | S rejim U-sahifalarni o'qiy/yoza oladi (`copy_from_user`) |
| 19 | MXR | bajariladigan sahifani o'qish mumkin |
| 20 | TVM | S da `satp` va `sfence.vma` taqiqlangan |
| 21 | TW | S da `wfi` taqiqlangan |
| 22 | TSR | S da `sret` taqiqlangan |

`xPIE` va `xPP` — "**ichma-ich trap uchun stek**ning bitta qavati": trap'ga kirishda joriy holat shu yerga
saqlanadi, qaytishda tiklanadi.

## 6.3. Trap'ga kirish — apparat nima qiladi

Trap — istisno (buyruq sababli: `ecall`, sahifa xatosi, noto'g'ri buyruq) yoki uzilish (tashqi: taymer,
qurilma). Ikkalasi uchun apparat **bir xil** qadamlarni bajaradi (M uchun; S uchun — `s` registrlar):

1. `mepc ← pc` — qaytish manzili. Istisnoda — **aynan xato buyruq** (qayta bajarish uchun, 4.2); uzilishda
   — hali bajarilmagan keyingi buyruq.
2. `mcause ← sabab`. 31-bit: 1 — uzilish, 0 — istisno.
3. `mtval ← qo'shimcha ma'lumot`: sahifa xatosida — xato manzil; noto'g'ri buyruqda — buyruqning o'zi; aks holda 0.
4. `MPP ← joriy rejim`; `MPIE ← MIE`; `MIE ← 0` (ishlovchi boshida uzilishlar o'chiq).
5. `rejim ← M`.
6. `pc ← mtvec`. Agar `mtvec` ning past 2 biti 1 (vektorli) va bu **uzilish** bo'lsa: `pc ← asos + 4 × sabab`.

| sabab | istisno | | sabab | uzilish (31-bit = 1) |
|---|---|---|---|---|
| 0 | buyruq manzili tekis emas | | 1 | S dasturiy (SSI) |
| 1 | buyruq o'qishda kirish xatosi | | 3 | M dasturiy (MSI) |
| 2 | noto'g'ri buyruq | | 5 | S taymer (STI) |
| 3 | breakpoint (`ebreak`) | | 7 | M taymer (MTI) |
| 4, 6 | load/store manzili tekis emas | | 9 | S tashqi (SEI) — PLIC |
| 5, 7 | load/store kirish xatosi | | 11 | M tashqi (MEI) — PLIC |
| 8, 9, 11 | `ecall` U, S, M rejimdan | | | |
| 12, 13, 15 | sahifa xatosi: buyruq, load, store/AMO | | | |

### Real trace: U rejimdan tizim chaqiruvi

`testlar/emu/trap.S` testidan (`./build/vk -S -t testlar/emu/build/trap.elf`):

```text
[S] 80000138: 10200073  sret                         <- yadro U rejimga "tushdi"
[U] 800000fc: 04d00513  addi a0, zero, 77
[U] 80000100: 00000073  ecall                        <- dastur: "yadro, yordam ber"
    ~~ istisno 8 (tval=0x00000000) pc=0x80000100 [U] -> pc=0x80000104 [S]
[S] 80000104: 142024f3  csrrs s1, scause, zero       <- stvec dagi ishlovchi: sabab = 8
[S] 80000108: 14102973  csrrs s2, sepc, zero         <-   sepc = 0x80000100
[S] 8000010c: 143029f3  csrrs s3, stval, zero
```

Qavs ichidagi harf o'zgarishini kuzating: `[U]` → `[S]`. Bu — `trap_kirish` ishi. `~~` bilan boshlangan
qator — emulyatorning trap izi (`-t` rejimida har trap va uzilish shunday ko'rinadi): sabab 8 (U dan `ecall`),
qayerdan, qayerga. Bu yerda `ecall`
**to'g'ridan-to'g'ri S ga** bordi, M ga emas — chunki delegatsiya qilingan (6.5).

## 6.4. Qaytish: `mret` va `sret`

[emu/trap.c](../emu/trap.c) dagi tayyor kod — trap'ga kirishning **teskarisi**:

```c
void trap_qaytish_s(struct cpu *c)
{
    c->rejim = (c->mstatus & MSTATUS_SPP) ? REJIM_S : REJIM_U;
    if (c->mstatus & MSTATUS_SPIE)              /* SIE <- SPIE */
        c->mstatus |= MSTATUS_SIE;
    else
        c->mstatus &= ~MSTATUS_SIE;
    c->mstatus |= MSTATUS_SPIE;                 /* spetsifikatsiya: SPIE <- 1, SPP <- U */
    c->mstatus &= ~MSTATUS_SPP;
    c->mstatus &= ~MSTATUS_MPRV;                /* M dan pastga qaytilsa MPRV o'chadi */
    c->band_bor = 0;
    c->pc = c->sepc;
}
```

Bu funksiyani diqqat bilan o'qing — E6 uning ko'zgudagi aksi. `mret` — yagona yo'l M rejimdan pastga
tushishning: firmware yadroni ishga tushirishda ham `MPP = S`, `mepc = yadro manzili` qilib, `mret`
bajaradi (10-bob). Ya'ni "qaytish" buyrug'i aslida "istalgan rejimga, istalgan manzilga **o'tish**"
buyrug'i — faqat yuqori rejim uni sozlay oladi.

## 6.5. Delegatsiya — nega har trap firmware'ga bormaydi

Sukut bo'yicha **har** trap M ga boradi. Lekin dasturning sahifa xatosini yoki tizim chaqiruvini **yadro**
hal qilishi kerak. Ularni M orqali o'tkazish — sekin va noqulay (har `read()` uchun ikki trap). Shuning uchun
M rejim ikki registr bilan "bu turlarni to'g'ridan-to'g'ri S ga yubor" deydi:

- `medeleg` — istisnolar: `n`-bit = 1 → sabab `n` ni S ga;
- `mideleg` — uzilishlar: `n`-bit = 1 → uzilish `n` ni S ga.

**Qoida:** trap S ga boradi, agar (a) u delegatsiya qilingan **va** (b) protsessor hozir **S yoki U** da
bo'lsa. M rejimdagi trap **hech qachon** pastga tushmaydi — aks holda yadro firmware'ni "tutib olishi"
mumkin bo'lardi.

```text
                    medeleg[8] = 1                medeleg[9] = 0
  U: ecall  ----------------------->  S        S: ecall  --------->  M (SBI, 10-bob)
  U: sahifa xatosi (13) ----------->  S        M: noto'g'ri buyruq --->  M (doim)
```

Bizning firmware'ning sozlamasi ([firmware/asosiy.c](../firmware/asosiy.c)):

```c
/* yadroga topshiriladigan trap'lar (medeleg): sahifa xatolari, tekislik, noto'g'ri buyruq, breakpoint,
   U rejimdan ecall — bularning hammasini Linux O'ZI hal qiladi. S rejimdan ecall (9) — SBI, M da qoladi. */
#define DELEG_ISTISNOLAR ((1u << 0) | (1u << 1) | (1u << 2) | (1u << 3) | (1u << 4) | (1u << 5) | (1u << 6) | \
                          (1u << 7) | (1u << 8) | (1u << 12) | (1u << 13) | (1u << 15))
#define DELEG_UZILISHLAR ((1u << 1) | (1u << 5) | (1u << 9))   /* SSI, STI, SEI */
```

## 6.6. E6 — `trap_kirish`

Endi siz 6.3 va 6.5 ni kodga aylantirasiz. Funksiya imzosi:

```c
void trap_kirish(struct cpu *c, uint32_t sabab, uint32_t tval);
```

Reja (izohda ham bor):

1. `raqam = sabab & ~UZILISH_BITI`; uzilish bo'lsa `mideleg`, aks holda `medeleg` ni oling.
2. `lr/sc` band qilishini bekor qiling (`band_bor = 0`) — 4.6.
3. Rejim `<= S` **va** delegatsiya biti 1 → S yo'li; aks holda M yo'li.
4. Har yo'lda 6.3 ning 1–6-qadamlari. Yordamchi `vektor(tvec, sabab)` tayyor.

Bit "ko'chirish" (`SPIE ← SIE`) uchun 1.4 dagi jadvalning oxirgi qatoriga qarang. `MPP` — **2 bitli** maydon:
avval o'chirib (`& ~MSTATUS_MPP`), keyin yangi qiymatni `<< MSTATUS_MPP_SILJISH` bilan yozing.

**Tekshirish:** test 5 ta stsenariyni tekshiradi (U→S delegatsiya bilan, S→M delegatsiyasiz, M da
delegatsiya e'tiborsiz, vektorli M taymer, S taymer U dan):

```console
$ make test 2>&1 | grep -B7 "E6 "
        c->mstatus & (MSTATUS_SIE | MSTATUS_SPIE)      SPIE <- SIE(1), SIE <- 0     -> olindi 0x00000002, kutilgan 0x00000020
  [XATO] E6 trap_kirish: delegatsiya (trap.c): 17/18
```

Bu xabar: SIE (`0x2`) o'chmagan, SPIE (`0x20`) yoqilmagan — "ko'chirish" qilinmagan.

## 6.7. Uzilishlar: `mip`, `mie` va qabul qilish qoidasi

Ikki registr, bir xil bit tartibi (bit raqami = uzilish raqami):

- `mip` (pending) — **sodir bo'lgan** uzilishlar. Ko'p bitlari qurilmalardan keladi ([emu/csr.c](../emu/csr.c)):

```c
uint32_t mip_qiymati(struct mashina *m)
{
    struct cpu *c = &m->cpu;
    uint32_t mip = c->mip_dasturiy & MIP_S_BITLAR;              /* SSIP, STIP, SEIP ni M rejim o'zi qo'ya oladi */
    if (m->clint.msip & 1u)
        mip |= MIP_MSIP;
    if (c->instret >= m->clint.mtimecmp)
        mip |= MIP_MTIP;
    if (plic_kutyapti(&m->plic, 0))
        mip |= MIP_MEIP;
    if (plic_kutyapti(&m->plic, 1))
        mip |= MIP_SEIP;
    if ((c->menvcfgh & MENVCFGH_STCE) && c->instret >= c->stimecmp)
        mip |= MIP_STIP;                        /* Sstc yoqilgan: STIP ni stimecmp boshqaradi */
    return mip;
}
```

- `mie` (enable) — **ruxsat etilgan** uzilishlar.

Uzilish **qabul qilinadi**, agar u `mip & mie` da bo'lsa va **global ruxsat** bo'lsa:

| uzilish kimga tegishli | protsessor rejimi | qabul qilinadimi |
|---|---|---|
| M (delegatsiya qilinmagan) | U yoki S | **doim** (pastki rejim yuqorini to'xtata olmaydi) |
| M | M | faqat `mstatus.MIE = 1` |
| S (delegatsiya qilingan) | U | doim |
| S | S | faqat `mstatus.SIE = 1` |
| S | M | **hech qachon** |

Bir nechta bo'lsa — **ustuvorlik**: MEI > MSI > MTI > SEI > SSI > STI.

Ikki nozik joy:

- **"Doim"** qatorlari muhim: Linux `SIE = 0` qilib o'tirgan bo'lsa ham, firmware'ning M taymeri uni
  to'xtatadi. Aks holda yadro firmware'ni "o'chirib" qo'yishi mumkin bo'lardi.
- **`wfi` uyg'onishi**: kutayotgan (`mip & mie`) uzilish bo'lsa protsessor uyg'onadi — hatto global ruxsat
  bo'lmasa ham. Linux'ning idle sikli shunga tayanadi: `SIE = 0` bilan `wfi` qiladi, uyg'ongach uzilishlarni
  yoqadi. Agar uyg'onish ruxsatga bog'liq bo'lsa — abadiy uyqu.

## 6.8. E9 — `uzilish_tekshir`

```c
int uzilish_tekshir(struct mashina *m);    /* 1 — uzilish qabul qilindi (trap_kirish chaqirildi) */
```

Funksiya har `cpu_qadam` boshida chaqiriladi (4.1). Reja izohda: `kutayotgan = mip_qiymati(m) & c->mie` →
uyg'otish → M va S qismlarga bo'lish → ruxsatlar → ustuvorlik tartibida birinchisi uchun
`trap_kirish(c, UZILISH_BITI | irq, 0)`.

**Ishlash tezligi haqida:** bu funksiya **har buyruqda** chaqiriladi (sekundiga ~16 mln marta). Shuning uchun
eng ko'p uchraydigan holatni — "hech narsa kutmayapti" — birinchi tekshirib, darhol `return 0` qiling.

```console
$ make test 2>&1 | grep -E "E9|taymer|plic"
  [ OK ] E9 uzilish_tekshir (trap.c): 9/9
  [ OK ] taymer
  [ OK ] plic
```

## 6.9. Haqiqiy xato hikoyasi: "uzilishlar bo'roni"

Linux birinchi marta deyarli yuklandi — va UART drayveri ishga tushgan zahoti **qotib qoldi**. `-s`
statistikasi: protsessor millionlab buyruq bajaryapti, lekin hammasi bitta joyda — tashqi uzilish
ishlovchisida. Ishlovchi PLIC'dan "kim chaqirdi?" deb so'raydi — PLIC "hech kim" (0) deydi — ishlovchi
qaytadi — **darhol yana uzilish**. Cheksiz.

Sabab — spetsifikatsiyadagi bitta jumla. `mip.SEIP` biti ikki manbadan keladi: dasturiy bit (M rejim yoza
oladi) **OR** PLIC signali. `csrrs`/`csrrc` buyruqlari "o'qi → o'zgartir → yoz" qiladi. Firmware boshqa bitni
o'zgartirish uchun `csrs mip, ...` qilganda, o'qilgan qiymatdagi **PLIC signali** (o'sha paytda 1 edi)
qaytarib **dasturiy bitga** yozildi. Endi SEIP abadiy 1: PLIC signal o'chsa ham, dasturiy bit "yopishib"
qolgan. Spetsifikatsiya bu holat uchun maxsus qoida beradi — `csrrs`/`csrrc` da SEIP uchun faqat dasturiy
bit ishlatiladi:

```c
/* NOZIK QOIDA (spetsifikatsiya, mip.SEIP): ... */
uint32_t asos = eski;
if (raqam == CSR_MIP && tur != 1)
    asos = (eski & ~MIP_SEIP) | (c->mip_dasturiy & MIP_SEIP);
uint32_t yangi = tur == 1 ? manba : tur == 2 ? (asos | manba) : (asos & ~manba);
```

Bu xatoni topish uchun: `-t` bilan trace olib, uzilishlar orasidagi buyruqlarni o'qish; `mip` ni har
o'zgarishda chop etish; va spetsifikatsiyaning "Machine Interrupt Registers" bo'limini **qatorma-qator**
o'qish. Saboq: **"taxminan to'g'ri" emulyator Linux'ni yuklamaydi**. Har bir jumla muhim.

## Savol-javob

**Savol:** Nega `ecall` da `sepc` = `ecall` ning o'z manzili? Yadro qaytganda yana `ecall` bajarilmaydimi?
**Javob:** Bajariladi — agar yadro `sepc += 4` qilmasa. Linux shunday qiladi (bizning firmware ham:
`k->mepc += 4`, 10-bob). Apparat **hamma** istisnolar uchun bir xil qoidaga amal qiladi (xato buyruq
manzili), dasturiy ta'minot esa qaysilarini qayta bajarish (sahifa xatosi), qaysilarini o'tkazib yuborish
(`ecall`) kerakligini o'zi hal qiladi.

**Savol:** Ichma-ich trap bo'lsa-chi (trap ishlovchisida yana trap)?
**Javob:** `xPIE/xPP` faqat **bitta** qavat saqlaydi. Ikkinchi trap ularni ustidan yozadi. Shuning uchun
ishlovchi (Linux'ning `handle_exception`) boshida `sepc`, `sstatus` ni **stekka** saqlaydi va faqat shundan
keyin uzilishlarni qayta yoqadi. Firmware'ning `trap.S` i ham shunday (10-bob).
