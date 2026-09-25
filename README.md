# MyOS — C tilida noldan yozilgan 64-bitli operatsion tizim yadrosi

O'rganish uchun yozilgan, lekin **haqiqiy** yadro: x86-64 Long Mode, virtual xotira, slab allocator,
preemptive multitasking, himoyalangan user rejimi, syscall'lar, ELF yuklovchi, fayl tizimi va shell.
Hech qanday tashqi kutubxona yo'q: har bir bayt shu repozitoriyada yozilgan.

**Har bir fayl va deyarli har bir qator o'zbek tilida izohlangan:** kod nima qiladi va **nega** aynan shunday yozilgan.

```
MyOS shell'iga xush kelibsiz! (pid 2). 'help' - yordam.
myos$ hello salom
Salom, dunyo! Men user rejimida (ring 3) ishlayapman.
myos$ crash kernel
Yadro kodini (0x100000) o'qiyapman...
[kernel] 'crash' (pid 4) o'ldirildi: Page Fault (sahifa xatosi), RIP=0x400000b8
  Manzil (CR2) = 0x0000000000100000
  Sabab: ruxsat buzildi (sahifa bor), O'QISH, user rejimida
myos$ memtest
  ...
memtest: PASSED
myos$ ps
  PID  PPID  TUR     HOLAT         CPU(tik)  XOTIRA  NOMI
    0    -1  yadro   ishlayapti           0     0 KB  idle
    1     0  yadro   kutyapti             0     0 KB  init
    2     1  user    kutyapti             3    80 KB  sh
    5     2  user    ishlayapti           0    84 KB  ps
```

## Tez boshlash

```bash
# Ubuntu / Debian / WSL:
sudo apt install build-essential nasm qemu-system-x86 gdb \
                 grub-pc-bin grub-efi-amd64-bin grub-common xorriso mtools ovmf
make                    # build/myos.iso - BIOS va UEFI kompyuterlarda yuklanadi
make run                # QEMU, BIOS rejimi
make run-uefi           # QEMU, UEFI rejimi
make run-nographic      # faqat terminal (chiqish: Ctrl-A, keyin X)
make test               # avtomatik testlar (BIOS + UEFI)
make debug              # GDB bilan (docs/08-test-debug.md)
```

Xato namoyishlari (yadro qanday himoyalanishini ko'rish uchun):

```bash
make run-nographic APPEND=selftest        # yadro ichki testlari
make run-nographic APPEND=demo=uaf        # use-after-free ushlanadi
make run-nographic APPEND=demo=stack      # yadro steki to'lishi -> double fault (IST)
make run-nographic APPEND=threads         # yadro oqimlari parallel ishlaydi
```

## Nima qurilgan

| Qatlam | Fayllar | Asosiy g'oyalar |
|---|---|---|
| **1. Yuklash** | `kernel/boot/boot.asm`, `linker.ld` | Multiboot, CPUID, 32→64 bit, identity paging, GDT |
| **2. Uzilishlar** | `kernel/arch/*` , `drivers/pic.c`, `pit.c`, `keyboard.c` | GDT/TSS/IST, IDT, 256 ISR stub, PIC, 100 Hz taymer |
| **3. Fizik xotira** | `kernel/mm/pmm.c` | E820 xarita, bitmap, `ctz`, next-fit, double-free aniqlash |
| **4. Virtual xotira** | `kernel/mm/vmm.c` | 4 darajali jadvallar, manzil maydonlari, NULL/guard sahifalar, TLB |
| **5. Heap** | `kernel/mm/heap.c` | slab allocator, use-after-free/double-free aniqlash (zahar) |
| **6. Jarayonlar** | `kernel/proc/process.c`, `switch.asm` | context switch, round-robin, preemption, sleep/wakeup, zombie/wait |
| **7. User rejimi** | `kernel/sys/*`, `kernel/fs/*`, `proc/exec.c`, `user/*` | ring 3, `int 0x80`, ELF, tarfs, fd'lar, libc, malloc, shell |
| **8. Sifat** | `kernel/tests/*`, `tools/test.sh`, `.github/workflows/ci.yml` | unit + integratsion testlar, CI, GDB |

Hajmi: yadro ~5000 qator C va ~500 qator assembly; user dasturlari ~1300 qator; hujjatlar ~1100 qator.

## Qanday o'rganish kerak

1. [docs/00-kirish.md](docs/00-kirish.md) dan boshlang va hujjatlarni **tartib bilan** o'qing (01 → 08).
2. Har bir hujjat bilan birga tegishli kodni o'qing. Izohlar kodning o'zida.
3. Git tarixi ham o'quv qo'llanma: har bir commit bitta bosqich.
   ```bash
   git log --oneline --reverse
   git checkout <1-bosqich-commit>   # o'sha bosqichdagi holatni yig'ib, ishga tushiring
   git checkout -                    # qaytish
   ```
4. Har bir hujjat oxiridagi **"Sinab ko'ring"** bo'limini bajaring: kodni ataylab buzing va natijani kuzating.
5. [docs/09-yangi-arxitektura.md](docs/09-yangi-arxitektura.md) — hozirgi yadro: GRUB/UEFI, higher-half,
   buddy, slab, vmalloc, framebuffer. 01–08 bo'limlar kichik versiyani (`e5906bb`) tushuntiradi. Ular oddiyroq,
   shuning uchun avval o'shalarni o'qing.
6. [docs/mashqlar.md](docs/mashqlar.md) — o'zingiz qo'shadigan narsalar. **Eng muhim qism shu.**

## Tuzilma

```
kernel/
  boot/     boot.asm (32→64 bit), multiboot.h
  arch/     GDT, TSS, IDT, ISR stub'lari, uzilishlar dispetcheri, port I/O
  drivers/  VGA, serial, klaviatura, PIT, PIC, konsol
  lib/      kprintf, string, panic + backtrace
  mm/       pmm (fizik), vmm (virtual), heap (slab)
  proc/     jarayonlar, scheduler, context switch, exec
  fs/       tarfs, fayl obyektlari
  sys/      syscall'lar, ELF yuklovchi
  tests/    selftest, crashdemo
include/myos/abi.h   yadro <-> user shartnomasi (syscall raqamlari, strukturalar)
user/
  lib/      crt0, syscall o'ramlari, printf, malloc
  bin/      sh, hello, echo, ls, cat, ps, free, kill, spin, crash, memtest
docs/       har bir qatlam bo'yicha batafsil tushuntirish
tools/      test.sh, gdbinit
```

## Xotira xaritasi (bir qarashda)

```
virtual manzil               nima                                  ruxsat
0x0000000000000000           NULL sahifa - xaritalanmagan          —
0x0000000000001000-1 GB      yadro (identity): kod, heap, RAM      faqat ring 0
0x0000000040000000           user dastur (ELF), keyin heap (sbrk)  ring 3
0x000000007FFF0000-80000000  user steki (64 KB), ostida guard      ring 3
```

## Litsenziya va hissa

O'quv loyihasi. Xato topsangiz yoki mashqni bajarsangiz, pull request oching. `make test` o'tishi shart.
