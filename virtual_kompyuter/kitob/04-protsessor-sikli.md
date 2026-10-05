# 4-bob. Protsessor sikli: olish, ajratish, bajarish — va atomik amallar

> **Bu bobda nima o'rganasiz:** `cpu_qadam()` — emulyatorning yuragi; istisno bo'lgan buyruq nega holatni
> o'zgartirmasligi kerak; fetch'ning ikki sahifa chegarasidagi nozikligi; `wfi` va "vaqtni surish"; A
> kengaytmasi — `lr.w`/`sc.w` va `amo*` buyruqlari, qulf (spinlock) va hisoblagichlar qanday quriladi.
> **Oldindan nima kerak:** 2, 3-boblar. Darslik 15 va 26-boblar (parallellik) — A kengaytmasi uchun.
> **Mashq:** E10 ([emu/cpu.c](../emu/cpu.c), `atomik()` ichida).   **Vaqt:** 3 soat.

## Bu bob nima haqida?

Oldingi boblarda "qismlar"ni yasadik: dekoder, ALU. Endi ularni **bitta siklga** ulaymiz. Har qanday
protsessor — eng oddiy mikrokontrollerdan serverdagi 128 yadroli gigantgacha — mantiqan aynan shu siklni
bajaradi:

```text
          +------------------------------------------------------+
          v                                                      |
   uzilish bormi? --ha--> trap (6-bob) --------------------------+
          | yo'q                                                 |
   FETCH: pc dan buyruqni o'qi (MMU orqali, 7-bob)               |
          |                                                      |
   DECODE: 16 bitli bo'lsa -> 32 bitliga (5-bob); maydonlar      |
          |                                                      |
   EXECUTE: opcode bo'yicha bajar --istisno--> trap -------------+
          | OK                                                   |
   pc <- keyingi_pc;  instret++  ---------------------------------+
```

**Hayotdan misol: oshpaz va retsept.** Oshpaz retseptning **joriy qatorini** o'qiydi (fetch), nima qilish
kerakligini tushunadi (decode), bajaradi (execute), keyingi qatorga o'tadi. Ba'zi qatorlar "5-qadamga qayt"
deydi (sakrash). Telefon jiringlasa (uzilish) — oshpaz qayerda to'xtaganini eslab qoladi, javob beradi, qaytadi.
Agar qatorda "qozonga 3 kg tuz" deyilsa (xato) — oshpaz bajarmaydi, bosh oshpazni chaqiradi (istisno).

## 4.1. `cpu_qadam()` — to'liq

[emu/cpu.c](../emu/cpu.c) dagi funksiya — siklning **bitta aylanishi**. `main.c` uni `toxtadi` bayrog'i
qo'yilguncha qayta-qayta chaqiradi. Asosiy qismi:

```c
void cpu_qadam(struct mashina *m)
{
    struct cpu *c = &m->cpu;

    /* 0) qurilmalarning uzilish signallari PLIC ga */
    qurilmalar_yangila(m);

    /* 1) Uzilish kutayaptimi? Qabul qilinsa — pc trap ishlovchisiga o'tdi, bu qadam tugadi. */
    if (uzilish_tekshir(m))
        return;

    /* (wfi holati — 4.4 da) */

    /* 2) FETCH */
    uint32_t b, uz, tval;
    if (!sakrash_tekis(c->pc)) {
        trap_kirish(c, SABAB_BUYRUQ_TEKIS_EMAS, c->pc);
        return;
    }
    int xato = olish(m, c->pc, &b, &uz, &tval);
    if (xato) {
        trap_kirish(c, (uint32_t)xato, tval);
        return;
    }

    /* 3) DECODE: siqilgan buyruq bo'lsa — 32 bitliga aylantiramiz */
    uint32_t b32 = b;
    if (uz == 2) {
        b32 = c_kengaytir((uint16_t)b);
        if (b32 == 0) {
            trap_kirish(c, SABAB_NOTOGRI_BUYRUQ, b);    /* mtval/stval = asl 16 bitli buyruq */
            return;
        }
    }

    /* 4) EXECUTE */
    uint32_t keyingi_pc = c->pc + uz;
    struct natija n = bajar(m, b32, uz, &keyingi_pc);
    if (n.bor) {
        if (n.sabab == SABAB_NOTOGRI_BUYRUQ)
            n.tval = b;                         /* noto'g'ri buyruqda tval = ASL buyruq (16 yoki 32 bit) */
        trap_kirish(c, n.sabab, n.tval);        /* pc HALI o'zgarmagan: mepc/sepc = aynan shu buyruq */
        return;
    }

    /* 5) pc ni yangilash va hisoblagich */
    c->pc = keyingi_pc;
    c->x[0] = 0;
    c->instret++;
}
```

