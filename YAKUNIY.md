# YAKUNIY QO'LLANMA — MyOS'ni haqiqiy kompyuterda ishlatish, kodni tushunish va kengaytirish

> Bu hujjat loyihaning **oxirgi** qo'llanmasi. Unda hamma narsa bitta joyda:
>
> - **I qism** — MyOS'ni haqiqiy kompyuterga qanday o'rnatish (USB fleshka yoki mavjud Linux'ning GRUB'i orqali).
> - **II qism** — yuklangandan keyin tizimdan qanday foydalanish.
> - **III qism** — C kodidagi har bir belgi (`;` `.` `->` `*` `&` `#` ...) nima qiladi va nega kerak.
> - **IV qism** — tizimni qanday kengaytirish: to'liq misollar, keyin **"Endi o'zing davom et"** vazifalari va maslahatlar.
> - **V qism** — qanday ishlash kerak: kuchli yadro dasturchisi bo'lish yo'li.
>
> O'qish tartibi: [QOLLANMA.md](QOLLANMA.md) → `docs/00`–`15` → [labs/README.md](labs/README.md) → **shu hujjat**.

---

## Mundarija

- [I qism. Haqiqiy kompyuterda ishga tushirish](#i-qism-haqiqiy-kompyuterda-ishga-tushirish)
  - [1. Nima kerak](#1-nima-kerak)
  - [2. Haqiqiy apparatda nima ishlaydi, nima ishlamaydi](#2-haqiqiy-apparatda-nima-ishlaydi-nima-ishlamaydi)
  - [3. Xavfsizlik: disklaringizni himoya qilish](#3-xavfsizlik-disklaringizni-himoya-qilish)
  - [4. 1-qadam: yig'ish va QEMU'da "fleshka" sifatida sinash](#4-1-qadam-yigish-va-qemuda-fleshka-sifatida-sinash)
  - [5. 2-qadam: ISO'ni USB fleshkaga yozish](#5-2-qadam-isoni-usb-fleshkaga-yozish)
  - [6. 3-qadam: BIOS/UEFI sozlamalari](#6-3-qadam-biosuefi-sozlamalari)
  - [7. 4-qadam: yuklash va ekrandagi yozuvlarni o'qish](#7-4-qadam-yuklash-va-ekrandagi-yozuvlarni-oqish)
  - [8. Muqobil yo'l: fleshkasiz, Linux'ning GRUB menyusidan](#8-muqobil-yol-fleshkasiz-linuxning-grub-menyusidan)
  - [9. Haqiqiy diskda ext2 bilan ishlash](#9-haqiqiy-diskda-ext2-bilan-ishlash)
  - [10. Muammolar va yechimlar](#10-muammolar-va-yechimlar)
- [II qism. Tizimdan foydalanish](#ii-qism-tizimdan-foydalanish)
- [III qism. C kodidagi har bir belgi nima qiladi](#iii-qism-c-kodidagi-har-bir-belgi-nima-qiladi)
- [IV qism. Tizimni kengaytirish — "o'zing davom et"](#iv-qism-tizimni-kengaytirish--ozing-davom-et)
- [V qism. Qanday ishlash kerak: kuchli yadro dasturchisi bo'lish yo'li](#v-qism-qanday-ishlash-kerak-kuchli-yadro-dasturchisi-bolish-yoli)

---

# I qism. Haqiqiy kompyuterda ishga tushirish

## 1. Nima kerak

| Narsa | Talab | Nega |
|---|---|---|
| Kompyuter | 64-bitli x86 (Intel yoki AMD), taxminan 2008-yildan keyingi | Yadro `x86-64` uchun yozilgan: `syscall`, NX bit, APIC kerak |
| RAM | kamida 256 MB | Yadro, dasturlar va root fayl tizimi (tmpfs) RAM'da turadi |
| USB fleshka | kamida 64 MB. **Undagi hamma narsa o'chadi!** | ISO fleshkaga "xom" holda (bayt-baybayt) yoziladi |
| Yig'ish kompyuteri | Linux (Ubuntu/Debian) yoki Windows'da WSL | `make` shu yerda ishlaydi (README → "Tez boshlash") |
| Klaviatura | noutbukning o'z klaviaturasi yoki PS/2 klaviatura | Drayver hozircha faqat PS/2 (2-bo'limga qarang) |

**Birinchi marta uchun eng yaxshi tanlov — eski noutbuk yoki ikkinchi kompyuter.** Unda yo'qotadigan narsangiz yo'q, noutbukning ichki klaviaturasi esa deyarli har doim PS/2 orqali ulangan.

## 2. Haqiqiy apparatda nima ishlaydi, nima ishlamaydi

QEMU — "ideal" kompyuter. Haqiqiy apparat esa har xil, shuning uchun nimaga tayanish mumkinligini oldindan biling:

| Qism | Holat | Izoh |
|---|---|---|
| Yuklash (BIOS/Legacy va UEFI) | ✅ ishlaydi | `grub-mkrescue` ISO'ga ikkala yuklovchini ham qo'yadi |
| Ekran | ✅ ishlaydi | GRUB beradigan framebuffer (24/32 bpp). BIOS rejimida framebuffer bo'lmasa, VGA matn rejimi ishlatiladi |
| Ko'p yadroli CPU (SMP) | ✅ 64 tagacha | ACPI MADT jadvalidan topiladi |
| Taymer | ✅ | LAPIC. PIT soati o'chirilgan kompyuterlarda CPUID'dan olinadigan zaxira chastota ishlatiladi |
| Noutbukning ichki klaviaturasi | ✅ odatda ishlaydi | Ko'pchilik noutbuklarda ichki klaviatura ichki PS/2 (i8042) kontrolleri orqali ulangan |
| PS/2 klaviatura (dumaloq ulagich) | ✅ ishlaydi | |
| **USB klaviatura** | ⚠️ faqat "Legacy USB" bilan | BIOS **Legacy/CSM** rejimida yuklanganda, BIOS'ning "USB Legacy Support" funksiyasi USB klaviaturani PS/2 ga o'xshatib ko'rsatadi. Sof UEFI rejimida USB klaviatura **ishlamaydi**: xHCI drayveri hali yo'q (IV qism, 14-loyiha) |
| SATA disk (AHCI) | ✅ o'qish + yozish | BIOS'da SATA rejimi **AHCI** bo'lishi kerak, "RAID/Intel RST" emas |
| IDE / "SATA Legacy" | ✅ ATA PIO | |
| **NVMe SSD** | ❌ yo'q | Zamonaviy noutbuklarning ko'pida NVMe bor. Drayveri — IV qism, 15-loyiha |
| USB fleshkani disk sifatida ishlatish | ❌ yo'q | Fleshkadan faqat **yuklanadi**: fayllarni GRUB RAM'ga yuklab beradi, keyin fleshka kerak emas |
| Tarmoq, ovoz, sichqoncha, Wi-Fi | ❌ yo'q | Tarmoq — IV qism, 17-loyiha |
| O'chirish / qayta yuklash | ✅ | ACPI S5 (`poweroff`), klaviatura kontrolleri yoki ACPI orqali qayta yuklash (`reboot`) |

**Muhim tushuncha:** tizim to'liq **RAM'dan** ishlaydi. GRUB `kernel.elf` va `initrd.tar` ni xotiraga yuklaydi, yadro esa tar arxivni tmpfs ildiziga (`/`) ochadi. `/` da qilgan o'zgarishlaringiz qayta yuklashdan keyin **yo'qoladi**. Doimiy saqlash uchun ext2 disk kerak (9-bo'lim).

## 3. Xavfsizlik: disklaringizni himoya qilish

Yadro disklarga **yozadi**. Ehtiyot choralari:

1. `rootfs/etc/rc` yuklanishda `/dev/sda1` ni `/mnt` ga ulashga harakat qiladi. `sda` — MyOS topgan **birinchi** SATA/IDE disk. Haqiqiy kompyuterda bu sizning asosiy diskingiz bo'lishi mumkin!
2. Qanday himoya bor: ext2 drayveri tushunmaydigan "INCOMPAT" xususiyatli fayl tizimini ulashdan **bosh tortadi**. Linux'ning standart **ext4** tizimida `extents`, `64bit`, `flex_bg` bor, shuning uchun u ulanmaydi. Windows'ning NTFS'i, FAT va boshqalar ext2 emas, ular ham ulanmaydi.
3. Qolgan xavf: agar birinchi bo'lim **ext2 yoki ext3** bo'lsa, u ulanadi va unga yozish mumkin bo'ladi.
4. Tavsiya: birinchi sinovlarda `rootfs/etc/rc` dagi `mount` qatorini `#` bilan izohga aylantiring, keyin `make` qiling. Diskni o'zingiz, bilib turib `mount` qilasiz.
5. Muhim ma'lumotlaringiz bo'lsa — zaxira nusxa. Har doim.

## 4. 1-qadam: yig'ish va QEMU'da "fleshka" sifatida sinash

```bash
make                     # build/myos.iso  (BIOS + UEFI)
make test                # hamma testlar o'tishiga ishonch hosil qiling
```

`grub-mkrescue` **gibrid** ISO yaratadi: uni CD sifatida ham, oddiy disk (fleshka) sifatida ham yuklash mumkin. Fleshkaga yozishdan oldin xuddi fleshkadagidek sinab ko'ring: ISO'ni CD emas, **disk** qilib bering.

```bash
# BIOS kompyuter, fleshkadan yuklangandek:
qemu-system-x86_64 -m 512M -smp 4 -drive file=build/myos.iso,format=raw

# UEFI kompyuter, fleshkadan yuklangandek:
qemu-system-x86_64 -m 512M -smp 4 -drive file=build/myos.iso,format=raw \
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd
```

(OVMF fayli distributivga qarab `OVMF_CODE.fd` deb nomlanishi mumkin: `ls /usr/share/OVMF/`.)

Ikkalasida ham `myos:/$` taklifi (prompt) chiqsa, ISO tayyor.

## 5. 2-qadam: ISO'ni USB fleshkaga yozish

ISO fleshkaga **bayt-baybayt** ("DD rejimi") yozilishi kerak. Faylni oddiy nusxalash ishlamaydi.

### Linux

```bash
lsblk                    # fleshkani qo'ying va nomini toping (masalan sdb, 8-16 GB hajmli)
                         # DIQQAT: sda odatda SIZNING ASOSIY DISKINGIZ! Adashmang!
sudo umount /dev/sdX*    # agar avtomatik ulangan bo'lsa (X o'rniga harf)
sudo dd if=build/myos.iso of=/dev/sdX bs=4M status=progress conv=fsync
sync
```

- `if=` — kirish fayli (input file), `of=` — chiqish (output file). `of=` ga **bo'lim emas, butun disk** (`/dev/sdb`, `/dev/sdb1` emas).
- `conv=fsync` — tugatishdan oldin hamma narsa haqiqatan yozilganiga ishonch hosil qiladi.
- `of=` ni xato yozish diskdagi ma'lumotni yo'q qiladi. Ikki marta tekshiring.

### Windows

1. [Rufus](https://rufus.ie) ni oching va fleshkani tanlang.
2. "SELECT" → `myos.iso`.
3. Rufus so'raganda **"Write in DD Image mode"** ni tanlang ("ISO Image mode" emas!).
4. START.

Muqobil: **balenaEtcher** — u doim DD rejimida yozadi.

### macOS

```bash
diskutil list                                  # fleshka: /dev/diskN
diskutil unmountDisk /dev/diskN
sudo dd if=build/myos.iso of=/dev/rdiskN bs=4m # rdisk - tezroq
```

## 6. 3-qadam: BIOS/UEFI sozlamalari

Kompyuter yoqilishi bilan sozlash tugmasini bosib turing:

| Ishlab chiqaruvchi | Sozlamalar | Bir martalik yuklash menyusi |
|---|---|---|
| Dell | F2 | F12 |
| HP | Esc yoki F10 | F9 |
| Lenovo | F1 / F2 (yoki "Novo" tugmasi) | F12 |
| ASUS | F2 yoki Del | F8 yoki Esc |
| Acer | F2 | F12 |
| MSI, Gigabyte, ASRock (stol kompyuter) | Del | F11 / F12 |

Sozlanadigan narsalar:

| Sozlama | Qiymat | Nega |
|---|---|---|
| **Secure Boot** | **Disabled** | Secure Boot faqat Microsoft (yoki ishlab chiqaruvchi) imzolagan yuklovchilarga ruxsat beradi. Bizning GRUB va yadro imzolanmagan |
| SATA Mode | **AHCI** | "RAID On" / "Intel RST" rejimida AHCI drayveri diskni ko'rmaydi |
| USB Legacy Support | Enabled | Faqat USB klaviatura bilan, Legacy rejimida kerak |
| CSM / Legacy Boot | Klaviaturangizga qarab | USB klaviatura bo'lsa — CSM ni yoqib, **Legacy** rejimida yuklang. Noutbukning ichki klaviaturasi bo'lsa — UEFI ham bo'ladi |
| Fast Boot | Disabled | Aks holda USB qurilmalar boshlanmasdan o'tib ketishi mumkin |

Keyin bir martalik yuklash menyusini (F12 ...) oching va fleshkani tanlang. Menyuda fleshka ikki marta ko'rinishi mumkin: `UEFI: SanDisk...` va `SanDisk...`. Birinchisi UEFI rejimi, ikkinchisi Legacy.

## 7. 4-qadam: yuklash va ekrandagi yozuvlarni o'qish

`grub.cfg` da `timeout=0`, shuning uchun GRUB menyusi ko'rinmaydi va yadro darhol yuklanadi. Ekranda yadro jurnali (log) chiqadi. Har bir qator bitta qatlamning ishga tushganini bildiradi:

```text
[cpu]  GenuineIntel "..."  NX:ha APIC:ha ...        <- CPU imkoniyatlari (CPUID)
[boot] Yuklovchi: GRUB 2.xx                         <- Multiboot2 ma'lumotlari
[boot] Framebuffer: 1920x1080, 32 bpp ...           <- ekran
[memblock] ... jami 7890 MB                         <- RAM xaritasi (haqiqiy RAM hajmingiz)
[pmm]  Buddy: ...                                   <- sahifa allocatori tayyor
[slab] 12 ta kmalloc keshi ...                      <- kmalloc tayyor
[acpi] CPU'lar: 8, IOAPIC: 1, LAPIC @ ..., S5: topildi  <- ACPI jadvallari o'qildi
[tsc]  TSC chastotasi: 2893.221 MHz                 <- vaqt kalibrlandi
[apic] LAPIC taymeri: ... tik/ms                    <- jarayonlarni almashtirish taymeri
[pci]  23 ta qurilma:                               <- PCI qurilmalar ro'yxati
[ahci] ... / [ata] ...                              <- disk kontrollerlari
[blk]  sda: 476 GB "Samsung SSD ..."                <- disklar va bo'limlar
[initrd] ... -> tmpfs (/)                           <- /bin, /etc RAM'ga ochildi
[smp]  8 ta CPU ishlayapti (BSP + 7 AP)             <- qolgan yadrolar ishga tushdi
[int]  Uzilishlar yoqilmoqda ...
myos:/$                                             <- shell tayyor!
```

**Qaysi qatorda to'xtagan bo'lsa, muammo keyingi qatlamda.** Masalan, `[tsc]` chiqib, `[apic]` chiqmasa, APIC'ni tekshirish kerak. Telefon bilan ekranni suratga oling — bu haqiqiy apparatda eng asosiy debug vositasi.

Birinchi buyruqlar:

```sh
uname -a       # tizim haqida
free           # xotira: haqiqiy RAM hajmini ko'rasiz
lspci          # kompyuteringizdagi haqiqiy qurilmalar!
dmesg | head   # yadro jurnali
ps
```

`lspci` chiqishini saqlab qo'ying: keyingi drayveringizni (NVMe, xHCI, tarmoq) shu ro'yxatdan tanlaysiz.

## 8. Muqobil yo'l: fleshkasiz, Linux'ning GRUB menyusidan

Kompyuteringizda Linux o'rnatilgan bo'lsa, MyOS'ni **uning GRUB menyusiga** qo'shish mumkin. Fleshka kerak emas va har `make` dan keyin fayllarni nusxalash kifoya. Tez tajriba uchun eng qulay yo'l shu.

```bash
make
sudo mkdir -p /boot/myos
sudo cp build/kernel.elf build/initrd.tar /boot/myos/
sudo nano /etc/grub.d/40_custom
```

Faylning oxiriga qo'shing:

```text
menuentry "MyOS" {
    insmod all_video
    insmod part_gpt
    insmod part_msdos
    insmod ext2
    search --no-floppy --file --set=root /boot/myos/kernel.elf
    multiboot2 /boot/myos/kernel.elf
    module2 /boot/myos/initrd.tar initrd
    boot
}
```

Keyin:

```bash
# /etc/default/grub da: GRUB_TIMEOUT_STYLE=menu  va  GRUB_TIMEOUT=5  (menyu ko'rinsin)
sudo update-grub                                   # Ubuntu/Debian
# sudo grub2-mkconfig -o /boot/grub2/grub.cfg      # Fedora
```

Qayta yuklab, menyuda "MyOS" ni tanlang.

- **Secure Boot o'chirilgan bo'lishi shart**: Secure Boot yoqilganda GRUB "lockdown" rejimida `multiboot2` ni rad etadi.
- `search --file` MyOS fayllarini qaysi bo'limda bo'lsa ham topadi (`/boot` alohida bo'lim bo'lsa ham). `insmod ext2` modulini GRUB ext2/ext3/ext4 hammasi uchun ishlatadi.
- `module2 ... initrd` dagi oxirgi so'z — modulning nomi (jurnalda `[boot] Modul: ... "initrd"` bo'lib ko'rinadi). Yadro (`fs/initrd.c`) **birinchi** modulni tar arxiv sifatida ochadi, shuning uchun `module2` qatori bitta bo'lsin.
- Yadroga parametr berish: `multiboot2 /boot/myos/kernel.elf selftest` (II qismga qarang).

## 9. Haqiqiy diskda ext2 bilan ishlash

Doimiy saqlash uchun alohida disk yoki bo'sh bo'lim oling (ikkinchi SATA disk juda qulay). Linux'da:

```bash
sudo mkfs.ext2 -L myos /dev/sdY2    # sdY2 - BO'SH bo'lim! (mkfs undagi hammasini o'chiradi)
```

`mkfs.ext2` ning standart sozlamalari (4 KB blok, 256 baytli inode) tekshirilgan: MyOS o'qiydi, yozadi, `e2fsck` xato topmaydi. **ext4 emas, aynan ext2** bo'lishi kerak.

MyOS'da disklarning nomlari **o'zining** topish tartibida beriladi (Linux'dagi bilan bir xil bo'lmasligi mumkin):

```sh
ls /dev                         # sda, sda1, sda2, sdb ... qaysilari borligini ko'ring
dmesg | grep blk                # "[blk] sda: 476 GB ..." - qaysi disk qaysi ekanini hajmidan bilib oling
mount -t ext2 /dev/sda2 /mnt
ls /mnt
echo "salom haqiqiy disk" > /mnt/birinchi.txt
sync                            # buferlarni diskka yozish
umount /mnt                     # o'chirishdan oldin!
poweroff
```

Keyin Linux'da tekshiring:

```bash
sudo e2fsck -fn /dev/sdY2       # -n: faqat tekshirish, hech narsa o'zgartirmaslik
```

"clean" chiqsa, sizning yadroingiz haqiqiy diskka to'g'ri yozgan bo'ladi. Bu katta natija!

## 10. Muammolar va yechimlar

| Belgi | Ehtimoliy sabab | Nima qilish |
|---|---|---|
| Fleshka yuklash menyusida yo'q | ISO "ISO rejimida" yozilgan yoki Fast Boot | DD rejimida qayta yozing; Fast Boot → Disabled |
| "Secure Boot Violation" / "Invalid signature" | Secure Boot yoqilgan | BIOS'da Secure Boot → Disabled |
| GRUB `error: no multiboot header` | Yadro buzilgan yoki eski ISO | `make clean && make`, qayta yozing |
| Qora ekran, hech narsa yo'q | Framebuffer formati mos emas (24/32 bpp emas) | Legacy rejimida sinang. Serial port bo'lsa — kabel bilan ulang (pastda) |
| Jurnal chiqadi, klaviatura javob bermaydi | USB klaviatura, sof UEFI | CSM + Legacy rejimi + USB Legacy Support; yoki noutbukning ichki klaviaturasi / PS/2 |
| `[kbd] PS/2 kontroller topilmadi` | Kompyuterda i8042 yo'q | Faqat Legacy USB emulyatsiyasi yordam beradi yoki xHCI drayverini yozing |
| `[tsc] OGOHLANTIRISH: PIT ishlamayapti` | BIOS "8254 Clock Gating" | Tizim ishlaydi (zaxira chastota). Xohlasangiz BIOS'da gating'ni o'chiring |
| Qayta-qayta yuklanib turadi (reboot loop) | Triple fault: IDT o'rnatilishidan oldingi xato | Qaysi qatorgacha chiqqanini suratga oling. QEMU'da `-d int,cpu_reset -no-reboot` bilan qidiring |
| `PANIC: ...` | Yadro xatoni ushladi | Ekrandagi RIP manzilini yozib oling: `addr2line -e build/kernel.elf <RIP>` qaysi qator ekanini aytadi |
| Disk ko'rinmaydi | NVMe yoki RAID rejimi | `lspci`: `01:08` = NVMe. SATA uchun AHCI rejimini yoqing |
| `mount` ext2 ni ulamaydi | ext4 yoki ext3 journal | `mkfs.ext2` bilan yarating. `dmesg` sababini aytadi |

**Serial port** (stol kompyuterlarida COM1 ulagichi yoki anakartdagi "COM" pinlari): yadro barcha jurnalni COM1 ga ham yozadi (38400 bod, 8N1). USB-serial kabel bilan ikkinchi kompyuterda `screen /dev/ttyUSB0 38400` ni oching. Shunda butun jurnal matn ko'rinishida bo'ladi, hatto shell bilan ham ishlash mumkin.

---

# II qism. Tizimdan foydalanish

## 11. Yuklanish zanjiri: tugmani bosganingizdan `$` gacha

```text
Quvvat tugmasi
  → firmware (BIOS yoki UEFI): RAM'ni tekshiradi, yuklovchini topadi
  → GRUB: kernel.elf (Multiboot2) va initrd.tar ni RAM'ga yuklaydi, framebuffer so'raydi
  → kernel/boot/boot.asm: sahifa jadvallari, 64-bit rejim, higher-half
  → kernel/main.c: xotira → ACPI → vaqt → APIC → fayl tizimlari → jarayonlar → qurilmalar → SMP
  → initrd.tar → tmpfs "/" (/bin, /etc ...)
  → /bin/init (PID 1): /etc/rc ni bajaradi, keyin /bin/sh ni ishga tushiradi (sh chiqsa — qayta)
  → /bin/sh: myos:/$
```

## 12. Shell: asosiy buyruqlar

`help` hammasini ko'rsatadi. Eng muhimlari:

| Guruh | Buyruqlar | Misol |
|---|---|---|
| Papkalar | `cd`, `pwd`, `ls [-la]`, `mkdir [-p]`, `rmdir` | `mkdir -p /tmp/a/b && cd /tmp/a` |
| Fayllar | `cat`, `cp`, `mv`, `rm [-rf]`, `touch`, `stat` | `cp /etc/motd /tmp/m.txt` |
| Matn | `echo`, `head`, `tail`, `wc [-lwc]`, `grep [-ivnc]`, `tee`, `seq`, `cksum` | `seq 100 \| grep 7 \| wc -l` |
| Muharrir | `edit <fayl>` | Ctrl-S — saqlash, Ctrl-Q — chiqish, Ctrl-F — qidirish |
| Tizim | `ps`, `kill`, `free`, `dmesg`, `uname`, `lspci`, `date`, `sleep` | `dmesg \| tail` |
| Disklar | `mount`, `umount`, `sync` | `mount -t ext2 /dev/sda1 /mnt` |
| Quvvat | `poweroff`, `reboot` | |
| Testlar | `hello`, `forktest`, `fstest`, `memtest`, `sigtest`, `libctest`, `spin`, `crash` | `crash` — yadro user xatosini qanday ushlashini ko'ring |

Shell sintaksisi:

```sh
a | b            # a ning chiqishi b ning kirishiga (pipe)
a > f            # chiqishni faylga (ustiga yozish)
a >> f           # faylning oxiriga qo'shish
a < f            # kirishni fayldan olish
a 2>&1           # xatolarni ham oddiy chiqishga
a ; b            # ketma-ket
a && b           # a muvaffaqiyatli bo'lsa, b
a || b           # a xato bo'lsa, b
a &              # fonda ishga tushirish
echo $?          # oxirgi buyruqning chiqish kodi
NOM=qiymat       # o'zgaruvchi;  echo "$NOM"
ls /bin/*grep*   # glob: * va ?
. skript.sh      # skriptni shu shell ichida bajarish
```

Job control: `Ctrl-C` — to'xtatish (SIGINT), `Ctrl-Z` — pauza (SIGTSTP), `jobs`, `fg %1`, `bg %1`, `kill %1`.

Qator tahriri: strelkalar, yuqori/pastga — tarix, `Tab` — to'ldirish, `Ctrl-A`/`Ctrl-E` — boshi/oxiri, `Ctrl-U`/`Ctrl-K` — o'chirish, `Ctrl-W` — so'zni o'chirish, `Ctrl-L` — ekranni tozalash.

## 13. Fayl tizimi tuzilmasi

| Yo'l | Nima | Qayerda saqlanadi |
|---|---|---|
| `/bin` | Dasturlar (`user/bin/*.c` dan) | RAM (tmpfs) |
| `/etc/rc`, `/etc/motd` | Boshlang'ich skript va salomlashuv | RAM (`rootfs/etc/` dan) |
| `/dev` | Qurilmalar: `console`, `null`, `zero`, `random`, `urandom`, `kmsg`, `sda`, `sda1`... | Yadro (devfs) |
| `/tmp`, `/home` | Vaqtinchalik va foydalanuvchi fayllari | RAM |
| `/mnt` | Ulangan ext2 disk | **Disk** — o'zgarishlar saqlanadi |

## 14. Yadro parametrlari

QEMU'da: `make run-nographic APPEND="..."`. Haqiqiy kompyuterda: GRUB'dagi `multiboot2 ... kernel.elf <parametrlar>` qatorida (8-bo'lim), yoki yuklanishda GRUB menyusida `e` tugmasi bilan.

| Parametr | Nima qiladi |
|---|---|
| `selftest` | Yadroning ichki testlari (buddy, slab, vmalloc, vmm, jarayonlar) |
| `demo=uaf`, `demo=stack`, ... | Ataylab xato qilib, himoya qanday ishlashini ko'rsatadi (`kernel/tests/crashdemo.c`) |
| `threads` | Yadro oqimlari parallel ishlashi namoyishi |
| `init=/bin/sh` | PID 1 sifatida boshqa dastur (masalan, `/etc/rc` siz to'g'ridan-to'g'ri shell) |

## 15. O'z dasturingizni tizimga qo'shish (eng qisqa yo'l)

1. `user/bin/salom.c` faylini yarating (IV qism, 22-bo'limda to'liq misol).
2. `make` — Makefile `user/bin/*.c` dagi **har bir** faylni avtomatik dastur qiladi va `/bin` ga qo'yadi.
3. QEMU yoki haqiqiy kompyuterda: `salom`.

---

# III qism. C kodidagi har bir belgi nima qiladi

> Bu qismning maqsadi: kodga qaraganingizda **birorta belgi** tushunarsiz qolmasligi. Har bir belgi uchun
> **nima qiladi**, **nega kerak** va **qo'yilmasa nima bo'ladi** yozilgan. C xususiyatlari (`void`, `static`,
> `volatile` ...) — [QOLLANMA.md](QOLLANMA.md), 4-bo'lim.

## 16. Tinish belgilari (punctuation)

### `;` — nuqta-vergul: "buyruq tugadi"

```c
x = 5;
kprintf("salom\n");
```

- **Nima qiladi:** bitta ifoda-buyruqni (statement) tugatadi.
- **Nega kerak:** C uchun yangi qator (Enter) **hech narsa** anglatmaydi. `x = 5` ni bir qatorga, `kprintf` ni keyingisiga yozish kompilyatorga farqsiz: u buyruqlarni **faqat** `;` bo'yicha ajratadi. Python yangi qatorga qaraydi, C esa `;` ga.
- **Qo'yilmasa:** `error: expected ';' before ...`. Xato ko'pincha **keyingi** qatorda ko'rsatiladi, chunki kompilyator buyruqning tugamaganini faqat keyingi so'zni ko'rganda tushunadi.
- **Qayerda qo'yilmaydi:** `{ }` blokidan keyin (`if (...) { ... }` ning oxirida), `#include`/`#define` oxirida (ular C emas, preprotsessor buyruqlari) va funksiya tanasining `}` idan keyin.
- **Qayerda albatta kerak:** `struct` ta'rifidan keyin: `struct nuqta { int x, y; };`. Bu `}` dan keyin ham nuqta-vergul kerak, chunki `struct ... { }` dan keyin shu turdagi o'zgaruvchi e'lon qilish mumkin (`struct nuqta { int x; } p;`), ya'ni bu hali buyruqning o'rtasi.
- **Tuzoq:** `if (x > 0);` — `;` bo'sh buyruqni yaratadi. Keyingi `{ ... }` endi `if` ga bog'liq emas va **har doim** bajariladi. Xuddi shunday `for (...);` — tanasi bo'sh sikl.
- **`for` ichida:** `for (int i = 0; i < n; i++)` — bu yerda `;` qavs ichidagi uchta qismni ajratadi: boshlash; shart; qadam.

> **DIQQAT — assembly'da (NASM, `.asm` fayllar) `;` IZOH belgisi!** `mov rax, 1 ; rax ga 1` — `;` dan keyingi hamma narsa izoh. C'dagi `//` ning o'rni. Loyihada ikkalasi ham bor, adashmang.

### `.` — nuqta: ko'p ma'noli belgi

C'da va loyihada nuqta **olti xil** vazifada keladi:

**1) Struktura a'zosiga murojaat (qiymat orqali):**

```c
struct nuqta p;
p.x = 10;          /* p ning x maydoni */
```

- **Nima qiladi:** `p` strukturasining ichidagi `x` maydonini oladi. Kompilyator buni "p ning manzili + x ning siljishi (offset)" ga aylantiradi.
- **Nega:** struktura — bir nechta qiymatni bitta nom ostida saqlash. Nuqta shu qiymatlardan birini tanlaydi.
- **Xato:** `p` ko'rsatkich (`struct nuqta *p`) bo'lsa, `p.x` ishlatib bo'lmaydi: `error: request for member 'x' in something not a structure`. Ko'rsatkich uchun `->` kerak (pastga qarang).

**2) Designated initializer — nom bilan boshlash:**

```c
static const struct file_ops pipe_fops = {
    .read  = pipe_read,     /* "read maydoniga pipe_read ni qo'y" */
    .write = pipe_write,
};
```

- Bu yerda `.read` "read nomli maydon" degani. Tartib muhim emas, yozilmagan maydonlar **0 (NULL)** bo'ladi.
- **Nega:** yadroda drayver jadvallari (`file_ops`, `pci_driver`) shunday yoziladi. Struktura o'zgarsa ham kod buzilmaydi.

**3) Kasr son:** `1.5`, `0.25`. Yadroda deyarli yo'q: yadro FPU/SSE registrlarini saqlamaydi, shuning uchun `-mgeneral-regs-only` bilan yig'iladi va `float` ishlatib bo'lmaydi.

**4) `...` — uch nuqta (variadic):** `int printf(const char *fmt, ...);` — "bundan keyin istalgancha argument". Ular `va_list`/`va_arg` bilan o'qiladi (`kernel/lib/kprintf.c`).

**5) Fayl nomlarida:** `.c` — C kodi; `.h` — sarlavha (e'lonlar); `.asm` — assembly; `.o` — obyekt fayl (kompilyatsiya qilingan, hali bog'lanmagan); `.a` — `.o` fayllar arxivi (`libc.a`); `.elf` — tayyor dastur; `.ld` — linker skripti; `.md` — hujjat. Kompilyator ham, Make ham faylga nima qilishni kengaytmasidan biladi.

**6) Yo'llarda (shell):** `.` — joriy papka, `..` — ota papka, `./dastur` — "shu papkadagi dasturni ishga tushir". `. skript` (nuqta + bo'shliq) esa shell buyrug'i: skriptni **shu** shell ichida bajaradi.

**7) Assembly va linker skriptida:**

```nasm
section .text        ; .text - kod bo'limi, .data - ma'lumot, .bss - nollangan ma'lumot
```

```ld
. = 0xFFFFFFFF80000000 + 1M;   /* "." - JORIY MANZIL (location counter) */
. = ALIGN(4096);                /* joriy manzilni 4096 ga karrali qilib surish */
```

Linker skriptida (`kernel/linker.ld`) `.` — "hozir qaysi manzilga yozyapman" degan hisoblagich. `. = ...;` uni o'zgartiradi. Oxiridagi `;` bu yerda ham "buyruq tugadi" degani.

### `->` — ko'rsatkich orqali a'zo

```c
struct process *p = current;
p->pid              /* bu aynan (*p).pid bilan bir xil */
```

- **Nima qiladi:** avval ko'rsatkich ko'rsatgan joyga boradi (`*p`), keyin maydonni oladi (`.pid`).
- **Nega kerak:** yadroda deyarli hamma narsa ko'rsatkich orqali uzatiladi (`struct process *`, `struct inode *`). `(*p).pid` yozish noqulay, `->` qisqartma.
- **Xato:** `p` NULL bo'lsa, `p->pid` — **NULL dereference**. Yadroda bu page fault va `PANIC`, user dasturida SIGSEGV.
- **Zanjir:** `current->mm->pml4` — "joriy jarayonning xotira tuzilmasining sahifa jadvali".

### `,` — vergul

- **Ajratuvchi:** `f(a, b)`, `int x, y;`, `{1, 2, 3}`.
- **Vergul operatori** (kam uchraydi): `for (i = 0, j = n; i < j; i++, j--)` — ikkala ifoda ham bajariladi. Chapdagi hisoblanib tashlanadi, o'ngdagi qiymat bo'ladi.

### `:` — ikki nuqta

- `case 5:` va `default:` — `switch` ichidagi yorliqlar.
- `xato:` — `goto xato;` uchun yorliq (yadroda xatodan keyin tozalash uchun ishlatiladi).
- `uint32_t present : 1;` — **bit maydoni**: shu maydon faqat 1 bit egallaydi.
- `shart ? a : b` — ternar operatorning ikkinchi qismi.
- Inline assembly'da: `__asm__("..." : chiqishlar : kirishlar : buziladiganlar)` — qismlarni ajratadi.

### `?` va `:` — ternar operator

```c
int max = a > b ? a : b;     /* shart rost bo'lsa a, aks holda b */
```

Qisqa `if/else`, lekin **qiymat qaytaradi**, shuning uchun ifoda ichida ishlatiladi.

### Qavslar

| Belgi | Vazifalari |
|---|---|
| `( )` | 1) funksiya chaqirish: `f(x)`; 2) guruhlash: `(a + b) * c`; 3) tur o'zgartirish (cast): `(uint64_t)x`; 4) `if (...)`, `while (...)` sharti; 5) funksiya parametrlari ro'yxati |
| `{ }` | 1) **blok** — bir nechta buyruqni bitta qilish; 2) **ko'rinish sohasi (scope)** — blok ichida e'lon qilingan o'zgaruvchi blokdan tashqarida yo'q; 3) boshlang'ich qiymatlar: `int a[] = {1, 2}` |
| `[ ]` | 1) massiv e'loni: `char buf[64]`; 2) indeks: `a[i]` — bu aynan `*(a + i)` degani (C'da massiv indeksi — ko'rsatkich arifmetikasi!); 3) assembly'da xotiraga murojaat: `mov rax, [rsp+8]` |
| `< >` | `#include <stdio.h>` — "tizim sarlavhasi" (`-I`/`-isystem` papkalaridan qidiriladi) |
| `" "` | `#include "fs/vfs.h"` — avval shu loyihadan qidiriladi; kodda — **satr**: `"salom"` = `{'s','a','l','o','m','\0'}`, oxiriga avtomatik `\0` qo'shiladi |
| `' '` | **Bitta belgi**: `'a'` — bu shunchaki son (97). `"a"` esa ikki baytli massiv (`'a'`, `'\0'`). Adashtirish — klassik xato |

`{ }` qo'yilmasa nima bo'ladi:

```c
if (xato)
    kprintf("xato\n");
    return -1;          /* DIQQAT: bu if ga TEGISHLI EMAS - har doim bajariladi! */
```

Chekinish (indent) C uchun hech narsa anglatmaydi. `if` dan keyin `{}` bo'lmasa, faqat **bitta** buyruq unga tegishli bo'ladi. 2014-yilda Apple'ning SSL'idagi mashhur "goto fail" xatosi aynan shunday bo'lgan.

### `*` — yulduzcha: uch ma'no

```c
int *p;          /* 1) E'londa: "p - int ga ko'rsatkich" */
*p = 5;          /* 2) Ifodada: "p ko'rsatgan joy" (dereference) */
x = a * b;       /* 3) Ko'paytirish */
```

`/* ... */` — izoh ichidagi `*` esa oddiy belgi.

### `&` — ampersand: uch ma'no

```c
int *p = &x;           /* 1) Manzil olish: "x qayerda turibdi" */
flags & 0x80           /* 2) Bitli VA: faqat 7-bitni ajratish */
a && b                 /* 3) Mantiqiy VA (ikkita &!) */
```

`&` va `&&` ni adashtirish ko'pincha jim xatoga olib keladi: `if (a & b)` ham kompilyatsiya bo'ladi, lekin boshqa narsani tekshiradi.

### `=` va `==`

- `=` — **qiymat berish**: `x = 5`.
- `==` — **taqqoslash**: `x == 5` (rost bo'lsa 1).
- Tuzoq: `if (x = 0)` — taqqoslamaydi, x ga 0 yozadi va shart har doim yolg'on. `-Wall` bu haqda ogohlantiradi, shuning uchun loyiha `-Werror` bilan yig'iladi.

### Qolgan operatorlar

| Belgi | Nima | Yadroda qayerda |
|---|---|---|
| `!` | mantiqiy EMAS: `!p` = "p NULL" | `if (!page) return -ENOMEM;` |
| `\|\|` | mantiqiy YOKI. **Qisqa tutashuv:** chap tomon rost bo'lsa, o'ng tomon **hisoblanmaydi** | `if (!p \|\| p->x)` — xavfsiz: p NULL bo'lsa, `p->x` ga yetib bormaydi |
| `&&` | mantiqiy VA, qisqa tutashuv bilan | `if (p && p->x)` |
| `\|` | bitli YOKI — bitni **yoqish** | `pte = phys \| PTE_PRESENT \| PTE_WRITE` |
| `^` | bitli XOR — bitni **almashtirish** | buddy'da juftni topish: `idx ^ (1 << order)` |
| `~` | bitli teskari — barcha bitlar | bitni **o'chirish**: `flags &= ~PTE_WRITE` |
| `<<` `>>` | bitni surish: `1 << 12` = 4096 | `addr >> 12` — sahifa raqami |
| `%` | bo'linma qoldig'i | `i % 26`, halqa bufer: `(head + 1) % SIZE` |
| `++` `--` | 1 ga oshirish/kamaytirish. `i++` — avval eski qiymat, keyin oshirish; `++i` — avval oshirish | `buf[n++] = c;` |
| `+=` `-=` `\|=` `&=` `<<=` ... | `a += b` = `a = a + b` | `flags \|= O_APPEND` |
| `sizeof` | turning yoki o'zgaruvchining hajmi (baytda), **kompilyatsiya paytida** hisoblanadi | `kmalloc(sizeof(*p))` |

### `#` — preprotsessor

Kompilyator C kodni ko'rishidan **oldin** ishlaydigan "matn almashtirgich":

```c
#include "lib/kprintf.h"   /* faylning mazmunini shu yerga KO'CHIRIB qo'yish */
#define PAGE_SIZE 4096      /* PAGE_SIZE so'zini hamma joyda 4096 ga almashtirish */
#pragma once                /* sarlavha ikki marta qo'shilmasin */
#ifdef HOST_TEST ... #endif /* shartli kompilyatsiya: kodning bir qismini tashlab yuborish */
```

- `#` bilan boshlanadigan qatorlar oxirida `;` **yo'q**: ular C buyrug'i emas.
- Makro ichida `#x` — argumentni satrga aylantiradi (`CHECK(x > 0)` → `"x > 0"`, testlar xabari shunday chiqadi). `a##b` — ikki so'zni yopishtiradi.
- `\` qator oxirida — "makro keyingi qatorda davom etadi".

### Izohlar

- `/* ... */` — ko'p qatorli izoh. Ular ichma-ich qo'yilmaydi: birinchi `*/` izohni tugatadi. Izoh ichida `/*` yozilsa, `-Wall -Werror` uni xato deb hisoblaydi.
- `// ...` — qator oxirigacha izoh.
- Kompilyator ikkalasini ham butunlay tashlab yuboradi. Ular faqat odam uchun.

### Escape belgilar (`\`)

`"\n"` — yangi qator, `"\t"` — tab, `"\0"` — nol bayt (satr oxiri), `"\\"` — `\` ning o'zi, `"\""` — qo'shtirnoq, `"\x1b"` — ESC (terminal ranglari: `"\x1b[1;32m"`), `"\r"` — karetkani qator boshiga qaytarish.

## 17. Kalit so'zlar (bir qarashda)

| So'z | Vazifasi |
|---|---|
| `if` `else` | shart |
| `for` `while` `do ... while` | sikllar. `do` kamida bir marta bajariladi |
| `switch` `case` `default` `break` | ko'p tanlov. `break` bo'lmasa, keyingi `case` ga **"tushib ketadi"** (fallthrough) |
| `break` `continue` | sikldan chiqish / keyingi aylanishga o'tish |
| `return` | funksiyadan chiqish (va qiymat qaytarish) |
| `goto` | yorliqqa sakrash. Yadroda faqat xatodan keyin tozalash uchun |
| `struct` `union` `enum` | yangi turlar: maydonlar to'plami / bir xotirada turli ko'rinishlar / nomlangan sonlar |
| `typedef` | turga yangi nom (loyihada kam ishlatiladi: `struct process` aniqroq) |
| `void` `char` `short` `int` `long` `unsigned` `signed` `_Bool`/`bool` | asosiy turlar. Yadroda aniq o'lchamli `uint8_t`...`uint64_t` afzal |
| `static` `extern` `const` `volatile` `inline` `restrict` | QOLLANMA.md, 4-bo'lim — har biri batafsil |
| `sizeof` `_Alignof` `_Static_assert` | kompilyatsiya paytidagi savollar/tekshiruvlar |
| `__attribute__((...))` | GCC'ga maxsus ko'rsatma: `packed`, `aligned(4096)`, `noreturn`, `section(".text")` |
| `__asm__ volatile(...)` | C ichida assembly: `cli`, `hlt`, `rdmsr`, `syscall` |

## 18. Haqiqiy kodni belgima-belgi o'qish

`kernel/fs/pipe.c` dagi haqiqiy `pipe_read` (izohlar qisqartirilgan):

```c
static int64_t pipe_read(struct file *f, void *dst, size_t len, uint64_t off)
{
    (void)off;
    struct pipe *p = f->priv;
    if (len == 0)
        return 0;
    spin_lock(&p->lock);
    while (p->count == 0) {
        if (p->writers == 0) {
            spin_unlock(&p->lock);
            return 0;
        }
        if (signal_interrupted(current)) {
            spin_unlock(&p->lock);
            return -EINTR;
        }
        proc_sleep(&p->count, &p->lock);
    }
    size_t n = MIN(len, p->count);
    for (size_t i = 0; i < n; i++)
        ((uint8_t *)dst)[i] = p->buf[(p->head + i) % PIPE_SIZE];
    p->head = (p->head + n) % PIPE_SIZE;
    p->count -= n;
    proc_wakeup(&p->writers);
    spin_unlock(&p->lock);
    return (int64_t)n;
}
```

| Kod | Ma'nosi |
|---|---|
| `static` | funksiya faqat shu faylda ko'rinadi. Tashqariga faqat `pipe_fops` jadvali orqali (`.read = pipe_read`) beriladi |
| `int64_t` | qaytish turi: o'qilgan baytlar soni yoki manfiy xato kodi (`-EINTR`). Yadroda "manfiy = xato" qoidasi |
| `pipe_read(` ... `)` | nom va parametrlar ro'yxati. Vergul parametrlarni ajratadi |
| `struct file *f` | `f` — ochiq faylni tasvirlaydigan strukturaga **ko'rsatkich** (`*`) |
| `void *dst` | "istalgan turdagi xotiraga ko'rsatkich": o'qilgan ma'lumot qayerga yozilishi |
| `size_t len` | necha bayt so'ralgan (`size_t` — ishorasiz, manzil o'lchamidagi son) |
| `uint64_t off` | fayldagi joy. Pipe'da ma'nosi yo'q, lekin VFS barcha `read` funksiyalariga bir xil parametrlar beradi |
| `{` | funksiya tanasi boshlandi |
| `(void)off;` | "bu parametr ataylab ishlatilmaydi". Busiz `-Wextra` "unused parameter" ogohlantirishini beradi, `-Werror` esa uni xatoga aylantiradi |
| `struct pipe *p = f->priv;` | `->` — `f` ko'rsatgan strukturaning `priv` maydoni. VFS har bir faylda drayverning o'z ma'lumoti uchun joy beradi. `;` — buyruq tugadi |
| `if (len == 0)` / `return 0;` | `==` — taqqoslash. 0 bayt so'ralsa — darhol 0. `{}` yo'q, shuning uchun `if` ga faqat keyingi **bitta** buyruq tegishli |
| `spin_lock(&p->lock);` | `&` — qulfning **manzili**: funksiya qulfning o'zini o'zgartirishi kerak, nusxasini emas |
| `while (p->count == 0) {` | bufer bo'sh ekan, aylanamiz |
| `if (p->writers == 0) {` ... `return 0;` | yozuvchilar qolmagan — fayl oxiri (EOF, POSIX qoidasi). Qaytishdan **oldin** qulf albatta bo'shatiladi! |
| `if (signal_interrupted(current))` | kutayotganimizda Ctrl-C bosildi — `-EINTR` bilan chiqamiz, aks holda dastur abadiy qotadi |
| `proc_sleep(&p->count, &p->lock);` | uxlaymiz. `&p->count` — "kutish kaliti" (qaysi hodisani kutyapmiz). Funksiya qulfni **atomar** bo'shatadi va uyg'onganda qayta oladi (aks holda "yo'qolgan uyg'otish" xatosi bo'ladi) |
| `size_t n = MIN(len, p->count);` | `MIN` — makro (`lib/common.h`): so'ralgandan ko'p va bordan ko'p o'qimaymiz |
| `for (size_t i = 0; i < n; i++)` | `;` bu yerda qavs ichidagi uch qismni ajratadi: boshlash; shart; qadam |
| `((uint8_t *)dst)[i]` | `void *` ni bayt ko'rsatkichiga **cast** qilib, keyin `[i]` bilan indekslaymiz. `void *` bilan to'g'ridan-to'g'ri indekslab bo'lmaydi: uning o'lchami yo'q |
| `(p->head + i) % PIPE_SIZE` | halqa bufer: oxiriga yetganda `%` bizni 0 ga qaytaradi |
| `p->count -= n;` | `a -= b` = `a = a - b` |
| `proc_wakeup(&p->writers);` | joy bo'shadi — shu kalitda uxlayotgan yozuvchilarni uyg'otamiz |
| `return (int64_t)n;` | `(int64_t)` — `size_t` ni qaytish turiga aniq aylantirish |
| `}` | funksiya tugadi (`;` yo'q!) |

Mashq: `kernel/mm/pmm.c` dan `buddy_alloc` ni oching va xuddi shunday jadval tuzing. Har bir belgini tushuntira olmaguningizcha keyingi funksiyaga o'tmang.

## 19. Assembly (NASM) sintaksisi — qisqa

```nasm
global isr_common          ; boshqa fayllar ko'rsin (C'dagi extern ning teskarisi)
extern interrupt_dispatch  ; bu nom boshqa faylda (C'da)
section .text              ; kod bo'limi

isr_common:                ; yorliq (label) - manzilga nom
    push rax               ; stekka qo'yish
    mov rdi, rsp           ; rdi = rsp  (NASM'da: QAYERGA, QAYERDAN)
    call interrupt_dispatch; C funksiyasini chaqirish (1-argument rdi da)
    mov rax, [rsp + 8]     ; [ ] - xotiradan o'qish (rsp+8 manzilidan)
    add rsp, 16
    iretq                  ; uzilishdan qaytish
```

- `;` — izoh (C'da esa buyruq tugashi!).
- Operandlar tartibi: **`mov QAYERGA, QAYERDAN`** (Intel sintaksisi). GCC inline asm'da esa AT&T sintaksisi ishlatiladi: `movq %rax, %rbx` — teskari tartib va `%` belgilari.
- System V ABI: argumentlar `rdi, rsi, rdx, rcx, r8, r9`, natija `rax`. Syscall'da `rcx` o'rniga `r10`.

## 20. Makefile sintaksisi — qisqa

```make
CC := gcc                       # := - darhol hisoblanadigan o'zgaruvchi
QEMU_MEM ?= 256M                # ?= - agar tashqaridan berilmagan bo'lsa
$(BUILD)/kernel/%.c.o: kernel/%.c   # qoida: NISHON: BOG'LIQLIKLAR
	$(CC) $(KERNEL_CFLAGS) -c $< -o $@   # TAB bilan boshlanishi SHART (bo'shliq emas!)
```

`$@` — nishon fayl, `$<` — birinchi bog'liqlik, `$^` — hamma bog'liqliklar, `%` — naqsh ("har qanday nom"). `.PHONY` — "bu fayl emas, buyruq nomi" (`test`, `clean`).

---

# IV qism. Tizimni kengaytirish — "o'zing davom et"

> **Qanday o'qish kerak:** 21–25-bo'limlarda **to'liq misollar** bor: qadam-baqadam bajaring, ishlashini ko'ring.
> 26-bo'limda esa loyihalar: ularda faqat **qayerda**, **qanday qadamlar**, **maslahat** va **qanday tekshirish** berilgan.
> Kodni **o'zingiz** yozasiz. Har bir loyiha oxirida "**Endi o'zing davom et**" — ya'ni keyingi qadam sizniki.

## 21. Kengaytirishning oltin qoidalari

1. **Avval o'qing, keyin yozing.** Yangi narsa qo'shishdan oldin, unga o'xshash mavjud kodni toping va oxirigacha o'qing (yangi dastur uchun `hello.c`, yangi syscall uchun `sys_getppid`, yangi drayver uchun `ahci.c`).
2. **Kichik qadamlar.** Har bir qadamdan keyin `make && make run`. 200 qator yozib, keyin debug qilish — eng yomon yo'l.
3. **Test bilan.** Yangi xususiyat = `tools/test.sh` da yangi tekshiruv. Test bo'lmasa, 3 oydan keyin kimdir (siz!) uni buzadi va bilmaydi.
4. **Har bir bosqich — bitta commit.** Buzilsa, `git bisect` qaysi commit aybdorligini topadi.
5. **Haqiqiy apparatda sinang.** QEMU kechiradigan xatolarni (vaqt, keshlanish, qurilmaning sekinligi) haqiqiy apparat kechirmaydi.

## 22. To'liq misol A: yangi user dasturi (`uptime`)

**Maqsad:** tizim qancha vaqtdan beri ishlayotganini ko'rsatish.

**Qayerda:** `user/bin/uptime.c` (yangi fayl). Makefile uni o'zi topadi.

```c
/* =============================================================================
 *  user/bin/uptime.c - tizim qancha vaqtdan beri ishlayapti
 * =============================================================================
 *  uptime_ms() - MyOS'ga xos syscall (SYS_UPTIME): yadro taymer tiklarini
 *  millisekundga aylantirib qaytaradi. Linux'da bu /proc/uptime faylidan o'qiladi.
 * ============================================================================= */
#include <myos.h>
#include <stdio.h>

int main(void)
{
    unsigned long ms = (unsigned long)uptime_ms();
    unsigned long s = ms / 1000;
    printf("ishlayapti: %lu soat %lu daqiqa %lu soniya (%lu ms)\n",
           s / 3600, s / 60 % 60, s % 60, ms);
    return 0;
}
```

**Tekshirish:** `make && make run`, keyin `uptime`.

**Testga qo'shish** (`tools/test.sh`): `COMMANDS` ro'yxatiga `"uptime"`, `EXPECT` ro'yxatiga `"^ishlayapti: |uptime ishlaydi"` qo'shing (`|` dan oldin — regex, keyin — tekshiruv tavsifi).

**Endi o'zing davom et:** shu uslubda quyidagi dasturlarni yozing (Linux'dagi `man` sahifasini o'qib, eng kerakli bayroqlardan boshlang):

- `yes` — cheksiz chiqish (pipe va `head` bilan sinang: `yes | head -3`. **Maslahat:** `head` chiqib ketganda yadro `yes` ga SIGPIPE yuboradi va u tugaydi. SIGPIPE'ni e'tiborsiz qoldirsangiz, `write` `-1` qaytaradi va `errno == EPIPE` bo'ladi — shunda o'zingiz chiqishingiz kerak).
- `sort` — qatorlarni tartiblash. **Maslahat:** hammasini `malloc` bilan o'qing, `qsort` + `strcmp`.
- `uniq`, `tr`, `rev`, `nl`.
- `hexdump -C`. **Maslahat:** 16 baytdan o'qing, `%08lx  %02x ...  |ascii|`.
- `find papka -name naqsh`. **Maslahat:** `opendir/readdir` bilan rekursiya; naqsh uchun `sh.c` dagi `glob_match` ni nusxalang.
- `du`, `df`. **Maslahat:** `df` uchun ext2 superblokidan bo'sh bloklarni oladigan yangi syscall kerak — B misolga qarang.

## 23. To'liq misol B: yangi syscall (`getcpu`)

**Maqsad:** dastur qaysi CPU yadrosida ishlayotganini bilish. Dasturni user rejimidan yadroga olib boradigan butun yo'lni ko'rsatadigan eng kichik misol.

**Yo'l:** `getcpu()` (libc) → `syscall` instruksiyasi → `syscall_entry.asm` → `syscall_dispatch` (C) → `switch` → `cpu_id()` → `rax` orqali qaytish.

**1-qadam — raqam** (`include/myos/abi.h`). Yadro ham, libc ham shu faylni ishlatadi, shuning uchun raqam ikkalasida bir xil bo'ladi:

```c
#define SYS_PAUSE       51              /* signal kelguncha uxlash                   */
#define SYS_GETCPU      52              /* getcpu() -> joriy CPU raqami              */
#define SYS_COUNT       53              /* doim OXIRGI raqam + 1                     */
```

**2-qadam — yadroda bajarish** (`kernel/sys/syscall.c`, `switch (nr)` ichida, `default:` dan oldin):

```c
        case SYS_GETCPU:   ret = cpu_id(); break;
```

`cpu_id()` `arch/percpu.h` da: GS registri orqali shu CPU'ning per-CPU strukturasidan o'qiydi.

**3-qadam — libc o'rami** (`user/libc/unistd.c`, `uptime_ms` yonida):

```c
int getcpu(void)
{
    return (int)__syscall0(SYS_GETCPU);
}
```

**4-qadam — e'lon** (`user/include/myos.h`):

```c
int getcpu(void);                         /* joriy CPU raqami (0 dan) */
```

**5-qadam — dastur** (`user/bin/qaysicpu.c`):

```c
#include <myos.h>
#include <stdio.h>

int main(void)
{
    for (int i = 0; i < 5; i++) {
        printf("men hozir %d-CPU'daman\n", getcpu());
        sleep_ms(100);                  /* uxlab uyg'onganda boshqa CPU'ga tushishi mumkin */
    }
    return 0;
}
```

**Tekshirish:** `make run QEMU_SMP=4`, keyin `qaysicpu`. Raqamlar o'zgarib turishi mumkin: bu SMP scheduler ishlayotganining isboti.

**Muhim xavfsizlik qoidasi:** syscall user'dan **ko'rsatkich** olsa (masalan, `stat(path, &st)`), unga **hech qachon** to'g'ridan-to'g'ri yozmang yoki undan o'qimang. Avval manzil user hududida ekanini tekshiring: `copy_from_user`/`copy_to_user` dan foydalaning (misol: `kernel/proc/signal.c` dagi `sys_sigaction`). Aks holda dastur yadroga "`0xFFFFFFFF80000000` ga yoz" deb, yadroning o'zini buzadi.

**Endi o'zing davom et:**

- `getuid`/`getgid` (hozircha doim 0 qaytarsin, 26.2-loyihaga tayyorgarlik).
- `SYS_STATFS` — `df` uchun: fayl tizimining hajmi va bo'sh joyi. **Maslahat:** `struct super_block` ga `statfs` callback qo'shing. ext2 uchun `s_free_blocks_count`, tmpfs uchun `pmm` statistikasi.
- `nanosleep`, `gettimeofday` (millisekund aniqlikdagi vaqt). **Maslahat:** `time_now()` + `timer_ticks()`.

## 24. To'liq misol C: yadro ichki testi

**Qayerda:** `kernel/tests/selftest.c`. Namuna uchun `test_buddy` ni o'qing.

```c
static void test_mening(void)
{
    kprintf("[test] mening testim\n");
    char *p = kmalloc(100);
    CHECK(p != NULL);
    memset(p, 0xAB, 100);
    CHECK((uint8_t)p[99] == 0xAB);
    kfree(p);
}
```

Uni `selftest_run` dagi ro'yxatga qo'shing (`test_proc();` dan keyin: `test_mening();`), keyin `make run-nographic APPEND=selftest`.

**Endi o'zing davom et:** har bir yangi yadro funksiyasi uchun test yozing, ayniqsa chegaraviy holatlar uchun (0, 1, maksimum, to'lish, xotira tugashi).

## 25. To'liq misol D: shell'ga ichki buyruq (`history`)

**Qayerda:** `user/bin/sh.c` → `run_builtin` va `is_builtin` dagi nomlar ro'yxati (va Tab uchun `builtins[]` massivi).

**Nega ichki (builtin) bo'lishi kerak:** tarix shell jarayonining **xotirasida** turadi. Alohida dastur (`/bin/history`) boshqa jarayon bo'lgani uchun uni ko'ra olmaydi. `cd` ham shu sababdan ichki: bola jarayonning `chdir` qilishi otaga ta'sir qilmaydi.

**Qadamlar:**

1. `sh.c` da tarix massivi qanday nomlanganini toping (`grep -n hist user/bin/sh.c`).
2. `run_builtin` ga `if (strcmp(name, "history") == 0) { ... }` blokini qo'shing: har bir yozuvni raqami bilan chiqaring.
3. `is_builtin` dagi `names[]` va Tab uchun `builtins[]` ga `"history"` ni qo'shing.
4. `help` matniga qo'shing.

**Endi o'zing davom et:** `!n` (n-buyruqni qayta bajarish), `alias`, `export`, `$HOME`, `~` kengaytmasi, `if/then/fi` va `while` (skriptlar uchun — katta, lekin juda foydali loyiha).

## 26. Loyihalar — "Endi o'zing davom et"

Tartib qiyinlik bo'yicha. ★ — oson, ★★★★★ — juda qiyin. Har biri uchun: **qayerda**, **qadamlar**, **maslahat**, **tekshirish**.

### 26.1. `/proc` fayl tizimi ★★

- **Nega:** Linux'da `ps`, `free`, `uptime` ma'lumotni `/proc` dagi fayllardan oladi. Bizda esa har biri uchun alohida syscall bor (`SYS_PS`, `SYS_MEMINFO`). `/proc` — "hamma narsa fayl" g'oyasining eng chiroyli namunasi.
- **Qayerda:** yangi `kernel/fs/procfs.c`. Namuna — `devfs.c` va `tmpfs.c`.
- **Qadamlar:** 1) `/proc/uptime`, `/proc/meminfo` — `read` chaqirilganda matnni **o'sha paytda** `ksnprintf` bilan yaratadigan fayllar. 2) `main.c` da `/proc` ga ulash. 3) `/proc/<pid>/status` — papkalar dinamik: `lookup` pid raqamini sonli nom sifatida qabul qiladi.
- **Maslahat:** har bir fayl uchun "generator" funksiya: `int gen(char *buf, size_t size)`. `read(off, len)` — natijaning `[off, off+len)` qismini qaytaradi.
- **Tekshirish:** `cat /proc/meminfo`, keyin `ps` ni `/proc` orqali qayta yozing.

### 26.2. Foydalanuvchilar va ruxsatlar (uid/gid) ★★★

- **Qayerda:** `struct process` ga `uid, gid`; `vfs.c` dagi `open/mkdir/unlink` ga ruxsat tekshiruvi; inode'da `mode, uid, gid` (ext2'da ular allaqachon diskda bor!).
- **Qadamlar:** 1) `getuid/setuid` syscall'lari. 2) `vfs_permission(inode, MAY_READ)`. 3) `/etc/passwd` va `login` dasturi.
- **Maslahat:** root (uid 0) hamma narsani qila oladi. `rwx` bitlari: egasi / guruh / boshqalar.
- **Tekshirish:** oddiy foydalanuvchi `/etc/rc` ni o'chira olmasligi kerak.

### 26.3. Oqimlar (threads) va futex ★★★

- **Qayerda:** `proc/process.c` → `proc_fork` ning yonida `clone`: yangi jarayon eski `mm` ni **nusxalamaydi**, balki **ulashadi** (hisoblagichni oshiradi).
- **Qadamlar:** 1) `clone(fn, stack)`. 2) `futex_wait(addr, val)` / `futex_wake(addr, n)` — manzil bo'yicha kutish navbatlari (hash jadval). 3) libc'da `pthread_create`, `pthread_mutex`.
- **Maslahat:** `exit` da `mm` faqat oxirgi oqim chiqqanda bo'shatiladi. TLS uchun `FS` registrini (`wrfsbase` yoki MSR `0xC0000100`) har bir oqimda almashtiring.

### 26.4. Yaxshiroq scheduler ★★★

- **Qayerda:** `proc/process.c` (scheduler qismi).
- **G'oya:** hozir oddiy navbat. Keyingi qadam — ustuvorlik (`nice`), keyin Linux CFS'ga o'xshash "adolatli" rejalashtirish: har bir jarayonning `vruntime` i, eng kichigi tanlanadi (qizil-qora daraxt o'rniga boshlanishiga oddiy saralangan ro'yxat yetadi).
- **Tekshirish:** ikkita `spin` ni turli `nice` bilan ishga tushiring va `ps` dagi CPU vaqtini solishtiring.

### 26.5. ext2 — root fayl tizimi sifatida ★★★

- **Nega:** hozir `/` RAM'da — qayta yuklashda o'zgarishlar yo'qoladi. Haqiqiy OS diskdan ishlaydi.
- **Qadamlar:** 1) Yadro parametri `root=/dev/sda1`. 2) `main.c` da: parametr berilsa, initrd o'rniga shu ext2 ni `/` ga ulash (`/dev` ni esa ustiga devfs sifatida). 3) Diskka `/bin` va `/etc` ni yozadigan tasvir (`tools/mkdisk.py` ga `build/rootfs` ni qo'shing).
- **Maslahat:** Linux'dagi `pivot_root` g'oyasi: avval initrd'dan ishga tushib, keyin haqiqiy root'ga o'tish.

### 26.6. O'rnatuvchi (installer) ★★★★

- **Maqsad:** fleshkasiz, kompyuterning o'z diskidan yuklanish.
- **Eng oson yo'l (tavsiya):** 8-bo'limdagi kabi mavjud Linux'ning GRUB'idan foydalanish. U allaqachon o'rnatilgan va ishlaydi.
- **O'z o'rnatuvchingiz uchun qadamlar:** 1) MyOS ichida bo'lim jadvali yozadigan `fdisk` (MBR — 512 baytli 0-sektor: 4 ta yozuv 446-baytdan, oxirida `0x55 0xAA`). 2) `mkfs.ext2` — superblok, guruh deskriptorlari, bitmap'lar, root inode (`tools/mkdisk.py` ni C'ga ko'chiring — algoritm allaqachon o'sha yerda!). 3) Yuklovchi: GRUB'ni **Linux'dan** o'rnatish (`grub-install --boot-directory=/mnt/boot /dev/sdX`) yoki UEFI'da ESP (FAT32 bo'lim) ga `BOOTX64.EFI` nusxalash.
- **Maslahat:** yuklovchini o'zingiz yozmoqchi bo'lsangiz — bu alohida katta loyiha (Limine va BOOTBOOT manba kodlarini o'qing).

### 26.7. PS/2 sichqoncha va grafik ★★★

- **Qayerda:** `drivers/keyboard.c` ning yonida `mouse.c` (i8042 ning ikkinchi porti, IRQ12). Framebuffer'ni user dasturga `mmap` qiladigan `/dev/fb0`.
- **Qadamlar:** 1) Sichqoncha 3 baytli paketlar yuboradi: tugmalar, dx, dy. 2) `/dev/mouse`. 3) `/dev/fb0` + `ioctl(FBIOGET_INFO)` (kenglik, balandlik, pitch). 4) User dasturda kursor chizish.
- **Tekshirish:** ekranda sichqoncha bilan chiziladigan "paint" dasturi.

### 26.8. DOOM'ni ishga tushirish ★★★★ (eng yaxshi motivatsiya!)

- **G'oya:** [doomgeneric](https://github.com/ozkl/doomgeneric) — portlash uchun maxsus tayyorlangan DOOM. Faqat 5 ta funksiya yozish kerak: `DG_Init`, `DG_DrawFrame` (framebuffer'ga nusxalash), `DG_SleepMs`, `DG_GetTicksMs`, `DG_GetKey`.
- **Kerak:** 26.7 (`/dev/fb0`), tugmalarni bosish/qo'yib yuborish bilan o'qiydigan "raw" klaviatura rejimi, libc'da `fopen/fread/fseek`, `malloc`, `sscanf`, `strcasecmp`... (yetishmaganlarini qo'shasiz — libc uchun zo'r mashq).
- **Maslahat:** kompilyatsiya xatolari sizga libc'da nima yetishmayotganini aytadi. Birma-bir qo'shing. FPU kerak bo'ladi: user dasturlar uchun `-mgeneral-regs-only` ni olib tashlab, kontekst almashishda `fxsave/fxrstor` qo'shing (bu yadro qismi — o'zi alohida qiziq vazifa).

### 26.9. xHCI USB + USB klaviatura ★★★★★

- **Nega:** zamonaviy kompyuterlarda tashqi klaviaturalar USB. Busiz haqiqiy apparatda UEFI rejimida ishlash cheklangan.
- **Qayerda:** yangi `drivers/xhci.c`, `drivers/usb.c`, `drivers/usbkbd.c`.
- **Qadamlar:** 1) PCI: class `0x0C`, subclass `0x03`, prog_if `0x30` (`pci.c` dagi `pci_driver` jadvaliga `ahci.c` kabi qo'shing). 2) BAR0 ni `ioremap`. 3) **BIOS'dan egalik olish** (USB Legacy Support ishlatayotgan bo'lsa: xHCI Extended Capabilities → USB Legacy Support → "OS owned" bitini o'rnatish). 4) Kontrollerni reset, Command Ring, Event Ring, DCBAA. 5) Port'ni reset → Enable Slot → Address Device → deskriptorlarni o'qish. 6) HID "boot protocol": har 8 ms da 8 baytli hisobot (modifikatorlar + 6 ta tugma kodi). 7) HID kodlarini `console_input_char` ga (klaviatura drayveri kabi) uzatish.
- **Maslahat:** xHCI spetsifikatsiyasining 4-bobi ("Operational Model") — asosiy qo'llanma. OSDev Wiki "xHCI". Avval QEMU'da (`-device qemu-xhci -device usb-kbd`), keyin haqiqiy apparatda. Buni yozib tugatsangiz, siz haqiqiy drayver muhandisisiz.

### 26.10. NVMe drayveri ★★★★

- **Qayerda:** `drivers/nvme.c`. Namuna — `ahci.c` (u ham DMA va navbatlar bilan ishlaydi).
- **Qadamlar:** 1) PCI class `0x01`, subclass `0x08`, prog_if `0x02`. 2) BAR0 → registrlar (CAP, CC, CSTS, AQA, ASQ, ACQ). 3) Admin navbatini yaratish (submission + completion), kontrollerni yoqish (`CC.EN=1`, `CSTS.RDY` ni kutish — **vaqt chegarasi bilan!**). 4) `Identify` buyrug'i. 5) I/O navbati. 6) `Read`/`Write` buyruqlari (64 baytli buyruq, PRP ro'yxati). 7) `blkdev_register` bilan `nvme0n1` sifatida ro'yxatdan o'tkazish. `block.c` bo'lim nomini allaqachon `nvme0n1p1` qilib beradi!
- **Tekshirish:** QEMU: `-drive file=build/disk.img,if=none,id=nv -device nvme,drive=nv,serial=1`, keyin `mount -t ext2 /dev/nvme0n1p1 /mnt`.

### 26.11. Tarmoq: e1000 → ping → UDP → TCP ★★★★★

- **Qatlamlar (har biri alohida commit):**
  1. **e1000 drayveri** (`drivers/e1000.c`) — QEMU'ning sukut tarmoq kartasi (`-netdev user,id=n -device e1000,netdev=n`). RX/TX deskriptor halqalari, IRQ. Intel 8254x hujjati + OSDev "Intel Ethernet i217".
  2. **Ethernet** — 14 baytli sarlavha: qabul qiluvchi MAC, yuboruvchi MAC, tur (`0x0806` ARP, `0x0800` IPv4).
  3. **ARP** — "bu IP qaysi MAC'da?" so'rovi/javobi va kesh.
  4. **IPv4 + ICMP** — `ping` ga javob. **Birinchi katta g'alaba:** hostdan MyOS'ga ping.
  5. **UDP** + socket API (`socket`, `bind`, `sendto`, `recvfrom`) + DHCP klient.
  6. **TCP** — holat mashinasi (SYN, SYN-ACK, ACK ...), qayta yuborish, oyna. Eng qiyini. RFC 793 va "TCP/IP Illustrated".
- **Maslahat:** tarmoq paketlarida baytlar tartibi **big-endian** (`htons`, `htonl`). x86 esa little-endian. Birinchi xato odatda shu. Wireshark + QEMU `-object filter-dump,id=f,netdev=n,file=dump.pcap` — har bir paketni ko'rasiz.

### 26.12. Dinamik bog'lash, `/bin/sh` skriptlari, portlar ★★★★

- ELF dinamik bog'lovchi (`ld.so`) — umumiy `libc.so`.
- `#!` (shebang) — `exec` fayl boshida `#!/bin/sh` ni ko'rsa, interpretatorni ishga tushirsin (`sys/exec.c`, bu ★ oson qism!).
- Tayyor dasturlarni portlash: Lua, `sbase` utilitalari, TinyCC — **o'z tizimingizda C kompilyatori!**

## 27. Yangi drayver yozish — umumiy retsept

Deyarli har bir PCI drayveri shu qolipga tushadi:

```text
1. lspci          → qurilmaning vendor:device va class:subclass:prog_if kodlari
2. pci_driver     → ahci.c dagi kabi jadval: class kodlari + probe funksiyasi
3. probe()        → BAR'larni o'qish, ioremap (MMIO), PCI "bus master" yoqish (DMA uchun)
4. reset          → qurilmani ma'lum holatga keltirish. HAR BIR kutish — vaqt chegarasi bilan!
5. navbatlar/halqalar → pmm'dan fizik sahifalar (DMA manzillari - FIZIK!)
6. IRQ            → IOAPIC yoki MSI; handler'da tez ishlash, og'ir ishni keyinga qoldirish
7. ro'yxatdan o'tkazish → blkdev_register / chrdev_register / tarmoq qatlami
8. test           → QEMU'da, keyin haqiqiy apparatda. dmesg'ga batafsil jurnal
```

Uch oltin qoida (ularni buzish haqiqiy apparatda qotishga olib keladi):

1. **Hech qachon chegarasiz `while (!(reg & BIT));` yozmang.** Haqiqiy qurilma javob bermasligi mumkin. Bu loyihada ham aynan shunday xato bor edi: PS/2 kontrolleri yo'q kompyuterda `keyboard_init` abadiy aylanardi. Tuzatilgan versiyasini `drivers/keyboard.c` va `arch/tsc.c` da ko'ring.
2. **DMA manzillari — fizik.** Qurilma sizning virtual manzillaringizni bilmaydi.
3. **MMIO — `volatile`**, tartib muhim bo'lsa — to'siqlar (barrier). Kompilyator va CPU yozuvlar tartibini o'zgartirishi mumkin.

---

# V qism. Qanday ishlash kerak: kuchli yadro dasturchisi bo'lish yo'li

## 28. Kundalik ish uslubi

```text
1. Maqsadni bitta jumlada yozing ("pipe'ga yozish to'lganda bloklansin").
2. Mavjud kodni o'qing: qaysi fayllarga tegadi? (grep -rn "pipe_write" kernel/)
3. Testni birinchi yozing (tools/test.sh yoki selftest) - u hozir FAIL bo'lishi kerak.
4. Eng kichik o'zgarish → make → make run → test.
5. Ishladi? → git commit. Ishlamadi? → 29-bo'lim.
6. Haftada bir marta: haqiqiy kompyuterda sinash.
```

## 29. Debug usullari (eng ko'p yordam beradiganidan boshlab)

| Usul | Qachon | Qanday |
|---|---|---|
| `kprintf` / `dmesg` | har doim, birinchi qadam | Muammoli joy atrofida qiymatlarni chiqaring |
| `addr2line` | PANIC yoki SIGSEGV | `addr2line -e build/kernel.elf 0xffffffff8010abcd` → fayl:qator |
| GDB | nima bo'layotgani umuman tushunarsiz | `make debug` + `gdb -x tools/gdbinit`, `break pipe_read`, `bt`, `p *p` (docs/08) |
| QEMU jurnali | triple fault, reboot loop | `-d int,cpu_reset -no-reboot` — qaysi uzilish, qaysi RIP |
| `git bisect` | "oldin ishlardi" | `git bisect start; git bisect bad; git bisect good <commit>` — aybdor commitni topadi |
| `e2fsck`/`debugfs` | ext2 buzilgan | Disk tasvirini Linux'ning o'z vositalari bilan tekshirish (`tools/test.sh` shunday qiladi) |
| Serial port | haqiqiy apparat | 10-bo'limga qarang |

**Qoida:** xatoni avval **takrorlaydigan** qiling (har safar chiqadigan). Takrorlanmaydigan xatoni tuzatib bo'lmaydi, uni faqat yashirish mumkin.

## 30. O'qish uchun manbalar

| Manba | Nima uchun |
|---|---|
| **Intel SDM** (Software Developer's Manual), 3-jild | x86 haqidagi yakuniy haqiqat: paging, uzilishlar, APIC, MSR'lar |
| **OSDev Wiki** (wiki.osdev.org) | Har bir qurilma va mexanizm bo'yicha amaliy maqolalar |
| **Linux manba kodi** — elixir.bootlin.com | "Linux buni qanday qiladi?" — istalgan funksiyani qidirib, havolalar bo'ylab yurish mumkin |
| **"Operating Systems: Three Easy Pieces"** (bepul, ostep.org) | Nazariya: virtualizatsiya, parallellik, saqlash |
| **xv6** (MIT) va uning kitobi | Kichik, toza Unix yadrosi — MyOS bilan solishtirib o'qing |
| **"Linux Kernel Development"** (Robert Love) | Linux yadrosining ichki tuzilishi |
| **"Understanding the Linux Kernel"** | Chuqurroq: xotira, fayl tizimlari |
| Spetsifikatsiyalar: ACPI, AHCI, NVMe, xHCI, ext2 (`nongnu.org/ext2-doc`) | Drayver yozishda — faqat ular |

## 31. Yo'l xaritasi: bu loyihadan keyin

1. **MyOS'ni tugating:** 22 ta lab → 26-bo'limdagi loyihalardan kamida uchtasi (tavsiya: `/proc`, NVMe yoki xHCI, tarmoq ping'gacha).
2. **Noldan yozing:** [QOLLANMA.md](QOLLANMA.md), 11-bo'lim — 19 qadamli reja. Kodga qaramasdan. Bu sizning imtihoningiz.
3. **Linux'ga o'ting:** Linux yadrosini yig'ing, QEMU'da ishga tushiring, oddiy modul yozing (`hello.ko`), keyin `drivers/staging` dagi kichik tuzatishlar. kernelnewbies.org — "birinchi patch" qo'llanmasi. Birinchi qabul qilingan patch — rezyumedagi eng kuchli qator.
4. **Mutaxassislik tanlang:** xotira boshqaruvi, fayl tizimlari, tarmoq, drayverlar, virtualizatsiya (KVM), xavfsizlik. Birida chuqur bo'ling, qolganlarini umumiy biling.

---

## ENDI O'ZING DAVOM ET

Bu hujjat — xarita, yo'lning o'zi emas. Yo'lni siz yurasiz. Har kuni:

- **bitta funksiyani** belgima-belgi tushuntiring (18-bo'lim usulida);
- **bitta labni** kodga qaramasdan yozing (`tools/lab.py boshla ...`);
- **bitta kichik narsa** qo'shing (22–25-bo'limlar);
- **bitta commit** qiling.

Qiyin joyga kelsangiz, mana maslahatlar:

- **"Qayerdan boshlashni bilmayapman"** → o'xshash mavjud kodni toping va nusxasini oling (`hello.c` → yangi dastur, `ahci.c` → yangi drayver, `devfs.c` → yangi fayl tizimi). Keyin kerakmasini o'chiring.
- **"Ishlamayapti, sababini bilmayman"** → `kprintf`. Har bir qadamdan keyin. Qaysi `kprintf` chiqmadi — xato o'sha yerda.
- **"Spetsifikatsiya juda katta"** → faqat kerakli bobni o'qing. OSDev Wiki'dan boshlang, ishlamay qolganda spetsifikatsiyaga qayting.
- **"Juda qiyin, men uddalay olmayman"** → bu loyihadagi hamma narsa ham bir kunda yozilmagan. `git log --reverse` ni oching: 21 ta bosqich, har biri oldingisining ustiga qurilgan. Siz ham shunday qilasiz — bosqichma-bosqich.

Yadro dasturchisi — sehrgar emas. U shunchaki "bu qanday ishlaydi?" degan savolni hech qachon javobsiz qoldirmaydigan odam. Siz allaqachon shu yo'ldasiz.
