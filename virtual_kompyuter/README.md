# Virtual kompyuter: RISC-V emulyatori — Linux yuklanadigan darajada

C tilida **noldan** yozilgan to'liq kompyuter: RV32IMAC protsessor (M/S/U rejimlar, Sv32 MMU, TLB), CLINT
taymeri, PLIC uzilishlar kontrolleri, 16550 UART, DMA disk, qurilmalar daraxti (DTB) generatori va o'z SBI
firmware'imiz. Uning ustida **haqiqiy, o'zgartirilmagan Linux 6.6** yuklanadi, shell ochadi, jarayon yaratadi
va o'chadi.

```text
[vk-sbi] Virtual kompyuter firmware'i (SBI v2.0), M rejim
[    0.000000] Linux version 6.6.50 ...
[    0.000000] Machine model: virtual-kompyuter,rv32
...
Salom! Linux ishga tushdi. Men — init (pid 1), oddiy shell. 'help' — buyruqlar.
vk:/# run /bin/salom bir ikki
Men alohida jarayonman: pid 16, ota 1, argumentlar: 3 ta
```

Loyiha **o'quv** uchun: kodning har qatori o'zbek tilida batafsil izohlangan, yonida
[kitob](kitob/00-kirish.md) (12 bob) bor, va eng muhim **22 ta joy bo'sh** — ularni siz yozasiz
([mashqlar jadvali](kitob/mashqlar.md)). Testlar har bir mashqni alohida va aniq xabar bilan tekshiradi.

## Tez boshlash

```console
$ sudo apt install gcc clang lld make           # Linux uchun yana: flex bison bc rsync curl cpio
$ make                  # emulyator (build/vk) va firmware (build/firmware.elf)
$ make test             # hamma testlar: hozir [XATO] — mashqlar bo'sh
$ make bitlar           # 1-bob namoyishi
```

Keyin [kitob/00-kirish.md](kitob/00-kirish.md) dan boshlang.

## Buyruqlar

| Buyruq | Nima qiladi |
|---|---|
| `make` | emulyator va firmware'ni yig'ish |
| `make test` | birlik testlari (har mashq alohida) + 14 ta assembly testi + firmware (SBI) testi |
| `make bitlar` | bitlar bilan ishlash namoyishi (1-bob) |
| `make salom` | eng kichik "bare metal" dastur (0-bob) |
| `make linux` | Linux 6.6 ni yuklab olib yig'ish (bir marta, 5–15 daqiqa) |
| `make linux-ishga` | Linux'ni interaktiv ishga tushirish (`poweroff` — chiqish) |
| `make linux-test` | Linux'ni skriptli kirish bilan yuklab, natijani bosqichma-bosqich tekshirish |
| `make toza` | yig'ilgan fayllarni o'chirish |

Emulyatorning o'zi: `./build/vk` (bayroqlar ro'yxati — argumentsiz ishga tushiring). Eng foydalisi:
`-t` (har buyruq va trap'ni chop etish), `-n N` (N qadamdan keyin to'xtash), `-s` (statistika).

## Tuzilishi

```text
emu/        emulyator: cpu, dekod, alu, siqilgan (C), csr, trap, mmu, shina, clint, plic, uart, disk, dtb, elf, disasm
firmware/   M rejim: boshlash.S, trap.S, SBI xizmatlari, yadroni ishga tushirish
linux/      yig.sh, vk.config (Linux sozlamalari), init.c (pid 1 shell), salom.c, initramfs.list
misollar/   bitlar.c (1-bob), salom.c (0-bob)
testlar/    birlik/ (mashqlar), emu/ (assembly), sbi/ (firmware), linux/ (yakuniy)
kitob/      12 bob + mashqlar jadvali
```

## Kitob

| Bob | Mavzu |
|---|---|
| [0](kitob/00-kirish.md) | Kompyuterni dasturda yasaymiz |
| [1](kitob/01-bitlar.md) | Bitlar bilan gaplashish |
| [2](kitob/02-buyruqlar.md) | RISC-V buyruqlari va dekodlash |
| [3](kitob/03-alu.md) | ALU: arifmetika, shartlar, ko'paytirish/bo'lish |
| [4](kitob/04-protsessor-sikli.md) | Protsessor sikli va atomik amallar |
| [5](kitob/05-siqilgan.md) | C kengaytmasi: 16 bitli buyruqlar |
| [6](kitob/06-rejimlar-trap.md) | Imtiyoz rejimlari, CSR, trap, uzilishlar |
| [7](kitob/07-virtual-xotira.md) | Virtual xotira: Sv32, TLB, ASID |
| [8](kitob/08-qurilmalar.md) | Qurilmalar: shina, CLINT, PLIC, UART, disk |
| [9](kitob/09-dtb.md) | Qurilmalar daraxti (DTB) |
| [10](kitob/10-firmware.md) | Firmware va SBI |
| [11](kitob/11-linux.md) | Linux'ni yuklash — yakuniy imtihon |
| [Mashqlar](kitob/mashqlar.md) | 22 ta mashq, tartib, mustaqil loyihalar |

## Yechimlar haqida

Repozitoriyda yechimlar **yo'q** — ataylab. Loyihaning to'liq versiyasi muallif tomonidan yozilgan va
sinalgan (hamma testlar va Linux yuklanishi), lekin bu yerda faqat bo'sh joylar va ko'rsatmalar. Testlar
yechimingizning to'g'riligini to'liq tekshiradi.
