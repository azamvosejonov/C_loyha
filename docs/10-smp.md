# 10 — Ko'p yadroli tizim (SMP): ACPI, APIC, syscall, qulflar, PCI

Zamonaviy kompyuterda 4–32 ta CPU yadrosi bor. Bu bo'lim yadroni **barcha yadrolardan bir vaqtda**
foydalanadigan qiladi. OS muhandisligining eng qiyin mavzusi shu, chunki xatolar tasodifiy chiqadi va
qayta takrorlash qiyin.

Kod: `kernel/acpi/acpi.c`, `kernel/arch/{apic.c,smp.c,trampoline.asm,percpu.c,syscall_entry.asm,tsc.c}`,
`kernel/lib/spinlock.{h,c}`, `kernel/proc/process.c`, `kernel/drivers/pci.c`

## 1. ACPI — firmware'ning "kompyuter tavsifi"

```
RSDP (GRUB beradi yoki BIOS xotirasida qidiriladi)
 └─ XSDT (64-bit) / RSDT (32-bit)
     ├─ MADT "APIC": nechta CPU (APIC ID lari), IOAPIC manzillari, IRQ qayta yo'naltirishlari
     ├─ FADT "FACP": quvvat portlari (PM1a_CNT) → o'chirish; reset registri → qayta yuklash
     │   └─ DSDT: AML bayt-kodi. Undan faqat \_S5 (o'chirish holati) olinadi
     └─ MCFG, HPET, ...
```

- Har bir jadvalning **checksum**i bor: barcha baytlar yig'indisi 0 bo'lishi kerak. Buzilgan jadval rad etiladi.
- **IRQ qayta yo'naltirish:** ko'p kompyuterlarda taymer (ISA IRQ0) aslida IOAPIC'ning 2-kirishiga ulangan.
  Buni faqat MADT aytadi.
- **O'chirish (S5):** to'liq AML interpretatori (ACPICA ~100 000 qator) o'rniga `_S5_` paketini qidiruvchi keng
  tarqalgan usul ishlatiladi. Aksariyat kompyuterlarda ishlaydi.

## 2. Local APIC va IO APIC

| | 8259 PIC (1976) | APIC |
|---|---|---|
| CPU'lar | faqat bitta | har bir CPU'da Local APIC |
| Taymer | umumiy PIT | **har bir CPU'ning o'z taymeri** |
| CPU'lar orasida | — | **IPI** (Inter-Processor Interrupt) |
| Qurilma IRQ'lari | 15 ta | IOAPIC: har biri 24+ kirish, istalgan CPU'ga |

- **xAPIC** (MMIO, `0xFEE00000`) va **x2APIC** (MSR) ikkalasi ham qo'llab-quvvatlanadi. Ba'zi UEFI
  firmware'lar x2APIC'ni yoqib qo'yadi, bunday holatda MMIO ishlamaydi.
- LAPIC taymerining chastotasi CPU'ga bog'liq, shuning uchun u **PIT 2-kanali** bilan kalibrlanadi
  (3 marta o'lchab, minimumi olinadi).
- APIC yoqilgach, eski PIC butunlay niqoblanadi, aks holda "soxta" uzilishlar paydo bo'ladi.

## 3. AP'larni uyg'otish (`smp.c`, `trampoline.asm`)

Kompyuter yoqilganda faqat **BSP** ishlaydi. Qolgan yadrolar (AP) INIT → SIPI signallarini kutadi va
**16-bitli real mode**da uyg'onadi:

```
BSP: trampolinni 0x8000 ga nusxalash, CR3/stek/kirish nuqtasini yozish
     INIT IPI → 10 ms → SIPI(0x08) → (javob bo'lmasa) SIPI → kutish (100 ms)
AP:  16-bit real → lgdt → 32-bit protected → PAE, CR3, EFER.LME → 64-bit
     → ap_main(): GS=per-CPU, o'z GDT/TSS, IDT, LAPIC, syscall MSR → scheduler_loop()
```

AP uchun **vaqtinchalik sahifa jadvali** quriladi. Unda yadroning yuqori yarmi va `0..2 MB` identity bor,
aks holda paging yoqilgan zahoti trampolin kodi "yo'qolardi". Jadval 4 GB dan past joylashishi shart
(`GFP_DMA32`), chunki CR3 ga 32-bitli rejimda yoziladi.

## 4. Per-CPU ma'lumotlar va `swapgs`

Har bir CPU'ning o'z `struct cpu`i bor: joriy jarayon, GDT, TSS, ISR steki. U **GS** registri orqali topiladi:

```c
mov rax, gs:[24]     ; current - BITTA instruksiya: CPU almashib qolish xavfi yo'q
```

User dastur ham GS'dan foydalanishi mumkin. Shuning uchun yadroga kirishda (`syscall`, user rejimidan kelgan
uzilish) va chiqishda **`swapgs`** GS bazasini almashtiradi. `isr.asm` buni `CS & 3` ga qarab hal qiladi.

