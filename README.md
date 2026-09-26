# MyOS — C tilida noldan yozilgan 64-bitli operatsion tizim yadrosi

O'rganish uchun yozilgan, lekin **haqiqiy** yadro: x86-64 Long Mode, virtual xotira, slab allocator,
preemptive multitasking, himoyalangan user rejimi, syscall'lar, ELF yuklovchi, fayl tizimi va shell.
Hech qanday tashqi kutubxona yo'q: har bir bayt shu repozitoriyada yozilgan.

**Har bir fayl va deyarli har bir qator o'zbek tilida izohlangan:** kod nima qiladi va **nega** aynan shunday yozilgan.

> ## Qayerdan boshlash kerak?
>
> **C'ni endi boshlayotgan bo'lsangiz** — [REJA.md](REJA.md) (6 oylik haftama-hafta reja) va shu tartib:
>
> 1. [darslik/](darslik/README.md) — C tili noldan: 19 bob, har bir belgi (`;`, `void`, `*`, `->` ...) va
>    boshlovchining har bir "nega?" savoliga javob.
> 2. [mashqlar/](mashqlar/README.md) — 40 ta mashq, `tools/mashq.py` avtomatik tekshiradi (xotira
>    xatolarini ham ushlaydi): C asoslaridan sahifa jadvali, buddy, slab, ext2 va mini shell'gacha.
> 3. [QOLLANMA.md](QOLLANMA.md) — loyihaning to'liq qo'llanmasi: o'qish tartibi, texnologiyalar, xatolar
>    tarixi, noldan yadro yozish rejasi. Keyin `docs/` — yadroning har bir qatlami.
> 4. [labs/](labs/README.md) — 22 ta laboratoriya: yadro funksiyasini yopib, o'zingiz yozasiz (`tools/lab.py`).
> 5. [YAKUNIY.md](YAKUNIY.md) — haqiqiy kompyuterga o'rnatish va tizimni kengaytirish ("Endi o'zing davom et").

```
MyOS'ga xush kelibsiz! 'help' - buyruqlar, 'cat /README.txt' - qo'llanma.

myos:/$ ls /bin | grep s | head -3
crash
dmesg
false
myos:/$ echo salom > /home/a.txt ; cat /home/a.txt
salom
myos:/$ cd /home && mkdir -p loyiha/src && ls -l
-rw-r--r--  1          6 2026-09-25 10:08 a.txt
drwxr-xr-x  3          0 2026-09-25 10:08 loyiha/
jami 1 KB
myos:/home$ crash kernel
Yadro kodini (0xFFFFFFFF80110000) o'qiyapman...
[kernel] 'crash' (pid 10) o'ldirildi: Page Fault (sahifa xatosi), RIP=0x00000000004000ca
  Manzil (CR2) = 0xffffffff80110000
  Sabab: ruxsat buzildi (sahifa bor), O'QISH, user rejimida
myos:/home$ fstest
fstest: PASSED (53 tekshiruv)
```

## Tez boshlash

