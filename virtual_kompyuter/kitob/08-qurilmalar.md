# 8-bob. Qurilmalar: shina, taymer, uzilishlar kontrolleri, konsol va disk

> **Bu bobda nima o'rganasiz:** MMIO — qurilma bilan "xotira orqali" gaplashish; xotira xaritasi va shina;
> CLINT — taymer va vaqt; 32 bitli protsessorda 64 bitli registrni xavfsiz o'qish/yozish; PLIC — ko'p
> qurilmaning uzilishlarini taqsimlash (claim/complete); 16550 UART — haqiqiy drayver bilan mos konsol;
> DMA disk; deterministik vaqt va "odamdek yozish".
> **Oldindan nima kerak:** 6-bob (uzilishlar). Darslik 16-bob (apparat registrlari), 18-bob.
> **Mashq:** P1 (`eng_ustuvor`, [emu/plic.c](../emu/plic.c)).   **Vaqt:** 4 soat.

## Bu bob nima haqida?

Protsessor yolg'iz hech narsa qilolmaydi: ekran, klaviatura, disk, soat kerak. Bu bobda ularni yasaymiz.
Asosiy g'oya bitta: **protsessor qurilmalar bilan xotira orqali gaplashadi**. Maxsus "qurilma buyrug'i"
yo'q — `sw` va `lw` yetarli. Qaysi manzil RAM, qaysi biri qurilma — buni **shina** hal qiladi.

**Hayotdan misol: ko'p qavatli bino pochta qutilari.** Pochtachi (protsessor) har qutiga xat tashlaydi —
lekin ba'zi "qutilar" aslida trubalar: 5-qavatdagi qutiga tashlangan xat to'g'ridan-to'g'ri **direktor
xonasiga** tushadi (qurilmaga buyruq). Pochtachi farqni bilmaydi — u faqat "manzil va xat"ni biladi.
Binoning trubalar sxemasi (qaysi quti qayerga) — **xotira xaritasi**.

## 8.1. Xotira xaritasi va shina

[emu/qurilmalar.h](../emu/qurilmalar.h):

```text
  0x0010_0000  quvvat (sifive_test)  — 0x5555 yozilsa kompyuter o'chadi
  0x0200_0000  CLINT                 — taymer (mtime, mtimecmp) va dasturiy uzilish (msip)
  0x0C00_0000  PLIC                  — tashqi uzilishlar kontrolleri: qaysi qurilma "chaqirdi"
  0x1000_0000  UART (16550)          — konsol; uzilish raqami (PLIC manbasi) 10
  0x1000_1000  disk                  — oddiy blok qurilma (bizning dizayn); uzilish raqami 1
  0x8000_0000  RAM                   — operativ xotira (sukut 64 MB). Firmware shu yerdan boshlanadi.
```

Manzillar ataylab QEMU'ning `virt` mashinasi bilan bir xil — bu yerda o'rgangan narsa haqiqiy RISC-V
platformalarida ham ishlaydi. `shina_oqi()` — oddiy `if` zanjiri ([emu/shina.c](../emu/shina.c)):

```c
int shina_oqi(struct mashina *m, uint32_t fiz, int hajm, uint32_t *qiymat)
{
    if (ram_ichida(m, fiz, hajm)) { ... RAM dan baytlar ... }
    if (fiz >= UART_MANZIL && fiz < UART_MANZIL + UART_HAJM) {
        *qiymat = uart_oqi(&m->uart, fiz - UART_MANZIL);
        return 0;
    }
    ...
    return -1;                                  /* bu manzilda hech narsa yo'q: "access fault" */
}
```

`-1` — "bu manzilda hech narsa yo'q". Protsessor buni **kirish xatosi** istisnosiga aylantiradi (sabab 1,
5 yoki 7). Linux shu orqali ham qurilma bor-yo'qligini bilishi mumkin.

`ram_ichida` dagi tekshiruvga qarang — bu **xavfsizlik** chegarasi:

```c
static int ram_ichida(const struct mashina *m, uint32_t fiz, int hajm)
{
    return fiz >= RAM_BOSH && fiz - RAM_BOSH <= m->ram_hajm - (uint32_t)hajm;
}
```

