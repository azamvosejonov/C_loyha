# 11-bob. Linux'ni yuklash — yakuniy imtihon

> **Bu bobda nima o'rganasiz:** haqiqiy Linux yadrosini o'z kompyuteringiz uchun sozlash va yig'ish
> (`tinyconfig`, `LLVM=1`, kross-kompilyatsiya); libc'siz foydalanuvchi dasturlari (`nolibc`); yadro ichiga
> tikilgan fayl tizimi (initramfs); boot log'ni qatorma-qator "o'qish" — har qator qaysi bobingiz ishi;
> Linux'ni yuklashda uchragan haqiqiy xatolar va ularni qanday topish.
> **Oldindan nima kerak:** 0–10-boblar va **hamma** 22 ta mashq. Darslik 14, 23, 30-boblar.
> **Mashqlar:** yo'q — bu bobning o'zi imtihon: `make linux-test` o'tishi kerak.   **Vaqt:** 2–3 soat (+ yig'ish).

## Bu bob nima haqida?

Hamma birlik testlari o'tdi, assembly testlari o'tdi, firmware testi o'tdi. Endi — yakuniy imtihon: sizning
emulyatoringizda **o'zgartirilmagan** Linux 6.6 yadrosi. Linux — dunyodagi eng ko'p sinalgan dasturlardan biri;
u apparatdan **aniq** spetsifikatsiyani kutadi. Agar u sizning kompyuteringizda yuklanib, jarayon yaratib,
o'chsa — siz yasagan protsessor, MMU, uzilishlar va firmware **haqiqatan to'g'ri**.

## 11.1. Linux'ni yig'ish

```console
$ sudo apt install clang lld make flex bison bc rsync curl cpio
$ make linux          # bir marta: ~140 MB yuklab olish va 5-15 daqiqa yig'ish
...
>>> TAYYOR: .../build/Image   (ishga tushirish: make linux-ishga)
```

[linux/yig.sh](../linux/yig.sh) nima qiladi:

1. **Manba kodini yuklab oladi** — `linux-6.6.50.tar.xz` (kernel.org).
2. **`headers_install`** — foydalanuvchi dasturlari uchun yadro sarlavhalari (`struct new_utsname`,
   tizim chaqiruvi raqamlari).
3. **`/init` va `/bin/salom` ni yig'adi** — libc'siz (11.3).
4. **Sozlaydi:** `tinyconfig` + bizning [linux/vk.config](../linux/vk.config).
5. **Yig'adi:** `make ARCH=riscv LLVM=1 Image`.

`LLVM=1` — Linux'ni GCC o'rniga clang/lld bilan yig'ish. Afzalligi: bitta clang hamma arxitekturalar uchun,
alohida "riscv32-linux-gnu-gcc" kerak emas. `ARCH=riscv` + `CONFIG_ARCH_RV32I=y` — 32 bitli RISC-V.

### `tinyconfig` + `vk.config`

