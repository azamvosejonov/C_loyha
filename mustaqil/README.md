# Mustaqil yadro — MyOS'ga qaramasdan, bo'sh papkadan

> Bu bo'lim darslikning **eng oxirgi va eng muhim** qadami: o'qigan, lab'larda qayta yozgan hamma narsani
> endi **hech qanday tayyor qolipsiz** yozasiz. [QOLLANMA.md](../QOLLANMA.md) 11-bo'limidagi rejaning
> batafsil, **avtomatik tekshiriladigan** varianti.
>
> **Oldindan kerak:** darslik 00–18, 22–25; mashqlar 31–36; `labs/` dagi kamida `spinlock`, `buddy`,
> `slab`, `elf_load`, `fault_page`. Vaqt: kuniga 2–3 soat bilan B0–B12 — 6–10 hafta.

## Nega bu bo'lim kerak

Lab'larda siz funksiya **tanasini** yozdingiz: imzo, tuzilmalar, chaqiruvchi kod va testlar tayyor edi.
Yadroni noldan yozganda esa uchta **yangi** ko'nikma kerak bo'ladi — ularni hech bir lab o'rgatmaydi:

| Ko'nikma | Lab'da | Mustaqil yadroda |
|---|---|---|
| **Spetsifikatsiyani o'qish** | hujjat sizga kerakli joyni tushuntirib bergan | IDT yozuvining bit joylashuvini Intel SDM jadvalidan **o'zingiz** chiqarasiz |
| **Dizayn** | `struct page`, ro'yxatlar, qulflar — tayyor | qaysi tuzilma, qayerda turadi, kim ozod qiladi — **siz** hal qilasiz |
| **Jim xatoni debug qilish** | test aniq xabar beradi | ekran bo'sh, QEMU qayta yuklanyapti — sababni **o'zingiz** topasiz |

Shuning uchun bu yerdagi testlar yadroingizning **ichiga qaramaydi**. `tools/mustaqil.py` faqat tashqaridan
ko'rinadigan narsani tekshiradi: serialga nima chiqdi, CPU haqiqatan qaysi istisnoni oldi (QEMU logi),
registrlar qanday holatda (QEMU monitori). Ichki tuzilish — sizniki: higher-half yoki identity, bitmap
yoki ro'yxat, PIC yoki APIC — test uchun farqi yo'q.

## Qoidalar

1. **Yangi papka, yangi git repo** (masalan `~/yadrom`). Bu repodan faqat `tools/mustaqil.py` ni
   ishlatasiz — u yadroingizni tashqaridan tekshiradi.
