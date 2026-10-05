# 10-bob. Firmware va SBI

> **Bu bobda nima o'rganasiz:** kompyuter yoqilgandan Linux'ning birinchi buyrug'igacha nima bo'ladi;
> C'dan oldingi assembly qadamlari; M rejim trap ishlovchisi va `mscratch` hiylasi; SBI — yadro va firmware
> orasidagi "tizim chaqiruvi" (chaqirish qoidasi, kengaytmalar); taymerni yadroga "uzatish"; yadroni `mret`
> bilan ishga tushirish.
> **Oldindan nima kerak:** 6-bob (trap, delegatsiya, `mret`), 8-bob (CLINT). Darslik 17-bob (assembly,
> chaqirish kelishuvi).
> **Mashqlar:** F1, F2 ([firmware/sbi.c](../firmware/sbi.c)), F3 ([firmware/asosiy.c](../firmware/asosiy.c)).
> **Vaqt:** 4 soat.

## Bu bob nima haqida?

Emulyator — apparat. Endi apparat ustida ishlaydigan **birinchi** dasturni yozamiz: firmware. Haqiqiy
RISC-V kompyuterlarida bu OpenSBI (ko'pincha U-Boot bilan). Bizniki — soddalashtirilgan, lekin Linux uchun
yetarli (~500 qator C + assembly, izohlar bilan). U ikki ish qiladi:

1. **Yuklash:** apparatni sozlaydi va Linux'ni S rejimda ishga tushiradi.
2. **Xizmat:** keyin "orqa fonda" yashaydi. Linux ba'zi narsalarni (taymer, o'chirish, erta konsol) o'zi
   qila olmaydi — firmware'dan `ecall` orqali **so'raydi**. Bu xizmatlar to'plami — SBI (Supervisor Binary
   Interface).

**Hayotdan misol: bino qo'riqchisi.** Ertalab qo'riqchi (firmware) binoni ochadi, chiroqlarni yoqadi,
signalizatsiyani sozlaydi va kalitni direktorga (yadro) beradi. Kun davomida direktor o'zi ishlaydi, lekin
"bosh elektr shitiga" kirish uchun qo'riqchini chaqiradi. Qo'riqchi kalitni hech kimga bermaydi — u har
doim binoning "egasi".

## 10.1. Yuklanish ketma-ketligi

```text
emulyator: pc = 0x80000000, rejim = M, a0 = 0 (hart), a1 = DTB manzili
   │
   v
boshlash.S:  sp, mscratch, .bss = 0, mtvec = m_trap_kirish
   │
   v
firmware_asosiy(hartid, dtb):  xabar; delegatsiya; mcounteren; mie.MTIE;  MPP = S; mepc = 0x80400000   (F3)
   │  mret
   v
Linux (S rejim): pc = 0x80400000, a0 = hart, a1 = DTB
   │  ... ecall (SBI) ──> m_trap ──> sbi_ecall ──> mret ──> Linux
   │  ... M taymer ─────> m_trap ──> sbi_taymer_uzilishi ──> (STIP) ──> Linux
```

Xotira joylashuvi — [firmware/link.ld](../firmware/link.ld):

```text
0x8000_0000  firmware kodi, ma'lumoti, 16 KB stek
0x8040_0000  Linux Image (emulyatorning -k bayrog'i shu yerga yuklaydi)
...
RAM oxiri - 64 KB   DTB
```

Linux 0x80000000–0x80400000 oralig'iga tegmaydi:

```text
[    0.000000] OF: fdt: Ignoring memory range 0x80000000 - 0x80400000
```

## 10.2. `boshlash.S` — C'dan oldin

Yoqilish paytida **hech narsa** tayyor emas: stek yo'q, global o'zgaruvchilar nollanmagan. C funksiyani
chaqirishdan oldin minimal muhit assembly'da ([firmware/boshlash.S](../firmware/boshlash.S)):

```asm
_start:
    la sp, __stek_tepasi            /* 1) stek (pastga o'sadi) */
    csrw mscratch, sp               /* 2) trap uchun stek manzili (10.3) */

    la t0, __bss_boshi              /* 3) .bss = 0: C standarti global o'zgaruvchilar boshida 0 */
    la t1, __bss_oxiri
1:  bgeu t0, t1, 2f
    sw zero, 0(t0)
    addi t0, t0, 4
    j 1b
2:
    la t0, m_trap_kirish            /* 4) trap ishlovchisi */
    csrw mtvec, t0
    call firmware_asosiy            /* 5) a0 = hartid, a1 = DTB manzili — tegilmagan */
```