## 4.2. Oltin qoida: istisno — "hech narsa bo'lmagandek"

`bajar()` registr yoki xotirani o'zgartirishdan **oldin** hamma tekshiruvlarni qiladi. Agar `lw` sahifa
xatosiga uchrasa — `rd` o'zgarmaydi, `pc` o'zgarmaydi. Nega bu shunchalik muhim?

Linux'da `malloc(1 GB)` darhol xotira bermaydi (darslik 24-bob, "demand paging"). Dastur birinchi marta
yozganda — sahifa xatosi. Yadro sahifani ajratadi, jadvalga yozadi va **aynan shu buyruqqa** qaytadi.
Buyruq ikkinchi marta bajariladi — endi muvaffaqiyatli. Agar birinchi urinish yarim-yarti bajarilgan bo'lsa
(masalan `jalr` `rd` ga qaytish manzilini yozib qo'ygan bo'lsa), ikkinchi urinish **boshqa natija** beradi.
Dastur buni hech qachon sezmasligi kerak.

Shuning uchun `struct natija`: `bajar()` istisnoni **qaytaradi**, o'zi trap'ga kirmaydi:

```c
struct natija {
    int bor;                                    /* 1 — istisno */
    uint32_t sabab, tval;
};
```

Nega shunchaki `int` qaytarmaymiz? Chunki sabab **0** bo'lishi mumkin: `SABAB_BUYRUQ_TEKIS_EMAS = 0`. "0 —
muvaffaqiyat" kelishuvi bu yerda ishlamaydi. Kichik, lekin haqiqiy dizayn muammosi — va uning to'g'ri yechimi.

`jalr` dagi nozik joyni ko'ring:

```c
case 0x67: {                                /* JALR: pc <- (rs1 + imm) & ~1 */
    if (f3 != 0)
        return istisno(SABAB_NOTOGRI_BUYRUQ, b);
    uint32_t manzil = (a + imm_i(b)) & ~1u; /* a ni OLDINDAN o'qidik: rd == rs1 bo'lsa ham to'g'ri */
    if (!sakrash_tekis(manzil))
        return istisno(SABAB_BUYRUQ_TEKIS_EMAS, manzil);
    rd_yoz(c, rd, c->pc + uz);
    *keyingi_pc = manzil;
    return OK;
}
```

`jalr ra, 0(ra)` — `rd` va `rs1` bir xil. Agar avval `rd` ga yozib, keyin `rs1` ni o'qisak — eski manzil
yo'qoladi. `a` (rs1 qiymati) funksiya boshida **bir marta** o'qilgan, shuning uchun tartib xavfsiz.

## 4.3. FETCH: 16 + 16

C kengaytmasi bor (5-bob): buyruq 2 yoki 4 bayt. Uzunlikni birinchi 16 bitning past 2 biti aytadi.
`olish()` shuning uchun **ikki qadamda** o'qiydi:

```c
int xato = mmu_tarjima(m, pc, KIRISH_BAJARISH, &fiz);
...
if ((past & 3u) != 3u) {          /* 16 bitli */
    *buyruq = past;
    *uz = 2;
    return 0;
}
*tval = pc + 2;
xato = mmu_tarjima(m, pc + 2, KIRISH_BAJARISH, &fiz);    /* ALOHIDA tarjima! */
```