Nega `fiz + hajm <= RAM_BOSH + ram_hajm` emas? Chunki `fiz + hajm` **toshib** ketishi mumkin
(`0xFFFFFFFE + 4 = 2`) — va tekshiruv o'tib ketadi! Ayirish varianti toshmaydi. Emulyatordagi bitta shunday
xato — mehmon dastur emulyator xotirasidan tashqariga yoza oladi (haqiqiy "VM escape" zaifliklarining aksariyati
aynan shunday chegaralar xatosi).

### `volatile` — qurilma tomonda

Mehmon dastur (firmware, Linux) qurilma registrini o'qiganda **`volatile`** ishlatadi
([misollar/salom.c](../misollar/salom.c), [firmware/fw.h](../firmware/fw.h)). Sabab: kompilyator uchun
"bir manzildan ikki marta o'qish" = "bir marta o'qish". Lekin UART'ning RBR registridan har o'qish **yangi**
bayt beradi! `volatile` kompilyatorga: "bu xotira o'z-o'zidan o'zgaradi, har murojaatni aynan bajar".

## 8.2. CLINT — vaqt va taymer

```text
+0x0000 msip      (32 bit) — 0-bit: M dasturiy uzilish (boshqa yadroni "uyg'otish" uchun)
+0x4000 mtimecmp  (64 bit) — mtime >= mtimecmp bo'lsa M taymer uzilishi (MTIP)
+0xBFF8 mtime     (64 bit) — doim o'sib boradigan vaqt hisoblagichi
```

**Bizda `mtime` = bajarilgan buyruqlar soni** (`instret`). Haqiqiy kompyuterda u kvarts generatoridan
(masalan 10 MHz) o'sadi. Bizning tanlov emulyatorni **deterministik** qiladi: taymer uzilishi har safar
aynan bir xil buyruqda keladi → Linux har safar bit-bit bir xil yuklanadi → test natijalari barqaror. DTB'da
(9-bob) `timebase-frequency = 10000000` deymiz — Linux "10 mln buyruq = 1 soniya" deb hisoblaydi:

```text
[    0.000022] sched_clock: 64 bits at 10MHz, resolution 100ns, wraps every 4398046511100ns
```

### 32 bitli protsessorda 64 bitli registr

`mtime` 64 bitli, protsessor esa bir buyruqda 32 bit o'qiydi. Ikki o'qish orasida past yarmi `0xFFFFFFFF`
dan `0` ga o'tsa — natija **4 milliard** birlikka xato. Drayverlarning klassik yechimi — "yuqori–past–yuqori":

```c
do {
    hi = mmio_oqi32(MTIME + 4);
    lo = mmio_oqi32(MTIME);
} while (hi != mmio_oqi32(MTIME + 4));    /* yuqori yarmi o'zgardi — qaytadan */
```

**Yozish** ham xavfli — buni 10-bobdagi F1 mashqida o'zingiz hal qilasiz: `mtimecmp` ni ikki qadamda
yozganda, oraliqdagi "aralash" qiymat soxta uzilish keltirib chiqarmasligi kerak.

## 8.3. PLIC — kim chaqirdi?

Ko'p qurilma, bitta protsessor. PLIC — "dispetcher":

