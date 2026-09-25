# Mashqlar — kuchli muhandisga aylanish yo'li

Tayyor kodni o'qish — boshlanish, xolos. Haqiqiy o'sish **o'zingiz yangi narsa qo'shganingizda** va unda
xato qilib, uni topganingizda keladi. Mashqlar qiyinlik bo'yicha tartiblangan. Har birini alohida git branch'da
qiling va unga test yozing.

★ — oson (bir necha soat) · ★★ — o'rta (1–3 kun) · ★★★ — qiyin (bir hafta+) · ★★★★ — jiddiy loyiha

## Isitish

- ★ **`uptime` dasturi.** `SYS_UPTIME` bor, uni chiqaradigan user dasturini yozing (soat:daqiqa:soniya).
- ★ **`clear` buyrug'i.** Ekranni tozalash. Nima kerak: yangi syscall yoki ANSI escape ketma-ketliklari?
- ★ **Tezroq `memcpy`.** `kernel/lib/string.c` da 8 baytlik bo'laklar bilan nusxalang. Tekislanmagan boshi va
  oxirini to'g'ri hisoblang. Oldin va keyin tezligini o'lchang.
- ★ **Klaviatura strelkalari.** `keyboard.c` da 0xE0 prefiksli kodlarni qayta ishlang. Shell'ga buyruqlar
  tarixini (yuqori strelka) qo'shing.

## Xotira (loyihaning asosiy mavzusi)

- ★★ **`krealloc()`** va heap'da **redzone**: har bir obyekt oxiriga 8 bayt `0xBB` qo'shib, `kfree` da tekshiring.
  Shu bilan buffer overflow'ni aniqlaysiz.
- ★★ **Bo'sh jadvallarni qaytarish.** `vmm_unmap_page` bo'shab qolgan PT/PD jadvallarini PMM ga qaytarmaydi.
  Buni tuzating va testda isbotlang.
- ★★ **NX bit.** `EFER.NXE` ni yoqing, user stek va heap'ni bajarib bo'lmaydigan qiling. `crash` ga "stekdagi
  kodni bajarish" variantini qo'shing.
- ★★★ **Buddy allocator.** PMM'ni buddy tizimiga o'tkazing (Linux'dagidek). `pmm_alloc_frames(n)` O(log n) bo'lsin.
  Fragmentatsiya statistikasini chiqaring.
- ★★★ **Lazy allocation (demand paging).** `sbrk` sahifalarni darhol emas, birinchi murojaatda (page fault'da)
  ajratsin. Page fault handler'da "bu manzil haqiqatan heap ichidami?" degan savolni hal qiling.
- ★★★ **`fork()` + copy-on-write.** Manzil maydonini nusxalang, lekin sahifalarni "faqat o'qish" qilib umumiy
  qoldiring; yozishda page fault → nusxa. Har bir freym uchun **reference count** kerak bo'ladi.
- ★★★★ **Higher-half yadro.** Yadroni `0xFFFFFFFF80000000` ga ko'chiring, butun fizik RAM'ni
  `0xFFFF800000000000` ga xaritalang. Shunda user uchun butun pastki 128 TB bo'shaydi va 1 GB cheklovi yo'qoladi.

## Jarayonlar va parallellik

- ★★ **Ustuvorlik (priority)** va `nice` buyrug'i. Keyin **MLFQ** (multi-level feedback queue).
- ★★ **Pipe'lar:** `ls | cat`. Halqali bufer, ikkita fd, yozuvchi/o'quvchini uxlatish/uyg'otish. Shell'ga `|`.
- ★★ **`syscall`/`sysret`.** `int 0x80` dan tezroq. MSR'lar: `STAR`, `LSTAR`, `SFMASK`; `swapgs` bilan yadro steki.
- ★★★ **Yadro ichida preemption** va **mutex/semafor**. Hozir syscall IF=0 da ishlaydi. Uni yoqsangiz, qanday
  qulflar kerak bo'ladi?
- ★★★★ **SMP (ko'p yadroli).** APIC, AP'larni ishga tushirish (INIT-SIPI-SIPI), **spinlock**'lar, per-CPU
  `current`, TLB shootdown. Bu OS muhandisligining eng qiyin mavzularidan biri.

## Qurilmalar va fayl tizimi

- ★★ **VFS qatlami:** `struct file_ops { read, write, close }` — funksiya ko'rsatkichlari jadvali.
  `/dev/null`, `/dev/zero` qurilmalarini qo'shing.
- ★★ **RTC** (real vaqt soati, 0x70/0x71 portlari): `date` buyrug'i.
- ★★★ **ATA PIO disk drayveri** va **ext2 (faqat o'qish)**. QEMU'ga `-hda disk.img` bering.
- ★★★ **Framebuffer grafikasi** (Multiboot video rejimi yoki Bochs VBE): piksel chizish, shrift.
- ★★★★ **Tarmoq:** e1000 yoki virtio-net drayveri, ARP, IP, ICMP (`ping`), UDP.

## Sifat

- ★★ **Fuzzing:** `elf_load` va `tarfs` parser'larini oddiy Linux dasturiga ajratib oling va AFL++ yoki
  libFuzzer bilan sinang. Xato topsangiz, tuzatib, regression test qo'shing.
- ★★ **Syscall fuzzer** — user dasturi: tasodifiy raqamlar va argumentlar bilan syscall chaqiradi. Yadro panic
  bo'lmasligi kerak!
- ★★★ **Mikro-benchmarklar:** syscall narxi (taktlarda, `rdtsc` bilan), kontekst almashtirish narxi,
  `kmalloc` vs `malloc`. Natijalarni Linux bilan solishtiring.

## O'qish uchun

- **Intel SDM, 3-jild** — CPU haqida eng aniq manba (paging: 4-bob, uzilishlar: 6-bob).
- **OSDev wiki** — wiki.osdev.org — deyarli har bir mavzu bo'yicha amaliy maqolalar.
- **xv6** (MIT 6.1810 kursi) — o'qish uchun yozilgan Unix. Bizning loyiha bilan solishtirib o'qing.
- **"Operating Systems: Three Easy Pieces"** — bepul, juda yaxshi nazariy kitob (ostep.org).
- **"Understanding the Linux Kernel"**, **"Linux Kernel Development"** (Robert Love).