Nega ikkinchi yarmi uchun alohida tarjima? 32 bitli buyruq `0x...FFE` manzilda boshlanishi mumkin: birinchi
yarmi bir sahifada, ikkinchisi — **keyingi** sahifada, u esa butunlay boshqa fizik joyda (yoki umuman yo'q —
sahifa xatosi). Bunday holatda `tval = pc + 2` — xato aynan qaysi manzilda ekanini yadroga aytadi. Bu holat
haqiqiy Linux'da uchraydi va aynan shunday "kichik" narsalar Linux'ni yuklashda qotib qolishga sabab bo'ladi.

## 4.4. `wfi` — kutish va vaqtni "surish"

`wfi` (wait for interrupt) — "uzilish kelguncha uxla". Linux hech ish bo'lmaganda (idle) shuni bajaradi.
Haqiqiy protsessor quvvat tejash uchun to'xtaydi. Emulyatorda biz **vaqtni surib yuboramiz**: bizda vaqt =
bajarilgan buyruqlar soni (`instret`, 8-bob). Keyingi taymer hodisasi qachon? O'sha momentga sakraymiz:

```c
if (c->kutmoqda) {
    uint64_t keyingi = UINT64_MAX;
    if ((c->mie & MIP_MTIP) && m->clint.mtimecmp > c->instret)
        keyingi = m->clint.mtimecmp;
    if ((c->mie & MIP_STIP) && (c->menvcfgh & MENVCFGH_STCE) && c->stimecmp > c->instret && c->stimecmp < keyingi)
        keyingi = c->stimecmp;
    if (keyingi != UINT64_MAX) {
        ...
        c->instret = keyingi;
        return;
    }
    ...
    fprintf(stderr, "\nemulyator: protsessor 'wfi' da uyg'onmaydigan holda uxladi (pc=0x%08x)\n", c->pc);
```

Natija: Linux 5 soniya "yashaydi" (log vaqtlari), lekin emulyator 3–4 soniyada tugatadi — bo'sh vaqt
o'tkazib yuboriladi. Boot log'dagi bu sakrashni ko'ring:

```text
[    0.228182] workingset: timestamp_bits=30 max_order=14 bucket_order=0
[    2.938836] Serial: 8250/16550 driver, 1 ports, IRQ sharing disabled
```

2.7 "soniya" Linux kutdi (drayverlar ro'yxatdan o'tishini) — emulyator uchun bu bir necha `wfi` sakrashi.

Agar uyg'otadigan hech narsa bo'lmasa (taymer yoqilmagan, klaviatura yo'q) — bu **o'lik qulf**: emulyator
xabar berib to'xtaydi. Haqiqiy kompyuter abadiy qotib qolgan bo'lardi — emulyatorning xabari esa
debugging'da juda qimmatli.

### Trace'da trap'lar

`-t` rejimida trap'lar ham chop etiladi (`trap_qil()` — `trap_kirish` atrofidagi kichik o'ram):

```text
[U] 80000100: 00000073  ecall
    ~~ istisno 8 (tval=0x00000000) pc=0x80000100 [U] -> pc=0x80000104 [S]
```

Bu juda muhim: buyruq **o'qilmasa** (masalan `pc` mavjud bo'lmagan manzilda), chop etiladigan buyruq yo'q — trap
izisiz trace "jim" qoladi va siz nima bo'layotganini ko'rmaysiz (10-bobda shunday misol bor).

## 4.5. `instret`, `-n` va deterministiklik

Har muvaffaqiyatli buyruq `instret++`. Bu hisoblagich bir vaqtning o'zida: `instret` CSR (bajarilgan
buyruqlar), `cycle` CSR (takt), `time` va CLINT `mtime` (vaqt). Natijada emulyator **deterministik**: bir xil
kirish — bit-bit bir xil natija, har safar. Linux testi `pid 16` ni kutishi shunga tayanadi.

