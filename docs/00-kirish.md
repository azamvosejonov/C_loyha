# 00 — Kirish: OS yadrosi nima va bu loyihani qanday o'rganish kerak

## Operatsion tizim yadrosi nima?

Oddiy dastur (masalan, `python` yoki brauzer) hech qachon apparatga to'g'ridan-to'g'ri tegmaydi.
U **yadro**dan so'raydi:

| Dastur so'raydi | Yadro nima qiladi |
|---|---|
| "menga 1 MB xotira ber" (`malloc` → `sbrk`/`mmap`) | bo'sh fizik sahifalarni topadi, dasturning virtual manzillariga ulaydi |
| "ekranga yoz" (`write`) | drayver orqali qurilmaga yuboradi |
| "faylni o'qi" (`read`) | fayl tizimidan topib, nusxalab beradi |
| "yangi dastur ishga tushir" (`fork`/`exec`) | yangi jarayon, yangi manzil maydoni yaratadi |

Yadro yana **himoya** qiladi: bitta dastur boshqasining xotirasini o'qiy olmaydi, yadroni buza olmaydi.
Buni CPU'ning ikki mexanizmi ta'minlaydi: **himoya halqalari** (ring 0 = yadro, ring 3 = dastur)
va **sahifalash** (paging: har bir jarayonning o'z virtual xotirasi bor).

Bu loyihada shularning **hammasini** noldan quramiz.

## Loyiha tuzilishi

```
kernel/
  boot/        boot.asm - 32-bitdan 64-bitga o'tish; multiboot.h
  arch/        CPU'ga xos kod: GDT, IDT, uzilishlar, port I/O
  drivers/     VGA ekran, serial port, klaviatura, taymer, PIC, konsol
  lib/         kprintf, string, panic - yadroning "standart kutubxonasi"
  mm/          XOTIRA: pmm.c (fizik), vmm.c (virtual), heap.c (kmalloc)
  proc/        jarayonlar, scheduler, kontekst almashtirish
  fs/          tarfs - initrd.tar dan fayllarni o'qish
  sys/         syscall'lar va ELF yuklovchi
  tests/       yadro ichidagi o'z-o'zini tekshirish testlari
  main.c       kmain() - hammasini tartib bilan ishga tushiradi
user/
  lib/         user-mode kutubxona: crt0, syscall o'ramlari, printf, malloc
  bin/         dasturlar: sh (shell), hello, memtest, ...
docs/          har bir qatlam bo'yicha tushuntirish (siz o'qiyotgan fayllar)
tools/test.sh  avtomatik test
```

## Qatlamlar va o'qish tartibi

Har bir qatlam oldingisiga tayanadi. Aynan shu tartibda o'qing:

| # | Hujjat | Kod | Nima o'rganasiz |
|---|---|---|---|
| 1 | [01-yuklash.md](01-yuklash.md) | `boot/`, `linker.ld`, `drivers/vga.c`, `serial.c`, `lib/` | CPU qanday yoqiladi, 64-bitli rejim, sahifa jadvallari, MMIO |
| 2 | [02-uzilishlar.md](02-uzilishlar.md) | `arch/` , `drivers/pic.c`, `pit.c`, `keyboard.c` | GDT, TSS, IDT, exception'lar, apparat uzilishlari |
| 3 | [03-fizik-xotira.md](03-fizik-xotira.md) | `mm/pmm.c` | RAM xaritasi, bitmap allocator |
| 4 | [04-virtual-xotira.md](04-virtual-xotira.md) | `mm/vmm.c` | 4 darajali paging, manzil maydonlari, TLB |
| 5 | [05-heap.md](05-heap.md) | `mm/heap.c` | slab allocator, kmalloc/kfree, xotira buzilishini aniqlash |
| 6 | [06-jarayonlar.md](06-jarayonlar.md) | `proc/` | kontekst almashtirish, scheduler, uxlash/uyg'otish |
| 7 | [07-user-mode.md](07-user-mode.md) | `sys/`, `fs/`, `user/` | ring 3, syscall, ELF yuklash, fayl tizimi, shell |
| 8 | [08-test-debug.md](08-test-debug.md) | `tests/`, `tools/` | GDB, QEMU monitor, avtomatik testlar |
| ★ | [mashqlar.md](mashqlar.md) | — | o'zingiz qo'shadigan narsalar (eng muhim qism!) |

Git tarixi ham shu tartibda: `git log --oneline` qiling. Har bir commit bitta bosqich.
`git checkout <commit>` bilan istalgan bosqichga qaytib, o'sha holatda yig'ib ko'rishingiz mumkin.

## Kerakli vositalar

Ubuntu/Debian:

```bash
sudo apt install build-essential nasm qemu-system-x86 gdb
```

- **gcc** — C kompilyatori. Biz uni `-ffreestanding` rejimida ishlatamiz (Makefile'dagi izohlarga qarang).
- **nasm** — assembler (Intel sintaksisi, o'qish osonroq).
- **ld, objcopy** — linker va ELF formatini o'zgartirish (binutils).
- **qemu-system-x86_64** — kompyuter emulyatori. Yadroni haqiqiy kompyuterga yozmasdan sinaymiz.
- **gdb** — debugger. `make debug` bilan yadroni qadamma-qadam bajarish mumkin.

> **Cross-compiler haqida.** Jiddiy OS loyihalarida `x86_64-elf-gcc` degan alohida kompilyator
> yig'iladi, chunki tizimdagi gcc Linux uchun sozlangan. Biz tizim gcc'sini ishlatamiz va buni
> `-ffreestanding -fno-pie -fno-stack-protector -mno-red-zone` bayroqlari bilan to'g'irlaymiz.
> O'rganish uchun bu yetarli; OSDev wiki'dagi "GCC Cross-Compiler" maqolasi ham foydali.

## Ishga tushirish

```bash
make run              # QEMU oynasi + serial chiqish terminalda
make run-nographic    # faqat terminal (SSH orqali ishlasangiz). Chiqish: Ctrl-A, keyin X
make test             # avtomatik testlar
make debug            # GDB uchun (08-test-debug.md ga qarang)
```

## Qanday o'rganish kerak (muhim!)

1. **Faqat o'qimang, qayta yozing.** Har bir faylni o'qib bo'lgach, uni yopib, o'zingiz qaytadan
   yozishga harakat qiling. Tushunmagan joyingiz shunda ko'rinadi.
2. **Buzing.** `-mno-red-zone` ni olib tashlang, `boot.asm` da PAE ni yoqmang, `pmm.c` da yadro
   xotirasini "band" deb belgilamang — nima bo'lishini ko'ring. Xatolarni o'z ko'zingiz bilan
   ko'rish ularni tushunishning eng tez yo'li.
3. **Asosiy manbalarni o'qing.** Intel® 64 and IA-32 Architectures Software Developer's Manual
   (SDM), 3-jild — CPU haqidagi eng aniq manba. OSDev wiki (wiki.osdev.org) — amaliy maslahatlar.
   xv6 (MIT) — o'qish uchun yozilgan kichik Unix yadrosi.
4. **Mashqlarni bajaring** ([mashqlar.md](mashqlar.md)). Kuchli muhandis bo'lish tayyor kodni
   o'qishdan emas, yangi narsa qo'shish va xatoni o'zingiz topishdan keladi.