2. **Manbalar tartibi.** Qotib qolsangiz, shu tartibda qidiring:
   1. spetsifikatsiya (pastdagi [manbalar](#manbalar) — Intel SDM, Multiboot2, `man 5 elf`);
   2. darslik va `docs/` — **g'oyalar** uchun;
   3. MyOS kodi — **faqat bosqich o'tgandan keyin**, solishtirish uchun (lab'lardagi `yechim` kabi).
3. **Avval dizayn, keyin kod.** Har bosqichdan oldin repongizdagi `DIZAYN.md` ga 5–10 qator yozing:
   qaysi tuzilmalar, nega, qanday chegaralar. Bosqich bo'limidagi "**O'zingiz hal qiling**" savollariga
   javob bering. Keyin kod.
4. **Xato daftari.** `xatolar.md`: alomat → gipoteza → bitta tajriba → sabab → saboq. Bu daftar — yadro
   dasturchisining eng qimmatli boyligi (QOLLANMA 9-bo'limidagi jadvalga qarang — xuddi shunday).
5. **Bosqich o'tgach:** `git tag B4` va MyOS'dagi tegishli faylni o'qib solishtiring. Farqlarni
   `DIZAYN.md` ga yozing: kimniki yaxshiroq va **nega**. Farq — xato degani emas.
6. **Yechimni so'ramang** (odamdan ham, AI'dan ham). So'rash mumkin: "bu log nimani bildiradi?",
   "SDM'ning qaysi bo'limini o'qiy?", "kodimni qattiq tanqid qil".

## Vosita: `tools/mustaqil.py`

```bash
tools/mustaqil.py royxat                               # bosqichlar
tools/mustaqil.py tekshir B0 --elf ~/yadrom/build/yadro.elf   # ELF yo'li eslab qolinadi
tools/mustaqil.py tekshir B4                           # keyingi safar --elf shart emas
tools/mustaqil.py hammasi                              # B0 dan birinchi xatogacha + holat jadvali
tools/mustaqil.py log                                  # oxirgi ishga tushirishdagi istisnolar tahlili
tools/mustaqil.py tekshir B6 --gdb                     # QEMU gdb'ni kutadi: gdb yadro.elf -ex "target remote :1234"
tools/mustaqil.py tekshir B6 --korsat                  # QEMU buyrug'i va grub.cfg (qo'lda takrorlash uchun)
```

**Siz beradigan narsa — bitta fayl:** Multiboot2 sarlavhali ELF (`yadro.elf`). Vosita uni GRUB bilan ISO
qiladi va QEMU'da ishga tushiradi:

- RAM: `-m 128M`, `-no-reboot` (triple fault'da QEMU to'xtaydi — "qayta yuklanish sikli" bo'lmaydi);
- `-d int,cpu_reset` — har bir istisno va uzilish logga yoziladi (keyin `build/mustaqil/int.log`);
- GRUB buyruq qatori: `mustaqil=B<n> kalit=<tasodifiy son>`; B12 da yana Multiboot2 moduli `dastur`;
- GRUB xabarlari ham serialga chiqadi — "no multiboot header found" kabi xatolarni vosita o'zi taniydi.

**Protokol (yadroingiz nimani bajarishi kerak):**

1. Barcha chiqish — **COM1** (port `0x3F8`) ga, har qator `\n` bilan tugaydi (`\r\n` ham bo'ladi).
2. **B0–B3 qatorlari har yuklanishda** chiqadi (bular "banner": keyingi bosqichlarda ham buzilmasligi
   tekshiriladi — regressiya testi).
3. **B4 dan boshlab** yadro buyruq qatoridagi `mustaqil=Bn` ni o'qiydi va **faqat o'sha** testni bajaradi.
   Test oxirida `Bn TUGADI` chiqaradi va to'xtaydi (`cli; hlt` sikli). Vosita `TUGADI` ni ko'rgach,
   registrlarni o'qiydi va QEMU'ni yopadi.
4. Qator matni **aynan** jadvaldagidek (bo'sh joylar ham). Sonlar o'n oltilikda — kichik harflar.
5. Ixtiyoriy: QEMU'dan darhol chiqish uchun `outb(0xF4, 0)` (`isa-debug-exit` qurilmasi ulangan).

Test yiqilsa, vosita logni tahlil qiladi va **maslahat** beradi: qaysi istisno, sizning qaysi
funksiyangizda (`addr2line` orqali), eng ehtimoliy sabablar. Yechimni bermaydi.

### Xato ovchisi: `tools/ovchi.py`

Yadro yozishda vaqtning katta qismi xato **qidirishga** ketadi, va yadro xatosi ko'pincha jim. Bu
ko'nikmani B0 dan oldin va parallel ravishda MyOS ustida mashq qiling:

```bash
tools/ovchi.py royxat          # 8 ta ov: CPL, EOI, IST, ext2, TSS.rsp0, kontekst almashish, swapgs, COW
tools/ovchi.py boshla eoi      # MyOS'ning bitta faylida 1-2 qator haqiqiy xatoga almashadi (qaysi fayl - aytilmaydi)
tools/ovchi.py maslahat        # har chaqiruvda bitta keyingi maslahat (avval usul, oxirida deyarli javob)
tools/ovchi.py tekshir         # yig'ish + QEMU testi
tools/ovchi.py javob eoi       # topgandan keyin: nima edi va nega aynan shunday alomat berdi
tools/ovchi.py tiklash         # asl kodni qaytarish
```

Qoida: `git diff` — taqiqlangan; avval gipoteza, keyin bitta tajriba (`-d int`, QEMU monitori, gdb,
`addr2line`). Har ovni `xatolar.md` ga yozing. Bu yerdagi xatolarning har birini o'z yadroingizda ham
qilasiz — farqi shundaki, u yerda hech kim "alomat"ni oldindan aytmaydi.

---

## Bosqichlar

Har bosqichda: **Maqsad** → **O'qing** (spetsifikatsiya) → **Spetsifikatsiya mashqi** (kod yozishdan
oldin, qog'ozda) → **O'zingiz hal qiling** (dizayn) → **Chiqishi kerak** (protokol) → **Tuzoqlar** →
**O'zingizni tekshiring**.

### B0 — Yuklash va serial

**Maqsad:** GRUB sizning ELF faylingizni taniydi, yuklaydi va `_start` ga sakraydi; siz serialga birinchi
qatoringizni chiqarasiz.

**O'qing:** Multiboot2 spetsifikatsiyasi — "OS image format", "Header magic fields", "Machine state"
bo'limlari (GRUB sizga CPU'ni **qanday holatda** topshiradi: 32-bit himoyalangan rejim, sahifalash yo'q,
`EAX` = `0x36d76289`, `EBX` = ma'lumot manzili, **stek yo'q**, GDT ishonchsiz). OSDev: "Serial Ports".
Darslik 18.3–18.4 (linker skripti), 17-bob (NASM).

**Spetsifikatsiya mashqi:** sarlavhaning har bir maydonini (magic, architecture, header_length, checksum,
tugatuvchi teg) qog'ozga yozing va checksum'ni qo'lda hisoblang: `magic + arch + length + checksum` 32
bitda nechaga teng bo'lishi kerak?

**O'zingiz hal qiling:**
- Yadro qaysi manzilga **yuklanadi** (fizik) va qaysi manzilda **ishlaydi** (virtual)? Hozircha bir xil
  bo'lishi mumkin (1 MB), lekin B1 da qaror qilasiz: identity yoki higher-half.
- Linker skriptida sarlavha qaysi bo'limda va nega birinchi turishi kerak?
- Serialni assembly'da yozasizmi yoki C'ga o'tib? (B0 ni 32-bit assembly'da qilish — yaxshi mashq.)

**Chiqishi kerak:**
```text
B0 salom
```

**Tuzoqlar:** sarlavha birinchi 32 KB da emas (linker uni `.text` dan keyin qo'ygan); 8 baytga tekislanmagan;
tugatuvchi teg yo'q; `nasm -f elf32` va `ld -m elf_x86_64` aralashib ketgan. Tekshiruv: `grub-file
--is-x86-multiboot2 yadro.elf && echo OK`.

**O'zingizni tekshiring:** (1) Nega GRUB stek bermaydi va siz birinchi navbatda nima qilasiz? (2) `EBX` dagi
manzil fizikmi yoki virtual? (3) Nega serial uchun 0x3FD ning 5-bitini kutish kerak?

### B1 — Long mode va C

**Maqsad:** 32-bitdan 64-bitga o'tish (sahifa jadvallari, PAE, EFER.LME, CR0.PG, 64-bit GDT, far jump),
stek, `.bss` ni tozalash va C'dagi `kmain` ga o'tish.

**O'qing:** Intel SDM 3A — "System Architecture Overview" (CR0, CR3, CR4, EFER), "Initializing IA-32e
Mode" (o'tish tartibi aynan shu yerda), "Segment Descriptors" (64-bit kod segmentidagi L va D bitlari).
OSDev: "Setting Up Long Mode". Darslik 24-bob (sahifalash nazariyasi), docs/01, docs/09.

**Spetsifikatsiya mashqi:** 64-bit kod segmenti deskriptorini (8 bayt) bitma-bit qog'ozga yozing:
qaysi bitlar P, DPL, S, type, L, D, G. Sizning `dq 0x...` qiymatingiz qayerdan chiqadi?

**O'zingiz hal qiling:**
- **Identity yoki higher-half?** Identity (yadro 1 MB da) — sodda. Higher-half (`0xFFFFFFFF80000000`) —
  B11–B12 da user manzil maydoni toza qoladi. Ikkalasi ham testdan o'tadi; sababini `DIZAYN.md` ga yozing.
- Boot vaqtida qancha xotirani xaritalaysiz (1 GB? 2 MB sahifalar bilan?) va **nega** — keyingi
  bosqichlarda bu sizning sahifa jadvallaringizga yozish imkoniyatingizni belgilaydi.
- Yadro steki qayerda, qancha?

**Chiqishi kerak:**
```text
B1 kmain                     <- shu so'z bilan boshlansa, davomi ixtiyoriy
```
Vosita yana QEMU monitoridan tekshiradi: `EFER.LMA = 1`, `CR0.PG = 1`, `CR4.PAE = 1`.

**Tuzoqlar:** paging yoqilgan paytda bajarilayotgan kod yangi jadvalda yo'q → darhol #PF → triple fault
(vosita buni "RIP == CR2" deb ko'rsatadi); far jump selektori GDT'dagi 64-bit segmentga mos emas;
`-mno-red-zone` unutilgan (uzilishlar kelganda stek buziladi — B4/B7 da bilinadi); kompilyator SSE ishlatgan
(`-mgeneral-regs-only` yoki `-mno-sse -mno-mmx -mno-80387`).

**O'zingizni tekshiring:** (1) Nega long mode'ga o'tishdan oldin paging majburiy? (2) LME va LMA farqi?
(3) `-mcmodel=kernel` nimani anglatadi va qachon kerak?

### B2 — `kprintf`

**Maqsad:** formatlash dvigateli — butun yadro debug'ining asosi. Darslik 32-bobdagi M1–M2 ni yozgan
bo'lsangiz, endi uni **xotiradan** qayta yozing.

**O'qing:** darslik 12 (`printf` to'liq), 18 (`kprintf` loyihasi), 32-bob; `<stdarg.h>` — freestanding'da
ham bor (kompilyator beradi).

**O'zingiz hal qiling:** chiqish qayerga (serial, keyinroq VGA/framebuffer ham)? Avval buferga yig'ib,
keyin bir martada chiqarasizmi (B10 da bu muhim bo'ladi)? Uzilish ishlovchisidan chaqirilsa nima bo'ladi?

**Chiqishi kerak** — aynan shu chaqiruvlar (`kmain` da, B1 dan keyin):
```c
kprintf("B2 d=%d u=%u x=%x s=%s c=%c p=%p foiz=%%\n", -42, 3000000000u, 0xbeefu, "satr", 'Z',
        (void *)0xffffffff80001000);
kprintf("B2 min=%d max=%u lx=%lx\n", INT_MIN, UINT_MAX, 0x123456789abcdefUL);
kprintf("B2 [%5d] [%-5d] [%08x]\n", 42, 42, 0xbeefu);           /* ixtiyoriy ★ */
```
```text
B2 d=-42 u=3000000000 x=beef s=satr c=Z p=0xffffffff80001000 foiz=%
B2 min=-2147483648 max=4294967295 lx=123456789abcdef
B2 [   42] [42   ] [0000beef]
```

**Tuzoqlar:** `INT_MIN` ni manfiydan musbatga aylantirish — UB (vosita buni aniq aytadi); `%c` uchun
`va_arg(ap, char)`; `%lx` da `l` ni e'tiborsiz qoldirish.

### B3 — Multiboot2 ma'lumoti

**Maqsad:** GRUB bergan ma'lumotlarni o'qish: buyruq qatori (B4 dan boshlab test tanlash uchun kerak!) va
xotira xaritasi (B5 uchun).

**O'qing:** Multiboot2 — "Boot information format", "Basic tags structure", "Boot command line",
"Memory map". Darslik 09-bob (struct, tekislash), 25-bob.

**Spetsifikatsiya mashqi:** teglar bo'ylab yurish formulasini yozing: keyingi teg manzili = ? (`size`
ning o'zi emas!). Xotira xaritasi yozuvining maydonlari va hajmi qancha? Nega `entry_size` alohida beriladi?

**O'zingiz hal qiling:** Multiboot2 ma'lumoti (va B12 dagi modul) fizik xotirada qayerda turadi va B5 dagi
PMM uni **bo'sh** deb bermasligi uchun nima qilasiz — band qilasizmi yoki nusxalaysizmi?

**Chiqishi kerak:**
```text
B3 cmdline=mustaqil=B3 kalit=123456          <- GRUB bergan qator to'liq
B3 ram=130559 KB                              <- type == 1 hududlar yig'indisi (KB)
```
Vosita: `kalit=` qiymati to'g'rimi, RAM 120000–131072 KB oralig'idami (QEMU `-m 128M`).

### B4 — IDT va istisnolar

**Maqsad:** IDT, 256 ta kirish "stub"i (assembly), umumiy ishlovchi, `iretq` bilan qaytish. Ishlovchi
**saqlangan freymni o'zgartira** olishi (B4 da RIP, B12 da RAX).

**O'qing:** Intel SDM 3A — "Interrupt and Exception Handling": "Exception and Interrupt Vectors" (qaysi
vektorlarda CPU **xato kodi** qo'yadi), "64-Bit Mode IDT" (16 baytlik yozuv), "64-Bit Mode Stack Frame",
"Exception and Interrupt Reference" (har bir vektor: fault yoki trap, RIP nimani ko'rsatadi). Darslik 17
(stek, chaqirish qoidalari), docs/02.

**Spetsifikatsiya mashqi:** (1) 64-bit IDT yozuvini 16 bayt jadval qilib chizing va C `struct` yozing,
`_Static_assert(sizeof(struct idt_entry) == 16, "...")`. (2) 0–31 vektorlardan qaysilari xato kodi qo'yadi?
Ro'yxatni **SDM'dan** tuzing. (3) `int3` — trap, `ud2` — fault: ishlovchiga kelgan RIP har birida nimani
ko'rsatadi?

**O'zingiz hal qiling:** stub'lar makros bilan generatsiya qilinadimi? Freym `struct` i qanday tartibda
(push tartibiga teskari)? Ishlovchi qaytishi mumkin bo'lmagan istisnolarda nima qiladi (panic)?

**Chiqishi kerak** (`mustaqil=B4` bo'lsa): `int3` → ishlovchi chiqaradi va qaytadi; `ud2` → ishlovchi
chiqaradi, **saqlangan RIP ni 2 ga oshiradi** va qaytadi; nolga bo'lish (`div` inline assembly bilan — C'da
`x / 0` UB, kompilyator uni olib tashlashi mumkin) → ishlovchi chiqaradi, `TUGADI`, to'xtaydi.
```text
B4 vektor=3 rip=0x...
B4 int3 dan qaytdi
B4 vektor=6 rip=0x...
B4 ud2 dan qaytdi
B4 vektor=0 rip=0x...
B4 TUGADI
```
Vosita QEMU logidan haqiqiy `#BP`, `#UD`, `#DE` bo'lganini va chiqarilgan RIP CPU'niki bilan mosligini
tekshiradi.

**Tuzoqlar:** xato kodi bor/yo'q vektorlarda stek har xil — stub o'zi `push 0` qilmasa, `iretq` noto'g'ri
freymdan qaytadi (#GP); `iret` (32-bit) o'rniga `iretq`; `lidt` operandi `packed` emas; DF bayrog'i
(`cld`) va 16 baytlik stek tekislashi C chaqiruvidan oldin.

**O'zingizni tekshiring:** (1) Interrupt gate va trap gate farqi (IF bilan nima bo'ladi)? (2) Ishlovchi
qaytgandan keyin `ud2` qayta bajarilsa nima bo'ladi? (3) Nega ishlovchi C funksiyasini chaqirishdan oldin
**hamma** umumiy registrlar saqlanadi, callee-saved'lar ham?

### B5 — Fizik xotira (PMM)

**Maqsad:** 4 KB kadrlarni ajratuvchi: xotira xaritasidan bo'sh kadrlar, yadro/Multiboot2/modul band.

**O'qing:** darslik 25-bob (bitmap, ro'yxat, buddy), docs/03, mashq 32 (buddy).

**O'zingiz hal qiling:** bitmap, stek yoki bog'langan ro'yxat (kadrning o'zida)? Metama'lumotning o'zi
qayerda turadi va u ham band qilinadimi? Bitta kadrmi yoki ketma-ket `n` ta kerak bo'ladimi (keyinroq DMA
uchun)?