`__bss_boshi`, `__stek_tepasi` — bog'lovchi skriptda e'lon qilingan **belgilar** (darslik 22-bob). Ular
xotiraga joy egallamaydi — faqat manzil.

## 10.3. M trap kirishi va `mscratch` hiylasi

Trap istalgan payt keladi — Linux o'rtasida, har qanday registr qiymati bilan. Ishlovchi **hech bir**
registrni buzmasligi kerak. Lekin registrlarni saqlash uchun xotira manzili kerak, manzil uchun esa —
registr! "Tovuq va tuxum" muammosi.

Yechim — `mscratch` CSR: u oldindan firmware stekiga ishora qiladi. `csrrw sp, mscratch, sp` **bitta**
buyruqda `sp` va `mscratch` ni almashtiradi:

```asm
m_trap_kirish:
    csrrw sp, mscratch, sp              /* sp <-> mscratch: sp = firmware steki, mscratch = Linux'ning sp si */
    addi sp, sp, -36*4                  /* 33 so'z kerak, 16 baytga tekislash uchun 36 */
    sw ra, 1*4(sp)
    sw gp, 3*4(sp)
    ...                                 /* qolgan 29 registr */
    csrr t0, mscratch                   /* Linux'ning sp si */
    sw t0, 2*4(sp)
    csrr t0, mepc
    sw t0, 32*4(sp)
    addi t0, sp, 36*4                   /* mscratch ni yana firmware stek TEPASIGA qaytaramiz (keyingi trap uchun) */
    csrw mscratch, t0

    mv a0, sp                           /* m_trap(struct kadr *k) */
    call m_trap
    ...                                 /* teskari tartibda tiklash; oxirida sp */
    lw sp, 2*4(sp)
    mret
```

Stekdagi blok C'da `struct kadr` ([firmware/fw.h](../firmware/fw.h)) — xuddi Linux'ning `struct pt_regs` i:

```c
struct kadr {
    uint32_t x[32];
    uint32_t mepc;
};
```

C ishlovchi kadrni **o'zgartira oladi**: `k->x[A0] = natija` — tiklashda `a0` ga shu yoziladi va Linux
natijani ko'radi. Bu — har qanday yadrodagi tizim chaqiruvi natijasini qaytarish mexanizmi.

## 10.4. SBI chaqirish qoidasi

| registr | ma'nosi |
|---|---|
| `a7` | EID — kengaytma raqami |
| `a6` | FID — shu kengaytmadagi funksiya |
| `a0..a5` | argumentlar |
| → `a0` | xato kodi (0 — muvaffaqiyat, manfiy — xato) |
| → `a1` | qiymat |

EID'lar ko'pincha **ASCII matn** — "bitlar bilan gaplashish"ning yana bir misoli:

```c
#define EID_TIME 0x54494D45u                    /* "TIME" */
#define EID_SRST 0x53525354u                    /* "SRST" */
#define EID_DBCN 0x4442434Eu                    /* "DBCN" — debug konsol */
```

`0x54 = 'T'`, `0x49 = 'I'`, `0x4D = 'M'`, `0x45 = 'E'`. Bizning firmware ID — `0x564B` = "VK".

Linux yuklanganda birinchi qilgan ishi — firmware'ni **so'roq qilish** (BASE kengaytmasi: versiya, ID,
"bu kengaytma bormi?"):

```text
[    0.000000] SBI specification v2.0 detected
[    0.000000] SBI implementation ID=0x564b Version=0x1
[    0.000000] SBI TIME extension detected
[    0.000000] SBI IPI extension detected
[    0.000000] SBI RFENCE extension detected
[    0.000000] SBI SRST extension detected
[    0.000000] earlycon: sbi0 at I/O port 0x0 (options '')
```

`earlycon: sbi0` — UART drayveri hali yuklanmagan, lekin Linux allaqachon **SBI orqali** gapira oladi (eski
`putchar` chaqiruvi, EID 1). Shuning uchun boot log birinchi millisekunddan ko'rinadi.

Dispetcher — bitta `switch` ([firmware/sbi.c](../firmware/sbi.c)):

```c
void sbi_ecall(struct kadr *k)
{
    uint32_t eid = k->x[A7], fid = k->x[A6];
    k->mepc += 4;                               /* ecall dan KEYINGI buyruqqa qaytamiz (aks holda abadiy ecall) */

    switch (eid) {
    case EID_LEGACY_SET_TIMER:
        taymer_qoy(k->x[A0], k->x[A1]);         /* RV32: 64 bitli vaqt ikki registrda */
        ...
```

