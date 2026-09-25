# 13 — Disklar va ext2: ATA, AHCI (DMA), bo'limlar, buffer cache, ext2 fayl tizimi

Kod: `kernel/drivers/{ata,ahci}.c`, `kernel/fs/{block,ext2}.c`, `tools/mkdisk.py`, `user/bin/{init,cksum}.c`,
`rootfs/etc/rc`

Endi fayllar kompyuter o'chganda ham saqlanib qoladi:

```
[ahci] AHCI 1.0, 6 port, 64-bit DMA: ha
[ahci] port 0: sda
[blk]  sda: 64 MB "QEMU HARDDISK"
[blk]  sda1: bo'lim, sektor 2048, 63 MB
[ext2] sda1: "myos", 1 KB blok, 8 guruh, 30/16128 inode, 57 MB bo'sh
[rc] /dev/sda1 (ext2) -> /mnt
myos:/$ cksum /mnt/test/katta.bin
811540523 300000 /mnt/test/katta.bin        <- Linux'dagi `cksum` bilan aynan bir xil
```

## 1. Qatlamlar

```
  open("/mnt/a.txt") ─► VFS ─► ext2 ─────────────► "inode 12 ning 3-bloki = disk bloki 5021"
                                 │ bread(5021)
                                 ▼
                          buffer cache ───────────► RAM'da bormi? ha → darhol
                                 │ yo'q
                                 ▼
                          blok qatlami ───────────► sda1 → sda (+2048 sektor siljish)
                                 │ read(lba, count)
                         ┌───────┴────────┐
                       ata.c            ahci.c
                    (IDE, PIO)        (SATA, DMA)
```

Har bir qatlam faqat pastki qatlamning **interfeysini** biladi. ext2 diskning IDE yoki SATA ekanini bilmaydi,
drayver esa fayllar haqida hech narsa bilmaydi.

## 2. ATA PIO (`drivers/ata.c`)

1986 yildan beri deyarli o'zgarmagan interfeys. Kontroller **I/O portlar** orqali boshqariladi: `0x1F0..0x1F7`
buyruq registrlari, `0x3F6` esa boshqaruv registri.

```
o'qish:  BSY=0 ni kutish → DRIVE, SECCOUNT, LBA → COMMAND=0x24 (READ SECTORS EXT)
         har bir sektor: BSY=0 va DRQ=1 ni kutish → 256 marta inw (rep insw)
```

- **IDENTIFY** (`0xEC`) — disk o'zi haqida 512 bayt ma'lumot beradi. Undagi model nomining **baytlari
  almashgan** holda saqlanadi: "EQUMH RADDSI K".
- CD-ROM (ATAPI) IDENTIFY ni rad etadi va LBA registrlariga `0x14 0xEB` imzosini yozadi.
- **LBA28** 128 GB gacha ishlaydi. Undan katta disklar uchun **LBA48** kerak: har bir registrga ikki marta
  yoziladi, avval yuqori, keyin quyi bayt.
- Yozishdan keyin **FLUSH CACHE** buyrug'i beriladi. Diskning o'z ichki keshi (8–256 MB) bor. Busiz tok
  o'chganda "yozildi" deb hisoblangan ma'lumot yo'qolishi mumkin.

## 3. AHCI va DMA (`drivers/ahci.c`)

Zamonaviy kompyuterdagi SATA kontroller. PIO'dan asosiy farqi — **DMA**: CPU ma'lumotni o'zi tashimaydi,
faqat xotirada buyruq tayyorlab qo'yadi.

```
 CPU:  Command Table ga yozadi:  CFIS = {READ DMA EXT, LBA=5021, soni=8}
                                 PRDT = {fizik manzil 0x3A5000, 4096 bayt}
       PxCI |= 1                 "0-slotni bajar"
 AHCI: diskdan o'qiydi → RAM 0x3A5000 ga O'ZI yozadi → PxCI dagi bitni o'chiradi
 CPU:  bit o'chdi → tayyor
```

Muhim tafsilotlar:

- Registrlar **MMIO**. PCI BAR5 fizik manzilini `ioremap()` qilamiz (UC — keshlanmaydigan xotira).
- DMA **fizik** manzillar bilan ishlaydi. Shuning uchun buyruq tuzilmalari `pmm_alloc_page` bilan olinadi.
  Kontroller 64-bitli manzillarni qo'llamasa (`CAP.S64A=0`), `GFP_DMA32` bilan 4 GB dan past xotira olinadi.
- `pci_enable()` **bus master** bitini yoqadi. Bu bit bo'lmasa, qurilma xotiraga yoza olmaydi va DMA "jim"
  ishlamaydi.
- **BIOS handoff**: UEFI/BIOS kontrollerni ishlatayotgan bo'lishi mumkin. `BOHC` registri orqali egalik
  so'raladi.