**Chiqishi kerak** (`mustaqil=B5`): **hamma** kadrlarni birma-bir oling (sanang: `N1`); har birining
tekislanishini tekshiring; har biriga qandaydir belgi yozib takrorlarni aniqlang (masalan, olingan
kadrlarni kadrlarning o'zida bog'langan ro'yxatga tizing va ro'yxat uzunligini qayta sanang); keyin
hammasini qaytaring va yana hammasini oling (`N2`). Oxirida hammasini qaytaring.
```text
B5 birinchi=32408 ikkinchi=32408 takror=0 tekis_emas=0
B5 TUGADI
```
Vosita: `N1 == N2`, takror va tekislanmagan yo'q, `N1 × 4 KB` 100000–131072 KB oralig'ida.

### B6 — Sahifalash (VMM)

**Maqsad:** `map(va, pa, flags)` / `unmap(va)`: 4 darajali jadval bo'ylab yurish, kerak bo'lsa yangi
jadval ajratish, TLB'ni yangilash, #PF ishlovchisi (CR2, xato kodi).

**O'qing:** Intel SDM 3A — "Paging": "4-Level Paging" (jadval formatlari, "Formats of CR3 and
Paging-Structure Entries"), "Page-Fault Exceptions" (xato kodi bitlari), "Invalidation of TLBs" (`invlpg`).
Darslik 24-bob, mashq 31, docs/04, lab `fault_page`.