**Topilgan bug:** `gdt_load` avval GS'ni ham qayta yuklardi. 64-bitli rejimda bu GS bazasini **nolga** tushiradi,
ya'ni per-CPU ko'rsatkich yo'qoladi. Endi FS va GS'ga tegilmaydi.

## 5. `syscall` / `sysret`

`int 0x80` o'rniga `syscall` ishlatiladi. U ancha tez, lekin **stekni almashtirmaydi**, shuning uchun
`syscall_entry.asm` birinchi navbatda `swapgs` qiladi va yadro stekini `gs:8` dan oladi. Keyin
`int 0x80` bilan **aynan bir xil** freym quriladi: C kodi farqni bilmaydi.

**Xavfsizlik:** RIP kanonik bo'lmasa, `sysret` **ring 0 da, lekin user stekida** #GP beradi (CVE-2012-0217,
FreeBSD/Xen/Windows zaifligi). Shuning uchun RIP tekshiriladi. Shubhali holatda sekinroq `iretq` yo'li tanlanadi.

## 6. Qulflar (`spinlock.c`)

- `xchg` bilan atomar olish, **test-and-test-and-set**, `pause`.
- `spin_lock` uzilishlarni ichma-ich o'chiradi (`push_off/pop_off`, xv6 uslubi).
- **Deadlock aniqlash:** qulfning **egasi** (qaysi CPU) saqlanadi. O'sha CPU qulfni qayta olmoqchi bo'lsa,
  darhol panic beradi. Shu tekshiruv haqiqiy bug'ni topdi: yangi user jarayon `proc_lock` ni qo'yib
  yubormasdan ring 3 ga tushib ketayotgan edi (`exec.c` eski stek tuzilishini ishlatgan).
- **Qulflar tartibi** (deadlock oldini olish uchun doim shu tartibda olinadi):

```
console-input  →  proc
proc           →  zone (buddy)  →  slab keshi
vmalloc        →  zone
```

- **TLB shootdown:** yadro sahifasi o'chirilganda barcha CPU'larga IPI yuboriladi. Deadlock bo'lmasligi uchun
  har bir CPU'da har bir yuboruvchiga alohida "pochta qutisi" bor. Javob kutayotgan CPU o'z qutisini ham
  qayta ishlaydi.

## 7. SMP scheduler (xv6 modeli)

```
jarayon A ──sched()──► CPU scheduler tsikli ──context_switch──► jarayon B
```

- Har bir CPU'da o'z **scheduler tsikli** bor (`scheduler_loop`, CPU'ning idle stekida).
- **`proc_lock` context switch davomida USHLANGAN** bo'ladi va uni "narigi tomon" qo'yib yuboradi. Aks holda
  A hali o'z stekida turganida boshqa CPU uni olib ketishi mumkin edi, ya'ni ikki CPU bitta stekda ishlab ketardi.
- **`proc_sleep(chan, lk)`**: shartni himoya qiluvchi `lk` bilan chaqiriladi va uyg'otish yo'qolmaydi.
  `console_getc` misoli:

```c
spin_lock(&input_lock);
while (bufer_bo'sh)
    proc_sleep(&input_head, &input_lock);   // lk ATOMAR qo'yib yuboriladi
c = ...;
spin_unlock(&input_lock);
```

- Yadro **preemptiv**: taymer yadro kodini ham to'xtatishi mumkin, lekin faqat IF=1 bo'lganda, ya'ni hech qanday
  spinlock ushlanmaganda.

## 8. PCI (`drivers/pci.c`)

- `0xCF8/0xCFC` portlari orqali konfiguratsiya maydoni o'qiladi. Ko'priklar (06:04) rekursiv skanerlanadi,
  ko'p funksiyali qurilmalar ham hisobga olinadi.
- **BAR hajmi:** BAR'ga `0xFFFFFFFF` yoziladi, keyin o'qiladi: `~qiymat + 1`. 64-bitli BAR'lar ham qo'llab-quvvatlanadi.
- **Drayver modeli:** `pci_register_driver()` bilan (vendor, device, class) bo'yicha moslik topiladi va
  `probe()` chaqiriladi. Linux ham aynan shunday ishlaydi.

## Sinab ko'ring

```bash
make run QEMU_SMP=4                       # 4 ta CPU
make run APPEND=selftest QEMU_SMP=4       # 100 ta test, SMP spinlock stress testi bilan
```

Shell'da: `ps` (har bir CPU'ning idle'i), `lspci`, `uname`, `dmesg`, `spin a 20 &` + `spin b 20 &` + `spin c 20`.

1. `lock_thread` (selftest) dan `spin_lock` ni olib tashlang va `-smp 4` da ishga tushiring. Hisoblagich nechchi chiqdi?
2. `syscall_entry.asm` dagi kanonik RIP tekshiruvini o'chirsak, qanday hujum mumkin bo'lardi? (CVE-2012-0217 ni o'qing.)
3. **Mashq:** har bir CPU uchun alohida run queue va "work stealing" qo'shing.
4. **Mashq:** qurilma uzilishlarini bitta CPU'ga emas, hammasiga taqsimlang (IRQ balancing).