Linux'da ~20 000 ta sozlama bor. `tinyconfig` — eng kichik to'plam (deyarli hammasi o'chiq). Biz ustiga
**faqat kerakli** narsalarni yoqamiz. Har qator — bizning kompyuterimizning bir xususiyati:

```text
CONFIG_ARCH_RV32I=y            # 32 bit                        (2-bob)
CONFIG_MMU=y                   # virtual xotira, Sv32           (7-bob)
CONFIG_SMP=n                   # bitta yadro
CONFIG_FPU=n                   # suzuvchi nuqta yo'q (F/D kengaytmalari)
CONFIG_RISCV_ISA_C=y           # 16 bitli buyruqlar             (5-bob)
CONFIG_RISCV_SBI_V01=y         # eski SBI chaqiruvlari (putchar) (10-bob)
CONFIG_SERIAL_EARLYCON_RISCV_SBI=y   # erta konsol SBI orqali
CONFIG_SERIAL_8250=y           # 16550 UART drayveri            (8-bob)
CONFIG_SERIAL_OF_PLATFORM=y    # ... DTB dan topiladigan         (9-bob)
CONFIG_SIFIVE_PLIC=y           # PLIC                           (8-bob)
CONFIG_RISCV_TIMER=y           # taymer — SBI set_timer orqali   (10-bob)
CONFIG_BLK_DEV_INITRD=y        # yadro ichidagi fayl tizimi      (11.4)
CONFIG_CMDLINE="earlycon=sbi console=ttyS0 loglevel=7"
```

`vk.config` dagi har qatorning izohini o'qing — bu Linux qanday "qismlardan" yig'ilishining eng qisqa xaritasi.

### Yig'ishda uchragan muammo: `screen_info`

Birinchi yig'ishda bog'lovchi (linker) xato berdi: `undefined symbol: screen_info`. Bu — "virtual terminal"
(VT, ekran va klaviatura konsoli) kodi: u x86 PC'dagi ekranni nazarda tutadi va RISC-V da o'sha belgi
aniqlanmagan. Bizda ekran yo'q, faqat UART. Yechim — keraksiz qismlarni o'chirish:

```text
CONFIG_VT=n
CONFIG_DUMMY_CONSOLE=n
```

Saboq: katta loyihada "nega bu kod umuman yig'ilyapti?" degan savol ko'pincha xatoning yechimi.

## 11.2. Ishga tushirish

```console
$ make linux-ishga
```

Interaktiv shell ochiladi (`help` — buyruqlar). Chiqish — `poweroff`. Skriptli tekshiruv:

```console
$ make linux-test
  [ OK ] firmware ishga tushdi (M rejim)
  [ OK ] Linux yadrosi S rejimda gapirdi
  [ OK ] qurilmalar daraxti o'qildi (DTB)
  [ OK ] ISA kengaytmalari tanildi
  [ OK ] taymer ishlaydi (sched_clock, jiffies)
  [ OK ] UART konsoli (16550, PLIC orqali)
  [ OK ] init (pid 1) ishga tushdi
  [ OK ] shell buyrug'i bajarildi (echo)
  [ OK ] fork + execve + waitid
  [ OK ] /proc/cpuinfo: ISA satri
  [ OK ] poweroff (SBI SRST)
  [ OK ] Linux to'liq ishladi: yuklandi, buyruqlarni bajardi, o'chdi
```

[testlar/linux/tekshir.sh](../testlar/linux/tekshir.sh) chiqishni **bosqichma-bosqich** tekshiradi. Qaysi
bosqichda to'xtagani — xato qayerdaligiga ishora: har `[XATO]` qatori ostida "qarang:" bilan mashq raqamlari
yoziladi.

## 11.3. Libc'siz dasturlar: `nolibc`

Bizning initramfs'da glibc yo'q (u megabaytlab). Linux manba kodida `tools/include/nolibc/` — bitta sarlavha
faylidan iborat **mini-libc** bor: `printf`, `open`, `fork` ... — hammasi to'g'ridan-to'g'ri `ecall` bilan.
Masalan `write` RISC-V da (soddalashtirilgan):

```c
register long a7 __asm__("a7") = __NR_write;   /* 64 */
register long a0 __asm__("a0") = fd;
register long a1 __asm__("a1") = (long)buf;
register long a2 __asm__("a2") = count;
__asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory");
```

Bu — 6-bobdagi U → S trap. `a7` — tizim chaqiruvi raqami, natija `a0` da. Darslik 14-bobidagi tizim
chaqiruvlari aynan shu mexanizmda.

[linux/init.c](../linux/init.c) — `pid 1` bo'lib ishlaydigan oddiy shell (`vksh`). Uni o'qing — har buyruq
bitta-ikkita tizim chaqiruvi: `ls` → `getdents64`, `cat` → `open`/`read`/`write`, `run` → `fork` + `execve` +
`waitid`, `poweroff` → `reboot`.

### Uchta haqiqiy muammo

**1. Clang `printf` ni `puts` ga aylantirdi.** `printf("salom\n")` ni kompilyator "optimallashtirib"
`puts("salom")` qiladi — lekin nolibc'da `puts` yo'q (yoki boshqacha) → bog'lovchi xatosi. Yechim:
`-fno-builtin` — "standart funksiyalar haqidagi bilimingni ishlatma".

**2. `wait4` yo'q.** nolibc'ning `waitpid()` i ichida `wait4` tizim chaqiruvini ishlatadi — lekin **rv32**
Linux'da `__NR_wait4` umuman yo'q. Sabab: eski `wait4` olib tashlangan (32 bitli tizimlarda 2038-yil muammosi tufayli vaqt tuzilmalari
o'zgartirilgan, eski chaqiruvlar yangi arxitekturalarga qo'shilmagan). Faqat `waitid` bor:

```c
/* bolani kutish. DIQQAT: rv32 Linux'da eski wait4 tizim chaqiruvi YO'Q (64 bitli vaqtga o'tishda olib
   tashlangan) — faqat waitid bor. Chiqish kodi siginfo ichida (si_status). */
static int kut(pid_t p)
{
    siginfo_t s;
    memset(&s, 0, sizeof(s));
    if (my_syscall5(__NR_waitid, P_PID, p, &s, WEXITED, 0) < 0)
        return -1;
    return s.si_status;
}
```

**3. nolibc'da `snprintf`, `strcat`, `uname` yo'q.** Kichik libc — kichik imkoniyat. `uname` uchun yadroning
o'z tuzilmasi (`struct new_utsname`) va to'g'ridan-to'g'ri `my_syscall1(__NR_uname, &u)`.

## 11.4. initramfs — yadro ichidagi fayl tizimi

Diskimiz uchun Linux drayveri yo'q. Fayllar qayerdan? Yadro **ichiga tikilgan** arxivdan (cpio formati):

```text
# initramfs.list — gen_init_cpio formati
dir /dev 0755 0 0
nod /dev/console 0600 0 0 c 5 1
dir /proc 0755 0 0
dir /sys 0755 0 0
dir /bin 0755 0 0
dir /tmp 0755 0 0
file /init ../../build/init 0755 0 0
file /bin/salom ../../build/salom 0755 0 0
```

Yuklanish oxirida yadro arxivni xotiradagi fayl tizimiga (`tmpfs`) ochadi va `/init` ni `pid 1` sifatida
ishga tushiradi:

```text
[    3.079187] Run /init as init process
```

`/dev/console` — `c 5 1` (belgili qurilma, asosiy raqam 5, kichik 1): `/init` ning `stdin`/`stdout`i.

## 11.5. Boot log — har qator sizning ishingiz

```text
[vk-sbi] Virtual kompyuter firmware'i (SBI v2.0), M rejim           ← 10-bob: boshlash.S, firmware_asosiy
[vk-sbi] hart 0, DTB 0x83ff0000, yadro 0x80400000                   ← 9-bob: dtb_joylash (RAM oxiri - 64 KB)
[vk-sbi] yadroga o'tyapman (mret -> S rejim)                        ← F3, 6-bob: mret

[    0.000000] Linux version 6.6.50 ...                             ← Linux'ning 1-buyrug'i S rejimda; MMU (7-bob)
[    0.000000] OF: fdt: Ignoring memory range 0x80000000 - 0x80400000 ← firmware zonasi
[    0.000000] Machine model: virtual-kompyuter,rv32                 ← D1–D3: DTB to'g'ri o'qildi
[    0.000000] SBI specification v2.0 detected                       ← 10.4: BASE kengaytmasi
[    0.000000] SBI implementation ID=0x564b Version=0x1              ←   "VK"
[    0.000000] earlycon: sbi0 at I/O port 0x0 (options '')           ← SBI putchar orqali konsol
[    0.000000] riscv: base ISA extensions acim                       ← 9.3: riscv,isa-extensions
[    0.000000] Memory: 58728K/61440K available ...                   ← 64 MB - firmware - DTB
[    0.000000] riscv-intc: 32 local interrupts mapped                ← 6.7: mip/mie
[    0.000000] plic: plic@c000000: mapped 31 interrupts ...          ← 8.3: PLIC
[    0.000022] sched_clock: 64 bits at 10MHz, resolution 100ns ...   ← 8.2: timebase-frequency
[    0.039001] ASID allocator using 9 bits (512 entries)             ← 7.6: satp.ASID
[    0.120905] cpu0: ... unaligned accesses are fast                 ← 7.7: E8 (apparat bajaradi)
[    0.166178] clocksource: Switched to clocksource riscv_clocksource ← F1, F2, E9: taymer uzilishlari ishlaydi
[    2.988274] 10000000.serial: ttyS0 at MMIO 0x10000000 (irq = 2, base_baud = 230400) is a 16550A   ← 8.5
[    2.991937] printk: console [ttyS0] enabled                       ← UART drayveri + PLIC + SEI (P1, E9)
[    3.079187] Run /init as init process                             ← 11.4

Salom! Linux ishga tushdi. Men — init (pid 1), oddiy shell. 'help' — buyruqlar.   ← U rejim! ecall → S
vk:/# run /bin/salom bir ikki                                        ← UART RX uzilishi (8.5, E9, P1)
Men alohida jarayonman: pid 16, ota 1, argumentlar: 3 ta             ← fork: COW sahifa xatolari (E5, E6), ASID
...
[pid 16 tugadi, kod 7]                                               ← waitid
vk:/# poweroff
[    5.345223] reboot: Power down                                    ← SBI SRST (10.4)

[vk-sbi] yadro tizimni o'chirishni so'radi
```

Va statistika (`-s`):

```text
statistika: 56999744 ta buyruq bajarildi; TLB: 63337119 topildi, 767363 topilmadi (98.8%); disk: 0 o'qish, 0 yozish
```

57 million buyruq — sizning `cpu_qadam` ingiz shuncha marta muvaffaqiyatli ishladi.

## 11.6. Linux yuklanmasa — qanday qidirish

Linux'ni yuklashda uchragan haqiqiy xatolar kitob bo'ylab tarqalgan. Ularning hammasi uchun ish uslubi bir xil:

| Belgi | Avval qarang | Bob |
|---|---|---|
| hech narsa chiqmaydi | `-t -n 100000` bilan trace: firmware ishlayaptimi, `mret` bormi, `~~` trap'lar | 4, 10 |
| `[vk-sbi]` bor, Linux matni yo'q | `mret` dan keyingi trace; F3; SBI putchar | 10 |
| `Oops - illegal instruction`, `epc : funksiya+0x..` | `mtval`/`stval` dagi buyruqni dekodlang (1-bob) | 2, 3, 5, 6 |
| `Unable to handle kernel paging request` | MMU: E5, A/D bitlar, TLB, ASID | 7 |
| taymer satrida qotadi | F1, F2, E9, `mip`/`mie` | 6, 10 |
| UART yoqilganda qotadi yoki uzilishlar "bo'roni" | P1, `mip.SEIP` qoidasi, UART IIR | 6.9, 8 |
| `Run /init` dan keyin hech narsa | U rejim: E6 (ecall U→S), sahifa xatolari, E10 | 4, 6, 7 |
| `fork` da qotadi | sahifa xatosi sababi (12/13/15), AMO yozish xatosi | 4.7, 7 |

Uchta umumiy qoida:

1. **Determinizm — eng katta qurolingiz.** Xato har safar **aynan bir xil** buyruqda chiqadi. `-n` bilan
   o'sha joydan **biroz oldin** to'xtating, `-t` bilan oxirgi minglab buyruqlarni oling.
2. **Linux sizga gapiradi.** `Oops` xabari: `epc` (xato joyi, funksiya nomi bilan), `cause`, `badaddr`
   (stval), barcha registrlar. Funksiya nomini Linux manbasida (`build/linux-6.6.50/`) `grep` qiling.
3. **Spetsifikatsiyani qatorma-qator o'qing.** "Taxminan to'g'ri" emulyator Linux'ni yuklamaydi (6.9).

## 11.7. Tabriklayman

Agar `make linux-test` sizning kodingiz bilan o'tsa — siz:

- 32 bitli protsessorning buyruq dekoderi, ALU, M, A va C kengaytmalarini;
- imtiyoz rejimlari, CSR, trap va uzilishlar mantig'ini;
- Sv32 MMU va TLB ni;
- PLIC uzilishlar kontrollerini va DTB generatorini;
- SBI firmware'ning taymer va yuklash qismini

**o'zingiz yozdingiz**, va dunyodagi eng murakkab dasturlardan biri ularni "haqiqiy" deb qabul qildi.

Keyingi qadamlar — [mashqlar.md](mashqlar.md) oxiridagi mustaqil loyihalar: disk drayveri, ko'p yadro (SMP),
JIT, gipervizor kengaytmasi... Va albatta — asosiy loyihadagi MyOS yadrosini RISC-V'ga ko'chirish va **o'z
emulyatoringizda** ishga tushirish.