**Spetsifikatsiya mashqi:** `0x500000000000` va `0x500000201000` uchun PML4, PDPT, PD, PT indekslarini qo'lda
hisoblang. Qaysi darajalarda ular bitta yozuvni bo'lishadi, qaysilarida yo'q? (Javobni testdan oldin
yozing — keyin qaysi jadvallar ajratilganini tekshiring.)

**O'zingiz hal qiling:** yangi jadval kadriga **qanday yozasiz** — identity xarita orqali, butun RAM'ni
yuqori yarmiga xaritalash (HHDM) orqalimi, yoki rekursiv xarita orqali? Bu — butun VMM dizaynining asosiy
qarori. `unmap` bo'shab qolgan jadvallarni qaytaradimi?

**Chiqishi kerak** (`mustaqil=B6`): bitta kadrni `A = 0x500000000000` va `B = 0x500000201000` ga xaritalang
(yozish ruxsati bilan); A orqali `0x1122334455667788` yozing, B orqali o'qing va chiqaring. Keyin A ni
`unmap` qiling va A ga **yozing** → #PF ishlovchisi chiqaradi va to'xtaydi.
```text
B6 alias=1122334455667788
B6 #PF cr2=0x500000000000 err=0x2
B6 TUGADI
```

**Tuzoqlar:** `unmap` dan keyin `invlpg` yo'q — TLB eski tarjimani eslab qoladi va yozish **xatosiz**
o'tadi (QEMU TLB'ni ham emulyatsiya qiladi, vosita buni taniydi); yangi jadval nol bilan
to'ldirilmagan; yozuvga fizik manzil o'rniga virtual yozilgan.

