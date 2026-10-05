# 0-bob. Kompyuterni dasturda yasaymiz

> **Bu bobda nima o'rganasiz:** bu loyihada nima quriladi va nega; virtual kompyuterning qatlamlari;
> loyihani qanday yig'ish va ishga tushirish; mashqlar (TODO) bilan qanday ishlash.
> **Oldindan nima kerak:** asosiy darslikning 0–16-boblari (ayniqsa 3 — operatorlar, 7 — ko'rsatkichlar, 9 — struct,
> 11 — ko'p fayl va Makefile, 16 — bitlar). 17-bob (assembly) va 24-bob (virtual xotira) juda foydali.
> **Vaqt:** 1–2 soat.

## Bu kitob nima haqida?

Siz hozirgacha **kompyuter uchun** dastur yozdingiz. Bu kitobda **kompyuterning o'zini** yozamiz.

Aniqrog'i — RISC-V protsessorli to'liq kompyuterni C tilida, **dastur sifatida** quramiz: protsessor,
xotira, virtual xotira (MMU), taymer, uzilishlar kontrolleri, konsol (UART), disk. Keyin unga o'zimizning
**firmware**ni (BIOS'ning RISC-V dagi o'rinbosari) yozamiz va oxirida — **haqiqiy Linux yadrosini**
yuklaymiz. Linux bizning "kompyuterimiz"ni haqiqiy deb o'ylaydi: yuklanadi, shell ochadi, jarayonlar
yaratadi, buyruqlarni bajaradi va o'chadi.

```console
$ make linux-ishga
[vk-sbi] Virtual kompyuter firmware'i (SBI v2.0), M rejim
[vk-sbi] hart 0, DTB 0x83ff0000, yadro 0x80400000
[vk-sbi] yadroga o'tyapman (mret -> S rejim)

[    0.000000] Linux version 6.6.50 ...
[    0.000000] Machine model: virtual-kompyuter,rv32
...
Salom! Linux ishga tushdi. Men — init (pid 1), oddiy shell. 'help' — buyruqlar.
vk:/# uname
Linux 6.6.50 #7 Mon Oct  5 16:17:05 UTC 2026 riscv32
vk:/# run /bin/salom bir ikki
Men alohida jarayonman: pid 16, ota 1, argumentlar: 3 ta
...
vk:/# poweroff
```

Bu ekrandagi har bir qator — **siz yozgan** kod ishlagani natijasi: Linux'ning har bir buyrug'ini sizning
`cpu_qadam()` funksiyangiz bajaradi, har bir manzilni sizning `mmu_tarjima()` tarjima qiladi, har bir harf
sizning UART qurilmangiz orqali chiqadi.

**Nega aynan emulyator?** Chunki u bir vaqtning o'zida uchta narsani o'rgatadi:

1. **Apparat qanday ishlaydi** — protsessor buyruqni qanday "o'qiydi", uzilish qanday keladi, MMU nima qiladi.
   Kitobdan o'qish boshqa, uni o'zingiz yasab, xato qilib, tuzatish — butunlay boshqa.
2. **Operatsion tizim apparatdan nima kutadi** — Linux'ni yuklash uchun **hamma narsa** spetsifikatsiyadagidek
   bo'lishi shart. Bitta bit xato bo'lsa — yadro qotib qoladi. Bu sizni "taxminan to'g'ri" emas,
   **aniq to'g'ri** yozishga o'rgatadi.
3. **C'ning eng kuchli tomoni** — bitlar, ko'rsatkichlar, aniq o'lchamli turlar, `struct` lar bilan
   apparatni tasvirlash. Senior darajadagi tizim dasturchisi har kuni shu bilan ishlaydi.

**Hayotdan misol: avtomobil simulyatori.** Haydovchilar avval simulyatorda o'rganadi: rul, pedallar, yo'l —
hammasi dasturda, lekin haydovchi uchun "haqiqiy". Bizning emulyator ham shunday: Linux uchun u haqiqiy
kompyuter, biz uchun esa — har bir "vintini" ko'rish va o'zgartirish mumkin bo'lgan dastur.

| Haqiqiy kompyuterda | Bizning loyihada | Fayl |
|---|---|---|
| protsessor (kremniy) | `struct cpu` + `cpu_qadam()` | `emu/cpu.h`, `emu/cpu.c` |
| buyruq dekoderi | `imm_i()`, `d_rd()` ... | `emu/dekod.c` |
| arifmetik-mantiqiy blok (ALU) | `alu_asosiy()`, `m_amal()` | `emu/alu.c` |
| CSR registrlari | `csr_oqi()`, `csr_yoz()` | `emu/csr.c` |
| trap/uzilish mantig'i | `trap_kirish()`, `uzilish_tekshir()` | `emu/trap.c` |
| MMU va TLB | `mmu_tarjima()` | `emu/mmu.c` |
| shina (bus) | `shina_oqi()`, `shina_yoz()` | `emu/shina.c` |
| taymer (CLINT) | `struct clint` | `emu/clint.c` |
| uzilishlar kontrolleri (PLIC) | `struct plic` | `emu/plic.c` |
| konsol (16550 UART) | `struct uart` | `emu/uart.c` |
| disk | `struct disk` | `emu/disk.c` |
| BIOS/ACPI jadvallari | qurilmalar daraxti (DTB) | `emu/dtb.c` |
| firmware (OpenSBI) | bizning SBI | `firmware/*.c` |
| operatsion tizim | Linux 6.6 (haqiqiy!) | `linux/yig.sh` |

## Qatlamlar: kim kimni chaqiradi

```text
   +-------------------------------------------------------------+
   |  /init (vksh) va /bin/salom     — U rejim (foydalanuvchi)    |   linux/init.c
   +-----------------------------ecall----------------------------+
   |  Linux 6.6 yadrosi              — S rejim (supervisor)       |   build/Image
   +-----------------------------ecall (SBI)----------------------+
   |  bizning firmware               — M rejim (machine)          |   firmware/
   +==============================================================+
   |  EMULYATOR (C dasturi, sizning kompyuteringizda ishlaydi)    |   emu/
   |    cpu_qadam:  uzilish? -> fetch -> decode -> execute         |
   |    mmu  |  csr  |  trap  |  shina -> RAM, UART, CLINT, PLIC, disk |
   +-------------------------------------------------------------+
```

Chiziqning tepasidagi hamma narsa — **RISC-V mashina kodi**. Ular sizning x86 (yoki ARM) protsessoringizda
to'g'ridan-to'g'ri ishlay olmaydi. Emulyator ularni **bitta-bitta o'qib, talqin qiladi** — xuddi Python
interpretatori `.py` faylni talqin qilgandek. Farqi: bizning "til" — protsessor buyruqlari.

Uchta **imtiyoz rejimi** (U, S, M) — 6-bobning mavzusi. Hozircha bitta fikr: har bir qatlam faqat o'zidan
pastdagiga `ecall` buyrug'i bilan "murojaat" qiladi va har biri o'zidan yuqoridagini **himoya qiladi**.

## Loyiha tuzilishi

```text
virtual_kompyuter/
├── Makefile            make, make test, make linux, make linux-test ...
├── emu/                EMULYATOR (≈ 3000 qator C): har fayl — bitta "mikrosxema"
├── firmware/           M rejim dasturi: SBI xizmatlari, yadroni ishga tushirish
├── linux/              Linux'ni yig'ish skripti, sozlamalar, /init (shell)
├── misollar/           kichik namoyish dasturlari (bitlar.c, salom.c)
├── testlar/
│   ├── birlik/         har MASHQ funksiyasini alohida tekshiruvchi testlar (aniq xabar bilan)
│   ├── emu/            assembly testlari: butun protsessor (14 ta)
│   ├── sbi/            firmware testi
│   └── linux/          Linux'ni yuklab, natijani bosqichma-bosqich tekshirish
└── kitob/              shu kitob
```

## Kerakli dasturlar

```console
$ sudo apt install gcc clang lld make                       # emulyator va RISC-V kross-kompilyatsiya
$ sudo apt install flex bison bc rsync curl cpio            # faqat Linux'ni yig'ish uchun (11-bob)
$ sudo apt install device-tree-compiler                     # ixtiyoriy: DTB ni o'qish (9-bob)
```

`clang` bitta o'zi **hamma** protsessorlar uchun kod chiqara oladi: `--target=riscv32` deyish kifoya.
Alohida "RISC-V GCC" o'rnatish shart emas.

## Birinchi ishga tushirish

Avval emulyator va firmware'ni yig'amiz, keyin eng kichik "bare metal" dasturni ishga tushiramiz —
operatsion tizimsiz, libc'siz dastur ([misollar/salom.c](../misollar/salom.c)):

```console
$ make salom
gcc -Wall -Wextra -O2 -g emu/alu.c emu/clint.c ... -o build/vk
clang --target=riscv32 -march=rv32imac_zicsr_zifencei ... misollar/salom.c -o build/salom.elf
./build/vk -s -n 1000000 build/salom.elf
Salom, RISC-V! Men operatsion tizimsiz ishlayapman.
1 + 2 + ... + 100 = 5050
statistika: 116 ta buyruq bajarildi; TLB: 0 topildi, 0 topilmadi (0.0%); disk: 0 o'qish, 0 yozish
```

> **Diqqat (o'quvchi versiyasi):** agar siz mashqlarni hali yozmagan bo'lsangiz, bu buyruq ishlamaydi — chunki
> protsessor buyruqlarni dekodlay olmaydi (E1 hali bo'sh), emulyator `-n` chegarasigacha aylanib to'xtaydi.
> Bu normal holat! Bu dastur uchun 2, 3 va 5-boblarning mashqlari (E1–E3, E7) kerak — shulardan keyin qaytib
> keling. Qaysi bosqichda nima ishlashini [mashqlar.md](mashqlar.md) jadvali ko'rsatadi.

Dastur ichida printf yo'q. Ekranga harf chiqarish — bu **0x10000000 manziliga bayt yozish**:

```c
#define UART_THR ((volatile uint8_t *)0x10000000)   /* yuborish registri: bu yerga yozilgan bayt — ekranga */

static void harf(char c)
{
    *UART_THR = (uint8_t)c;
}
```

Emulyator bu yozuvni ko'rib (`shina_yoz` → `uart_yoz`), baytni o'zining `stdout` iga chiqaradi. Haqiqiy
kompyuterda ham aynan shunday: UART mikrosxemasi shu manzilga "ulangan".

`-t` bayrog'i bilan har bir bajarilgan buyruqni ko'ramiz:

```console
$ ./build/vk -t build/salom.elf 2>&1 | head -8
[M] 80000000: 80100137  lui sp, 0x80100
[M] 80000004: 2011      c.jal ra, 0x80000008
[M] 80000008: 10000537  lui a0, 0x10000
[M] 8000000c: 05300793  addi a5, zero, 83
[M] 80000010: 00f50023  sb a5, 0(a0)
[M] 80000014: 06100613  addi a2, zero, 97
[M] 80000018: 00c50023  sb a2, 0(a0)
[M] 8000001c: 06c00813  addi a6, zero, 108
```

O'qishni o'rganamiz: `[M]` — protsessor M rejimda; `80000000` — buyruq manzili (pc); `80100137` — buyruqning
o'zi (32 bit, hex); qolgani — uning assembly ko'rinishi. `2011` — **4 emas, 2 xonali** hex: bu 16 bitli
"siqilgan" buyruq (5-bob).

Qiziq narsalarni payqadingizmi?

- `83` — `'S'` harfining kodi, `97` — `'a'`, `108` — `'l'`. `sb a5, 0(a0)` — baytni `a0 = 0x10000000` ga yozish.
  Ya'ni `matn("Salom...")` funksiyasidan **tsikl ham, satr ham qolmagan** — clang uni `-O2` da bevosita
  "harfni yoz" buyruqlariga aylantirgan.
- 1 dan 100 gacha yig'indi uchun ham tsikl yo'q: jami **116** ta buyruq bajarildi. Kompilyator `5050` ni
  kompilyatsiya paytida o'zi hisoblab qo'ygan. Emulyatorning `-t` rejimi — kompilyator nima qilganini
  ko'rishning eng halol usuli.

## Mashqlar bilan qanday ishlash

Loyiha **aralash** usulda: kodning ko'p qismi tayyor va **juda batafsil izohlangan** — uni o'qib o'rganasiz.
Eng muhim va qiyin 22 ta joy esa bo'sh qoldirilgan — ularni **o'zingiz yozasiz**. Har bo'sh joy shunday ko'rinadi:

```c
uint32_t imm_b(uint32_t b)
{
    /*
     * TODO(E1c) — O'ZINGIZ YOZING: B-tur immediate (sakrash siljishi): bitlar ARALASHGAN, imm[0] doim 0.
     *   - imm[12] <- 31-bit,  imm[11] <- 7-bit,  imm[10:5] <- 30..25,  imm[4:1] <- 11..8
     *   - har bo'lakni BIT/BITLAR bilan olib, kerakli joyga << bilan qo'ying, OR qiling
     *   - 13 bitli son — ishora_kengaytir(v, 13)
     *   - kitob: 02-bob, 'B va J tur — nega bitlar aralash?'
     * Tekshirish: make test  (birlik testida 'E1' qatori)
     */
    (void)b;
    return 0;
}
```

Bo'sh joy **kompilyatsiya bo'ladi** (vaqtinchalik `return 0`), shuning uchun loyiha har doim yig'iladi —
faqat testlar `[XATO]` deydi. Ish tartibi:

1. Bobni o'qing (shu kitobdan).
2. Bo'sh joyni toping: `grep -rn "TODO(E1" emu/`.
3. Yozing.
4. `make test` — birlik testlari aynan qaysi holat buzilganini ko'rsatadi:

```console
$ make test
== birlik testlari (har mashq funksiyasi alohida) ==
        imm_b(0x80731063)                          bne t1, t2, -4096   -> olindi 0x00000000, kutilgan 0xfffff000
  [XATO] E1 imm_i/s/b/u/j (dekod.c): 12/14
  [ OK ] E2 shart_bajarildimi (alu.c): 10/10
...
```

Bu yerda `olindi` — sizning funksiyangiz qaytargani, `kutilgan` — to'g'ri javob, o'rtadagi matn — test
qaysi buyruqdan olingani (kutilgan qiymatlar **haqiqiy assembler** chiqishidan olingan, qo'lda emas).

5. Hamma birlik testlari `[ OK ]` bo'lgach — assembly testlari (butun protsessor), keyin firmware testi,
   oxirida `make linux-test` (Linux yuklanishi). Har bosqich oldingisiga tayanadi.

> **Eslab qoling:** test o'tmasa — bu **ma'lumot**, jazo emas. Har `[XATO]` qatori sizga aniq bitta kirish
> va bitta to'g'ri javob beradi. Qog'ozda bitlarni chizing, qo'lda hisoblang, keyin kodga qarang.
> Haqiqiy protsessor muhandislari ham aynan shunday ishlaydi: "test vektori" → "farq" → "tuzatish".

## Yechimlar qayerda?

Yo'q — ataylab. Loyihaning to'liq ishlaydigan versiyasi muallifda bor (Linux shu bilan tekshirilgan), lekin
repozitoriyda faqat bo'sh joylar. Sababi oddiy: tayyor yechimni ko'rgan miya uni "tushundim" deb hisoblaydi,
lekin o'zi yoza olmaydi. Testlar esa yechimning **to'g'riligini** to'liq tekshiradi — sizga boshqa hech narsa
kerak emas. Qotib qolsangiz: izohdagi har qadamni qog'ozga yozing, bobdagi misolni qo'lda hisoblang,
`-t` bilan buyruqlarni kuzating.

## Bob tartibi

| Bob | Mavzu | Mashqlar |
|---|---|---|
| [1](01-bitlar.md) | Bitlar bilan gaplashish: maska, siljish, ishora, endianness | — |
| [2](02-buyruqlar.md) | RISC-V buyruqlari va ularni dekodlash | E1 |
| [3](03-alu.md) | ALU: arifmetika, sakrash shartlari, yuklash, ko'paytirish/bo'lish | E2, E3, E4 |
| [4](04-protsessor-sikli.md) | Protsessor sikli: fetch → decode → execute; atomik amallar | E10 |
| [5](05-siqilgan.md) | C kengaytmasi: 16 bitli buyruqlar | E7 |
| [6](06-rejimlar-trap.md) | Imtiyoz rejimlari, CSR, trap, delegatsiya, uzilishlar | E6, E9 |
| [7](07-virtual-xotira.md) | Virtual xotira: Sv32, TLB, A/D bitlar, tekis bo'lmagan murojaat | E5, E8 |
| [8](08-qurilmalar.md) | Qurilmalar: shina, CLINT, PLIC, UART, disk | P1 |
| [9](09-dtb.md) | Qurilmalar daraxti (DTB) | D1, D2, D3 |
| [10](10-firmware.md) | Firmware va SBI | F1, F2, F3 |
| [11](11-linux.md) | Linux'ni yuklash | — (yakuniy imtihon) |
| [Mashqlar](mashqlar.md) | 22 ta mashq jadvali, qiyinlik, tavsiya etilgan tartib, keyingi loyihalar | |

**Asosiy manba:** "The RISC-V Instruction Set Manual" (I jild — Unprivileged, II jild — Privileged) —
<https://riscv.org/technical/specifications/>. Kitobda har bir qoida uchun spetsifikatsiyaning qaysi bo'limiga
qarash kerakligi aytiladi. Ingliz tilini bilmasangiz ham jadvallar va bit chizmalari tushunarli — ular "xalqaro".

## Savol-javob

**Savol:** Emulyator haqiqiy protsessordan necha marta sekin?
**Javob:** Bizning oddiy interpretator sekundiga taxminan 15–20 million buyruq bajaradi; haqiqiy protsessor
— milliardlab. Ya'ni 100 marta sekin. Linux'ni yuklashga (≈ 57 mln buyruq) shunga qaramay 3–4 soniya yetadi.
QEMU kabi emulyatorlar "JIT" (buyruqlarni x86 kodiga tarjima qilish) bilan 10 barobar tezroq — bu [mashqlar.md](mashqlar.md)
dagi mustaqil loyihalardan biri.

**Savol:** Nega RISC-V, x86 emas?
**Javob:** x86 buyruqlari 1 dan 15 baytgacha, minglab turi bor, 40 yillik tarix "qatlamlari" bilan. RISC-V
— toza, ochiq, 2010-yillarda o'quv uchun ham loyihalangan: asosiy to'plam (RV32I) — atigi 40 ta buyruq.
Lekin u **o'yinchoq emas**: Linux, Android, ko'plab mikrokontrollerlar unda ishlaydi.

**Savol:** Bu loyihani tugatsam, nimani bilaman?
**Javob:** Protsessor, MMU, uzilishlar, firmware va yadro chegarasini **o'z qo'lingiz bilan** yasagan
bo'lasiz. Bu — yadro dasturchisi, emulyator/virtualizatsiya (QEMU, KVM), firmware, kompilyator backend,
xavfsizlik (exploit va himoya) yo'nalishlaridagi ishning poydevori.
