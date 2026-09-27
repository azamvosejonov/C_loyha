# 30-bob. Yadro arxitekturasi va Linux'ga yo'l

> **Bu bobdan keyin:** yadro turlarini (monolit, mikroyadro, gibrid, exokernel, unikernel), MyOS, xv6
> va Linux'ni solishtirishni, Linux manba kodining xaritasini, uning kod uslubi va patch yuborish
> jarayonini bilasiz. Bu — MyOS'dan keyin haqiqiy katta yadroga o'tish uchun xarita. (Odatda "Linux
> Kernel Development" va "Understanding the Linux Kernel" kitoblaridan o'rganiladigan umumiy manzara.)

> **To'liq ishlaydigan misol:** [misollar/30_yadro_moduli/](misollar/30_yadro_moduli/salom_modul.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Yadro — shahar hokimiyati (30.1).** Hokimiyat besh ishni qiladi, yadro ham:
1. **Resurslarni taqsimlash** — yer uchastkalari (xotira), yo'llardagi navbat (CPU vaqti).
2. **Himoya** — bir fuqaro boshqasining uyiga kira olmaydi (jarayonlar izolyatsiyasi).
3. **Xizmatlar** — davlat xizmatlari markazi: ariza berasiz, ular bajaradi (syscall'lar).
4. **Umumiy infratuzilma** — yo'llar, suv quvurlari (drayverlar, fayl tizimlari, tarmoq).
5. **Abstraksiya** — fuqaro elektr stantsiyasi qanday ishlashini bilmaydi, faqat rozetkadan foydalanadi
   (dastur diskning turini bilmaydi, faqat `read` chaqiradi).

**Monolit yadro — bitta katta vazirlik binosi (30.2).** Hamma bo'limlar bir binoda: gaplashish tez
(oddiy funksiya chaqiruvi). Lekin bitta bo'limda yong'in chiqsa — butun bino yonadi (drayverdagi xato
butun tizimni qulatadi). Linux — monolit.

**Mikroyadro — alohida binolar (30.2).** Har bir bo'lim o'z binosida, faqat xat (xabar) orqali gaplashadi.
Bittasi yonsa, boshqalari ishlayveradi — xavfsizroq. Lekin xat yuborish sekinroq. Masalan, seL4, QNX.

**Modul — binoga vaqtinchalik qo'shiladigan bo'lim (30.6).** Yangi xizmat kerak bo'lsa, butun binoni
qayta qurmaysiz — bo'sh xonaga yangi bo'lim ko'chib kiradi (`insmod`) va keraksiz bo'lganda chiqib ketadi
(`rmmod`). Linux drayverlarining ko'pi modul.

**Patch yuborish — qonun loyihasi (30.7).** Taklif yozasiz (patch), u pochta ro'yxatida muhokama
qilinadi, mutaxassislar (maintainer) tanqid qiladi, siz tuzatasiz (v2, v3...), oxirida qabul qilinadi
va keyingi relizga kiradi. Jarayon sekin, lekin har bir qator ko'p ko'zdan o'tadi.

### To'liq dastur: syscall jadvali (yadroning "xizmatlar markazi")

Haqiqiy yadroda syscall raqami funksiyalar jadvalidan kerakli ishlovchini tanlaydi. Bu yerda xuddi shu
mexanizm oddiy dasturda — funksiya ko'rsatkichlari massivi bilan.

```c
/* xizmatlar.c - syscall jadvali: raqam -> ishlovchi funksiya, noma'lum raqamga -ENOSYS */
#include <errno.h>
#include <stdio.h>

typedef long (*ishlovchi)(long a, long b);

static long x_getpid(long a, long b) { (void)a; (void)b; return 42; }
static long x_qosh(long a, long b) { return a + b; }
static long x_bol(long a, long b)
{
    if (b == 0)
        return -EINVAL;                         /* noto'g'ri argument */
    return a / b;
}

/* Linux'da ham shunday: sys_call_table[__NR_xxx] = sys_xxx */
static const ishlovchi jadval[] = {
    [0] = x_getpid,
    [1] = x_qosh,
    [2] = x_bol,
};
static const char *nomlar[] = { "getpid", "qosh", "bol" };

static long syscall_kirish(unsigned long raqam, long a, long b)
{
    if (raqam >= sizeof(jadval) / sizeof(jadval[0]) || !jadval[raqam])
        return -ENOSYS;                         /* bunday xizmat yo'q */
    return jadval[raqam](a, b);
}

int main(void)
{
    struct { unsigned long raqam; long a, b; } arizalar[] = {
        { 0, 0, 0 }, { 1, 20, 22 }, { 2, 100, 7 }, { 2, 5, 0 }, { 99, 1, 1 },
    };

    for (int i = 0; i < 5; i++) {
        long r = syscall_kirish(arizalar[i].raqam, arizalar[i].a, arizalar[i].b);
        const char *nom = arizalar[i].raqam < 3 ? nomlar[arizalar[i].raqam] : "?";
        if (r < 0)
            printf("syscall %2lu (%-6s) -> xato %ld (%s)\n", arizalar[i].raqam, nom, r,
                   r == -ENOSYS ? "ENOSYS: bunday xizmat yo'q" : "EINVAL: noto'g'ri argument");
        else
            printf("syscall %2lu (%-6s) -> %ld\n", arizalar[i].raqam, nom, r);
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra xizmatlar.c -o xizmatlar
$ ./xizmatlar
syscall  0 (getpid) -> 42
syscall  1 (qosh  ) -> 42
syscall  2 (bol   ) -> 14
syscall  2 (bol   ) -> xato -22 (EINVAL: noto'g'ri argument)
syscall 99 (?     ) -> xato -38 (ENOSYS: bunday xizmat yo'q)
```

Yadro xatolarni **manfiy** son bilan qaytaradi (`-ENOSYS`, `-EINVAL`). libc esa uni musbat `errno` ga
aylantirib, funksiyadan −1 qaytaradi (12-bob).

**Sinab ko'ring:** `[3] = x_kopaytir` qo'shing. MyOS'ning haqiqiy jadvalini oching:
`grep -n "SYS_" kernel/syscall/*.c | head` — xuddi shu tuzilmani taniysizmi?

## 30.1. Yadro nima qiladi — besh vazifa

1. **CPU'ni bo'lish** — jarayonlar, oqimlar, rejalashtirish (23-bob).
2. **Xotirani bo'lish va himoya** — virtual xotira (24–25-boblar).
3. **Qurilmalarni boshqarish** — drayverlar (16, 27-boblar).
4. **Fayllar va saqlash** — VFS, fayl tizimlari (27-bob).
5. **Aloqa** — pipe, signallar, soketlar, tarmoq (14-bob).

Hammasi **himoya chegarasi** orqali: user rejimi (3-halqa) faqat syscall bilan so'raydi, yadro (0-halqa)
tekshiradi va bajaradi.

## 30.2. Arxitektura turlari

### Monolit yadro

Hamma xizmatlar (scheduler, xotira, fayl tizimlari, drayverlar, tarmoq) **bitta** manzil maydonida,
yadro rejimida. Bir-birini oddiy funksiya chaqiruvi bilan chaqiradi.
- ✅ Tez: qatlamlar orasida xabar almashish yo'q.
- ❌ Bitta drayvervagi xato butun tizimni qulatadi; kod bazasi ulkan.
- **Misollar:** Linux, FreeBSD, **MyOS**, xv6.
- Linux buni **yuklanadigan modullar** bilan yumshatadi: drayverlar alohida `.ko` fayllar, lekin yuklangach
  — baribir yadroning bir qismi.

### Mikroyadro

Yadroda faqat eng zarur: manzil maydonlari, oqimlar, **xabar almashish (IPC)**, uzilishlar.
Drayverlar, fayl tizimlari, tarmoq — alohida **user rejimidagi serverlar**.
- ✅ Ishonchlilik: drayver qulasa — faqat o'sha server qayta ishga tushiriladi. Yadro kichik — tekshirish
  mumkin (seL4 — matematik isbotlangan yadro).
- ❌ Har bir amal bir nechta IPC va kontekst almashishni talab qiladi — sekinroq (zamonaviy mikroyadrolar
  buni ancha kamaytirgan).
- **Misollar:** Minix 3, seL4, QNX (avtomobillarda), L4 oilasi, Fuchsia (Zircon).

1992-yildagi mashhur Tanenbaum–Torvalds bahsi aynan shu haqda edi: Minix muallifi "monolit eskirgan"
dedi, Linux muallifi — "amalda tezlik muhim". Ikkala yondashuv ham bugun yashayapti.

### Gibrid

Mikroyadro g'oyalari + ko'p xizmatlar tezlik uchun yadro ichida. **Windows NT**, **macOS XNU** (Mach mikroyadrosi + BSD qatlami).

### Boshqalar

- **Exokernel** — yadro faqat resurslarni xavfsiz taqsimlaydi, abstraksiyalarni (fayl tizimi) dastur
  kutubxonasi yaratadi.
- **Unikernel** — bitta dastur + unga kerakli OS qismlari birga, bitta manzil maydonida (bulutda, virtual
  mashinada tez ishga tushish uchun).

## 30.3. MyOS, xv6 va Linux

| | MyOS | xv6 (MIT o'quv yadrosi) | Linux |
|---|---|---|---|
| Hajm | ~17 600 qator yadro | ~6 000 qator yadro | ~30 000 000+ qator (drayverlar bilan) |
| Arxitektura | x86-64 | RISC-V (eski versiya x86) | 20+ arxitektura |
| Xotira | buddy + slab + vmalloc, COW, demand paging | oddiy ro'yxat | buddy, SLUB, vmalloc, NUMA, huge pages, swap... |
| Scheduler | round-robin, SMP | round-robin | EEVDF (CFS vorisi), real vaqt, deadline |
| Fayl tizimlari | tmpfs, devfs, ext2 (o'qish+yozish) | o'z oddiy FS'i (jurnal bilan) | 50+ (ext4, btrfs, XFS, NFS...) |
| Drayverlar | ATA, AHCI, PS/2, serial, framebuffer | virtio disk, UART | minglab |
| Tarmoq | yo'q | yo'q | to'liq TCP/IP |
| Maqsad | o'rganish + haqiqiy apparat | o'rganish | ishlab chiqarish |

MyOS'dagi har bir tushuncha Linux'da ham bor — faqat kattaroq va murakkabroq. Bu jadvalni ko'rib
qo'rqmang: Linux'ni o'rganish — "MyOS'dagi X ning Linux versiyasi qayerda?" savolidan boshlanadi.

## 30.4. Linux manba kodining xaritasi

```bash
git clone --depth 1 https://github.com/torvalds/linux.git
```

| Papka | Nima | MyOS'dagi o'xshashi |
|---|---|---|
| `arch/x86/` | x86'ga xos: yuklash, uzilishlar, sahifalash, syscall kirishi | `kernel/arch/`, `kernel/boot/` |
| `arch/x86/entry/` | syscall va uzilish kirish kodi (assembly) | `syscall_entry.asm`, `isr.asm` |
| `init/main.c` | `start_kernel()` — ishga tushirish tartibi | `kernel/main.c` (`kmain`) |
| `kernel/` | jarayonlar, scheduler (`kernel/sched/`), signallar, `fork.c`, `exit.c` | `kernel/proc/` |
| `mm/` | xotira: `page_alloc.c` (buddy), `slub.c`, `vmalloc.c`, `memory.c` (page fault), `mmap.c` | `kernel/mm/` |
| `fs/` | VFS (`namei.c`, `open.c`, `read_write.c`), `pipe.c`, `exec.c`, fayl tizimlari (`fs/ext2/`, `fs/ext4/`) | `kernel/fs/`, `kernel/sys/elf.c` |
| `drivers/` | drayverlar (kodning ~70%): `ata/` (AHCI), `nvme/`, `usb/`, `net/`, `tty/`, `gpu/` | `kernel/drivers/` |
| `block/` | blok qatlami, I/O rejalashtirish | `kernel/fs/block.c` |
| `net/` | tarmoq steki | — |
| `include/linux/` | umumiy sarlavhalar: `list.h`, `rbtree.h`, `spinlock.h`, `sched.h` | `kernel/lib/`, `*.h` |
| `lib/` | yordamchilar: `string.c`, `sort.c` (heapsort!), `rbtree.c`, `vsprintf.c` (printk formatlash) | `kernel/lib/` |
| `ipc/` | System V IPC, message queues | — |
| `security/` | SELinux, AppArmor, LSM | — |
| `tools/`, `scripts/` | `checkpatch.pl`, perf, testlar | `tools/` |
| `Documentation/` | hujjatlar (inglizcha) | `docs/` |

### Kod bo'ylab yurish usuli

"`read()` syscall qayerda?" → `grep -rn "SYSCALL_DEFINE3(read" fs/` → `fs/read_write.c` → `ksys_read`
→ `vfs_read` → `file->f_op->read_iter(...)` — 7-bobdagi funksiya jadvali! Brauzerda: **elixir.bootlin.com** —
Linux kodini istalgan belgini bosib, uning ta'rifi va hamma ishlatilishlariga sakrash bilan o'qish sayti
(kod inglizcha, lekin sayt interfeysi oddiy — belgilarni bosish yetarli).

Solishtirib o'qish uchun juftliklar:
- `kernel/mm/pmm.c` (MyOS buddy) ↔ `mm/page_alloc.c` (`__free_one_page` — juftni birlashtirish)
- `kernel/fs/pipe.c` ↔ `fs/pipe.c` (`pipe_read`, `pipe_write` — xuddi shu nomlar!)
- `kernel/lib/list.h` ↔ `include/linux/list.h`
- `kernel/proc/signal.c` ↔ `arch/x86/kernel/signal.c` (`setup_rt_frame`) va `kernel/signal.c`

## 30.5. Linux kod uslubi (qisqacha)

`Documentation/process/coding-style.rst` — asosiylari (MyOS ham ularga yaqin):
- chekinish — **TAB** (8 belgi), qator uzunligi ~80–100;
- funksiya ochuvchi `{` yangi qatorda, `if/for` niki — shu qatorda;
- nomlar — `kichik_harf_pastki_chiziq`; global nomlar tavsifiy, lokal — qisqa (`i`, `tmp`);
- `typedef` structlar uchun ishlatilmaydi (9-bob);
- funksiyalar qisqa, bitta ish qiladi; chuqur ichma-ichlik yomon;
- xatodan keyin tozalash — `goto` (4-bob);
- izohlar **nima** va **nega** haqida, **qanday** — kodning o'zidan ko'rinishi kerak.

`scripts/checkpatch.pl` — uslubni avtomatik tekshiradi.

## 30.6. Yadro moduli — Linux'dagi birinchi kodingiz

```c
#include <linux/module.h>
#include <linux/init.h>

static int __init salom_init(void)
{
    pr_info("salom: yadroga xush kelibsiz!\n");     /* printk - kprintf'ning Linux varianti */
    return 0;
}

static void __exit salom_exit(void)
{
    pr_info("salom: xayr!\n");
}

module_init(salom_init);
module_exit(salom_exit);
MODULE_LICENSE("GPL");
```

```make
obj-m += salom.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
```

```bash
make && sudo insmod salom.ko && sudo dmesg | tail -1 && sudo rmmod salom
```

(Ehtiyot: modul yadro ichida ishlaydi — undagi xato butun tizimni qulatishi mumkin. Birinchi tajribalarni
virtual mashinada yoki QEMU'da qiling.)

## 30.7. Linux'ga hissa qo'shish jarayoni

1. **Yig'ish va ishga tushirish:** `make defconfig && make -j$(nproc)`, QEMU'da yuklash.
2. **Kichik ish topish:** `drivers/staging/` (sifati past, tuzatishga muhtoj kod; har bir papkada `TODO`
   fayli), `checkpatch.pl` ogohlantirishlari, hujjatlardagi xatolar.
3. **Patch tayyorlash:** `git commit -s` (Signed-off-by — kodni yuborishga huquqingiz borligi haqida
   tasdiq), aniq sarlavha: `staging: rtl8723bs: fix spelling mistake`.
4. **Kimga yuborish:** `scripts/get_maintainer.pl 0001-*.patch` — qaysi mas'ul shaxs va ro'yxatga.
5. **Yuborish:** `git send-email` (Linux'da patch'lar elektron pochta orqali, GitHub orqali emas!).
6. **Sharhlarga javob:** mas'ul shaxslar tanqid qiladi — bu normal. Tuzatib `v2` yuborasiz.

Til masalasi: patch sarlavhasi va xat — qisqa inglizcha. Namunalar juda bir xil (`fix`, `remove unused`,
`add missing check`) — 31-bobdagi lug'at va mavjud commitlardan (`git log --oneline drivers/staging`)
naqsh olish yetarli.

## 30.8. Yadro dasturchisining umumiy yo'li

```text
C va tizimlar (darslik)  ->  MyOS ichida (labs)  ->  o'z yadroingiz (QOLLANMA 11)
      ->  Linux: yig'ish, modul, staging patch'lar  ->  bitta quyi tizimda chuqurlashish
          (xotira / fayl tizimlari / tarmoq / drayverlar / virtualizatsiya / xavfsizlik)
```

## 30.9. O'zingizni tekshiring

1. Monolit va mikroyadro farqi, har birining afzalligi?
2. Linux monolit bo'lsa, modullar nima uchun kerak?
3. MyOS'dagi buddy allocator'ning Linux'dagi o'xshashi qaysi faylda?
4. Linux'da patch qanday yuboriladi?
5. `drivers/staging` nima?

<details><summary>Javoblar</summary>

1. Monolit — hamma xizmat yadroda (tez); mikroyadro — xizmatlar user serverlarida (ishonchli, kichik yadro).
2. Drayverlarni kerak bo'lganda yuklash/olib tashlash (yadro hajmi, moslashuvchanlik) — lekin yuklangach ular yadro ichida.
3. `mm/page_alloc.c`.
4. `git format-patch` / `git send-email` bilan, `get_maintainer.pl` ko'rsatgan manzillarga, elektron pochta orqali.
5. Asosiy yadroga to'liq tayyor bo'lmagan, tozalashga muhtoj drayverlar — boshlovchilar uchun hissa qo'shish joyi.
</details>

## 30.10. Mashq

- Linux manbasini yuklab olib, 30.4-dagi "solishtirib o'qish" juftliklaridan bittasini tanlang: MyOS
  funksiyasini va Linux funksiyasini yonma-yon o'qing, farqlarini daftarga yozing.
- Virtual mashinada 30.6-dagi modulni yig'ib yuklang.

Keyingi bob: [31-bob. Lug'at: ingliz texnik atamalari](31-lugat.md)