**O'zingizni tekshiring:** (1) Nega xato kodi 0x2 (bitlarini o'qing)? (2) TLB'ni tozalamaslik qachon
**xavfsiz** (yangi xarita qo'shganda)? (3) SMP'da `invlpg` yetarlimi (docs/10, "TLB shootdown")?

### B7 — Taymer: PIC + PIT

**Maqsad:** apparat uzilishlari: 8259 PIC ni qayta raqamlash (IRQ0 → 0x20), PIT 100 Hz, EOI.

**O'qing:** 8259A va 8254 (PIT) ma'lumotnomalari yoki OSDev: "8259 PIC", "Programmable Interval Timer".
Darslik 16 (port I/O), docs/02.

**Spetsifikatsiya mashqi:** ICW1–ICW4 ning har bir baytini nega aynan shu qiymat ekanini yozing. Nega
qayta raqamlamasdan IRQ0 "double fault" ga o'xshab ko'rinadi?

**O'zingiz hal qiling:** PIC yoki LAPIC taymer (LAPIC — B10 dan keyin SMP uchun kerak bo'ladi)? Hisoblagich
qanday turda (`volatile`? atomik?) va nega?

**Chiqishi kerak** (`mustaqil=B7`): `sti; hlt` sikli; har 10-tikda qator, 100-tikda `TUGADI`:
```text
B7 tik=10
B7 tik=20
...
B7 tik=100
B7 TUGADI
```
Vosita: tartib, va 90 tik orasidagi **haqiqiy vaqt** ~0.9 s (0.3–5 s qabul qilinadi).

