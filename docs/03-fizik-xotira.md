# 03 — Fizik xotira menejeri (PMM)

> **Eslatma (v0.2):** bu bo'lim loyihaning birinchi, **kichik** versiyasini tasvirlaydi
> (`git checkout e5906bb`). O'sha versiyada yadro 1 MB da, identity paging bilan ishlagan.
> Hozirgi yadro **higher-half**, **buddy + slab** allocatorlar va GRUB/Multiboot2 bilan ishlaydi:
> [09-yangi-arxitektura.md](09-yangi-arxitektura.md) ga qarang. G'oyalarni tushunish uchun avval
> shu bo'limni, keyin 09-bo'limni o'qing: oddiydan murakkabga.


**Kod:** `kernel/mm/pmm.c`, `kernel/mm/pmm.h`, `kernel/tests/selftest.c`

## Vazifa

RAM'ni 4 KB lik **freym**larga bo'lamiz. PMM har bir freym haqida bitta savolga javob beradi:
**bo'shmi yoki bandmi?** Va ikkita amal bajaradi:

```c
uint64_t phys = pmm_alloc_frame();   // bo'sh freym ber (fizik manzil)
pmm_free_frame(phys);                // qaytarib ol
```

Yadroning qolgan barcha xotira tizimlari (sahifa jadvallari, heap, jarayon steklari, user dasturlari
xotirasi) oxir-oqibat shu ikki funksiyaga tayanadi.

## Qaysi xotira bo'sh ekanini qayerdan bilamiz?

O'zimiz bila olmaymiz — RAM'ning ba'zi joylarida BIOS ma'lumotlari, ACPI jadvallari, qurilmalar
xotirasi bor. Buni **yuklovchi** (BIOS'ning E820 funksiyasi orqali) biladi va Multiboot orqali
**xotira xaritasini** beradi. QEMU'da (`-m 128M`) u shunday ko'rinadi:

```
0000000000000000 - 000000000009fbff  bo'sh RAM        <- 640 KB "past xotira"
000000000009fc00 - 000000000009ffff  band             <- EBDA (BIOS ma'lumoti)
00000000000f0000 - 00000000000fffff  band             <- BIOS ROM
0000000000100000 - 0000000007fdffff  bo'sh RAM        <- asosiy RAM (bizning yadro shu yerda)
0000000007fe0000 - 0000000007ffffff  band             <- ACPI
00000000fffc0000 - 00000000ffffffff  band             <- BIOS flash
```

E'tibor bering: 0xA0000–0xEFFFF oralig'i xaritada **umuman yo'q** (VGA xotirasi, ROM'lar). Shu sababli
algoritm **avval hammasini band** deb hisoblaydi, keyin faqat aniq bo'sh deb aytilganlarini bo'shatadi.

## Bitmap

```
freym:   0 1 2 3 4 5 6 7 ...
bit:     1 1 1 0 0 1 0 0 ...     1 = band, 0 = bo'sh
```

- 1 GB uchun 262 144 bit = **32 KB**. Bitmap `.bss` da statik turadi.
- Qidiruvni tezlashtirish: bitmap `uint64_t` so'zlardan iborat. So'z `0xFFFF...FFFF` bo'lsa,
  64 ta freymning hammasi band — bitta taqqoslash bilan o'tkazib yuboramiz.
- `__builtin_ctzll(~word)` — bo'sh bitni **bitta CPU instruksiyasi** (`tzcnt`) bilan topadi.
- **Next-fit**: keyingi qidiruv oxirgi topilgan joydan boshlanadi (boshidagi to'la so'zlarni qayta
  ko'rmaslik uchun).

## Band qilinishi SHART bo'lgan hududlar

| Hudud | Nega |
|---|---|
| 0 – 1 MB | BIOS, VGA (`0xB8000`), multiboot ma'lumotlari. 0-freym ham shu yerda — shuning uchun `0` ni "xato" belgisi sifatida qaytara olamiz |
| `kernel_start` – `kernel_end` | yadro kodi, ma'lumotlari, `.bss` (bitmap'ning o'zi ham, steklar ham) |
| multiboot modullari | `initrd.tar` — bizning "disk"imiz |
| cmdline, mmap | keyinroq o'qiymiz |

Buni unutsak nima bo'ladi? Keyin kimdir yadro kodi turgan freymni "bo'sh" deb olib, ustiga yozadi.
Oqibati **tasodifiy**: ba'zan darhol qulaydi, ba'zan bir soatdan keyin mutlaqo boshqa joyda. Xotira
xatolarini topish eng qiyin bo'lgani ham shundan.

## Himoya tekshiruvlari

`pmm_free_frame()` quyidagilarni tekshiradi va xato bo'lsa `panic` qiladi:

- manzil 4 KB ga tekislanganmi
- boshqariladigan hududdami
- **double free**: freym allaqachon bo'shmi?

Double free — real tizimlardagi eng xavfli xatolardan biri (ko'plab xavfsizlik zaifliklari shundan
boshlanadi). Uni darhol ushlash keyinchalik soatlab debug qilishdan qutqaradi.

## Uzilishlardan himoya (`irq_save/irq_restore`)

Bitmap **umumiy resurs**. Agar `pmm_alloc_frame()` o'rtasida taymer uzilishi kelib, boshqa jarayon ham
`pmm_alloc_frame()` chaqirsa, ikkalasi bitta freymni olishi mumkin. Bitta protsessorli tizimda
uzilishlarni vaqtincha o'chirish yetarli. Ko'p protsessorli tizimda esa **spinlock** kerak bo'ladi
(mashqlarga qarang).

## Testlar

```bash
make run APPEND=selftest
```

`tests/selftest.c` ajratish/qaytarish, noyoblik, ketma-ket ajratish va hisoblagichlarning to'g'riligini
tekshiradi.

## Sinab ko'ring

1. `mark_region_used(kernel_start...)` qatorini o'chiring va selftest'ni ishga tushiring. Nima bo'ladi?
2. `pmm_free_frame(a)` ni ikki marta chaqiring — panic xabarini ko'ring.
3. **Mashq:** `pmm_alloc_frames()` hozir O(n). Buddy allocator yozib, uni O(log n) ga tushiring.