1. Har manbaning (qurilmaning) **ustuvorligi** (0 — o'chiq, 1..7).
2. Har **kontekst** (qabul qiluvchi: 0 — M rejim, 1 — S rejim) uchun **yoqilgan** manbalar va **chegara**
   (threshold): faqat chegaradan yuqori ustuvorlikdagilar.
3. Kutayotgan, yoqilgan, chegaradan yuqori manba bo'lsa → protsessorga `MEIP`/`SEIP` (6.7).
4. **CLAIM:** ishlovchi maxsus registrni o'qiydi → eng ustuvor manba raqami (masalan 10 — UART) va u
   "ishlanmoqda" holatiga o'tadi (qayta uzilish bermaydi).
5. Ishlovchi qurilmaga xizmat qiladi (UART'dan baytlarni o'qiydi).
6. **COMPLETE:** shu raqamni o'sha registrga yozadi → manba yana uzilish bera oladi.

Linux'da bu ketma-ketlik `plic_handle_irq()` funksiyasida — bizning PLIC'ni u bilan **o'zgartirishsiz**
ishlatadi ("sifive,plic-1.0.0" — 9-bob):

```text
[    0.000000] plic: plic@c000000: mapped 31 interrupts with 1 handlers for 2 contexts.
```

### Daraja bo'yicha (level-triggered)

Qurilma "simi" yoniq turar ekan — uzilish takrorlanadi. Shuning uchun drayver **sababni yo'q qilishi**
(UART buferini bo'shatishi) kerak, aks holda abadiy uzilish. Emulyatorda har qadam boshida qurilmalar
"simining" holati PLIC'ga beriladi ([emu/clint.c](../emu/clint.c)):

```c
void qurilmalar_yangila(struct mashina *m)
{
    plic_signal(&m->plic, IRQ_UART, uart_uzilish(&m->uart));
    plic_signal(&m->plic, IRQ_DISK, m->disk.uzilish);
}
```

`plic_signal` va claim/complete ([emu/plic.c](../emu/plic.c)) tayyor — o'qing. Sizga eng muhim qismi qoldi:

## 8.4. P1 — `eng_ustuvor`

```c
/* kontekst uchun eng ustuvor, yoqilgan, chegaradan yuqori kutayotgan manba (0 — yo'q) */
static uint32_t eng_ustuvor(const struct plic *p, int k);
```

Bu funksiyadan ikki joy foydalanadi: `plic_kutyapti()` (protsessorga SEIP/MEIP — har buyruqda!) va CLAIM.
Qoidalar: manba `kutmoqda` **va** `yoqilgan[k]` da bo'lishi kerak, ustuvorligi `chegara[k]` dan **qat'iy
katta**; bir nechta bo'lsa — eng katta ustuvorlik; teng bo'lsa — **kichik raqam** yutadi. 0-manba — "hech kim".

```console
$ make test 2>&1 | grep -E "P1|plic"
  [ OK ] P1 PLIC eng_ustuvor (plic.c): 6/6
  [ OK ] plic
```

## 8.5. UART — 16550

Linux'ning `8250` drayveri 1980-yillardagi IBM PC'ning ketma-ket port mikrosxemasi uchun yozilgan va
hali ham **eng ko'p ishlatiladigan** konsol drayveri. Biz uning minimal, lekin drayver bilan mos qismini
yasaymiz:

| siljish | o'qish | yozish |
|---|---|---|
| 0 | RBR — kelgan bayt | THR — yuboriladigan bayt |
| 1 | IER | IER — uzilishlarni yoqish: 0-bit "bayt keldi", 1-bit "yuborishga tayyor" |
| 2 | IIR — qaysi uzilish: 0x04 "bayt keldi", 0x02 "tayyor", 0x01 "yo'q" | FCR (e'tiborsiz) |
| 5 | LSR — 0-bit DR (bayt bor), 5-bit THRE (yuborishga tayyor), 6-bit TEMT | — |

Bizning qurilma **doim** "yuborishga tayyor" (stdout'ga yozish darhol). Kelgan baytlar — `stdin` dan.

### Nozik joy: "tayyor" uzilishi

16550 qoidasi: IER'da TX uzilishi **yoqilgan zahoti** (va bufer bo'sh bo'lsa) "tayyor" uzilishi keladi;
IIR o'qilgach u o'chadi; keyingi bayt yuborilgach yana keladi. Linux shunga tayanadi: chiqarish uchun TX
uzilishini yoqadi va uzilish kelishini **kutadi**. Agar uzilish kelmasa — konsol "qotadi" (matn chiqmaydi):

```c
if (siljish == 1 && (qiymat & UART_IER_TX) && !(u->registr[1] & UART_IER_TX))
    u->tx_uzilish = 1;                      /* TX uzilishi endi yoqildi: darhol "tayyor" */
```

### Terminal: "xom" rejim

Interaktiv ishlaganda (`make linux-ishga`) terminalni `termios` bilan **xom** rejimga o'tkazamiz: har tugma
darhol (Enter kutmasdan) va **aks-sadosiz** keladi — aks-sadoni Linux'ning o'zi chiqaradi, haqiqiy
kompyuterdagidek. Chiqishda terminal asl holatiga qaytariladi (`atexit`). `stdin` ni har buyruqda tekshirish
qimmat (tizim chaqiruvi!) — shuning uchun har 64-murojaatda bir marta `poll(..., 0)`.

### "Odamdek yozish" (`-i`) — va yana bir haqiqiy xato

Testlarda kirish fayldan keladi. Agar hamma baytlarni darhol bersak, ular Linux tayyor bo'lmasdan keladi:
terminal ularni so'rov (prompt) dan **oldin** aks ettiradi va chiqish chalkashadi. Yechim: kirishni
**qatorma-qator**, faqat dastur bir muddat (3 mln buyruq) jim tursa beramiz — xuddi odam ekranni o'qib,
keyin yozgandek.

Lekin birinchi versiyada **birinchi qator yo'qolardi**. Sabab: qator yuklanish paytida, terminal hali
ochilmasdan berilardi — drayver uni o'qib, "hech kim kutmayapti" deb tashlab yuborardi. Tuzatish — drayver
haqiqatan qabul qilishga tayyor ekanini **apparat signalidan** bilish: IER'da RX uzilishi yoqilgan bo'lsa:

```c
if (!(u->registr[1] & UART_IER_RX))
    return;                             /* drayver hali qabul qilishga tayyor emas (terminal ochilmagan) */
```

Saboq: qurilma modelini to'g'ri qilish uchun **drayver qanday ishlashini** ham bilish kerak.

## 8.6. Disk — DMA bilan

Bizning disk — o'quv dizayni (haqiqiy qurilma emas, lekin g'oyasi haqiqiy):

```text
0x00 SEKTOR   — qaysi sektor (512 bayt)
0x04 MANZIL   — RAM dagi FIZIK manzil
0x08 BUYRUQ   — 1: o'qi (disk -> RAM), 2: yoz (RAM -> disk)
0x0C HOLAT    — 0 — OK, 1 — xato. O'QILGANDA uzilish signali o'chadi
0x10 SONI     — sektorlar soni (faqat o'qish)
0x14 UZILISH  — 1 yozilsa: har buyruq tugaganda PLIC ga uzilish (IRQ 1)
```

**DMA** (direct memory access): drayver baytlarni bitta-bitta o'tkazmaydi — "shu sektorni RAM'ning shu
joyiga ko'chir" deydi va qurilma o'zi ko'chiradi. Protsessor shu vaqtda boshqa ish qiladi, tugaganda uzilish
keladi. Haqiqiy NVMe, virtio, AHCI — hammasi shu g'oyada (navbatlar bilan murakkablashtirilgan).

Xavfsizlik tekshiruviga e'tibor bering — drayver bergan manzilga **ishonmaymiz**:

```c
if (d->manzil < RAM_BOSH || d->manzil - RAM_BOSH > m->ram_hajm - DISK_SEKTOR)
    return;                                 /* bufer to'liq RAM ichida bo'lishi kerak */
```

(Linux'imiz bu diskdan foydalanmaydi — unga drayver yozish — [mashqlar.md](mashqlar.md) dagi mustaqil loyihalardan biri. Disk `testlar/` dagi
assembly testlari va MyOS uslubidagi o'z yadrongiz uchun.)

## 8.7. Quvvat qurilmasi

`0x100000` ga `0x5555` — emulyator 0 kodi bilan chiqadi; `(kod << 16) | 0x3333` — xato kodi bilan. Testlar
shu orqali natijani "tashqariga" beradi (`SINA` makrosi, 3-bob). QEMU'dagi `sifive_test` qurilmasi bilan
bir xil — Linux'ning `syscon-poweroff` drayveri ham u bilan ishlay olardi; bizda o'chirish SBI orqali
(10-bob).

## Savol-javob

**Savol:** Nega UART'ning 3, 4, 7-registrlari "yoziladi va o'qiladi, ta'siri yo'q"?
**Javob:** Drayver ularga yozadi (tezlik, bit formati) va ba'zan **qaytarib o'qib tekshiradi** ("bu haqiqatan
16550 mi?"). Agar o'qiganda 0 qaytsa, drayver qurilmani rad etishi mumkin. Shuning uchun ularni saqlaymiz.
Linux log'ida: `ttyS0 at MMIO 0x10000000 (irq = 2, base_baud = 230400) is a 16550A` — drayver bizni haqiqiy
16550A deb tanidi.

**Savol:** `irq = 2`? Bizda UART — 10-manba-ku.
**Javob:** Linux'ning ichki raqamlash tizimi (`virq`) bor: apparat raqami (PLIC'da 10) yadro ichida boshqa
raqamga xaritalanadi. `/proc/interrupts` da ikkalasini ham ko'rish mumkin (agar yadro sozlamasida yoqilgan bo'lsa).