`-n N` bayrog'i — `cpu_qadam` chaqiruvlari soni bo'yicha chegara (instret emas!). Nega? Chunki trap'ga
cheksiz tushib turgan protsessor (`trap → ishlovchi yo'q → trap → ...`) `instret` ni oshirmaydi —
chegara bunday holatni ham ushlashi kerak. (Bu xato muallifning o'z tajribasidan: avval `-n` instret bo'yicha
edi va buzilgan test hech qachon to'xtamasdi.)

## 4.6. A kengaytmasi: atomik amallar

Ikki yadro (yoki bitta yadroda ikki oqim + uzilish) bitta hisoblagichni oshirmoqchi: `h = h + 1`. Bu uchta
buyruq: o'qi, qo'sh, yoz. Ular orasida boshqasi aralashsa — bitta oshirish **yo'qoladi** (darslik 15-bob,
"poyga holati"). Yechim — **bo'linmas** buyruqlar.

### `amo*` — o'qi, hisobla, yoz: bitta buyruqda

`amoadd.w rd, rs2, (rs1)`: `rd ← xotira[rs1]; xotira[rs1] ← xotira[rs1] + rs2` — bitta, bo'linmas qadamda.
Clang `__atomic_fetch_add` ni aynan shunga aylantiradi:

```console
$ clang --target=riscv32 -march=rv32ima -O2 -S qulf.c -o -
hisob:                                  # __atomic_fetch_add(h, 1, __ATOMIC_RELAXED)
	li	a1, 1
	amoadd.w	a0, a1, (a0)
	ret
qulf_ol:                                # while (__atomic_exchange_n(q, 1, __ATOMIC_ACQUIRE)) ;
	li	a1, 1
.LBB0_1:
	amoswap.w.aq	a2, a1, (a0)
	bnez	a2, .LBB0_1
	ret
```

`qulf_ol` — eng oddiy **spinlock**: "qulfga 1 yoz va eski qiymatni ol; eski 1 bo'lsa (band edi) — qaytadan".
Linux yadrosining minglab joyi shunga tayanadi.

### `lr.w` / `sc.w` — "band qilib o'qi, o'zgarmagan bo'lsa yoz"

Murakkabroq amallar uchun (masalan "qiymat X bo'lsa Y ga almashtir" — compare-and-swap):

```console
almashtir:                              # __atomic_compare_exchange_n(p, &eski, yangi, ...)
.LBB3_1:
	lr.w.aqrl	a3, (a0)              # o'qi va manzilni "band qil"
	bne	a3, a1, .LBB3_3           # eski emas — chiq
	sc.w.rl	a4, a2, (a0)              # band hali kuchdami? -> yoz, a4 = 0; aks holda a4 = 1
	bnez	a4, .LBB3_1               # muvaffaqiyatsiz — qaytadan
.LBB3_3:
```

`sc.w` faqat oradagi vaqtda **hech kim** bu manzilga tegmagan bo'lsa yozadi. Emulyatorda "band qilish" —
ikki maydon: `band_bor`, `band_manzil`. **Trap band qilishni bekor qiladi** (`trap_kirish` va `mret`/`sret`
da `band_bor = 0`) — chunki trap ishlovchisi ham shu manzilga yozgan bo'lishi mumkin. `lr`/`sc` qismi
[emu/cpu.c](../emu/cpu.c) dagi `atomik()` funksiyasida tayyor — o'qing.

### `aq` va `rl` bitlari

`.aq` (acquire) va `.rl` (release) — xotira tartibi haqidagi ko'rsatmalar (darslik 26-bob): "bu amaldan
keyingi o'qishlar undan oldin bajarilmasin" va hokazo. Ko'p yadroli, buyruqlarni "aralashtirib" bajaradigan
protsessorda bu juda muhim. Bizning emulyator bitta yadroli va buyruqlarni **qat'iy tartibda** bajaradi —
shuning uchun bu bitlarni (va `fence` ni) e'tiborsiz qoldirsak ham to'g'ri.