Bitta yadroli kompyuterda IPI ("boshqa yadroga uzilish"), RFENCE ("boshqa yadrolarda TLB tozala"), HSM
("yadroni yoq/o'chir") deyarli bo'sh — lekin ular **bo'lishi kerak**: Linux ularni so'raydi va javob kutadi.

## 10.5. Taymer — eng muhim xizmat (F1, F2)

Linux'ga taymer kerak: jarayonlarni almashtirish (darslik 23-bob), `sleep`, vaqt hisobi. Lekin `mtimecmp` —
**M rejimning** qurilmasi; S rejim unga yoza olmaydi. Yechim — firmware "vositachi":

```text
Linux: sbi_set_timer(t)  ──ecall──>  firmware: mtimecmp <- t;  mip.STIP <- 0;  mie.MTIE <- 1      (F1)
                                          ...
                         vaqt keldi: M taymer uzilishi (MTI) ──> firmware: mip.STIP <- 1;  mie.MTIE <- 0  (F2)
Linux: S taymer uzilishi (STI) <──────────── mret
```

Linux o'zini **o'z taymeriga ega** deb his qiladi: u STI oladi, aslida esa bu M taymerning "aksi".

Nima uchun F2 da `MTIE ← 0`? `mtime >= mtimecmp` holati Linux yangi vaqt qo'ymaguncha **saqlanadi** (daraja
bo'yicha signal, 8.3). MTIE o'chirilmasa — M taymer uzilishi darhol qayta keladi, firmware cheksiz aylanadi va
Linux hech qachon ishlay olmaydi.

Nima uchun F1 da `STIP ← 0`? Linux yangi vaqt qo'ydi — eski "vaqt bo'ldi" signali endi noto'g'ri. U
o'chirilmasa, Linux darhol yana (soxta) taymer uzilishi oladi.

### F1 ning nozik joyi: 64 bitni 32 bitli yozuvlar bilan

`mtimecmp` — 64 bit; protsessor 32 bit yozadi. Eski qiymat `0x00000001_FFFFFFFF`, yangisi `0x00000002_00000010`.
Agar "past yarmi, keyin yuqori" tartibida yozsak:

```text
1) past <- 0x00000010:   mtimecmp = 0x00000001_00000010   <- KICHIK qiymat! mtime undan katta bo'lsa — soxta uzilish
2) yuqori <- 0x00000002: mtimecmp = 0x00000002_00000010
```

Oraliq qiymat soxta uzilish keltirib chiqarishi mumkin. Spetsifikatsiyadagi xavfsiz ketma-ketlik (izohda ham):
avval past yarmini **maksimal** qilamiz (shunda oraliq qiymat doim katta), keyin yuqori, keyin haqiqiy past.

### Sstc — firmware'siz taymer

Yangi protsessorlarda **Sstc** kengaytmasi bor: S rejimning o'z `stimecmp` registri — SBI chaqiruvi va ikki
trap kerak emas. Emulyatorimiz uni qo'llaydi (`menvcfgh.STCE`, 6.7 dagi `mip_qiymati`), lekin bizning Linux
sozlamasi undan foydalanmaydi — firmware yo'li o'quv uchun qiziqroq. `-S` rejimidagi assembly testlari
(`taymer.S`) Sstc'ni sinaydi.

## 10.6. F3 — yadroni ishga tushirish

`firmware_asosiy` ning markaziy qismi siz uchun:

1. **Delegatsiya:** `medeleg`, `mideleg` (6.5 dagi `DELEG_*` konstantalari tayyor).
2. **`mcounteren = 7`** — S rejim `cycle`, `time`, `instret` ni o'qiy olsin. Aks holda Linux'ning birinchi
   `rdtime` buyrug'i — noto'g'ri buyruq istisnosi (6.2 — hisoblagich ruxsati).
3. **`mie = MTIE`** — M taymer (F1/F2 uchun).
4. **`mret` ga tayyorgarlik:** `mstatus.MPP ← S` (2 bitli maydon — maska bilan!), `mepc ← YADRO_MANZIL`.

`mret` ning o'zi va `a0`/`a1` — tayyor:

```c
register uint32_t a0 __asm__("a0") = hartid;
register uint32_t a1 __asm__("a1") = dtb;
__asm__ volatile("mret" ::"r"(a0), "r"(a1));
```

`register ... __asm__("a0")` — GCC/Clang kengaytmasi: "bu o'zgaruvchi aynan a0 registrida bo'lsin". Inline
assembly'ga `"r"(a0)` bilan beramiz — kompilyator `mret` dan oldin registrlarni to'g'ri to'ldiradi.

## 10.7. Tekshirish

