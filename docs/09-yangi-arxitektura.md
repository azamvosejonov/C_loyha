# 09 — Yangi arxitektura: GRUB, higher-half yadro, buddy, slab, vmalloc

Bu bo'limdan boshlab loyiha **haqiqiy kompyuterda** ishlaydigan yadroga aylanadi. Kichik versiyadan farqlar:

| | v0.1 (kichik) | v0.2 (hozirgi) |
|---|---|---|
| Yuklovchi | QEMU `-kernel`, Multiboot1 | **GRUB**, Multiboot2, bitta ISO **BIOS va UEFI** uchun |
| Ekran | VGA matn rejimi (80x25) | **framebuffer** (piksellar, shrift), VGA matn zaxirada |
| Yadro manzili | 1 MB, identity | **0xFFFFFFFF80000000** (higher half) |
| Fizik xotiraga kirish | birinchi 1 GB | **butun RAM** (direct map, HHDM) |
| Fizik allocator | bitmap | **buddy** + `struct page` + zonalar |
| Yadro heap'i | oddiy slab | **SLUB uslubidagi `kmem_cache`**, redzone |
| Yadro steklari | ketma-ket freymlar | **vmalloc** + himoya sahifalari |
| Qurilma xotirasi | — | **ioremap** (UC/WC, PAT) |
| Himoya | — | **W^X**, NX, SMEP, global sahifalar |
| Qulflar | faqat `cli` | **spinlock** (SMP'ga tayyor) |

Kod: `kernel/boot/{boot.asm,bootinfo.c}`, `kernel/linker.ld`, `kernel/mm/*`, `kernel/drivers/fbcon.c`, `kernel/arch/cpu.c`

---

## 1. Yuklash: GRUB → Multiboot2 → yuqori yarim

```
BIOS yoki UEFI firmware
   └─► GRUB (ISO ichida, grub-mkrescue qo'ygan)
         ├─ grub.cfg:  multiboot2 /boot/kernel.elf ...
         │             module2    /boot/initrd.tar initrd
         ├─ yadro ELF'ini FIZIK manzillar (LMA) bo'yicha yuklaydi
         ├─ framebuffer rejimini o'rnatadi (UEFI: GOP, BIOS: VBE)
         └─► _start (32-bit, paging o'chiq), EAX=0x36D76289, EBX=boot ma'lumoti
               └─► boot.asm: vaqtinchalik jadvallar → Long Mode → jmp 0xFFFFFFFF801xxxxx
                     └─► kmain()
```

### Bitta ISO, ikki xil firmware

`grub-mkrescue` ISO ichiga ikkita yuklovchi qo'yadi: BIOS uchun **El Torito** va UEFI uchun **EFI tizim
bo'limi** (FAT tasviri). Kompyuter qaysi firmware bilan yuklansa ham, tegishlisini topadi.

```bash
make run        # BIOS (SeaBIOS)
make run-uefi   # UEFI (OVMF)
```

**Muhim tafsilot:** UEFI kompyuterlarda **VGA matn rejimi umuman yo'q**. `0xB8000` ga yozish hech narsa
ko'rsatmaydi. Shuning uchun framebuffer konsoli majburiy. `grub.cfg` dagi `insmod all_video` qatorisiz esa
GRUB UEFI'da framebuffer bermaydi. Bu xatoni aynan shu loyihani yozish paytida topdik.

### Multiboot2 teglari (`bootinfo.c`)

Ma'lumot **TLV** (type-length-value) ko'rinishida keladi: `[tur][hajm][ma'lumot]`, har bir teg 8 ga tekislangan.
Biz ulardan kerakli hamma narsani darhol **o'z strukturamizga** (`struct boot_info`) nusxalaymiz:
xotira xaritasi, modullar, framebuffer, buyruq qatori va **ACPI RSDP** (keyingi bosqichlarda kerak).
Yadroning qolgan qismi Multiboot haqida hech narsa bilmaydi. Ertaga boshqa yuklovchiga o'tsak, faqat
`bootinfo.c` o'zgaradi.

## 2. Higher-half yadro

### Nega yadro yuqori manzilda?

64-bitli manzil maydoni ikki yarimga bo'linadi (`mm/layout.h`):

```
0x0000_0000_0000_0000 ─┬─ USER (har bir jarayonga xos)     PML4[0..255]
0x0000_7FFF_FFFF_FFFF ─┘
        ... kanonik bo'lmagan "teshik" ...
0xFFFF_8000_0000_0000 ─── direct map: phys + HHDM_BASE       PML4[256..]
0xFFFF_C000_0000_0000 ─── vmalloc / ioremap                  PML4[384..447]
0xFFFF_FFFF_8000_0000 ─── yadro kodi va ma'lumotlari         PML4[511]
```

- **Butun pastki yarim (128 TB) user dasturlariga** qoladi. Yadro ularning yo'lida turmaydi.
- **Yuqori yarim barcha jarayonlarda umumiy.** Yangi jarayon PML4'ining 256..511 yozuvlari yadro PML4'idan
  nusxalanadi. Yadroning 256 ta PDPT jadvalini boshida yaratib qo'yamiz, shuning uchun yadro xaritasi keyin
  o'zgarsa ham, o'zgarish barcha jarayonlarda avtomatik ko'rinadi.
- `-mcmodel=kernel`: kompilyator yadro manzillarini ishorali 32-bitli son sifatida kodlaydi. Bu faqat
  eng yuqori 2 GB uchun ishlaydi.

### VMA va LMA (`linker.ld`)

```
.text : AT(ADDR(.text) - KERNEL_VMA)   ← kod 0xFFFFFFFF801xxxxx da ISHLAYDI (VMA),
                                          lekin 0x1xxxxx ga YUKLANADI (LMA)
```

GRUB paging'siz ishlaydi, shuning uchun `.boot.*` bo'limlari **past** manzilga link qilinadi. Ular
vaqtinchalik jadvallarni quradi: identity, direct map va yadro xaritasi, hammasi 2 MB lik sahifalar bilan.
Shundan keyin yuqori yarimga `jmp rax` qilinadi.

## 3. Xotira: tovuq va tuxum muammosi

```
memblock  ──►  vmm_init  ──►  pmm (buddy)  ──►  slab  ──►  vmalloc  ──►  fbcon
(ilk bump      (doimiy        (mem_map,        (kmalloc)   (steklar,     (ioremap_wc)
 allocator)     jadvallar)     zonalar)                     ioremap)
```

Har bir qatlamga pastdagisi kerak. Buddy allocator uchun `struct page` massivi kerak (256 MB RAM uchun 4 MB),
lekin uni ajratadigan allocator hali yo'q. Yechim: **memblock** (`mm/memblock.c`). U RAM hududlaridan faqat
"tishlab" oladi va hech narsani qaytarmaydi. Buddy tayyor bo'lgach, memblock'da qolgan bo'sh joylar buddy'ga
beriladi (Linux'da ham aynan shunday).

### Direct map qanday quriladi (`vmm_init`)

- **Faqat RAM va ACPI hududlari** xaritalanadi. Qurilma xotirasi (MMIO) xaritalanmaydi: uni oddiy WB kesh
  bilan xaritalash haqiqiy apparatda xavfli, chunki CPU spekulyativ o'qish qilib qurilmani buzishi mumkin.
- Eng katta mos sahifa tanlanadi: **1 GB** (CPU qo'llasa), keyin **2 MB**, bo'lmasa **4 KB**. TLB'da joy
  kamroq ketadi, jadvallar uchun xotira ham kamroq kerak bo'ladi.

### W^X — yadro tasvirining himoyasi

| Bo'lim | O'qish | Yozish | Bajarish |
|---|---|---|---|
| `.text` | ✓ | ✗ | ✓ |
| `.rodata` | ✓ | ✗ | ✗ (NX) |
| `.data`, `.bss` | ✓ | ✓ | ✗ (NX) |

Yadrodagi xato natijasida kod o'z-o'zini ustidan yoza olmaydi, ma'lumot esa kod sifatida bajarila olmaydi.
Hujum qiluvchilar uchun ham eng muhim to'siqlardan biri shu.

## 4. Buddy allocator (`mm/pmm.c`)

```
tartib:  0    1    2    3   ...  10
blok:   4KB  8KB 16KB 32KB     4MB      har bir tartib uchun bo'sh bloklar ro'yxati
```

- **Ajratish:** kerakli tartib bo'sh bo'lsa, kattaroq blokni ikkiga bo'lamiz, ortiqcha yarmi pastki ro'yxatga
  tushadi.
- **Qaytarish:** egizak (**buddy**) blokning manzili `pfn ^ (1 << order)`. U ham bo'sh bo'lsa, ikkalasini
  birlashtirib, yuqori tartibda takrorlaymiz. Shu sababli xotira mayda bo'laklarga parchalanib qolmaydi.
- **`struct page`** (aniq 64 bayt, `_Static_assert` bilan tekshiriladi) har bir sahifa haqida ma'lumot saqlaydi:
  bo'shmi, tartibi, `refcount` (7-bosqichdagi copy-on-write uchun), slab ma'lumoti.
- **Zonalar:** `DMA32` (< 4 GB) va `Normal`. Ba'zi disk kontrollerlari faqat 32-bitli manzilga DMA qila oladi,
  shuning uchun past xotira ular uchun asrab qo'yiladi.

`selftest`: 300 ta tasodifiy blok ajratiladi va aralash tartibda qaytariladi. Shundan keyin bo'sh sahifalar soni
**aynan** boshlang'ich qiymatga qaytishi kerak. Birlashtirish (coalescing) xato bo'lsa, test darhol yiqiladi.

## 5. Slab: `kmem_cache` va `kmalloc` (`mm/slab.c`)

- Slab ma'lumotlari (kesh, bo'sh obyektlar zanjiri, `inuse`) sahifa **ichida** emas, uning `struct page`'ida
  saqlanadi. Shuning uchun butun sahifa obyektlarga ishlatiladi va `kfree(p)` hajmni `virt_to_page(p)->cache`
  orqali **O(1)** da topadi.
- Keshlarni **o'zingiz yaratishingiz** mumkin: `kmem_cache_create("process", sizeof(struct process), 16, ctor)`.
  Kesh keraksiz bo'lsa, `kmem_cache_destroy()` bilan o'chiriladi. Unda faol obyekt qolgan bo'lsa, bu xotira
  oqishi hisoblanadi va panic beradi.
- **Debug (har doim yoqilgan):**

| Xato | Mexanizm |
|---|---|
| buffer overflow | har bir obyekt oxirida 8 bayt **redzone** (`0xBB`) |
| use-after-free | bo'sh obyekt `0x6B` bilan **zaharlanadi** |
| double free | `FREE_MAGIC` |
| noto'g'ri `kfree` | manzil direct map'da emas, sahifa slab emas yoki obyekt boshi emas |

## 6. vmalloc va ioremap (`mm/vmalloc.c`)

- `vmalloc(n)`: **virtual** jihatdan uzluksiz, fizik jihatdan tarqoq xotira. Har bir hudud atrofida
  xaritalanmagan **himoya sahifasi** bor.
- **Yadro steklari endi vmalloc'dan olinadi.** Yadroda cheksiz rekursiya bo'lsa, stek himoya sahifasiga
  uriladi: #PF → #DF → IST steki → aniq xabar. Linux'da bu `CONFIG_VMAP_STACK` deb ataladi.
- `ioremap(phys, n)` qurilma registrlarini **UC** (keshlanmaydigan) rejimda xaritalaydi, `ioremap_wc()` esa
  framebuffer'ni **WC** (write-combining) rejimida xaritalaydi. WC uchun PAT registrining 1-yozuvi qayta
  dasturlanadi (`cpu.c`).

## 7. Framebuffer konsoli (`drivers/fbcon.c`)

- Piksel manzili: `baza + y * pitch + x * (bpp/8)`. **Diqqat:** `pitch` `width * 4` dan katta bo'lishi mumkin.
- Rang komponentlarining o'rni yuklovchidan olinadi, chunki ba'zi kartalarda BGR tartib ishlatiladi.
- Shrift: **Spleen 8x16** (BSD litsenziyasi), `tools/psf2c.py` bilan C massivga aylantirilgan.
- **Ekrandan hech qachon o'qilmaydi.** WC xotirani o'qish juda sekin. Matnning "soya" nusxasi RAM'da saqlanadi,
  scroll paytida ekran shu nusxadan qayta chiziladi.
- **Log buferi (`lib/klog.c`):** ekran tayyor bo'lguncha barcha xabarlar 64 KB lik halqali buferga yoziladi,
  keyin ekranga qayta chiqariladi (Linux'dagi `dmesg` buferi kabi).

## 8. Spinlock (`lib/spinlock.h`)

Keyingi bosqichlarda bir nechta CPU yadrosi ishga tushiriladi, shuning uchun allocatorlar allaqachon
spinlock bilan himoyalangan:
- `xchg` bitta atomar amal bilan "eski qiymatni o'qib, yangisini yozadi";
- **test-and-test-and-set**: qulf bo'shaguncha faqat o'qiladi, shunda kesh qatori CPU'lar orasida "ping-pong" bo'lmaydi;
- `spin_lock_irqsave` uzilishlarni ham o'chiradi: aks holda o'sha CPU'dagi uzilish handleri shu qulfni olmoqchi
  bo'lib, **deadlock** yuz beradi.

## Sinab ko'ring

```bash
make run                           # BIOS
make run-uefi                      # UEFI
make run APPEND=selftest           # 100+ ichki test
make run APPEND=demo=stack         # yadro steki himoya sahifasiga uriladi
tools/screenshot.sh ekran.png 6    # oynasiz muhitda ekranni rasmga olish
```

1. `vmm_init` da `.text` ga `PTE_WRITABLE` qo'shing. Keyin yadro kodiga yozadigan kichik test yozing: nima bo'ladi?
   Oldin nima bo'lardi?
2. `grub.cfg` dan `insmod all_video` ni olib tashlab, `make run-uefi` qiling.
3. `pmm.c` dagi `buddy_free` da birlashtirishni o'chiring (`break`), keyin `pmm_dump()` natijasini solishtiring.
4. **Mashq:** `kmem_cache` uchun per-CPU keshlar qo'shing (Linux SLUB'dagi `cpu_slab`). SMP bosqichidan keyin bu
   tezlikka katta ta'sir qiladi.