**Tuzoqlar:** EOI yo'q — bitta tik va jimlik (vosita taymer uzilishlarini logdan sanaydi va aytadi);
PIC qayta raqamlanmagan; `sti` yo'q.

### B8 — Klaviatura

**Maqsad:** IRQ1, port 0x60, skan-kodlar (set 1) → harflar, qatorni yig'ish.

**O'qing:** OSDev: "PS/2 Keyboard", "PS/2 Controller". Darslik 16.

**Chiqishi kerak** (`mustaqil=B8`): IRQ1 ni ochib `B8 tayyor` chiqaring. Vosita QEMU monitori orqali
`s a l o m Enter` ni **o'zi bosadi**. Enter'da:
```text
B8 tayyor
B8 satr=salom
B8 TUGADI
```

**Tuzoqlar:** "break code" (tugma qo'yib yuborilganda, bit 7 = 1) ham harf deb olinsa — `ssaalloomm`;
port 0x60 o'qilmasa kontroller keyingi uzilishni bermaydi; EOI.

### B9 — Heap (`kmalloc` / `kfree`)

**Maqsad:** ixtiyoriy hajmli ajratuvchi: B5 (kadrlar) + B6 (xaritalash) ustida.

**O'qing:** darslik 08, 25 (bo'sh ro'yxatlar, chegara teglari, slab), 32-bob (M3–M4), docs/05, lab `slab`.

**O'zingiz hal qiling:** heap virtual xotiraning qayerida? Sahifalarni qachon xaritalaysiz (darhol yoki
o'sganda)? Sarlavha formati, bo'linish (split), birlashtirish (coalescing)? Kichik obyektlar uchun slab
kerakmi? Double free'ni qanday ushlaysiz (sehrli son, holat bayti)?

**Chiqishi kerak** (`mustaqil=B9`):
1. `p1 = kmalloc(1)`, `p2 = kmalloc(24)`, `p3 = kmalloc(100)` manzillari;
2. stress: 3 marta — 200 ta blok (hajmi turlicha, 1–2048), har birini o'z raqami bilan to'ldiring, hammasini
   tekshiring, avval juft, keyin toq indeksdagilarni bo'shating; `ajratish` = jami ajratishlar soni (≥ 600),
   `xato` = buzilgan bloklar soni;
3. `kmalloc(1 MB)` — to'ldirib, bo'shating;
4. `kfree(p1)` ni **ikki marta** chaqiring — ikkinchisini ushlang.
```text
B9 p1=0x... p2=0x... p3=0x...
B9 ajratish=600 xato=0
B9 katta=ok
B9 double-free ushlandi
B9 TUGADI
```
Vosita: manzillar 16 ga karrali, bloklar kesishmaydi, `xato=0`.

### B10 — Oqimlar va preemption

**Maqsad:** yadro oqimlari, kontekst almashish (assembly), round-robin, **taymer bilan majburiy** almashish.

**O'qing:** darslik 23 (kontekst almashish, scheduling), 15, 26; docs/06; mashq 35; lab `sleep_wakeup`.

**O'zingiz hal qiling:** kontekst qayerda saqlanadi — oqim stekida (xv6 kabi) yoki `struct` da? Yangi
oqimning steki qanday tayyorlanadi, toki `switch` birinchi marta unga "qaytganda" oqim funksiyasi boshlansin?
Oqim tugaganda uning steki kim tomonidan va **qachon** ozod qilinadi (o'z stekini o'zi ozod qila olmaydi!)?

**Chiqishi kerak** (`mustaqil=B10`): ikki oqim — A va B. Har biri `i = 0..4` uchun `B10 A<i>` (B uchun
`B10 B<i>`) chiqaradi, keyin **yield qilmasdan** taymer hisoblagichi 3 ga oshguncha faol kutadi (`while (tik < boshi + 3) {}`). Asosiy oqim ikkalasi
tugashini kutib, `TUGADI` chiqaradi.
```text
B10 A0
B10 B0
B10 A1
...
B10 TUGADI
```
Vosita: har oqim 0..4 tartib bilan; oqimlar **kamida 3 marta** almashgan (aks holda preemption yo'q);
qatorlar aralashmagan.

**Tuzoqlar:** yangi oqim uzilish ichidan boshlanadi (IF=0) — `sti` qayerda?; switch'da saqlanadigan
registrlar ro'yxati chaqirish qoidasiga mos emas; EOI `switch` dan **keyin** yuborilsa — keyingi oqim
taymersiz qoladi; ikki oqim bitta qatorni aralashtirib chiqaradi (`kprintf` atomar emas).

### B11 — User rejimi

**Maqsad:** GDT'da user segmentlari, TSS (`rsp0`), ring 3 ga `iretq` bilan o'tish, syscall, ring 3 dagi
istisnoni ushlash.

**O'qing:** Intel SDM 3A — "Protection" (CPL, DPL, RPL), "Task Management": "Task Management in 64-bit
Mode" (64-bit TSS formati), "Interrupt and Exception Handling" — imtiyoz o'zgarganda stek almashishi.
AMD64 APM 2-jild — `syscall`/`sysret` (ixtiyoriy). Darslik 14, 17; docs/07.

**Spetsifikatsiya mashqi:** 64-bit TSS deskriptorini (GDT'da **16 bayt**) chizing. `iretq` stekdan nimani,
qaysi tartibda oladi?

**O'zingiz hal qiling:** syscall: `int 0x80` yoki `syscall` (MSR'lar, `swapgs`)? Syscall ABI? (B12 dagi
tayyor ABI'ga mos qilish qulay.) User sahifalari qayerda (higher-half yadroda — butun pastki yarim;
identity'da — PML4'ning boshqa yozuvi)?

**Chiqishi kerak** (`mustaqil=B11`): user sahifasiga kichik kodni (o'zingiz assembly'da yozasiz)
joylashtiring va ring 3 ga o'ting. U `write` syscall'i bilan `B11 ring3 salom\n` ni chiqaradi, keyin `cli`
bajaradi → #GP (ring 3 da `cli` taqiqlangan). Ishlovchi CPL=3 ni ko'rib:
```text
B11 ring3 salom
B11 #GP cpl=3
B11 TUGADI
```
Vosita: QEMU logida cpl=3 dagi #GP bor (kod **haqiqatan** ring 3 da ishladi), TR (TSS) yuklangan.

**Tuzoqlar:** U bit faqat PTE da (hamma darajada kerak); selektorlarda RPL=3 yo'q; 0x80-yozuvida DPL=0
(user chaqira olmaydi — #GP, xato kodi 0x402: vosita buni ham taniydi); `ltr` qilinmagan yoki `rsp0` noto'g'ri.

### B12 — ELF yuklovchi

**Maqsad:** haqiqiy dasturni (fayl formati — ELF) yuklash: Multiboot2 moduli → PT_LOAD segmentlari →
xaritalash → ring 3 → syscall'lar → `exit`.

**O'qing:** `man 5 elf` (internetsiz ham bor!), System V ABI x86-64; darslik 22; mashq 36; lab `elf_load`;
docs/07. Multiboot2 — "Modules".

**Dastur tayyor:** [dastur.asm](dastur.asm) — vosita uni yig'adi (`dastur.ld`: `.text` = `0x8000000000`,
`.data`/`.bss` keyingi sahifada) va `dastur` nomli modul qilib beradi. **ABI** (Linux raqamlari):

| `rax` | syscall | argumentlar | natija |
|---|---|---|---|
| 1 | `write(fd, buf, len)` | `rdi`, `rsi`, `rdx` | `rax = len` |
| 60 | `exit(kod)` | `rdi` | qaytmaydi |

Sukut bo'yicha `int 0x80`; `syscall` buyrug'ini qilgan bo'lsangiz — `--abi syscall`.

**Chiqishi kerak** (`mustaqil=B12`; birinchi uchtasini **dastur** chiqaradi, oxirgi ikkitasini **yadro**):
```text
B12 ELF dan salom
B12 bss=0
B12 data=42
B12 exit=7
B12 TUGADI
```

**Tuzoqlar:** PMM modul xotirasini bo'sh deb berib yuborgan (ELF o'qilishidan oldin ustiga yozilgan);
`p_memsz > p_filesz` qismi tozalanmagan (`bss=xato`); data segmentiga W bit berilmagan; syscall natijasi
user'ning `rax` iga qaytmagan. **Xavfsizlik:** ELF — ishonchsiz ma'lumot: `e_phoff`, `p_offset + p_filesz`
modul chegarasidan chiqmasin, `p_vaddr` yadro hududiga tegmasin (lab `elf_load` dagi kabi).

---

## B12 dan keyin: testlarni endi o'zingiz yozasiz

B0–B12 — QOLLANMA 11-bo'limidagi 1–13-qadamlar. Qolganlari (14–19: fork/exec/wait + shell, VFS + pipe,
disk + ext2, signallar, SMP) uchun tayyor test yo'q — va bu ataylab: **o'z yadroingizga test yozish ham
mustaqillikning bir qismi.** Maslahatlar:

- Xuddi shu protokol: `mustaqil=B13` → test → `B13 TUGADI`. `tools/mustaqil.py` ni nusxalab, o'z
  bosqichlaringizni qo'shing (`BOSQICHLAR` ro'yxati va `t_bNN` funksiyalari — har biri 20–40 qator).
- Tashqi "hakam" tanlang: ext2 yozishni **`e2fsck -fn disk.img`** tekshiradi (MyOS'ning `tools/test.sh`
  shunday qiladi); fork/COW'ni — bola va ota bir xil manzilga har xil qiymat yozib, ikkalasini chiqarishi;
  SMP'ni — `-smp 4` va har CPU o'z raqamini chiqarishi; qulflarni — ikki CPU'da 1 000 000 ta oshirish va
  yakuniy son.
- Yadroingizga ichki testlar ham qo'shing (MyOS'dagi `kernel/tests/selftest.c` g'oyasi).

## Darajalar: o'zingizni qanday baholash

| Daraja | Mezon |
|---|---|
| **1** | B0–B4 o'tadi. Triple fault'ni `-d int` logidan o'zingiz tushuntira olasiz |
| **2** | B5–B9 o'tadi. VMM dizayningizni (jadvallarga qanday yozasiz, TLB) 5 daqiqada tushuntira olasiz |
| **3** | B10–B12 o'tadi. Kontekst almashishni qog'ozda stek chizib ko'rsata olasiz |
| **Mustaqil** | **Yopiq kitob sinovi:** yangi bo'sh papkada, faqat SDM va Multiboot2 bilan, B0–B7 ni **bir kunda** qayta yozasiz |
| **Kuchli** | 13+ bosqichlarni o'z testlaringiz bilan qildingiz; [xato ovchisi](../tools/ovchi.py)ning 8 ta xatosini `maslahat`siz topdingiz |