SBI testi — kichik "yadro" ([testlar/sbi/sbi_test.S](../testlar/sbi/sbi_test.S)) S rejimda firmware'ga SBI
chaqiruvlari qiladi va javoblarni tekshiradi: BASE versiyasi, probe, konsol, `rdtime`, `set_timer` → aynan
**bitta** STI, SRST:

```console
$ ./build/vk -n 20000000 build/firmware.elf build/sbi_test.elf

[vk-sbi] Virtual kompyuter firmware'i (SBI v2.0), M rejim
[vk-sbi] hart 0, DTB 0x83ff0000, yadro 0x80400000
[vk-sbi] yadroga o'tyapman (mret -> S rejim)

SBI testi: konsol ishlayapti
SBI testlari: OK

[vk-sbi] yadro tizimni o'chirishni so'radi
```

(Ikkinchi ELF — `0x80400000` ga yuklangan "yadro". `make test` buni avtomatik bajaradi.)

F3 dagi xatolar turlicha ko'rinadi. Ikki haqiqiy misol (muallif ataylab buzib ko'rgan):

**`mepc` yozilmagan** — firmware yadroga emas, 0-manzilga "qaytadi". Emulyator hech narsa chiqarmay,
`-n` chegarasigacha aylanadi. `-t` bilan sabab darhol ko'rinadi:

```console
$ ./build/vk -t -n 5000 -k build/Image build/firmware.elf 2>&1 | grep -A3 'mret$'
[M] 800006fa: 30200073  mret
    ~~ istisno 1 (tval=0x00000000) pc=0x00000000 [S] -> pc=0x00000000 [S]
    ~~ istisno 1 (tval=0x00000000) pc=0x00000000 [S] -> pc=0x00000000 [S]
    ~~ istisno 1 (tval=0x00000000) pc=0x00000000 [S] -> pc=0x00000000 [S]
```

`mret` dan keyin protsessor S rejimda, `pc = 0`. U yerda xotira yo'q — istisno 1 (buyruq o'qishda kirish
xatosi). Bu istisno delegatsiya qilingan, shuning uchun S ga boradi — `stvec` ga. Lekin Linux hali `stvec`
ni yozmagan (u ham 0) — yana `pc = 0`, yana istisno... Cheksiz. `~~` qatorlari — emulyatorning trap
izi: `istisno N`, qayerdan (`pc`, rejim) va qayerga.

**`mcounteren` unutilgan** — Linux yuklanadi, lekin taymer sozlanayotganda:

```text
[    0.000000] Oops - illegal instruction [#1]
[    0.000000] CPU: 0 PID: 0 Comm: swapper Not tainted 6.6.50 #7
[    0.000000] Hardware name: virtual-kompyuter,rv32 (DT)
[    0.000000] epc : riscv_sched_clock+0x0/0x12
[    0.000000]  ra : sched_clock_register+0xe2/0x236
...
[    0.000000] Kernel panic - not syncing: Fatal exception in interrupt
```

Bu safar xatoni **Linux** ushladi (noto'g'ri buyruq delegatsiya qilingan — 6.5). `epc : riscv_sched_clock+0x0`
— funksiyaning birinchi buyrug'i: `rdtime` (vaqtni o'qish). S rejimga `time` hisoblagichini o'qishga ruxsat
berilmagan (6.2) — noto'g'ri buyruq. `+0x0/0x12` — "funksiya boshidan 0 bayt, funksiya uzunligi 0x12 bayt":
Linux bu nomni `CONFIG_KALLSYMS` (11-bob) tufayli biladi. Bu ma'lumotlar bilan xatoni topish — bir daqiqalik ish.

## Savol-javob

**Savol:** Firmware ham trap'da registrlarni saqlaydi, Linux ham. Bu ikki marta ish emasmi?
**Javob:** Ha — va shuning uchun SBI chaqiruvi "qimmat" (yuzlab buyruq). Shu sababli Sstc kabi kengaytmalar
paydo bo'ldi: tez-tez kerak bo'ladigan xizmatlarni apparatga qaytarish. Muhandislik — doim muvozanat:
moslashuvchanlik (dasturiy) vs tezlik (apparat).

**Savol:** Haqiqiy OpenSBI qancha katta?
**Javob:** Taxminan 50 000 qator: ko'p yadro (HSM, IPI), ko'p platforma, PMP, tekis bo'lmagan murojaatni
dasturiy bajarish, gipervizor qo'llovi... OpenSBI manbasini o'qib chiqish
(`lib/sbi/sbi_ecall*.c`, `sbi_timer.c`) — bu bobdan keyingi tabiiy qadam. Bizniki uning yuragi, xolos.
