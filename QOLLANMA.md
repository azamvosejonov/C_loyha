# QO'LLANMA — shu yadroni tushunish va keyin o'zingiz noldan yozish uchun

> Bu faylni **birinchi** o'qing. Unda quyidagilar bor:
> 1. qayerdan boshlash va fayllarni qaysi tartibda o'qish;
> 2. loyihada ishlatilgan **har bir texnologiya**: nima qiladi, nega kerak, ishlatilmasa nima bo'ladi;
> 3. biz duch kelgan **haqiqiy xatolar** va ular qanday topilgani;
> 4. o'z yadroingizni **noldan, yordamsiz** yozish rejasi (bosqichma-bosqich, tekshiruv nuqtalari bilan).
>
> Batafsil tushuntirishlar `docs/` papkasida (00–15), izohlar esa kodning o'zida.
> Amaliy mashqlar (kodni yopib, o'zingiz yozish): [labs/README.md](labs/README.md).
> Haqiqiy kompyuterda ishlatish va tizimni kengaytirish: [YAKUNIY.md](YAKUNIY.md).

---

## MUNDARIJA

- [0. Maqsad va to'g'ri o'rganish usuli](#0-maqsad-va-togri-organish-usuli)
- [1. Oldindan kerakli bilimlar](#1-oldindan-kerakli-bilimlar)
- [2. Ish muhitini tayyorlash](#2-ish-muhitini-tayyorlash)
- [3. Loyiha xaritasi: nima qayerda](#3-loyiha-xaritasi-nima-qayerda)
- [4. O'qish tartibi: qaysi fayldan boshlash](#4-oqish-tartibi-qaysi-fayldan-boshlash)
- [5. C tili: har bir vosita — nima, nega, bo'lmasa nima bo'ladi](#5-c-tili-har-bir-vosita)
- [6. Kompilyator, linker va Makefile](#6-kompilyator-linker-va-makefile)
- [7. Assembly va x86-64 protsessor mexanizmlari](#7-assembly-va-x86-64-protsessor-mexanizmlari)
- [8. Operatsion tizim g'oyalari](#8-operatsion-tizim-goyalari)
- [9. Haqiqiy xatolar va ularni topish usullari](#9-haqiqiy-xatolar-va-ularni-topish-usullari)
- [10. Debug qilish vositalari](#10-debug-qilish-vositalari)
- [11. O'z yadroingizni noldan yozish rejasi](#11-oz-yadroingizni-noldan-yozish-rejasi)
- [12. Lug'at](#12-lugat)

---

## 0. Maqsad va to'g'ri o'rganish usuli

Maqsad — kodni **yodlash emas**. Maqsad shunday darajaga yetish: har bir qator **nima uchun** borligini
tushuntira olasiz va kodni yopib qo'yib, xuddi shunday tizimni o'zingiz yoza olasiz. Buning uchun har bir
qatlamda to'rt bosqichli usuldan foydalaning:

```
1. O'QISH      docs/NN-*.md ni o'qing, keyin tegishli .c faylni izohlari bilan o'qing.
2. BUZISH      kodda bitta narsani ataylab o'zgartiring (masalan, qulfni olib tashlang),
               `make test` ni ishga tushiring va NIMA buzilganini, NEGA buzilganini tushuning.
3. QAYTA YOZISH  funksiyani o'chiring va qog'ozga qaramasdan qayta yozing. Test o'tsin.
4. TUSHUNTIRISH  shu qatlamni boshqa odamga (yoki ovoz chiqarib o'zingizga) 5 daqiqada tushuntiring.
               Tushuntira olmagan joyingiz - hali tushunmagan joyingiz.
```

**Eng muhim qoida:** o'zingiz "nima uchun?" deb so'ramagan birorta ham qatorni o'tkazib yubormang. Masalan,
`volatile` turibdi — nima uchun? Olib tashlasangiz nima bo'ladi? Javobni 5- va 7-bo'limlardan toping,
keyin o'zingiz sinab ko'ring.

**Git tarixi ham darslik.** Har bir commit bitta bosqich, shuning uchun istalgan bosqichga qaytib, o'sha
holatni ishga tushirish mumkin:

```bash
git log --oneline --reverse          # barcha bosqichlar ro'yxati
git checkout e5906bb                 # birinchi, KICHIK yadro (v0.1) - boshlash uchun eng yaxshisi
make run
git checkout -                       # hozirgi holatga qaytish
```

---

## 1. Oldindan kerakli bilimlar

| mavzu | qanchalik kerak | qayerdan o'rganish |
|---|---|---|
| C tili: ko'rsatkichlar, struct, massivlar, bit amallari | **majburiy**, erkin darajada | K&R "The C Programming Language" |
| Ikkilik va o'n oltilik sanoq tizimi | majburiy | 0x1000 = 4096 ekanini tez ko'rish kerak |
| Linux terminali: `cd`, `ls`, `grep`, `make`, `git` | majburiy | amaliyot |
| Kompyuter arxitekturasi: registr, stek, xotira, kesh | kerak | "Computer Systems: A Programmer's Perspective" (CS:APP) |
| x86 assembly asoslari | loyiha davomida o'rganiladi | Intel SDM, 1-jild; `docs/01-yuklash.md` |
| Operatsion tizim nazariyasi | loyiha davomida | "Operating Systems: Three Easy Pieces" (OSTEP), bepul |

**Kitoblar va manbalar (dunyo standarti):**
- **OSTEP** (ostep.org) — OT nazariyasi uchun eng yaxshi bepul kitob.
- **xv6** (MIT 6.S081) — o'quv yadrosi. Bizning scheduler va `sleep/wakeup` modelimiz xv6 dan olingan.
- **Intel SDM, 3-jild** — CPU'ning rasmiy qo'llanmasi (paging, uzilishlar, APIC).
- **OSDev Wiki** (wiki.osdev.org) — amaliy maqolalar: har bir qurilma bo'yicha.
- **Linux manbasi** (elixir.bootlin.com) — "haqiqiy dunyoda qanday qilingan?" savoliga javob.

---

## 2. Ish muhitini tayyorlash

```bash
# Ubuntu / Debian / WSL:
sudo apt install build-essential nasm qemu-system-x86 gdb e2fsprogs \
                 grub-pc-bin grub-efi-amd64-bin grub-common xorriso mtools ovmf python3

make                 # build/myos.iso va build/disk.img ni yig'ish
make run             # QEMU: ekran + serial terminal
make run MACHINE=q35 # SATA/AHCI disk bilan
make run-uefi        # UEFI rejimida
make test            # to'liq avtomatik test (BIOS + UEFI), ~2 daqiqa
make debug           # GDB bilan (docs/08-test-debug.md)
make disk            # toza disk tasvirini qayta yaratish
make clean           # hammasini o'chirish
```

Yuklangandan keyin tizim ichida `help`, `cat /README.txt`, `ls /bin` buyruqlarini sinab ko'ring.

**Har bir o'zgarishdan keyin** `make test` ni ishga tushiring. 41 ta tekshiruv 2 daqiqada nima buzilganini
aytadi. Katta jamoalarda ham shunday ishlanadi: test o'tmasa, kod qabul qilinmaydi.

---

## 3. Loyiha xaritasi: nima qayerda

```
C_loyha/
├── QOLLANMA.md          ← SIZ SHU YERDASIZ
├── README.md            qisqa tavsif
├── Makefile             yig'ish qoidalari (izohli)
├── docs/                00-15: har bir qatlam bo'yicha batafsil tushuntirish, mashqlar
├── include/myos/abi.h   yadro <-> dasturlar SHARTNOMASI: syscall raqamlari, errno, strukturalar
├── kernel/              YADRO (~15 000 qator: C + assembly)
│   ├── boot/            yuklanish: 32-bit -> 64-bit, Multiboot2 ma'lumotlari
│   ├── arch/            x86 ga xos: GDT, IDT, uzilishlar, APIC, SMP, syscall kirish nuqtasi
│   ├── mm/              xotira: memblock, buddy, slab, vmalloc, sahifa jadvallari, VMA/COW
│   ├── proc/            jarayonlar, scheduler, fork/exec, signallar
│   ├── fs/              VFS, tmpfs, devfs, pipe, blok qatlami, ext2
│   ├── drivers/         konsol, terminal, klaviatura, serial, ATA, AHCI, PCI, taymer, RTC
│   ├── sys/             syscall'lar, ELF yuklovchi, user xotirasiga xavfsiz murojaat
│   ├── acpi/            ACPI jadvallari (CPU'lar ro'yxati, o'chirish)
│   ├── lib/             kprintf, spinlock, mutex, string, panic
│   ├── tests/           yadro ichidagi testlar (selftest)
│   └── main.c           kmain() - hamma narsa shu yerdan tartib bilan ishga tushadi
├── user/                FOYDALANUVCHI DASTURLARI (ring 3)
│   ├── include/         bizning libc sarlavhalarimiz: stdio.h, unistd.h, signal.h ...
│   ├── libc/            bizning libc: syscall o'ramlari, printf, malloc, stdio
│   └── bin/             sh, ls, cat, edit, ... (44 ta dastur)
├── rootfs/              /etc/rc, /etc/motd - boot diskiga (initrd) qo'shiladi
└── tools/               test.sh, mkdisk.py, gdbinit, psf2c.py
```

**Bitta tugma bosilgandan ekranda harf paydo bo'lguncha** (butun tizim bo'ylab yo'l):

```
klaviatura ─IRQ1─► IOAPIC ─► CPU: IDT[33] ─► isr.asm ─► interrupt_dispatch()
   ─► keyboard_irq(): scancode -> 'a' ─► console_input_char() ─► kiritish navbati
   ─► uxlab turgan sh uyg'otiladi (proc_wakeup) ─► scheduler sh ni tanlaydi
   ─► sh: read() tugadi ─► sh: write(1, "a") ─► syscall_entry.asm ─► sys_write()
   ─► file_write() ─► tty_write() ─► console_write_tty() ─► vt_putc() ─► fbcon_draw()
   ─► framebuffer xotirasiga piksellar ─► videokarta ekranga chiqaradi
```

Bu zanjirning har bir halqasini tushuntira olsangiz, yadroni tushungan bo'lasiz.

---

## 4. O'qish tartibi: qaysi fayldan boshlash

### 1-yo'l (tavsiya etiladi): avval kichik yadro

`git checkout e5906bb` — bu v0.1: taxminan 5000 qator, bitta CPU, identity paging. `docs/00-08` aynan shu
versiyani tushuntiradi. U oddiyroq, g'oyalar esa bir xil. Uni to'liq tushunib olgach, hozirgi versiyaga
o'ting.

### 2-yo'l: hozirgi yadroni qatlamma-qatlam o'qish

Har bir qadamda: **hujjat → kod → "buzib ko'ring" → mashq**.

| # | hujjat | fayllar (shu tartibda) | tushunishingiz kerak bo'lgan savol |
|---|---|---|---|
| 1 | 00, 01, 09 | `kernel/linker.ld`, `kernel/boot/boot.asm`, `boot/bootinfo.c` | CPU yoqilgandan `kmain()` gacha nima sodir bo'ladi? |
| 2 | 01 | `kernel/main.c` (faqat `kmain`) | Nega ishga tushirish tartibi aynan shunday? |
| 3 | 02 | `drivers/serial.c`, `lib/kprintf.c`, `drivers/console.c` | Birinchi xabar ekranga qanday chiqadi? |
| 4 | 02 | `arch/gdt.c`, `arch/idt.c`, `arch/isr.asm`, `arch/interrupts.c` | Uzilish kelganda CPU va biz aniq nima qilamiz? |
| 5 | 03, 09 | `mm/memblock.c`, `mm/pmm.c` | Bo'sh fizik sahifalarni qanday topamiz va qanday beramiz? |
| 6 | 04, 09 | `mm/vmm.c`, `mm/layout.h` | Virtual manzil fizik manzilga qanday aylanadi? |
| 7 | 05, 09 | `mm/slab.c`, `mm/vmalloc.c` | `kmalloc(24)` qanday ishlaydi? |
| 8 | 06, 10 | `lib/spinlock.c`, `proc/process.c`, `proc/switch.asm` | Ikki jarayon bitta CPU'da qanday almashadi? |
| 9 | 10 | `arch/apic.c`, `arch/smp.c`, `arch/trampoline.asm`, `arch/percpu.c` | Ikkinchi CPU qanday uyg'otiladi? |
| 10 | 07, 10 | `arch/syscall_entry.asm`, `sys/syscall.c`, `sys/elf.c`, `proc/exec.c` | `printf` qanday qilib yadroga yetadi? |
| 11 | 11 | `mm/mm.c` (`mm_fork`, `mm_handle_fault`) | `fork()` nega xotirani darhol nusxalamaydi? |
| 12 | 12 | `fs/vfs.c`, `fs/tmpfs.c`, `fs/pipe.c`, `sys/sys_fs.c` | `ls \| wc` da ma'lumot qayerdan qayerga oqadi? |
| 13 | 13 | `fs/block.c`, `drivers/ata.c`, `drivers/ahci.c`, `fs/ext2.c` | Fayl diskda qanday saqlanadi? |
| 14 | 14 | `proc/signal.c`, `drivers/tty.c` | Ctrl-C qanday qilib `spin` ni to'xtatadi? |
| 15 | 15 | `drivers/vt.c`, `drivers/keyboard.c`, `user/bin/edit.c` | Muharrir ekranni qanday chizadi? |
| + | 12 | `user/libc/*`, `user/bin/sh.c` | libc va shell yadroning ustida qanday qurilgan? |

**Har bir fayldagi o'qish tartibi:** avval fayl boshidagi katta izohni o'qing, keyin `.h` faylni
(interfeysni), keyin eng pastdagi "kirish nuqtasi" funksiyasini (`*_init`, `sys_*`), va shundan keyingina
yordamchi funksiyalarni.

---

## 5. C tili: har bir vosita

Har bir vosita uchun to'rt savolga javob beriladi: **Nima? — Loyihada qayerda? — Nega? — Bo'lmasa nima
bo'ladi?**

### 5.1. `void`

`void` uchta turli ma'noda ishlatiladi:

**a) "Hech narsa qaytarmaydi":** `void kmain(...)`, `void spin_lock(spinlock_t *l)`.
- *Nega:* funksiya faqat ish bajaradi, natija bermaydi.
- *Bo'lmasa:* `int` deb yozib, hech narsa qaytarmasangiz, chaqiruvchi RAX dagi tasodifiy qiymatni oladi.
  Bu aniqlanmagan xatti-harakat (undefined behavior).

**b) `void *` — "istalgan turdagi ma'lumotga ko'rsatkich":** `void *kmalloc(size_t)`, `memcpy(void *dst, ...)`,
`inode->priv`.
- *Nega:* `kmalloc` nima uchun xotira so'ralayotganini bilmaydi. `void *` istalgan ko'rsatkichga cast'siz
  aylanadi: `struct inode *in = kmalloc(sizeof(*in));`. VFS'da `inode->priv` har bir fayl tizimining o'z
  ma'lumotiga ko'rsatadi (tmpfs uchun bitta tur, ext2 uchun boshqasi) — C tilidagi "generics"ning bir
  ko'rinishi.
- *Bo'lmasa:* har bir tur uchun alohida `kmalloc_inode`, `kmalloc_file` yozishga to'g'ri kelardi.
- *Xavf:* `void *` turi tekshirilmaydi. ext2 ning `priv` ini tmpfs'niki deb o'qisangiz, kompilyator hech
  narsa demaydi. Shuning uchun `FS(in)`, `NODE(in)` kabi makrolar faqat bir joyda cast qiladi.

**c) `(void)x;` — "bu o'zgaruvchini ataylab ishlatmayapman":** `(void)frame;` uzilish handler'larida.
- *Nega:* bizda `-Wextra -Werror` bayroqlari bor. Ishlatilmagan parametr ogohlantirish beradi, u esa xato
  deb hisoblanadi.
- *Bo'lmasa:* yig'ish to'xtaydi.

**d) `f(void)` va `f()` farqi:** C'da `int f()` "argumentlari noma'lum" degan ma'noni beradi (eski K&R
uslubi), `int f(void)` esa "argumenti yo'q" degani. Biz doim `(void)` yozamiz, shunda `f(5)` kabi xato
chaqiruvni kompilyator ushlaydi.

### 5.2. Ko'rsatkichlar va ko'rsatkich arifmetikasi

- `uint64_t *pte = &table[index];` — sahifa jadvali yozuviga ko'rsatkich (`mm/vmm.c`).
- `p + 1` baytga emas, **bitta elementga** siljiydi: `uint64_t *` uchun +8 bayt. Baytlar bo'yicha siljish
  kerak bo'lsa, avval `uint8_t *` ga cast qilinadi (`fbcon.c`: `fb + y * pitch`).
- **Manzil va son:** yadroda manzil ko'pincha son sifatida hisoblanadi (`uint64_t phys`), keyin
  ko'rsatkichga aylantiriladi: `phys_to_virt(phys)`. Buning uchun `uintptr_t` turi ishlatiladi, chunki u
  ko'rsatkichni sig'dirishi kafolatlangan.
- **Funksiya ko'rsatkichlari** — "C tilidagi polimorfizm":
  ```c
  struct file_ops { int64_t (*read)(struct file *, void *, size_t, uint64_t); ... };
  f->fops->read(f, buf, len, off);   // tmpfs_read, ext2_read, pipe_read yoki tty_read - qaysi fayl bo'lsa
  ```
  *Nega:* VFS qaysi fayl tizimi bilan ishlayotganini bilmaydi. Linux'da ham aynan shu naqsh
  (`struct file_operations`) ishlatiladi. *Bo'lmasa:* VFS da har bir fayl tizimi uchun `if (tmpfs) ...
  else if (ext2) ...` shoxlari bo'lardi, yangi FS qo'shish esa butun VFS'ni o'zgartirishni talab qilardi.

### 5.3. `struct`, `enum`, `typedef`, `union`

- **`struct`** — bog'liq ma'lumotlarni birlashtiradi (`struct process`, `struct inode`). Xotirada
  maydonlar **ketma-ket** turadi (tekislash bilan). Apparat tuzilmalarini (GDT yozuvi, ext2 superbloki)
  aynan shu xususiyat tufayli struct orqali o'qiymiz.
- **`enum`** — nomlangan sonlar: `enum proc_state { PROC_UNUSED, PROC_READY, ... }`. *Nega:*
  `p->state == 2` o'rniga `p->state == PROC_READY` deb yoziladi, `switch` da esa kompilyator unutilgan
  holatlar haqida ogohlantiradi.
- **`typedef`** — turga yangi nom beradi: `typedef struct { ... } spinlock_t;`. Biz uni kam ishlatamiz
  (Linux uslubi): `struct inode` deb yozish turning struct ekanini ko'rsatib turadi.
- **`union`** — bitta xotirada bir nechta talqin. Bizda deyarli yo'q, chunki xavfli.

### 5.4. `static`

**a) Fayl darajasidagi `static` (funksiya yoki o'zgaruvchi):** `static struct process procs[MAX_PROCS];`,
`static void sched(void)`.
- *Nega:* nom faqat shu `.c` fayl ichida ko'rinadi (inkapsulyatsiya). `process.c` dagi `procs` massiviga
  boshqa fayllar to'g'ridan-to'g'ri tega olmaydi, faqat `proc_slot()` orqali murojaat qiladi.
- *Bo'lmasa:* ikki faylda bir xil nomli `lock` o'zgaruvchisi bo'lsa, linker "multiple definition" xatosini
  beradi yoki, undan ham yomoni, ular bitta o'zgaruvchiga aylanib qolishi mumkin. Katta loyihada har bir
  global nom butun dastur uchun "ifloslanish" hisoblanadi.

**b) Funksiya ichidagi `static`:** `static int next;` (`blkdev_assign_sd`).
- *Nega:* o'zgaruvchi chaqiruvlar orasida qiymatini **saqlaydi** va stekda emas, `.bss`/`.data` da turadi.
- *Xavf:* bunday o'zgaruvchi hamma chaqiruvchilar uchun umumiy. Ikki CPU bir vaqtda o'zgartirsa, poyga
  holati yuzaga keladi. Shuning uchun u yerda `__atomic_fetch_add` ishlatilgan.

**c) Katta massivlar uchun `static`:** `static char buf[64 * 1024];` (`dmesg.c`).
- *Nega:* user stek boshida 64 KB, yadro steki esa atigi 16 KB. Stekda katta massiv e'lon qilinsa, stek
  to'lib ketadi.

### 5.5. `extern`

`extern spinlock_t proc_lock;` (`process.h`) — "bu o'zgaruvchi boshqa faylda **aniqlangan**, bu yerda
faqat e'lon qilinmoqda". `extern const uint8_t font8x16_glyphs[][16];` ham shunday.
*Bo'lmasa:* `.h` faylda `spinlock_t proc_lock;` yozilsa, uni include qilgan har bir `.c` faylda alohida
nusxa paydo bo'ladi va linker xato beradi.

### 5.6. `const`

`const char *name`, `static const struct file_ops tmpfs_file_ops = {...}`.
- *Nega:* (1) funksiya bu ma'lumotni o'zgartirmaydi degan **va'da** — o'qiyotgan odam uchun ham,
  kompilyator uchun ham; (2) `const` global ma'lumot `.rodata` bo'limiga tushadi. Bizning yadroda u
  **faqat o'qish uchun** sahifalarda turadi (W^X, `vmm.c`). `file_ops` jadvalini (funksiya ko'rsatkichlari)
  hujumchi o'zgartira olmaydi.
- *Bo'lmasa:* funksiya ko'rsatkichlari jadvali yoziladigan xotirada bo'lardi. Bu klassik hujum yo'li:
  xotiraga bitta yozish orqali yadroning boshqaruvini egallash mumkin bo'ladi.

### 5.7. `volatile` — eng ko'p noto'g'ri tushuniladigan so'z

**Nima:** "bu xotirani har safar HAQIQATAN o'qi/yoz, keshlab yoki optimallashtirib yuborma". Kompilyator
odatda qiymatni registrda saqlaydi va takroriy o'qishlarni olib tashlaydi.

**Loyihada qayerda:**
1. **Qurilma registrlari (MMIO):** `volatile uint8_t *abar` (`ahci.c`),
   `#define VGA_MEMORY ((volatile uint16_t *)...)`. AHCI'dagi `PxCI` registri har o'qilganda **qurilma**
   o'zgartirgan yangi qiymatni qaytaradi.
   *`volatile` bo'lmasa:* `while (rd(p, PX_CI) & 1);` tsiklida kompilyator qiymatni bir marta o'qiydi va
   tsikl cheksiz aylanadi (yoki umuman olib tashlanadi). Framebuffer'ga yozish ham "keraksiz" deb olib
   tashlanishi mumkin, chunki kompilyator nuqtai nazaridan hech kim bu xotirani o'qimaydi.
2. **Signal handler'i o'zgartiradigan o'zgaruvchi:** `static volatile sig_atomic_t hits;` (`sigtest.c`).
   *`volatile` bo'lmasa:* `while (!hits);` tsiklida `hits` registrga olinadi va handler uni xotirada
   o'zgartirganini dastur hech qachon ko'rmaydi.
3. **`__asm__ volatile`** — inline assembly'ni olib tashlamaslik va joyini o'zgartirmaslik (7.1-bo'lim).

**`volatile` NIMA QILMAYDI:** u **atomar emas** va **CPU'lar orasida tartibni kafolatlamaydi**.
Ko'p CPU'da umumiy o'zgaruvchi uchun `volatile` yetarli emas: `__atomic_*` funksiyalari yoki qulf kerak
(5.13). Bu eng keng tarqalgan xatolardan biri.

### 5.8. `inline` va `static inline`

`static inline void outb(uint16_t port, uint8_t v)` (`arch/io.h`), `static inline uint64_t rdtsc(void)`.
- *Nega:* bunday funksiyalar 1–2 instruksiyadan iborat. Chaqiruv (`call`/`ret`, registrlarni saqlash)
  funksiyaning o'zidan qimmatroq tushadi. `inline` funksiya kodini chaqiruv joyiga qo'yadi. `.h` faylda
  `static inline` shart: aks holda include qilgan har bir fayl nusxa yaratadi va linker xato beradi.
- *Nega makro emas?* `#define outb(p, v) ...` da tur tekshiruvi yo'q va argument ikki marta hisoblanishi
  mumkin. `inline` funksiya esa xavfsiz.

### 5.9. Aniq kenglikdagi turlar: `uint8_t` ... `uint64_t`, `size_t`

- *Nega:* C'da `int` va `long` o'lchami platformaga bog'liq. Apparat tuzilmalarida esa har bir bit aniq
  joyda turishi kerak: GDT yozuvi aynan 8 bayt, ext2 superbloki aynan 1024 bayt. `uint32_t` hamma joyda
  aniq 32 bit.
- `size_t` — xotira hajmi uchun (x86-64 da 64 bit). `ssize_t` — hajm yoki manfiy xato.
- `int64_t` syscall natijalari uchun: musbat qiymat natija, manfiy qiymat `-errno`.
- *Bo'lmasa:* ext2 da `unsigned long` 64 bitli bo'lgani uchun inode tuzilmasi siljib ketadi va disk
  "buzilgan" bo'lib ko'rinadi.
- **Ishorali/ishorasiz tuzoq:** `if (len - pos > 0)` — agar `len` va `pos` `size_t` bo'lsa, ayirma hech
  qachon manfiy bo'lmaydi va `len < pos` holatida juda katta son chiqadi. Kodda `pos < len` shaklida
  taqqoslash shu sababdan.

### 5.10. Bit amallari va niqoblar

```c
#define PTE_PRESENT (1ULL << 0)
#define PTE_WRITE   (1ULL << 1)
pte = phys | PTE_PRESENT | PTE_WRITE;     // bitlarni yoqish
if (pte & PTE_PRESENT)                    // tekshirish
pte &= ~PTE_WRITE;                        // o'chirish (COW: faqat o'qish)
uint64_t idx = (virt >> 39) & 0x1FF;      // manzildan 9 bitli indeksni ajratish
```
- *Nega:* apparat (sahifa jadvali, GDT, APIC registrlari) ma'lumotni **bitlarda** saqlaydi.
- **`1ULL` ga e'tibor bering:** `1 << 40` — `int` (32 bit) siljishi, ya'ni aniqlanmagan xatti-harakat.
  `1ULL << 40` esa 64 bitli. 63-bit (NX) uchun bu majburiy.
- `(x + 4095) & ~4095` — yuqoriga 4096 ga yaxlitlash (`ALIGN_UP`). Bu faqat 2 ning darajalari uchun
  ishlaydi.

### 5.11. Makrolar va preprotsessor

- `#pragma once` — sarlavha faylni bir marta include qilish. *Bo'lmasa:* `struct` ikki marta aniqlanib,
  xato beradi (eski usul: `#ifndef X_H #define X_H ... #endif`).
- `#define MIN(a,b) ((a) < (b) ? (a) : (b))` — qavslar shart. Busiz `MIN(x, y) * 2` noto'g'ri
  hisoblanadi.
- **`container_of` / `list_entry`** (`lib/list.h`) — Linux'ning eng mashhur hiylasi:
  ```c
  struct buf { ...; struct list_head node; ... };
  // ro'yxat faqat `node` larni biladi; node dan butun struct buf ga qaytish:
  struct buf *b = list_entry(ptr, struct buf, node);   // = (char*)ptr - offsetof(struct buf, node)
  ```
  *Nega:* bitta umumiy ro'yxat kodi istalgan struct uchun ishlaydi va qo'shimcha xotira ajratilmaydi.
- `_Static_assert(sizeof(struct ext2_inode) == 128, "...")` — **kompilyatsiya vaqtidagi** tekshiruv.
  Struct hajmi xato bo'lsa, kod umuman yig'ilmaydi. *Bo'lmasa:* xato faqat diskdagi ma'lumot buzilganda,
  ya'ni juda kech, ko'rinadi.

### 5.12. GCC atributlari (`__attribute__`)

| atribut | qayerda | nima uchun | bo'lmasa |
|---|---|---|---|
| `packed` | ext2 superbloki, GDT, ACPI jadvallari | kompilyator maydonlar orasiga bo'shliq qo'shmasin — apparat/disk formati aynan shunday | tekislash uchun bo'shliqlar qo'shiladi va maydonlar noto'g'ri baytlardan o'qiladi |
| `noreturn` | `panic`, `proc_exit`, `exit` | funksiya qaytmaydi — kompilyator keyingi kodni kutmaydi | "control reaches end" ogohlantirishlari, kamroq optimallashtirish |
| `format(printf, 1, 2)` | `kprintf`, `printf` | kompilyator `%d` ga `char *` berilganini ushlaydi | formatdagi xatolar faqat ish vaqtida chiqadi (bizda shunday bo'lgan: `%#x`, 9-bo'lim) |
| `aligned(16)` | stek, TSS | apparat tekislashni talab qiladi | #GP xatosi yoki sekinlashish |

**`packed` ning tuzog'i:** packed struct maydoniga ko'rsatkich olish (`&inode->i_block[0]`) tekislanmagan
bo'lishi mumkin. Bizda aynan shu xato chiqdi (`-Waddress-of-packed-member`). Yechim: tabiiy tekislangan
struct'ni `packed` qilmaslik (`ext2_inode`).

### 5.13. Atomar amallar va xotira tartibi (`__atomic_*`)

```c
__atomic_add_fetch(&inode->refcount, 1, __ATOMIC_RELAXED);        // iget
if (__atomic_sub_fetch(&in->refcount, 1, __ATOMIC_ACQ_REL) == 0)  // iput: oxirgimi?
```
- *Nega:* `x++` uchta amaldan iborat: o'qish, qo'shish, yozish. Ikki CPU bir vaqtda bajarsa, qo'shishlardan
  biri yo'qoladi. `selftest` da buni ko'rsatuvchi test bor: 3 ta oqim `shared_counter++` ni 1000 martadan
  bajaradi. Atomarsiz natija 3000 dan kam chiqadi.
- **Xotira tartibi:** CPU va kompilyator o'qish/yozishlarni joyini almashtirishi mumkin. `ACQUIRE` —
  keyingi amallar bundan oldinga o'tmaydi, `RELEASE` — oldingi amallar bundan keyinga o'tmaydi.
  Spinlock aynan shu ikki kafolatga tayanadi.
- *Oddiy qoida:* bir nechta o'zgaruvchi birga o'zgarsa, **qulf** ishlating. Faqat bitta sanagich bo'lsa,
  atomar amal yetarli.

### 5.14. Inline assembly (`__asm__ volatile`)

```c
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port) : "memory");
}
```
- **Tuzilishi:** `"instruksiya" : chiqishlar : kirishlar : buziladiganlar (clobbers)`.
- **Cheklovlar:** `"a"` — RAX/AL, `"D"` — RDI, `"S"` — RSI, `"d"` — RDX, `"N"` — 8 bitli konstanta,
  `"=a"(r)` — natija RAX dan.
- **`"memory"` clobber:** "bu instruksiya xotirani o'qishi yoki yozishi mumkin, uning atrofidagi xotira
  amallarini joyidan siljitma". *Bo'lmasa:* `syscall` dan oldin buferga yozish kompilyator tomonidan
  syscall'dan **keyinga** surilishi mumkin va yadro eski ma'lumotni o'qiydi.
- **`volatile`:** "natijasi ishlatilmasa ham olib tashlama". `cli`, `hlt`, `outb` natija qaytarmaydi,
  `volatile` siz ular umuman yo'qolishi mumkin.
- **Nega C da yozib bo'lmaydi:** `cli`, `lgdt`, `mov cr3`, `in`/`out`, `rdmsr`, `syscall` kabi
  instruksiyalar uchun C'da ekvivalent yo'q.
- User tomonda `syscall` instruksiyasi RCX va R11 ni buzadi, shuning uchun ular clobber ro'yxatida
  (`user/libc/syscall.h`). *Bo'lmasa:* kompilyator RCX dagi qiymatni saqlangan deb hisoblaydi va dastur
  tasodifan yiqiladi.

### 5.15. O'zgaruvchan argumentlar (`va_list`)

`kprintf(const char *fmt, ...)` → `va_start`, `va_arg(ap, int)`, `va_end`. Argumentlar turini **format
satri** aytadi, kompilyator esa tekshirmaydi. Shuning uchun `format(printf)` atributi kerak (5.12).
`%lu` ga `int` berilsa, `va_arg` 8 bayt o'qiydi va natija axlat bo'ladi.

### 5.16. `goto` — xatolarni tozalash uchun

```c
    if (!fs->gd) { err = -ENOMEM; goto fail; }
    ...
fail:
    kfree(fs->gd);
    kfree(fs);
    return err;
```
- *Nega:* bir nechta resurs ajratilgan va ulardan biri muvaffaqiyatsiz tugagan bo'lsa, oldingilarini
  teskari tartibda bo'shatish kerak. `goto` bilan bu bitta joyda qilinadi. Linux yadrosida bu standart
  naqsh.
- *Bo'lmasa:* har bir `if` ichida tozalash kodi takrorlanadi va birida albatta nimadir unutiladi, natijada
  xotira oqib ketadi (memory leak).

### 5.17. Belgilangan initsializatorlar (designated initializers)

```c
static const struct file_ops tty_fops = { .read = tty_read, .write = tty_write, .seekable = false };
```
Ko'rsatilmagan maydonlar **nol** (NULL) bo'ladi. *Nega:* maydon tartibi o'zgarsa ham kod to'g'ri qoladi,
`NULL` esa "bu amal qo'llanmaydi" degani (VFS `if (!fops->ioctl) return -ENOTTY;` deb tekshiradi).

### 5.18. Moslashuvchan massiv maydoni

`struct ext2_dirent { uint32_t inode; uint16_t rec_len; uint8_t name_len; uint8_t file_type; char name[]; };`
— diskdagi papka yozuvining nomi o'zgaruvchan uzunlikda. `name[]` struct oxirida "davomi" bo'ladi.
Diskdagi baytlar ustiga ko'rsatkich qo'yib o'qiladi.

### 5.19. Xato qaytarish konvensiyasi: `-errno`

Yadroda funksiya muvaffaqiyatda `0` yoki musbat qiymat, xatoda `-ENOENT` kabi manfiy kod qaytaradi. libc
esa uni POSIX uslubiga aylantiradi: `errno = ENOENT; return -1;`.
*Nega:* bitta qaytish qiymatida ham natija, ham xato sababi bor. *Bo'lmasa:* `NULL` yoki `-1` qaytarib,
sababni yo'qotasiz: fayl yo'qmi, ruxsat yo'qmi yoki disk to'lganmi?

### 5.20. Aniqlanmagan xatti-harakat (Undefined Behavior) — yashirin minalar

| UB | misol | oqibati |
|---|---|---|
| NULL ni o'qish | `*p` da `p == NULL` | bizda #PF: NULL sahifa ataylab xaritalanmagan |
| massiv chegarasidan chiqish | `buf[256]` 256 elementli massivda | boshqa o'zgaruvchi jimgina buziladi |
| ishorali son to'lishi | `INT_MAX + 1` | kompilyator "bu hech qachon bo'lmaydi" deb kodni olib tashlashi mumkin |
| katta siljish | `1 << 40` (`int`) | noto'g'ri natija; `1ULL` ishlating |
| ishga tushirilmagan o'zgaruvchi | `int x; return x;` | tasodifiy qiymat |
| bo'shatilgan xotira | `kfree(p); p->x` | slab'dagi "zahar" (0x6B) buni ushlaydi (`demo=uaf`) |

---

## 6. Kompilyator, linker va Makefile

### 6.1. Yadro bayroqlari (`Makefile: KERNEL_CFLAGS`)

| bayroq | nima qiladi | nega kerak | bo'lmasa nima bo'ladi |
|---|---|---|---|
| `-ffreestanding` | "standart kutubxona yo'q muhit" | yadroda `printf`, `malloc`, libc umuman yo'q | kompilyator `memcpy` o'rniga libc'dagi versiyani chaqirishi mumkin, u esa yadroda yo'q |
| `-nostdlib` (linker) | libc va crt0 ulanmaydi | yadro o'zi — kirish nuqtasi `_start` bizniki | linker glibc'ni ulashga urinadi va xato beradi |
| `-mno-red-zone` | "qizil zona"ni o'chiradi | uzilish kelganda CPU **joriy stekka** yozadi | funksiya RSP dan 128 bayt pastdagi ma'lumoti uzilish freymi bilan buziladi (juda qiyin topiladigan xato!) |
| `-mgeneral-regs-only` | SSE/AVX registrlari ishlatilmaydi | biz uzilishda SSE registrlarini saqlamaymiz | kompilyator `memcpy` ni XMM bilan tezlashtiradi va user dasturining XMM registrlari jimgina buziladi |
| `-mcmodel=kernel` | kod yuqori 2 GB da (0xFFFFFFFF80000000) | 32-bitli manzil ofsetlari yetishi uchun | "relocation truncated" linker xatolari |
| `-fno-stack-protector` | stek kanareykasi o'chiq | u `__stack_chk_fail` va TLS talab qiladi — yadroda ular yo'q | linker xatosi |
| `-fno-pic -fno-pie` | pozitsiyadan mustaqil kod emas | yadro aniq bir manzilga yuklanadi | GOT/PLT jadvallari kerak bo'ladi, ular esa yo'q |
| `-fno-omit-frame-pointer` | RBP freym zanjirini saqlaydi | `panic` backtrace chiqaradi (RBP zanjiri bo'ylab) | panic'da qayerdan kelganini bilolmaysiz |
| `-fno-tree-loop-distribute-patterns` | tsiklni `memset` chaqiruviga aylantirmaydi | `memset` ning o'zi tsikl bilan yozilgan | `memset` o'zini chaqiradi → cheksiz rekursiya |
| `-O2` | optimallashtirish | tezlik; yashirin UB xatolarini ham yuzaga chiqaradi | kod sekin; `-O0` da ko'rinmagan xatolar keyinroq chiqadi |
| `-g` | debug ma'lumot | GDB va `addr2line` manzilni fayl:qatorga aylantiradi | backtrace'da faqat sonlar ko'rinadi |
| `-Wall -Wextra -Werror` | barcha ogohlantirishlar = xato | ogohlantirishlarning ko'pchiligi haqiqiy xato | xatolar "shunchaki ogohlantirish" bo'lib qoladi |
| `-MMD -MP` | `.d` bog'liqlik fayllari | `.h` o'zgarsa, uni ishlatgan `.c` qayta yig'iladi | eski obyekt fayllar bilan g'alati xatolar |

User dasturlari uchun (`USER_CFLAGS`) farqlar:
- **red zone ruxsat etilgan**, chunki uzilish kelganda CPU **yadro stekiga** o'tadi (TSS.rsp0) va user
  stekiga tegmaydi. Shu sababli signal freymi `rsp - 128` dan pastga yoziladi (`signal.c`).
- **`-nostdinc`**: kompyuterdagi glibc sarlavhalari **tasodifan** ulanib qolmasligi uchun. Aks holda
  `<stdio.h>` Linux'ning `FILE` tuzilmasini olib kelardi.

### 6.2. Linker skripti (`kernel/linker.ld`)

Linker `.o` fayllarni birlashtiradi. Skript esa **qaysi bo'lim qaysi manzilga** qo'yilishini aytadi:
- `.boot` — 1 MB da (fizik = virtual), chunki paging hali yoqilmagan.
- `.text`, `.rodata`, `.data`, `.bss` — `0xFFFFFFFF80000000 + ...` da (higher half). `AT(...)` bilan
  fizik yuklanish manzili (LMA) virtual manzildan (VMA) farq qiladi.
- `__text_start`, `__kernel_end` kabi belgilar — C kodi yadro chegaralarini shu belgilardan biladi
  (masalan, `.text` ni faqat o'qish/bajarish, `.data` ni esa NX qilish uchun).
- *Bo'lmasa:* linker kodni 0x400000 ga qo'yadi, GRUB uni noto'g'ri joyga yuklaydi va birinchi
  instruksiyadayoq triple fault bo'ladi.

### 6.3. Statik kutubxona (`libc.a`) va `ar`

`ar rcs libc.a *.o` — obyekt fayllar arxivi. Linker arxivdan **faqat kerakli** `.o` larni oladi: `true`
dasturiga `printf` kodi qo'shilmaydi. **Tartib muhim:** `ld crt0.o prog.o libc.a` — kutubxona oxirida
bo'lishi kerak, chunki linker undan faqat oldingi fayllarda yetishmayotgan belgilarni qidiradi.

### 6.4. Makefile g'oyasi

`nishon: bog'liqliklar` → buyruq. Make fayl vaqtlarini solishtiradi va faqat o'zgarganlarini qayta
yig'adi. Pattern qoidalar: `$(BUILD)/%.c.o: %.c`. Maxsus nishonlar:
- `.PHONY` — fayl emas, buyruq (`run`, `test`).
- `.PRECIOUS` — oraliq fayllar (`.elf`) o'chirilmasin.
- `FLAGS_STAMP` — bayroqlar o'zgarsa, hamma narsa qayta yig'ilsin. Bu bizda haqiqiy xatodan keyin
  qo'shilgan (9-bo'lim).

---

## 7. Assembly va x86-64 protsessor mexanizmlari

### 7.1. Yuklanish: CPU yoqilgandan `kmain` gacha

```
Quvvat ─► firmware (BIOS/UEFI) ─► GRUB ─► boot.asm: _start (32-bit, paging o'chiq)
   ─► vaqtinchalik sahifa jadvallari ─► PAE + Long Mode (EFER.LME) + paging ─► 64-bit
   ─► jmp 0xFFFFFFFF801xxxxx (higher half) ─► kmain(magic, mbi)
```

| texnologiya | nima | nega kerak | bo'lmasa |
|---|---|---|---|
| **Multiboot2** | yuklovchi va yadro o'rtasidagi standart | GRUB yadroni yuklaydi, xotira xaritasi, framebuffer va initrd'ni beradi | o'z yuklovchimizni yozishimiz kerak bo'lardi (disk drayveri, 16-bitli kod ...) |
| **Long Mode** | x86-64 ning 64-bitli rejimi | 64-bit registrlar, 48-bitli manzillar, NX | 4 GB dan ortiq xotira yo'q, NX himoyasi yo'q |
| **GRUB + ISO** | bitta fayl BIOS va UEFI'da yuklanadi | haqiqiy kompyuterda USB fleshkadan yuklash uchun | faqat QEMU `-kernel` bilan ishlaydi |

### 7.2. GDT, TSS — segmentatsiya qoldiqlari

- **GDT** — segment deskriptorlari jadvali. 64-bit rejimda segmentatsiya deyarli o'chirilgan, lekin GDT
  **baribir kerak**: CS dagi deskriptor CPU'ning imtiyoz darajasini (ring 0 yoki 3) belgilaydi.
- **TSS** — `rsp0`: user rejimida uzilish kelganda CPU **qaysi yadro stekiga** o'tishi. `IST` — double
  fault uchun alohida, zaxira stek.
  *TSS bo'lmasa:* user dasturda uzilish kelganda CPU yadro kodini **user stekida** bajaradi. Bu ham
  xavfsizlik teshigi, ham darhol qulash. *IST bo'lmasa:* yadro steki to'lganda double fault ham o'sha
  to'lgan stekka yozadi va triple fault (qayta yuklanish) bo'ladi.

### 7.3. IDT va uzilishlar

- **IDT** — 256 ta "darvoza": vektor → handler manzili. 0–31 CPU exception'lari (#PF=14, #GP=13,
  #DF=8), 32+ esa apparat IRQ'lari va dasturiy uzilishlar.
- **isr.asm**: har bir vektor uchun stub barcha registrlarni stekka saqlaydi (`struct interrupt_frame`),
  C'dagi `interrupt_dispatch` ni chaqiradi, keyin registrlarni tiklaydi va `iretq` qiladi.
- *Nega registrlarni saqlash kerak:* uzilish dasturni **istalgan instruksiyada** to'xtatadi. Birorta
  registr o'zgarib qolsa, dastur buzilgan qiymat bilan davom etadi.
- **EOI**: uzilish kontrolleriga "tayyor" signali. *Bo'lmasa:* keyingi uzilishlar kelmaydi va taymer
  "qotib" qoladi.

### 7.4. Paging — virtual xotira (eng muhim mexanizm)

```
virtual manzil (48 bit): [PML4 9][PDPT 9][PD 9][PT 9][ofset 12]
CR3 ─► PML4 ─► PDPT ─► PD ─► PT ─► fizik sahifa (4 KB) + ofset
```
| nima | nega | bo'lmasa |
|---|---|---|
| har bir jarayonning o'z jadvali (CR3) | jarayonlar bir-birining xotirasini ko'rmaydi | bitta dasturdagi xato hammani buzadi |
| U/S biti | user rejimi yadro sahifalariga tega olmaydi | istalgan dastur yadroni o'qiy oladi (`crash kernel` testi) |
| R/W biti | faqat o'qish sahifalari; COW shu bitga tayanadi | `fork` darhol butun xotirani nusxalashi kerak bo'lardi |
| NX biti (63) | ma'lumot sahifasini bajarib bo'lmaydi | stekka kod yozib, uni bajarish mumkin (buffer overflow hujumi) |
| higher half (0xFFFFFFFF80000000) | yadro har bir jarayonning yuqori qismida | har bir syscall'da CR3 almashishi kerak bo'lardi (sekin) |
| direct map (HHDM) | butun RAM `0xFFFF800000000000 + fizik` da | fizik sahifaga har murojaat qilishda vaqtinchalik xaritalash kerak bo'lardi |
| **TLB** | manzil tarjimalari keshi | har bir xotira murojaati 4 ta qo'shimcha o'qishga aylanardi |
| TLB shootdown | jadval o'zgarsa, BOSHQA CPU'larga ham xabar | boshqa CPU eski tarjima bilan bo'shatilgan sahifaga yozadi |

**Page fault — xato emas, vosita:** demand paging (sahifa faqat birinchi murojaatda ajratiladi),
copy-on-write, stekning o'sishi. Barchasi `mm_handle_fault` da.

### 7.5. Imtiyoz halqalari va syscall

- Ring 0 (yadro) istalgan instruksiyani bajara oladi. Ring 3 (dasturlar) `cli`, `hlt`, `in/out` ni
  bajarolmaydi (`crash cli` → #GP).
- **`syscall`/`sysret`**: tez tizim chaqiruvi. `syscall` RIP ni RCX ga, RFLAGS ni R11 ga saqlaydi va
  LSTAR MSR dagi manzilga sakraydi. Bunda **stek o'zgarmaydi** — yadro stekiga o'zimiz o'tamiz
  (`syscall_entry.asm`).
- **`swapgs`**: GS registrini user qiymatidan yadro per-CPU tuzilmasiga almashtiradi. Yadroga kirishda
  va chiqishda bir marta bajariladi. *Xato qilinsa:* yadro user boshqaradigan manzildagi "per-CPU"
  ma'lumotni o'qiydi. Bu jiddiy zaiflik.
- **CVE-2012-0217**: kanonik bo'lmagan RIP bilan `sysret` qilish Intel CPU'larda #GP ni ring 0 da, lekin
  user stekida chaqiradi. Shuning uchun RIP tekshiriladi.

### 7.6. APIC, taymer, SMP

- **LAPIC** — har bir CPU'ning o'z uzilish kontrolleri va taymeri. **IOAPIC** — qurilma IRQ'larini
  CPU'larga yo'naltiradi. Eski **8259 PIC** faqat bitta CPU'ni qo'llaydi, shuning uchun o'chiriladi.
- **Taymer uzilishi** (100 Hz) — preemptiv multitasking: cheksiz tsikldagi dastur ham CPU'ni
  majburan beradi.
- **SMP**: boshqa CPU'lar 16-bitli real rejimda uyg'onadi (INIT–SIPI), shuning uchun `trampoline.asm`
  ularni 16 → 32 → 64 bitga o'tkazadi.
- **Per-CPU (GS)**: har bir CPU'ning o'z `current`, stek va hisoblagichlari bor.

### 7.7. Qulflar

| qulf | qachon | nega | noto'g'ri ishlatilsa |
|---|---|---|---|
| **spinlock** | qisqa kritik bo'lim, uzilish handler'lari | uxlab bo'lmaydigan joyda ishlaydi; uzilishlarni ham o'chiradi | uzilishlarni o'chirmasa: handler o'sha qulfni kutadi → deadlock |
| **mutex** | uzoq amallar (disk, fayl tizimi) | kutayotgan jarayon uxlaydi, CPU bekor turmaydi | spinlock ushlagan holda mutex olinsa → uxlab bo'lmaydi → panic |
| **atomar** | bitta sanagich | eng arzon | bir nechta maydon uchun yetarli emas |

**Qulf tartibi (lock ordering):** A va B qulflari doim bir xil tartibda olinishi kerak. `vfs_rename`
ikki papka qulfini **manzil tartibida** oladi. *Bo'lmasa:* CPU1 A ni olib B ni kutadi, CPU2 B ni olib A ni
kutadi va ikkalasi abadiy qotadi.

**Lost wakeup:** "shart bajarilmagan → uxlayman" oralig'ida uyg'otish kelsa, u yo'qoladi va jarayon
abadiy uxlaydi. `proc_sleep(chan, lock)` qulfni **atomar** bo'shatadi (xv6 usuli).

### 7.8. Qurilmalar bilan ishlash

| usul | misol | xususiyati |
|---|---|---|
| I/O portlar (`in`/`out`) | serial 0x3F8, ATA 0x1F0, PIT | eski, sekin, oddiy |
| MMIO (`volatile` ko'rsatkich) | AHCI, APIC, framebuffer | tez; `ioremap` + keshlanmaydigan xotira (UC/WC) |
| DMA | AHCI | qurilma RAM'ga **fizik** manzil bo'yicha o'zi yozadi; PCI "bus master" biti yoqilishi kerak |
| PCI konfiguratsiya | 0xCF8/0xCFC | qurilmani topish, BAR (resurs manzillari) |

**Keshlash turi (PAT):** qurilma registri **keshlanmasligi** kerak (UC), aks holda CPU eski qiymatni
o'qiydi. Framebuffer uchun WC (write-combining) — yozish tez, o'qish juda sekin.

---

## 8. Operatsion tizim g'oyalari

| g'oya | bizda | nega shunday | muqobil yondashuv |
|---|---|---|---|
| **Buddy allocator** | `mm/pmm.c` | 2^n sahifali bloklar; tez birlashtirish (fragmentatsiyaga qarshi) | bitmap (v0.1): oddiy, lekin sekin |
| **Slab** | `mm/slab.c` | bir xil o'lchamli obyektlar uchun tez; qizil zona va zahar xatolarni ushlaydi | umumiy `malloc`: sekinroq, xatolarni ushlamaydi |
| **vmalloc** | `mm/vmalloc.c` | virtual jihatdan uzluksiz; stek atrofida himoya sahifasi | direct map: himoya sahifasini qo'yib bo'lmaydi |
| **Round-robin scheduler** | `proc/process.c` | oddiy va adolatli | Linux CFS/EEVDF: ustuvorlik, adolatli ulush |
| **fork + COW** | `mm/mm.c` | fork arzon; shell darhol exec qiladi | `spawn` (Windows uslubi) |
| **VFS** | `fs/vfs.c` | bitta interfeys — ko'p fayl tizimi | har bir FS uchun alohida syscall'lar |
| **inode / file / fd** | `fs/vfs.h` | nom ≠ fayl ≠ ochiq fayl; hard link, unlink qilingan ochiq fayl | nomga bog'langan fayllar (FAT) |
| **Buffer cache (write-through)** | `fs/block.c` | disk o'qishlarini tejash; tok o'chsa ham xavfsiz | write-back + jurnal (ext4): tez, lekin murakkab |
| **Signallar** | `proc/signal.c` | asinxron xabar; user rejimiga qaytishda yetkaziladi | xabar navbatlari |
| **TTY line discipline** | `drivers/tty.c` | tahrir yadroda — har bir dastur buni yozishi shart emas | har bir dastur o'z tahririni qiladi |
| **"Hamma narsa fayl"** | `/dev/*`, pipe | `read`/`write` hamma joyda bir xil | har bir qurilma uchun alohida API |

---

## 9. Haqiqiy xatolar va ularni topish usullari

Bular shu loyihani yozish davomida **haqiqatan** yuzaga kelgan xatolar. Har birini o'qing va o'zingizdan
so'rang: "men buni qanday topgan bo'lardim?"

| # | alomat | sabab | qanday topildi | saboq |
|---|---|---|---|---|
| 1 | QEMU ba'zan **o'z-o'zidan qayta yuklanadi** (6 urinishdan ~2 tasida) | CPU0 scheduler'da o'lgan jarayonning CR3'i bilan qolgan, u sahifa jadvali esa boshqa CPU'da bo'shatilgan | `-d cpu_reset,int` logi: yadro kodida #PF (`e=0010`), CR3 esa bo'shatilgan sahifaga ko'rsatadi | SMP'da "bu xotiradan HALI kim foydalanyapti?" degan savolni doim bering. `docs/12-vfs.md`, 11-bo'lim |
| 2 | Kernel panic, noto'g'ri manzil | `kprintf` `%#x` ni tushunmaydi, shuning uchun argumentlar siljidi | `addr2line` → `format_core` | format satrini tekshiring; `format(printf)` atributi |
| 3 | GS baza manzili nolga tushib qoldi | `gdt_load` segment registrlarini qayta yuklagan, bu esa GS bazasini tozalaydi | per-CPU `current` noto'g'ri qiymat qaytardi | FS/GS ni qayta yuklamang (64-bit bazasi MSR da) |
| 4 | Deadlock panic | yangi user jarayon `proc_lock` ni bo'shatmasdan ishga tushgan | spinlock egasini kuzatish (owner tracking) | birinchi ishga tushish ham `proc_first_run` orqali o'tishi kerak |
| 5 | Stek to'lishi sahifa jadvallarini buzdi | yadro stekida himoya sahifasi yo'q edi | tasodifiy buzilishlar | vmalloc stek + himoya sahifasi → darhol #PF |
| 6 | `tail` oxirgi qatorni yo'qotdi | `fgets` EOF da buferga `'\0'` yozgan | `seq 10 \| tail -3` 2 ta qator chiqardi | C standartini aynan kuzating |
| 7 | `fstest` jimgina to'xtadi | SIGPIPE dasturni o'ldirgan (shell SIGPIPE'ni ko'rsatmaydi) | test ketma-ketligini qisqartirish (bisect) | yangi xususiyat (signal) eski testlarni ham o'zgartiradi |
| 8 | `false \|\| echo $?` 0 chiqardi | shell `$?` ni butun qator uchun oldindan ochib qo'ygan | bash bilan solishtirish | ochish (expansion) har bir buyruq oldidan bajarilishi kerak |
| 9 | Ctrl-C test o'rniga boshqa dasturni o'ldirdi | test Ctrl-C ni dastur ishga tushmasdan yuborgan | logdagi vaqt tartibi | asinxron testlarni sinxronlang (`@kut:`) |
| 10 | 1 s dan keyin `sleep 30` tugadi | stop signali uxlashni uzdi, syscall 0 qaytardi | Ctrl-Z + bg | qayta boshlashda **qolgan** vaqt kerak |
| 11 | Yangi `make` hech narsani yig'madi | `.SECONDARY` o'chirilgan oraliq fayllarni qayta yig'masdan qoldirgan | `make -d` | `.PRECIOUS` va Make semantikasini tushunish |
| 12 | Kompilyator bayrog'ini o'zgartirgandan keyin g'alati qulashlar | eski `.o` fayllar eski bayroqlar (`-mcmodel`) bilan yig'ilgan edi | `make clean` dan keyin xato yo'qoldi | bayroqlar ham bog'liqlik: `FLAGS_STAMP` |
| 13 | Disk bo'lgani uchun QEMU yuklanmadi | BIOS MBR imzosi bor, lekin yuklovchisi yo'q diskdan yuklanishga urindi | bo'sh ekran | `-boot d` |

**Umumiy qoida:** xato har doim sizning kodingizda. "QEMU xatosi" yoki "tasodif" deb o'ylamang: har
bir tasodifiy xatoning aniq sababi bor (1-xato kabi).

---

## 10. Debug qilish vositalari

| vosita | qachon | qanday |
|---|---|---|
| **serial log** | har doim | `make run-nographic`; yadro barcha xabarlarni COM1 ga chiqaradi |
| **panic + backtrace** | yadro qulasa | `addr2line -f -e build/kernel.elf 0xffffffff8011...` |
| **QEMU `-d int,cpu_reset -D log`** | triple fault, o'z-o'zidan qayta yuklanish | logda oxirgi exception'lar, CR2, CR3 va RIP |
| **QEMU monitor** | tizim qotib qolsa | `info registers -a`: har bir CPU qayerda turibdi |
| **GDB** | qadamma-qadam kuzatish | `make debug` + `gdb -x tools/gdbinit`; `break kmain`, `stepi`, `info registers` |
| **selftest** | yadro qismlarini tekshirish | `make run APPEND=selftest` (100 ta tekshiruv) |
| **crashdemo** | himoya mexanizmlarini ko'rish | `APPEND=demo=uaf`, `demo=stack` ... |
| **e2fsck / debugfs** | ext2 yozish kodini tekshirish | `make test` o'zi bajaradi |
| **bisect** | "qachondan buzildi?" | `git bisect`, yoki buyruqlar ketma-ketligini yarmiga qisqartirish |

**Debug usuli:** (1) xatoni **takrorlanadigan** qiling; (2) holatni kichraytiring (minimal misol);
(3) gipoteza qo'ying; (4) uni tasdiqlaydigan yoki rad etadigan **bitta** tajriba o'tkazing; (5) sababni
topmaguningizcha "tuzatmang". Belgini (alomatni) yashirish xatoni tuzatish degani emas.

---

## 11. O'z yadroingizni noldan yozish rejasi

Bu reja — kodni yopib qo'yib, yangi papkada **o'zingiz** yozish uchun. Har bir bosqichning aniq
**tekshiruv nuqtasi** bor: u ishlamaguncha keyingisiga o'tmang. Qiyin joyga kelsangiz, avval tegishli
hujjatni o'qing, kodga esa eng oxirida qarang.

| # | bosqich | tekshiruv nuqtasi | yordamchi hujjat |
|---|---|---|---|
| 1 | Multiboot sarlavhasi + `_start`: VGA ga `OK` yozish (32-bit) | ekranda `OK` | 01 |
| 2 | Stek, C ga o'tish, `kmain` | C'dan harf chiqadi | 01 |
| 3 | Serial port + `kprintf` (`%d %x %s %p`) | terminalda formatlangan matn | 02 |
| 4 | 64-bit: sahifa jadvallari, Long Mode, GDT | 64-bitli `kmain` | 01, 09 |
| 5 | IDT + exception handler'lar | `int3` → xabar; 0 ga bo'lish → xato ekrani | 02 |
| 6 | PIC/PIT + taymer uzilishi | har soniyada "tik" | 02 |
| 7 | Klaviatura | bosilgan harf ekranda | 02 |
| 8 | Fizik xotira: xotira xaritasi + bitmap (keyin buddy) | 1000 ta ajratish/bo'shatish testi | 03 |
| 9 | Virtual xotira: `map_page`, `unmap`, NULL sahifa | NULL ga yozish → #PF | 04 |
| 10 | `kmalloc` (slab) | stress test, double-free ushlanadi | 05 |
| 11 | Yadro oqimlari + context switch + scheduler | ikki oqim navbatma-navbat yozadi | 06 |
| 12 | User rejim: TSS, ring 3, `int 0x80` / `syscall` | ring 3 dan `write` | 07 |
| 13 | ELF yuklovchi + initrd (tar) | `hello` dasturi ishlaydi | 07 |
| 14 | fork/exec/wait + shell | `ls`, `cat` shell'dan | 07, 11 |
| 15 | VFS + tmpfs + pipe | `ls \| wc` | 12 |
| 16 | Disk drayveri + ext2 o'qish | diskdagi faylni `cat` qilish | 13 |
| 17 | ext2 yozish | `e2fsck` toza | 13 |
| 18 | Signallar | Ctrl-C | 14 |
| 19 | SMP | `ps` da `idle/1` | 10 |

**Qoida:** 1–14 bosqichlarni yolg'iz, faqat Intel SDM va OSDev Wiki yordamida qila olsangiz, siz yadro
dasturchisisiz. 15–19 bosqichlar esa sizni kuchli yadro dasturchisiga aylantiradi.

**Tavsiya etilgan sur'at** (kuniga 2–3 soat): 1–7 bosqichlar — 2–3 hafta, 8–14 — 1–2 oy, 15–19 — yana
1–2 oy. Shoshilmang: 4-, 9- va 11-bosqichlarni **ikki marta** yozing.

**Laboratoriyalar — noldan yozishdan OLDIN:** [labs/README.md](labs/README.md) da 22 ta laboratoriya
bor: `strlen` dan `sigreturn` gacha. `tools/lab.py boshla <nom>` asl kodni yashiradi, siz funksiyani
qayta yozasiz, `tools/lab.py tekshir <nom>` esa natijani tekshiradi (libc uchun 1 soniyada, yadro uchun
QEMU'da ~1 daqiqada). Tavsiya: 11-bo'limdagi rejaga o'tishdan oldin 22 ta lab'ning hammasini bajaring.

---

## 12. Lug'at

| atama | ma'nosi |
|---|---|
| **ABI** | ikkilik interfeys: syscall raqamlari, registrlar, struct joylashuvi |
| **BAR** | PCI qurilmaning resurs manzili (port yoki MMIO) |
| **COW** | copy-on-write: yozilganda nusxalash |
| **CR3** | joriy sahifa jadvalining fizik manzili |
| **DMA** | qurilmaning RAM'ga to'g'ridan-to'g'ri murojaati |
| **EOI** | "uzilish tugadi" signali |
| **HHDM** | higher-half direct map: butun RAM yadroning virtual manzillarida |
| **inode** | faylning o'zi (nomsiz): tur, hajm, bloklar |
| **IRQ** | apparat uzilish so'rovi |
| **MMIO** | xotira orqali ko'rinadigan qurilma registrlari |
| **MSR** | modelga xos registr (`rdmsr`/`wrmsr`): EFER, LSTAR, GS bazasi |
| **PID / PGID / SID** | jarayon / jarayon guruhi (job) / sessiya raqami |
| **preemption** | jarayonni majburan to'xtatib, boshqasiga o'tish |
| **ring 0 / 3** | yadro / foydalanuvchi imtiyoz darajasi |
| **TLB** | manzil tarjimalari keshi |
| **triple fault** | double fault ham ishlay olmadi → CPU qayta yuklanadi |
| **UB** | undefined behavior — C standarti kafolat bermaydigan holat |
| **VMA** | virtual xotira hududi (kod, heap, stek, mmap) |
| **W^X** | sahifa yo yoziladigan, yo bajariladigan bo'ladi — ikkalasi bir vaqtda emas |

---

*Qayerdan boshlash kerak?* — **Hozir:** `git checkout e5906bb`, `make run`, keyin `docs/00-kirish.md`.
Omad! Har bir "nima uchun?" savoli sizni kuchli muhandisga bir qadam yaqinlashtiradi.