## 4.7. E10 — AMO amallari

`atomik()` ning oxirida, xotiradan `eski` o'qilgandan keyin, `yangi` ni hisoblash bo'sh qoldirilgan:

```c
/* AMO: avval YOZISH ruxsatini tekshiramiz (o'qish ham, yozish ham kerak) — aks holda yarim bajarilib qoladi */
uint32_t fiz, eski;
int xato = mmu_tarjima(m, manzil, KIRISH_YOZISH, &fiz);
...
uint32_t yangi;
/* TODO(E10) ... */
if (shina_yoz(m, fiz, 4, yangi) != 0)
    return istisno(SABAB_YOZISH_KIRISH, manzil);
rd_yoz(c, rd, eski);
```

`funct5` (buyruqning 31..27-bitlari) bo'yicha:

| funct5 | buyruq | yangi |
|---|---|---|
| 0x00 | amoadd.w | eski + manba |
| 0x01 | amoswap.w | manba |
| 0x04 | amoxor.w | eski ^ manba |
| 0x08 | amoor.w | eski \| manba |
| 0x0C | amoand.w | eski & manba |
| 0x10 | amomin.w | min (ishorali) |
| 0x14 | amomax.w | max (ishorali) |
| 0x18 | amominu.w | min (ishorasiz) |
| 0x1C | amomaxu.w | max (ishorasiz) |

(0x02 — lr, 0x03 — sc: ular yuqorida alohida ishlangan.) Boshqa funct5 — noto'g'ri buyruq.

Nega tarjima `KIRISH_YOZISH` bilan? AMO o'qiydi **va** yozadi, shuning uchun ruxsatni **bir marta**, yozish
uchun tekshiramiz (yozish ruxsati bo'lsa, o'qish ham bor). Spetsifikatsiya: AMO uchun sahifa xatosi har doim
**yozish** xatosi (sabab 15, "store/AMO page fault"). Bu yadro uchun muhim: `fork` dan keyingi "nusxa-yozishda"
(copy-on-write) sahifalari faqat o'qish uchun belgilangan, va yadro sahifani nusxalash kerakligini aynan
15-sabab bo'yicha taniydi. Agar AMO 13 (load page fault) bersa, yadro "sahifa bor, o'qish mumkin" deb hech
narsa qilmay qaytadi — dastur abadiy shu buyruqda qotadi.

**Tekshirish:**

```console
$ make test 2>&1 | grep -E "E10|atomik"
  [ OK ] E10 AMO amallari (cpu.c): 18/18
  [ OK ] atomik
```

Birlik testi har AMO'ni `cpu_qadam` orqali **haqiqiy buyruq** sifatida bajaradi (masalan `0x80B5252F` =
`amomin.w a0, a1, (a0)`) va xotira hamda `rd` ni tekshiradi.

## Savol-javob

**Savol:** Bitta yadroli emulyatorda atomiklik "o'z-o'zidan" bor. Unda `lr/sc` ni nega bunchalik aniq
modellashtiramiz?
**Javob:** Chunki Linux kodi **apparat qoidalariga** tayanadi. Masalan Linux ichida `lr` va `sc` orasida
taymer uzilishi kelib, kontekst almashsa — `sc` muvaffaqiyatsiz bo'lishi **shart**, aks holda boshqa jarayon
o'zgartirgan qiymat ustidan yozib yuboriladi. Bitta yadroda ham "ikki oqim" bor — uzilish orqali.

**Savol:** Haqiqiy ko'p yadroli protsessorda `amoadd` qanday bo'linmas bo'ladi?
**Javob:** Kesh-kogerentlik protokoli (MESI, darslik 21-bob) orqali: yadro kesh qatorini "eksklyuziv" holatda
oladi, boshqalar unga tegolmaydi, amal bajariladi, keyin qator bo'shatiladi. Yoki amal to'g'ridan-to'g'ri
xotira kontrolleri yonida bajariladi ("far atomics").