```bash
# Ubuntu / Debian / WSL:
sudo apt install build-essential nasm qemu-system-x86 gdb e2fsprogs \
                 grub-pc-bin grub-efi-amd64-bin grub-common xorriso mtools ovmf
make                    # build/myos.iso - BIOS va UEFI kompyuterlarda yuklanadi
make run                # QEMU, BIOS rejimi
make run-uefi           # QEMU, UEFI rejimi
make run MACHINE=q35    # SATA/AHCI disk bilan (sukut: IDE)
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

Loyiha ikki bosqichda o'sgan. **v0.1** (`git checkout e5906bb`) — kichik, o'qish oson yadro (01–08 hujjatlar).
**Hozirgi versiya** — haqiqiy kompyuterda ishlaydigan, ko'p yadroli tizim (09+ hujjatlar):

| Qatlam | Fayllar | Asosiy g'oyalar | Hujjat |
|---|---|---|---|
| Yuklash | `kernel/boot/*`, `linker.ld` | GRUB, Multiboot2, BIOS + UEFI, higher-half | 09 |
| Xotira | `kernel/mm/*` | memblock, buddy + `struct page`, slab, vmalloc, ioremap, W^X/NX | 09 |
| Ekran | `drivers/fbcon.c`, `console.c` | framebuffer, shrift, dmesg ring buffer | 09 |
| Apparat | `kernel/acpi/*`, `arch/apic.c`, `drivers/pci.c` | ACPI (MADT, FADT, S5), LAPIC/IOAPIC, PCI | 10 |
| SMP | `arch/smp.c`, `trampoline.asm`, `lib/spinlock.c` | INIT-SIPI, per-CPU (GS), TLB shootdown, qulflar | 10 |
| Jarayonlar | `kernel/proc/*`, `arch/syscall_entry.asm` | SMP scheduler, `syscall/sysret`, preemption | 10 |
| Virtual xotira | `kernel/mm/mm.c` | VMA, demand paging, **fork + COW**, exec, mmap | 11 |
| Fayllar | `kernel/fs/*`, `sys/sys_fs.c` | **VFS**, tmpfs, `/dev`, pipe, mount, errno | 12 |
| Disklar | `drivers/{ata,ahci}.c`, `fs/block.c` | ATA PIO, SATA AHCI (DMA), MBR/GPT, buffer cache | 13 |
| ext2 | `fs/ext2.c` | o'qish + yozish, e2fsck bilan tekshiriladi | 13 |
| Signallar | `proc/signal.c` | handler, sigreturn, niqob, alarm, STOP/CONT, process group | 14 |
| Terminal | `drivers/{tty,vt,keyboard}.c` | line discipline, VT100/ANSI emulyatori, UTF-8, strelkalar | 12, 15 |
| libc | `user/libc/*`, `user/include/*` | `stdio.h`, `unistd.h`, `FILE*` buferlash, `libc.a` | 12 |
| Shell va utilitalar | `user/bin/*` | `|` `>` `<` `&&` `$?` glob, job control, tarix, Tab; `edit` muharriri; 44 ta dastur | 12, 14, 15 |
| Sifat | `kernel/tests/*`, `tools/test.sh`, CI | 100 ta yadro testi + 41 ta integratsion test (BIOS/IDE + UEFI/AHCI), e2fsck | 08 |

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
5. [docs/09-yangi-arxitektura.md](docs/09-yangi-arxitektura.md), [docs/10-smp.md](docs/10-smp.md), [docs/11-fork-cow.md](docs/11-fork-cow.md),
   [docs/12-vfs.md](docs/12-vfs.md), [docs/13-disk-ext2.md](docs/13-disk-ext2.md), [docs/14-signallar.md](docs/14-signallar.md), [docs/15-terminal.md](docs/15-terminal.md) — hozirgi yadro: GRUB/UEFI,
   higher-half, buddy, slab, SMP, fork/COW, VFS, pipe, terminal, libc, shell, disklar, ext2, signallar va terminal. 01–08 bo'limlar kichik versiyani (`e5906bb`) tushuntiradi. Ular oddiyroq,
   shuning uchun avval o'shalarni o'qing.
6. [docs/mashqlar.md](docs/mashqlar.md) — o'zingiz qo'shadigan narsalar. **Eng muhim qism shu.**

## Tuzilma

```
kernel/
  boot/      boot.asm (32→64 bit, higher-half), bootinfo.c (Multiboot2)
  arch/      GDT/TSS, IDT, ISR, APIC, SMP, syscall kirish nuqtasi, per-CPU
  acpi/      ACPI jadvallari: MADT (CPU'lar), FADT (o'chirish), DSDT (_S5)
  mm/        memblock, buddy (pmm), slab, vmalloc, vmm (sahifa jadvallari), mm (VMA, COW)
  proc/      jarayonlar, SMP scheduler, context switch, exec, fork
  fs/        vfs, tmpfs, devfs, pipe, initrd, block (disklar, bo'limlar), ext2
  drivers/   ata, ahci, console, fbcon, tty, keyboard, serial, pci, pit, rtc
  sys/       syscall'lar, sys_fs (fayl syscall'lari), uaccess, ELF
  lib/       kprintf, spinlock, mutex, klog, panic
  tests/     selftest (100 ta tekshiruv), crashdemo
include/myos/abi.h   yadro <-> user shartnomasi (syscall raqamlari, errno, strukturalar)
user/
  include/   libc sarlavhalari: stdio.h, unistd.h, fcntl.h, dirent.h, sys/stat.h ...
  libc/      crt0, syscall o'ramlari, stdio, printf, malloc, string, dirent, time
  bin/       sh va utilitalar -> diskdagi /bin
rootfs/      /etc/rc, /etc/motd, /README.txt -> initrd.tar ga qo'shiladi
docs/        har bir qatlam bo'yicha batafsil tushuntirish
tools/       test.sh, mkdisk.py (MBR + ext2 disk), gdbinit, psf2c.py (shrift), screenshot.sh
```

## Xotira xaritasi (bir qarashda)

```
virtual manzil                  nima                                   ruxsat
0x0000000000000000              NULL sahifa - xaritalanmagan           —
0x0000000000400000              user dastur (ELF), keyin heap (sbrk)   ring 3
...0x00007000_00000000 pastga   mmap hududlari                         ring 3
...0x00007FFF_FFFFF000 pastga   user steki (8 MB gacha o'sadi)         ring 3
0xFFFF800000000000              butun fizik RAM (direct map, HHDM)     ring 0, NX
0xFFFFC00000000000              vmalloc: yadro steklari, ioremap       ring 0
0xFFFFFFFF80000000              yadro: .text (r-x) .rodata (r--) .data (rw-)
```

## Litsenziya va hissa

O'quv loyihasi. Xato topsangiz yoki mashqni bajarsangiz, pull request oching. `make test` o'tishi shart.