- Soddalashtirish: bitta slot, polling va 64 KB bounce bufer. Linux 32 slot va NCQ (Native Command Queuing)
  ishlatadi, bu esa diskka buyruqlarni o'zi uchun eng qulay tartibda bajarish imkonini beradi.

**Haqiqiy kompyuterda:** BIOS/UEFI sozlamalarida SATA rejimi odatda "AHCI" bo'ladi. "RAID/RST" (Intel) rejimida
kontroller boshqa class kod bilan ko'rinadi va bizning drayver uni tanimaydi. "IDE/Compatible" rejimida esa
`ata.c` ishlaydi.

## 4. Bo'limlar va buffer cache (`fs/block.c`)

- **MBR**: 0-sektorning 446-baytidan 4 ta 16 baytli yozuv boshlanadi, oxirida `55 AA` imzosi turadi.
  `tools/mkdisk.py` aynan shunday disk yaratadi: 1-bo'lim 2048-sektordan (1 MB) boshlanadi.
- **GPT**: MBR'da `0xEE` turi bo'lsa ("himoya MBR"), 1-sektordagi `EFI PART` sarlavhasi va bo'limlar
  jadvali o'qiladi. Zamonaviy kompyuterlarning ko'pchiligi GPT ishlatadi.
- Har bir bo'lim alohida `block_device` sifatida qo'shiladi (`sda1`, 8:1). `blk_read(sda1, 0)` so'rovi
  `sda` ning 2048-sektoriga aylanadi.
- **Buffer cache**: oxirgi 1024 ta blok RAM'da (LRU) saqlanadi. `bread()` blokni **qulflangan** holda
  qaytaradi, `brelse()` esa qulfni bo'shatadi. Bitta blokni ikki jarayon bir vaqtda o'zgartira olmaydi.

## 5. ext2 (`fs/ext2.c`)

### Disk tuzilishi

```
[boot][SUPERBLOK][guruh deskriptorlari][blok bitmap][inode bitmap][inode jadvali][ma'lumot ...]
       └──── bayt 1024 ────┘            └───────────── har bir blok guruhida ──────────────┘
```

`debugfs -R stats build/test-part-bios.img` buyrug'i bilan hammasini ko'rish mumkin.

### Faylning bloklari: `i_block[15]`

```
i_block[0..11] → 12 ta ma'lumot bloki               (1 KB blokda: 12 KB)
i_block[12]    → [256 ko'rsatkich] → ma'lumot         (+256 KB)
i_block[13]    → [256] → [256] → ma'lumot             (+64 MB)
i_block[14]    → [256] → [256] → [256] → ma'lumot     (+16 GB)
```

`bmap(inode, n, alloc)` n-blokning disk manzilini topadi. `alloc=true` bo'lsa, yetishmayotgan bloklarni
(bilvosita bloklarni ham) ajratadi. 300 KB'li `katta.bin` ikki karra bilvosita bloklardan foydalanadi va
test uni CRC bilan tekshiradi.

`ext2_truncate` teskari ishni bajaradi: `trunc_tree` daraxtni rekursiv aylanib chiqadi va chegaradan
keyingi hamma narsani bo'shatadi. Oxirgi qisman blokning qolgan qismi **nollanadi**. Aks holda fayl keyin
kattalashtirilganda eski ma'lumot "qaytib chiqardi".

### Papkalar

Papka — yozuvlar ro'yxatidan iborat fayl: `[inode][rec_len][name_len][tur][nom]`.

- **Qo'shish**: shunday yozuv qidiriladiki, uning `rec_len` i haqiqiy hajmidan kattaroq bo'lsin. Keyin u
  ikkiga bo'linadi. Joy topilmasa, papkaga yangi blok qo'shiladi.
- **O'chirish**: yozuvning joyi oldingisining `rec_len` iga qo'shib yuboriladi. Blokdagi birinchi yozuv
  bo'lsa, `inode = 0` qilinadi.