Yopiq kitob sinovini 2–3 hafta oraliq bilan **ikki marta** qiling: ikkinchisida vaqt yarmiga qisqarsa —
bilim mustahkamlangan.

## Ikkinchi yadro: RISC-V, o'z emulyatoringizda

[virtual_kompyuter](../virtual_kompyuter/README.md) dagi 22 ta mashqni tugatgan bo'lsangiz, sizda **o'zingiz
yozgan kompyuter va firmware** bor. Uning ustida ikkinchi, kichik yadro yozish — x86 dan keyin eng kuchli
mashq: arxitektura boshqa, lekin g'oyalar o'sha. Bu safar test vositasini ham **o'zingiz** yozasiz.

- Firmware yadroni **S rejimda** `0x80400000` dan boshlaydi: `a0` = hart raqami, `a1` = DTB manzili
  (`firmware/asosiy.c`). Ishga tushirish: `./build/vk -m 64 build/firmware.elf yadro.elf`.
- Konsol: SBI (`ecall`, `a7` = EID: legacy putchar `0x01` yoki `DBCN`) — kitob 10-bob.
- Bosqichlar: R0 SBI orqali salom → R1 `stvec` va trap (istisno ma'lumoti: `scause`, `sepc`, `stval`)
  → R2 DTB'dan RAM hajmi → R3 PMM → R4 Sv32 sahifalash (`satp`, `sfence.vma`) → R5 SBI taymer va
  preemption → R6 U rejim va `ecall` syscall → R7 ELF.
- Spetsifikatsiya: RISC-V Privileged ISA (kitob 6–7-boblar shu hujjatga tayanadi), SBI spetsifikatsiyasi.
- Taqqoslash uchun keyin o'qing: **xv6-riscv** (MIT 6.1810) — o'qish uchun yozilgan kichik Unix.

## Manbalar

Hammasi inglizcha — bu ham mashq (REJA.md: kuniga 30 daqiqa ingliz tili). Yuklab oling, offline saqlang:

| Hujjat | Nima uchun | Qayerdan |
|---|---|---|
| Intel® 64 and IA-32 Architectures SDM, 3A-jild | paging, IDT, TSS, himoya, long mode | intel.com/sdm |
| Intel SDM, 2-jild | har bir buyruq (`iretq`, `invlpg`, `lidt`, `syscall`) | o'sha joyda |
| AMD64 Architecture Programmer's Manual, 2-jild | o'sha mavzular, ko'pincha soddaroq tilda | amd.com, "AMD64 APM" |
| Multiboot2 Specification | sarlavha, mashina holati, teglar | gnu.org/software/grub/manual/multiboot2 |
| System V ABI, AMD64 | chaqirish qoidalari, ELF | gitlab.com/x86-psABIs/x86-64-ABI |
| `man 5 elf` | ELF tuzilmalari | kompyuteringizda bor |
| OSDev Wiki | amaliy maqolalar (8259, PIT, PS/2, serial) | wiki.osdev.org |
| xv6 kitobi va kodi | tugatgandan keyin solishtirish | pdos.csail.mit.edu/6.1810 |

**OSDev'dan kod ko'chirmang** — "Bare Bones" kabi sahifalarni o'qing, yoping, keyin yozing. Ko'chirilgan
`boot.asm` dagi xatoni tushunmasdan topa olmaysiz.

**SDM'ni qanday o'qish kerak:** butunlay emas! Kerakli bobning boshidagi umumiy qismni, keyin
jadval/rasmni o'qing. Bo'lim raqamlari nashrdan nashrga o'zgaradi — shuning uchun bu yerda **sarlavhalar**
berilgan: PDF'da sarlavha bo'yicha qidiring.
