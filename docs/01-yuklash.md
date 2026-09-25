# 01 — Yuklash: tokdan `kmain()` gacha

**Kod:** `kernel/boot/boot.asm`, `kernel/linker.ld`, `kernel/main.c`, `kernel/drivers/{vga,serial,console}.c`,
`kernel/lib/{kprintf,string,panic}.c`, `Makefile`

## 1. Kompyuter yoqilganda

```
Tok  ─►  CPU real mode (16-bit)  ─►  BIOS  ─►  yuklovchi (QEMU/GRUB)  ─►  _start (32-bit)  ─►  kmain (64-bit)
```

1. CPU **real mode**da uyg'onadi. Bu 1978-yilgi 8086 bilan moslik rejimi: 16-bitli registrlar,
   atigi 1 MB manzil.
2. **BIOS** apparatni tekshiradi va yuklovchini ishga tushiradi.
3. **Yuklovchi** yadro faylidan **Multiboot sarlavhasi**ni qidiradi (birinchi 8 KB ichida, `0x1BADB002`
   bilan boshlanadi), ELF faylni xotiraga yuklaydi, CPU'ni **32-bitli protected mode**ga o'tkazadi va
   `_start` ga sakraydi. Bunda `EAX = 0x2BADB002`, `EBX = multiboot_info` manzili bo'ladi.

Biz QEMU'ning ichki Multiboot yuklovchisidan foydalanamiz (`qemu -kernel`). U faqat 32-bitli ELF qabul
qiladi, shuning uchun Makefile `objcopy -O elf32-i386` bilan faqat ELF sarlavhasini o'zgartiradi.

## 2. 32-bitdan 64-bitga (boot.asm)

Intel qoidasi: **Long Mode (64-bit) faqat sahifalash (paging) yoqilgan holda ishlaydi.** Shuning uchun
avval sahifa jadvallarini qurishimiz kerak.

### Sahifa jadvallari — virtual xotiraning yuragi

64-bitli CPU har bir manzilni (virtual) fizik manzilga **4 darajali** jadval orqali tarjima qiladi:

```
 virtual manzil (48 bit ishlatiladi):
 ┌─────────┬─────────┬─────────┬─────────┬──────────────┐
 │ PML4 (9)│ PDPT (9)│  PD (9) │  PT (9) │ siljish (12) │
 └────┬────┴────┬────┴────┬────┴────┬────┴──────┬───────┘
      │         │         │         │           └─► sahifa ichidagi bayt (0..4095)
      │         │         │         └─► PT[i]   -> 4 KB sahifaning fizik manzili
      │         │         └─► PD[i]   -> PT jadvali   (yoki PS=1 bo'lsa: 2 MB sahifa!)
      │         └─► PDPT[i] -> PD jadvali
      └─► PML4[i] -> PDPT jadvali          CR3 registri = PML4 ning fizik manzili
```

Har bir jadval 4 KB: 512 ta 8 baytlik yozuv. 9 bit = 512 variant.

`boot.asm` eng sodda variantni quradi: **PD darajasida 2 MB lik "katta sahifalar"** bilan birinchi
1 GB ni `virtual = fizik` ko'rinishida (**identity mapping**) xaritalaydi. Uchta jadval yetarli.

Nega identity? `CR0.PG` yoqilgan **keyingi** instruksiyani CPU allaqachon virtual manzildan o'qiydi.
Agar kodimiz turgan manzil o'ziga xaritalanmagan bo'lsa — darhol xato.

### O'tish ketma-ketligi

| Qadam | Nima | Nega |
|---|---|---|
| `CR3 = PML4` | jadvalni ko'rsatish | CPU tarjimani qayerdan boshlashni bilishi uchun |
| `CR4.PAE = 1` | 64-bitli jadval yozuvlari | Long Mode talabi |
| `EFER.LME = 1` | Long Mode'ga ruxsat | MSR registri, `rdmsr/wrmsr` bilan |
| `CR0.PG = 1` | paging yoqish | shu lahzada CPU "compatibility mode"ga o'tadi |
| `lgdt` + `jmp 0x08:...` | 64-bitli kod segmenti | far jump CS ni yangilaydi — endi to'liq 64-bit |

## 3. Linker skripti

`linker.ld` yadroni **1 MB** ga joylaydi (pastda BIOS va VGA xotirasi bor) va `kernel_start`, `kernel_end`
belgilarini e'lon qiladi. Fizik xotira menejeri (3-bosqich) shu belgilardan yadro qayerda turganini biladi.

Tekshirib ko'ring:

```bash
readelf -l build/kernel.elf     # segmentlar va ularning manzillari
nm build/kernel.elf | sort      # hamma simvollar manzillari
objdump -d build/kernel.elf | less   # disassembly
```

## 4. Birinchi drayverlar

- **VGA** (`vga.c`): `0xB8000` dagi xotiraga yozamiz — bu MMIO. Har bir belgi 2 bayt: ASCII + rang.
  `volatile` kalit so'zi shart, aks holda kompilyator "keraksiz" yozishlarni o'chirib yuboradi.
- **Serial** (`serial.c`): COM1 porti. QEMU uni terminalga ulaydi. Log saqlash va avtomatik test uchun
  asosiy vosita.
- **Konsol** (`console.c`): ikkalasini birlashtiradi. Yadroning qolgan qismi faqat `kprintf` ni biladi.

## 5. Freestanding C

Yadroda `glibc` yo'q. Demak:

- `printf` yo'q → `kprintf.c` ni o'zimiz yozdik (`va_list` bilan).
- `memcpy/memset` yo'q → `string.c`. **Diqqat:** gcc bu funksiyalarga o'zi ham chaqiruv yaratishi mumkin
  (masalan, katta strukturani nusxalashda), shuning uchun ularning nomi aynan shunday bo'lishi shart.
- `-mno-red-zone`: x86-64 ABI funksiyalarga stek ostidagi 128 baytni ogohlantirishsiz ishlatishga ruxsat
  beradi. Uzilish kelsa, CPU aynan shu joyga yozadi → ma'lumot buziladi. Yadroda bu taqiqlanadi.
- `panic()` + `ASSERT()`: tuzatib bo'lmaydigan xatoda to'xtaymiz va **backtrace** chiqaramiz
  (`rbp` zanjiri bo'ylab). Manzilni funksiya nomiga aylantirish:
  `addr2line -f -e build/kernel.elf 0x10234a`

## Sinab ko'ring

1. `boot.asm` da `or eax, 1 << 5` (PAE) qatorini o'chiring. QEMU nima qiladi? (`-d int,cpu_reset` bilan qarang)
2. `kmain` ichida `ASSERT(1 == 2);` yozing va backtrace'ni `addr2line` bilan tahlil qiling.
3. `vga.c` dagi `volatile` ni olib tashlab, `-O2` da `objdump -d` bilan nima o'zgarganini ko'ring.