- `fstest` dagi "ko'p fayl" testi 300 ta fayl yaratadi (papka ~15 blokka o'sadi), yarmini o'chiradi va
  bo'shagan joylarga 100 ta yangisini qo'shadi.

### Inode keshi: bitta inode — bitta obyekt

Bir inode xotirada **ikki nusxada** bo'lsa, bir jarayon faylga yozganini ikkinchisi ko'rmaydi. Hajm va
blok ro'yxati bir-birini bosib ketadi. Shuning uchun `ext2_iget` avval keshdan qidiradi.

Nozik holat: inode ayni paytda o'chirilayotgan bo'lishi mumkin (refcount 0 ga tushgan, lekin hali
ro'yxatdan chiqarilmagan). `get_unless_zero` bunday inode'ni "tiriltirmaydi". Linux'da ham xuddi shu
g'oya ishlatiladi (`igrab`, `atomic_inc_not_zero`).

### Yozish tartibi — jurnalsiz ishonchlilik

Jurnal yo'q, shuning uchun har bir amal quyidagi tartibda bajariladi: avval **ajratish** (bitmap), keyin
**to'ldirish** (inode, blok), eng oxirida **havola qilish** (papka yozuvi). Oraliqda tok o'chsa, eng yomon
holatda hech kimga tegishli bo'lmagan band blok qoladi va `e2fsck` uni qaytaradi. Ikki fayl bitta blokni
**ulashib qolmaydi**, bu esa ancha xavfli xato bo'lardi.

`rename` avval yangi nomni qo'shadi, keyin eskisini o'chiradi. Oraliqda tok o'chsa, fayl ikki nomga ega
bo'ladi, lekin yo'qolmaydi.

### Linux vositalari bilan tekshirish

`make test` tizim o'chgandan keyin disk tasvirini Linux'ning o'z vositalari bilan tekshiradi:

```bash
dd if=build/test-disk-bios.img of=part.img bs=1M skip=1   # bo'limni ajratib olish
e2fsck -fn part.img                                       # tuzilma to'g'rimi?
debugfs -R "cat /yangi/seq.txt" part.img | cksum          # mazmun to'g'rimi?
```

Bu **standartga moslik** testi. Yadromiz Linux yaratgan fayl tizimini o'qiydi, Linux esa bizning
yozganimizni o'qiydi. Yozish kodidagi har bir xato (`i_blocks` noto'g'ri, `nlink` noto'g'ri, bitmap bilan
sanagich mos emas) e2fsck'ning 5 bosqichidan birida albatta chiqadi.

## 6. init va /etc/rc

Endi yadro faqat `/bin/init` ni ishga tushiradi. Qolgan ishlar user rejimida bajariladi:

```
yadro ─► /bin/init ─┬─► sh /etc/rc          (bir marta: mount -t ext2 /dev/sda1 /mnt)
                    └─► sh ─► sh ─► ...     (shell tugasa, yangisi ochiladi)
```

Buyruq qatoriga `init=/bin/sh` yozilsa, init o'tkazib yuboriladi va darhol shell ochiladi. Bu tiklash
rejimi: masalan, `/etc/rc` buzilgan bo'lsa ishlatiladi.

## Sinab ko'ring

```bash
make run                     # pc: IDE disk (ata.c)
make run MACHINE=q35         # q35: SATA disk (ahci.c)
make disk                    # toza disk tasvirini qayta yaratish
make disk DISK_BLOCK=4096    # 4 KB blokli ext2 (haqiqiy disklardagi kabi)
```

```
myos:/$ fstest /mnt                       # 60 tekshiruv, endi diskda
myos:/$ seq 100000 > /mnt/s.txt ; poweroff
$ debugfs -R "cat /s.txt" part.img | tail -1   # Linux'da: 100000
```

## Mashqlar

1. **Oson:** `df` dasturi: `statfs` syscall'i yordamida superblokdagi bo'sh joyni ko'rsating.
2. **Oson:** `readlink` syscall'i va `ls -l` da `havola -> ../docs/12-vfs.md` ko'rinishi (tezkor symlink
   `i_block` ichida saqlanadi). Keyin VFS'da symlink'lar bo'ylab yurishni qo'shing.
3. **O'rta:** ATA drayverini **uzilishlar** bilan qayta yozing (IRQ 14/15): polling o'rniga `proc_sleep`,
   IRQ handler'da esa `wakeup`. CPU yuklamasi qanday o'zgaradi?
4. **O'rta:** AHCI'da bounce bufer o'rniga to'g'ridan-to'g'ri DMA qiling: buferning har bir sahifasi uchun
   alohida PRDT yozuvi (scatter-gather). Tezlikni `seq 1000000 > /mnt/x` bilan o'lchang.
5. **O'rta:** hard link (`ln`). ext2'da bu bor-yo'g'i `dir_add` + `nlink++`. e2fsck bilan tekshiring.
6. **Qiyin:** **write-back** kesh: `bwrite` darhol yozmasin, "iflos" bloklarni har 5 soniyada fon oqimi
   yozsin. Tezlik qanchalik oshadi? Tok o'chsa nima bo'ladi? ext3 aynan shu muammo uchun **jurnal**ni
   qo'shgan.
7. **Qiyin:** NVMe drayveri (M.2 SSD). AHCI'ga o'xshash, lekin navbatlar (submission/completion queue)
   ishlatiladi. QEMU: `-device nvme,drive=...,serial=1`.
8. **Katta:** root fayl tizimini diskka ko'chiring: `/bin`, `/etc` ext2'da bo'lsin, initrd esa faqat diskni
   ulab, `pivot_root` qilsin. Haqiqiy kompyuterga o'rnatish (installer) aynan shunga tayanadi.
