# 30-bob. Yadro arxitekturasi va Linux'ga yo'l

> **Bu bobda nima o'rganasiz:** yadro **nima qilishini** (besh vazifa); yadro turlarini (monolit, mikroyadro, gibrid) va ularning farqini; syscall jadvali qanday ishlashini; yuklanadigan modul nima ekanini; MyOS, xv6 va Linux'ni solishtirishni;
> Linux manba kodining xaritasini; Linux'ga o'zgartirish (patch) yuborish tartibini. Bu — MyOS'dan keyin haqiqiy katta yadroga o'tish uchun xarita. (Odatda "Linux Kernel Development" va "Understanding the Linux Kernel" kitoblaridan o'rganiladigan umumiy manzara.)
> **Oldindan nima kerak:** 14-, 23-, 24-, 27-boblar (syscall, jarayonlar, xotira, fayl tizimlari).   **Vaqt:** 5–7 soat.

> **To'liq ishlaydigan misol:** [misollar/30_yadro_moduli/](misollar/30_yadro_moduli/salom_modul.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Siz endi yadroning har bir bo'lagini (xotira, jarayonlar, fayl tizimi, qulflar) alohida bilasiz. Bu bobda ularni **bitta rasmga** yig'amiz: yadro **butun** nima qiladi, uni qanday **tuzish** mumkin (arxitektura tanlovi) va katta haqiqiy yadro
(Linux) ning kodini **qayerdan** o'qish kerak.

**Hayotdan misol: yadro — shahar hokimiyati.**

| Hokimiyat nima qiladi | Yadro nima qiladi |
|---|---|
| yer uchastkalarini va yo'ldagi navbatni taqsimlaydi | **resurslarni taqsimlash** — xotira, CPU vaqti |
| bir fuqaro boshqasining uyiga kira olmaydi | **himoya** — jarayonlar izolyatsiyasi |
| davlat xizmatlari markazi: ariza berasiz, bajarishadi | **xizmatlar** — syscall'lar |
| yo'llar, suv quvurlari | **umumiy infratuzilma** — drayverlar, fayl tizimlari, tarmoq |
| fuqaro elektr stantsiyasi qanday ishlashini bilmaydi, rozetkadan foydalanadi | **abstraksiya** — dastur diskning turini bilmaydi, faqat `read` chaqiradi |

## 30.1. Yadro nima qiladi — besh vazifa

| # | Vazifa | Bobda |
|---|---|---|
| 1 | **CPU'ni bo'lish** — jarayonlar, oqimlar, rejalashtirish | 23 |
| 2 | **Xotirani bo'lish va himoya** — virtual xotira | 24, 25 |
| 3 | **Qurilmalarni boshqarish** — drayverlar | 16, 27 |
| 4 | **Fayllar va saqlash** — VFS, fayl tizimlari | 27 |
| 5 | **Aloqa** — pipe, signallar, soketlar, tarmoq | 14 |

Hammasi **himoya chegarasi** orqali: **user rejimi** (3-halqa) faqat syscall bilan so'raydi, **yadro** (0-halqa) tekshiradi va bajaradi.

### Xizmatlar jadvali (syscall jadvali)

**Oddiy qilib aytganda:** dastur yadrodan xizmat so'raganda **raqam** beradi ("7-xizmat"). Yadro shu raqam bo'yicha **funksiyalar jadvalidan** kerakli ishlovchini topib chaqiradi. Bu 7-bobdagi funksiya ko'rsatkichlari massivining
bevosita qo'llanishi.

**Bu dastur nima qiladi (umumiy):** uchta "xizmat" (`getpid`, `qosh`, `bol`) funksiyalarini jadvalga joylaydi va "arizalar" (raqam + argumentlar) ni qabul qiluvchi `syscall_kirish` funksiyasini yozadi. Noma'lum raqamga yadro xato qaytaradi.

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

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `typedef long (*ishlovchi)(long a, long b)` | "ikkita `long` oladigan, `long` qaytaradigan funksiya" turi (funksiya ko'rsatkichi, 7-bob) |
| `jadval[] = { [0] = x_getpid, ... }` | **raqam → funksiya** jadvali. `[0] =` — "0-indeksga shuni qo'y" (designated initializer, 9-bob) |
| `syscall_kirish(raqam, a, b)` | yadroning "qabulxonasi": raqamni tekshiradi, jadvaldan funksiyani topib, chaqiradi |
| `raqam >= sizeof(jadval)/sizeof(jadval[0]) \|\| !jadval[raqam]` | raqam jadval chegarasidan tashqarida yoki shu katak bo'sh → `-ENOSYS` |
| `return -EINVAL` (`x_bol` ichida) | argument noto'g'ri (0 ga bo'lish) → xato kodi |

**Nima ko'rdik:** 5 ariza: `getpid` → 42; `qosh(20, 22)` → 42; `bol(100, 7)` → 14; `bol(5, 0)` → `-22` (**EINVAL**); `99` raqamli xizmat yo'q → `-38` (**ENOSYS**).

Yadro xatolarni **manfiy** son bilan qaytaradi (`-ENOSYS`, `-EINVAL`): musbat sonlar — muvaffaqiyatli natija. libc esa uni musbat `errno` ga aylantirib, funksiyadan `-1` qaytaradi (12-bob).

**Linux va MyOS'da:** Linux — xuddi shunday jadval: `sys_call_table[__NR_xxx] = sys_xxx`. MyOS esa `kernel/sys/syscall.c` da katta `switch (nr) { case SYS_GETPID: ... }` ishlatadi — g'oya bir xil ("raqam → ishlovchi"), faqat yozilishi boshqa.
Raqamlar ro'yxati — `include/myos/abi.h`.

> **Eslab qoling:** syscall = **raqam** + argumentlar. Yadro jadval (yoki `switch`) orqali ishlovchini topadi; xato — manfiy son (`-ENOSYS` = "bunday xizmat yo'q").

## 30.2. Arxitektura turlari: yadro qanday tuzilishi mumkin

**Oddiy qilib aytganda:** yadro ko'p qismdan iborat (scheduler, xotira, fayl tizimlari, drayverlar, tarmoq). Savol: **bularning hammasi bitta katta dasturdami yoki alohida dasturlarmi?** Javobga qarab yadro turi o'zgaradi.

| Tur | G'oya | Hayotdan misol |
|---|---|---|
| **Monolit** | hammasi yadro rejimida, bitta manzil maydonida | bitta katta vazirlik binosi: bo'limlar bir binoda, gaplashish tez; bittasida yong'in chiqsa — butun bino yonadi |
| **Mikroyadro** | yadroda faqat eng zarur; qolgani — alohida user serverlari, xabar bilan gaplashadi | alohida binolar, xat orqali aloqa: bittasi yonsa, boshqalari ishlayveradi; lekin xat sekinroq |
| **Gibrid** | mikroyadro g'oyasi + ko'p xizmat tezlik uchun yadroda | aralash |

### Monolit va mikroyadro: farqni "o'lchaymiz"

**Bu dastur nima qiladi (umumiy):** bitta `read()` chaqiruvini ikki arxitekturada modellaydi va ikki narsani ko'rsatadi: (1) **tezlik** — bitta `read` uchun nechta rejim/kontekst almashish kerak; (2) **ishonchlilik** — disk drayverida xato
(3-blokni o'qishda buziladi) bo'lsa nima bo'ladi.

```c
/* monolit_mikro.c - bitta read() ikki arxitekturada: o'tishlar soni va drayver xatosi */
#include <stdio.h>

static long almashish;                          /* rejim/kontekst almashishlar soni */
static int tizim_qulagan;                       /* monolitda drayver xatosi shu holatga olib keladi */

/* --- eng pastki qatlam: disk drayveri --- */
static int disk_oqi(int blok, int *xato)
{
    if (blok == 3) {                            /* drayverda XATO bor: 3-blokda buziladi */
        *xato = 1;
        return -1;
    }
    *xato = 0;
    return blok * 10;                           /* "blok mazmuni" */
}

static int fs_oqi(int blok, int *xato)          /* fayl tizimi qatlami */
{
    return disk_oqi(blok, xato);
}

/* --- MONOLIT: hammasi yadro ichida, qatlamlar oddiy funksiya chaqiruvi --- */
static int monolit_read(int blok)
{
    if (tizim_qulagan)
        return -1000;
    almashish++;                                /* user -> yadro (syscall) */
    int xato;
    int r = fs_oqi(blok, &xato);                /* yadro ichida: o'tish yo'q */
    if (xato)
        tizim_qulagan = 1;                      /* yadro xotirasida xato = butun tizim qulaydi (panic) */
    almashish++;                                /* yadro -> user */
    return r;
}

/* --- MIKROYADRO: har qatlam alohida jarayon (server), faqat xabar bilan gaplashadi --- */
struct xabar {
    int blok, natija;
};

typedef void (*server)(struct xabar *);

static void ipc_chaqir(server s, struct xabar *m)
{
    almashish += 2;                             /* yuboruvchi -> yadro -> qabul qiluvchi server */
    s(m);
    almashish += 2;                             /* javob: server -> yadro -> yuboruvchi */
}

static int disk_qayta_ishga_tushdi;

static void disk_server(struct xabar *m)
{
    int xato;
    int r = disk_oqi(m->blok, &xato);
    if (xato) {
        disk_qayta_ishga_tushdi++;              /* faqat shu SERVER o'ladi va qayta ishga tushiriladi */
        m->natija = -5;                         /* EIO */
    } else {
        m->natija = r;
    }
}

static void fs_server(struct xabar *m)
{
    struct xabar d = { m->blok, 0 };
    ipc_chaqir(disk_server, &d);                /* fayl serveri disk serveridan so'raydi */
    m->natija = d.natija;
}

static int mikro_read(int blok)
{
    struct xabar m = { blok, 0 };
    ipc_chaqir(fs_server, &m);                  /* dastur fayl serveridan so'raydi */
    return m.natija;
}

int main(void)
{
    printf("1) Bitta read() uchun rejim/kontekst almashishlar soni:\n");
    almashish = 0;
    monolit_read(1);
    long m1 = almashish;
    almashish = 0;
    mikro_read(1);
    long m2 = almashish;
    printf("   monolit:    %ld ta\n   mikroyadro: %ld ta\n", m1, m2);
    printf("   1000 ta read: monolit %ld, mikroyadro %ld almashish (%ld marta ko'p)\n\n", m1 * 1000, m2 * 1000, m2 / m1);

    printf("2) Drayverda xato bor (3-blokni o'qishda buziladi); bloklar 1..5 o'qiladi:\n");
    printf("   blok | monolit                    | mikroyadro\n");
    for (int blok = 1; blok <= 5; blok++) {
        int a = monolit_read(blok), b = mikro_read(blok);
        char sa[40], sb[40];
        snprintf(sa, sizeof(sa), a == -1000 ? "tizim ishlamayapti" : a < 0 ? "PANIC: butun tizim qulaydi" : "%d", a);
        snprintf(sb, sizeof(sb), b < 0 ? "xato (disk serveri qayta ishga tushdi)" : "%d", b);
        printf("   %4d | %-26s | %s\n", blok, sa, sb);
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 monolit_mikro.c -o monolit_mikro
$ ./monolit_mikro
1) Bitta read() uchun rejim/kontekst almashishlar soni:
   monolit:    2 ta
   mikroyadro: 8 ta
   1000 ta read: monolit 2000, mikroyadro 8000 almashish (4 marta ko'p)

2) Drayverda xato bor (3-blokni o'qishda buziladi); bloklar 1..5 o'qiladi:
   blok | monolit                    | mikroyadro
      1 | 10                         | 10
      2 | 20                         | 20
      3 | PANIC: butun tizim qulaydi | xato (disk serveri qayta ishga tushdi)
      4 | tizim ishlamayapti         | 40
      5 | tizim ishlamayapti         | 50
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `disk_oqi`, `fs_oqi` | qatlamlar: fayl tizimi disk drayverini chaqiradi; `disk_oqi` 3-blokda ataylab "buziladi" |
| `almashish` | rejim/kontekst almashishlar sanagichi |
| `monolit_read` | `almashish++` (user → yadro), qatlamlar **oddiy funksiya chaqiruvi** (almashish yo'q), `almashish++` (yadro → user) = **2** |
| `ipc_chaqir(server, xabar)` | xabar yuborish: `+2` (yuboruvchi → yadro → server), `s(m)`, `+2` (javob qaytishi) |
| `mikro_read` | dastur → fayl serveri → disk serveri (ikki ichma-ich IPC) = 2×4 = **8** |
| `tizim_qulagan` | monolitda drayver xatosi yadro xotirasini buzadi → butun tizim to'xtaydi (panic) |
| `disk_qayta_ishga_tushdi` | mikroyadroda faqat **shu server** o'ladi va qayta yoqiladi |

**Nima ko'rdik:**

| | Monolit | Mikroyadro |
|---|---|---|
| Bitta `read` uchun almashishlar | **2** | **8** (4 marta ko'p → sekinroq) |
| Drayver xatosi (3-blok) | **PANIC** — 4-, 5-bloklar ham o'qilmaydi, butun tizim o'ldi | faqat 3-blok xato berdi, disk serveri qayta ishga tushdi, **4-, 5-bloklar normal** |

**Afzallik/kamchilik:**

- **Monolit:** ✅ tez (qatlamlar orasida xabar yo'q); ❌ bitta drayver xatosi butun tizimni qulatadi; kod bazasi ulkan. **Misollar:** Linux, FreeBSD, **MyOS**, xv6. Linux buni **yuklanadigan modullar** bilan yumshatadi (30.6).
- **Mikroyadro:** ✅ ishonchli (drayver qulasa — faqat o'sha server qayta yoqiladi), yadro kichik — tekshirish mumkin (seL4 — matematik isbotlangan yadro); ❌ har amal bir nechta IPC va kontekst almashish (zamonaviy mikroyadrolar buni ancha kamaytirgan).
  **Misollar:** Minix 3, seL4, QNX (avtomobillarda), L4 oilasi, Fuchsia (Zircon).
- **Gibrid:** **Windows NT**, **macOS XNU** (Mach mikroyadrosi + BSD qatlami).

1992-yildagi mashhur Tanenbaum–Torvalds bahsi aynan shu haqda edi: Minix muallifi "monolit eskirgan" dedi, Linux muallifi — "amalda tezlik muhim". Ikkala yondashuv ham bugun yashayapti.

**Boshqalar:** **Exokernel** — yadro faqat resurslarni xavfsiz taqsimlaydi, abstraksiyalarni (fayl tizimi) dastur kutubxonasi yaratadi. **Unikernel** — bitta dastur + unga kerakli OS qismlari birga, bitta manzil maydonida (bulutda tez ishga tushish uchun).

> **Eslab qoling:** monolit — tez, lekin bitta xato hammani o'ldiradi (Linux, MyOS); mikroyadro — ishonchli, lekin sekinroq (seL4, QNX). Bu **tezlik ↔ ishonchlilik** kelishuvi.

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

MyOS'dagi har bir tushuncha Linux'da ham bor — faqat kattaroq va murakkabroq. Bu jadvalni ko'rib qo'rqmang: Linux'ni o'rganish — **"MyOS'dagi X ning Linux versiyasi qayerda?"** savolidan boshlanadi.

## 30.4. Linux manba kodining xaritasi

```bash
git clone --depth 1 https://github.com/torvalds/linux.git
```

| Papka | Nima | MyOS'dagi o'xshashi |
|---|---|---|
| `arch/x86/` | x86'ga xos: yuklash, uzilishlar, sahifalash, syscall kirishi | `kernel/arch/`, `kernel/boot/` |
| `arch/x86/entry/` | syscall va uzilish kirish kodi (assembly) | `kernel/arch/syscall_entry.asm`, `isr.asm` |
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

"`read()` syscall qayerda?" → `grep -rn "SYSCALL_DEFINE3(read" fs/` → `fs/read_write.c` → `ksys_read` → `vfs_read` → `file->f_op->read_iter(...)` — 7-bobdagi funksiya jadvali!

MyOS'da shu savol: `grep -n "SYS_READ" ~/C_loyha/kernel/sys/*.c` → `syscall.c` dagi `switch` → fayl tizimiga yo'naltiruvchi funksiya (`sys_fs.c`).

Brauzerda: **elixir.bootlin.com** — Linux kodini istalgan belgini bosib, uning ta'rifi va hamma ishlatilishlariga sakrash bilan o'qish sayti (kod inglizcha, lekin sayt interfeysi oddiy — belgilarni bosish yetarli).

Solishtirib o'qish uchun juftliklar:

- `kernel/mm/pmm.c` (MyOS buddy) ↔ `mm/page_alloc.c` (`__free_one_page` — juftni birlashtirish; 25-bob)
- `kernel/fs/pipe.c` ↔ `fs/pipe.c` (`pipe_read`, `pipe_write` — xuddi shu nomlar!)
- `kernel/lib/list.h` ↔ `include/linux/list.h`
- `kernel/proc/signal.c` ↔ `arch/x86/kernel/signal.c` (`setup_rt_frame`) va `kernel/signal.c`

## 30.5. Linux kod uslubi (qisqacha)

`Documentation/process/coding-style.rst` — asosiylari (MyOS ham ularga yaqin):

| Qoida | Izoh |
|---|---|
| chekinish — **TAB** (8 belgi), qator ~80–100 belgi | bir xil ko'rinish |
| funksiya `{` yangi qatorda, `if/for` niki — shu qatorda | |
| nomlar — `kichik_harf_pastki_chiziq`; global nomlar tavsifiy, lokal — qisqa (`i`, `tmp`) | |
| `typedef` structlar uchun ishlatilmaydi (9-bob) | `struct list_head` yozasiz, `list_head_t` emas |
| funksiyalar qisqa, bitta ish qiladi; chuqur ichma-ichlik yomon | |
| xatodan keyin tozalash — `goto` (4-bob) | |
| izohlar **nima** va **nega** haqida, **qanday** — kodning o'zidan ko'rinishi kerak | |

`scripts/checkpatch.pl` — uslubni avtomatik tekshiradi.

## 30.6. Yadro moduli — Linux'dagi birinchi kodingiz

**Oddiy qilib aytganda:** modul — yadroga **ish vaqtida** qo'shiladigan va olib tashlanadigan kod bo'lagi. **Hayotdan misol:** binoga vaqtinchalik qo'shiladigan bo'lim: yangi xizmat kerak bo'lsa, butun binoni qayta qurmaysiz — bo'sh xonaga yangi bo'lim
ko'chib kiradi (`insmod`) va keraksiz bo'lganda chiqib ketadi (`rmmod`). Linux drayverlarining ko'pi modul.

Haqiqiy Linux modulida ikkita funksiya: `init` (yuklanganda) va `exit` (olib tashlanganda):

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

(Ehtiyot: modul yadro ichida ishlaydi — undagi xato butun tizimni qulatishi mumkin. Birinchi tajribalarni virtual mashinada yoki QEMU'da qiling.)

### Modul mexanizmi nima qiladi — oddiy dasturda modellash

Haqiqiy modulni bu yerda yuklab bo'lmaydi (yadro sarlavhalari va ruxsat kerak), lekin **g'oyani** oddiy dasturda ko'rish mumkin.

**Bu dastur nima qiladi (umumiy):** "yadro" qurilmalar jadvalini saqlaydi (`/dev`). `insmod` modulning `init` funksiyasini chaqiradi — u yadro jadvaliga qurilma **qo'shadi**; `rmmod` `exit` ni chaqiradi — u qurilmani **olib tashlaydi**. Dastur
modul yuklanishidan oldin, yuklangandan keyin va olib tashlangandan keyin `read` natijasini ko'rsatadi; `init` xato qaytarsa modul yuklanmasligini ham.

```c
/* modul_misol.c - yuklanadigan modul g'oyasi: insmod/rmmod yadro jadvaliga drayver qo'shadi/oladi */
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* ---------- "YADRO" tomoni ---------- */
struct qurilma {
    const char *nom;
    long (*oqi)(void);
};
static struct qurilma qurilmalar[4];            /* yadroning qurilmalar jadvali (/dev) */

static int qurilma_qosh(const char *nom, long (*oqi)(void))
{
    for (int i = 0; i < 4; i++)
        if (qurilmalar[i].nom && strcmp(qurilmalar[i].nom, nom) == 0)
            return -EEXIST;
    for (int i = 0; i < 4; i++)
        if (!qurilmalar[i].nom) {
            qurilmalar[i] = (struct qurilma){ nom, oqi };
            return 0;
        }
    return -ENOMEM;
}

static void qurilma_ol(const char *nom)
{
    for (int i = 0; i < 4; i++)
        if (qurilmalar[i].nom && strcmp(qurilmalar[i].nom, nom) == 0)
            qurilmalar[i].nom = NULL;
}

static long yadro_read(const char *nom)         /* syscall: read("/dev/nom") */
{
    for (int i = 0; i < 4; i++)
        if (qurilmalar[i].nom && strcmp(qurilmalar[i].nom, nom) == 0)
            return qurilmalar[i].oqi();
    return -ENODEV;
}

struct modul {
    const char *nom;
    int (*init)(void);                          /* module_init: yuklanganda */
    void (*exit)(void);                         /* module_exit: olib tashlanganda */
    int yuklangan;
};

static int insmod(struct modul *m)
{
    if (m->yuklangan)
        return -EEXIST;
    int r = m->init();
    if (r < 0) {
        printf("  insmod %s: xato %d, modul YUKLANMADI\n", m->nom, r);
        return r;
    }
    m->yuklangan = 1;
    return 0;
}

static void rmmod(struct modul *m)
{
    if (m->yuklangan) {
        m->exit();
        m->yuklangan = 0;
    }
}

/* ---------- MODUL tomoni (alohida "fayl" bo'lishi mumkin edi) ---------- */
static long tasodif_oqi(void)
{
    static long x = 12345;
    x = (x * 1103515245 + 12345) & 0x7fffffff;
    return x % 100;
}

static int tasodif_init(void)
{
    printf("  [dmesg] tasodif: yadroga xush kelibsiz!\n");
    return qurilma_qosh("tasodif", tasodif_oqi);
}

static void tasodif_exit(void)
{
    printf("  [dmesg] tasodif: xayr!\n");
    qurilma_ol("tasodif");
}

static int buzuq_init(void)
{
    return -ENOMEM;                             /* init xato qaytardi - modul yuklanmaydi */
}

static void buzuq_exit(void) { }

static struct modul tasodif = { "tasodif", tasodif_init, tasodif_exit, 0 };
static struct modul buzuq = { "buzuq", buzuq_init, buzuq_exit, 0 };

int main(void)
{
    printf("insmod'dan oldin: read(tasodif) = %ld  (ENODEV = %d: bunday qurilma yo'q)\n", yadro_read("tasodif"), -ENODEV);

    printf(">> insmod tasodif\n");
    insmod(&tasodif);
    long a = yadro_read("tasodif");             /* alohida chaqiramiz: printf argumentlari tartibi noaniq */
    long b = yadro_read("tasodif");
    long c = yadro_read("tasodif");
    printf("read(tasodif) uch marta: %ld %ld %ld\n", a, b, c);

    printf(">> insmod tasodif   (ikkinchi marta)\n  natija: %d (EEXIST = %d: allaqachon yuklangan)\n", insmod(&tasodif), -EEXIST);

    printf(">> rmmod tasodif\n");
    rmmod(&tasodif);
    printf("rmmod'dan keyin: read(tasodif) = %ld  (yana ENODEV)\n", yadro_read("tasodif"));

    printf(">> insmod buzuq\n");
    insmod(&buzuq);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 modul_misol.c -o modul_misol
$ ./modul_misol
insmod'dan oldin: read(tasodif) = -19  (ENODEV = -19: bunday qurilma yo'q)
>> insmod tasodif
  [dmesg] tasodif: yadroga xush kelibsiz!
read(tasodif) uch marta: 6 75 24
>> insmod tasodif   (ikkinchi marta)
  natija: -17 (EEXIST = -17: allaqachon yuklangan)
>> rmmod tasodif
  [dmesg] tasodif: xayr!
rmmod'dan keyin: read(tasodif) = -19  (yana ENODEV)
>> insmod buzuq
  insmod buzuq: xato -12, modul YUKLANMADI
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `qurilmalar[4]` | yadroning qurilmalar jadvali: nom → `oqi` funksiyasi |
| `qurilma_qosh` / `qurilma_ol` | yadro ichki interfeysi: modul shu orqali o'zini yadroga **ro'yxatdan o'tkazadi** va ketayotganda **olib tashlaydi** |
| `yadro_read(nom)` | `read("/dev/nom")` syscall'i: jadvaldan topadi, topmasa `-ENODEV` ("bunday qurilma yo'q") |
| `struct modul { init, exit }` | modul — ikki funksiya (`module_init` / `module_exit`) |
| `insmod` | `init` ni chaqiradi; u **manfiy** son qaytarsa — modul **yuklanmaydi**; allaqachon yuklangan bo'lsa `-EEXIST` |
| `rmmod` | `exit` ni chaqiradi (tozalash) |

**Nima ko'rdik:** modul yuklanmasdan `read` → `-19` (ENODEV). `insmod` dan keyin `read` ishlaydi (modul qo'shgan funksiya chaqirildi). Ikkinchi `insmod` → `-17` (EEXIST). `rmmod` dan keyin yana ENODEV — modul ketdi, yadro
yo'qotilgan funksiyani chaqirmaydi. `buzuq` modulning `init` i `-ENOMEM` qaytardi → **yuklanmadi** (haqiqiy `insmod` ham shuni qiladi: init xatosi → "Cannot allocate memory", modul `lsmod` da yo'q).

> **Eslab qoling:** modul = `init` + `exit`; yadroga **ish vaqtida** qo'shiladi; `init` xato qaytarsa yuklanmaydi; yuklangach modul yadro ichida ishlaydi — xatosi butun tizimga ta'sir qiladi (monolit!).

## 30.7. Linux'ga hissa qo'shish jarayoni

**Oddiy qilib aytganda:** Linux'ga kod qo'shish — **qonun loyihasi** taklif qilishga o'xshaydi: siz patch yozasiz, u pochta ro'yxatida muhokama qilinadi, mutaxassislar (maintainer) tanqid qiladi, siz tuzatasiz (v2, v3...), oxirida qabul
qilinadi. Jarayon sekin, lekin har bir qator ko'p ko'zdan o'tadi.

| # | Qadam | Qanday |
|---|---|---|
| 1 | **Yig'ish va ishga tushirish** | `make defconfig && make -j$(nproc)`, QEMU'da yuklash |
| 2 | **Kichik ish topish** | `drivers/staging/` (sifati past, tuzatishga muhtoj kod; har papkada `TODO` fayli), `checkpatch.pl` ogohlantirishlari, hujjatlardagi xatolar |
| 3 | **Patch tayyorlash** | `git commit -s` (Signed-off-by — kodni yuborishga huquqingiz borligi tasdig'i), aniq sarlavha: `staging: rtl8723bs: fix spelling mistake` |
| 4 | **Kimga yuborish** | `scripts/get_maintainer.pl 0001-*.patch` — qaysi mas'ul shaxs va ro'yxatga |
| 5 | **Yuborish** | `git send-email` (Linux'da patch'lar elektron pochta orqali, GitHub orqali emas!) |
| 6 | **Sharhlarga javob** | mas'ul shaxslar tanqid qiladi — bu normal. Tuzatib `v2` yuborasiz |

### Patch qanday ko'rinadi — real tajriba

**Bu dastur nima qiladi (umumiy):** bu — dastur emas, **git buyruqlari ketma-ketligi**: kichik repozitoriy yaratamiz, README'dagi imloviy xatoni (`recieve` → `receive`) tuzatamiz, `-s` bilan commit qilamiz va `git format-patch` bilan **haqiqiy
patch** hosil qilamiz — Linux'ga elektron pochta bilan aynan shu shaklda yuboriladi.

```console
$ rm -rf demo_repo && git init -q demo_repo && cd demo_repo && git config user.name "Ali Valiyev" && git config user.email ali@example.com && printf 'recieve data\n' > README && git add README && git commit -q -m "initial"
$ cd demo_repo && printf 'receive data\n' > README && git commit -q -a -s -m "staging: demo: fix spelling mistake in README" && git format-patch -1 --stdout | grep -E '^(Subject|Signed-off-by|-rec|\+rec)'
Subject: [PATCH] staging: demo: fix spelling mistake in README
Signed-off-by: Ali Valiyev <ali@example.com>
-recieve data
+receive data
```

**Qanday o'qiladi:**

| Satr | Ma'nosi |
|---|---|
| `Subject: [PATCH] staging: demo: fix spelling mistake in README` | sarlavha: **quyi tizim** (`staging: demo:`) + qisqa inglizcha tavsif (`fix ...`) |
| `Signed-off-by: Ali Valiyev <ali@example.com>` | `commit -s` qo'shdi: "bu kodni yuborishga huquqim bor" (DCO tasdig'i) |
| `-recieve data` / `+receive data` | o'zgarish: `-` — olib tashlangan, `+` — qo'shilgan satr |

Til masalasi: patch sarlavhasi va xat — qisqa inglizcha. Namunalar juda bir xil (`fix`, `remove unused`, `add missing check`) — 31-bobdagi lug'at va mavjud commitlardan (`git log --oneline drivers/staging`) naqsh olish yetarli.

## 30.8. Yadro dasturchisining umumiy yo'li

```text
C va tizimlar (darslik)  ->  MyOS ichida (labs)  ->  o'z yadroingiz (QOLLANMA 11)
      ->  Linux: yig'ish, modul, staging patch'lar  ->  bitta quyi tizimda chuqurlashish
          (xotira / fayl tizimlari / tarmoq / drayverlar / virtualizatsiya / xavfsizlik)
```

## Hayotdan misol va to'liq dastur

**Syscall jadvali.** Bobning to'liq dasturi — 30.1 dagi `xizmatlar.c`: yadroning "xizmatlar markazi". Raqam → ishlovchi funksiya jadvali; noma'lum raqamga `-ENOSYS`, noto'g'ri argumentga `-EINVAL`. Bobdagi qolgan dasturlar shu tizimning
boshqa tomonlarini ko'rsatadi:

| Dastur | Nimani ko'rsatadi |
|---|---|
| `xizmatlar.c` | yadroga xizmat so'rash mexanizmi (syscall jadvali) |
| `monolit_mikro.c` | arxitektura kelishuvi: tezlik ↔ ishonchlilik |
| `modul_misol.c` | yuklanadigan modul: `init`/`exit` va yadro jadvali |

**Sinab ko'ring:** `xizmatlar.c` ga `[3] = x_kopaytir` qo'shing. MyOS'ning haqiqiy `switch`'ini oching: `grep -n "SYS_" ~/C_loyha/kernel/sys/syscall.c | head` — xuddi shu g'oyani taniysizmi?

<!-- katta:boshi -->
## Katta loyiha: foydalanuvchi maydonidagi "mini-yadro" (jarayonlar, syscall, kontekst almashish)

**Umumiy fikr.** Yadroning asosiy ishi — **bitta CPU da ko'p jarayonni** ishlatish: har biriga "o'zimda ishlayapman" degan tuyg'u berish. Buning uchun yadro **ishlayotgan jarayonning registrlarini saqlaydi**, boshqasiniki **tiklaydi** (*kontekst almashish*). Bu bosqichda shuni **oddiy dastur ichida** qilamiz: `ucontext` (`getcontext`, `makecontext`, `swapcontext`) — registrlar va stekni saqlash/tiklash uchun tayyor funksiyalar. Natijada **haqiqiy** rejalashtiruvchi, **jarayonlar jadvali**, **syscall jadvali**, **uxlash/kutish** — yadroning asosiy qismlari, faqat "protsessor emas, funksiya chaqiruvi" bilan.

**Hayotiy o'xshatish:** bir ishchi ikki ish bilan shug'ullanadi. Birinchisini to'xtatayotganda **qaysi joyga yetganini yozib qo'yadi** (kontekst), ikkinchisiga o'tadi. Qaytganda yozuvni o'qib, aynan o'sha joydan davom etadi.

### Arxitektura

```text
  +--------------------------------------------------------+
  |  YADRO  (yadro_kirish: rejalashtiruvchi sikl)            |
  |    jarayonlar jadvali: jadval[MAKS]                      |
  |    syscall jadvali:    jadval_syscall[]                  |
  +----------------------^---------------------------------+
            swapcontext  |  trap()  (jarayon -> yadro)
            (yadro -> jarayon)
  +--------+  +--------+  +--------+  +--------+
  |  init  |  |   A    |  |   B    |  |   C    |   <- "user rejimi": faqat sys_*() chaqiradi
  +--------+  +--------+  +--------+  +--------+
   o'z steki   o'z steki   o'z steki   o'z steki
```

**Qoida:** jarayon yadro ma'lumotlariga **to'g'ridan-to'g'ri tegmaydi**. Yadro xizmati kerak bo'lsa — `sys_write()`, `sys_sleep()` ... chaqiradi. Ular **`trap()`** ichida yadroga o'tadi (haqiqiy mashinada bu `syscall` buyrug'i).

### 1) Jarayon (PCB — process control block)

```c
enum holat { BOSH, TAYYOR, UXLAYDI, KUTADI, TUGADI };
```

```c
struct jarayon {
    int pid;
    char nom[8];
    enum holat holat;
    ucontext_t ctx;                             /* saqlangan registrlar va stek ko'rsatkichi: "to'xtatilgan jarayon surati" */
    char *stek;                                 /* har jarayonning O'Z steki */
    void (*kirish)(void);
    long uyg_tik;                               /* UXLAYDI: qaysi tikda uyg'onadi */
    int kutilgan_pid;                           /* KUTADI: qaysi jarayon tugashini kutyapti */
    int chiqish_kodi;
    long a, b, natija;                          /* syscall argumentlari va natijasi (user -> yadro -> user) */
    int nr;                                     /* qaysi syscall so'ralgan */
};
```

| Maydon | Ma'nosi |
|---|---|
| `ctx` | **saqlangan kontekst**: registrlar + stek ko'rsatkichi. "To'xtatilgan jarayonning surati" |
| `stek` | jarayonning **o'z steki** (64 KB) — jarayonlar bir-birining stekini buzmasin |
| `holat` | `TAYYOR` (CPU kutayapti), `UXLAYDI` (`sys_sleep`), `KUTADI` (`sys_join`), `TUGADI` |
| `a`, `b`, `nr`, `natija` | syscall **argumentlari, raqami va natijasi** (jarayon ↔ yadro "pochta qutisi") |

### 2) Syscall jadvali

Syscall — **raqam → funksiya**. Yadro raqamni oladi va jadvaldan ishlovchini topadi (xuddi Linux `sys_call_table` kabi):

```c
static long (*const jadval_syscall[SYS_SONI])(struct jarayon *) = {
    [SYS_YIELD] = s_yield, [SYS_SLEEP] = s_sleep, [SYS_EXIT] = s_exit,
    [SYS_SPAWN] = s_spawn, [SYS_JOIN] = s_join,   [SYS_WRITE] = s_write,
};
```

Oltita syscall: `YIELD` (CPU ni o'z ixtiyori bilan berish), `SLEEP` (n tik uxlash), `EXIT`, `SPAWN` (yangi jarayon), `JOIN` (boshqasi tugashini kutish), `WRITE` (matn chiqarish). Masalan **`SPAWN`** yangi jarayon uchun joy topadi, **stek ajratadi**, `makecontext` bilan "ishga tushishga tayyor" kontekst yaratadi:

```c
static long s_spawn(struct jarayon *j)
{
    for (int i = 1; i < MAKS; i++)
        if (jadval[i].holat == BOSH) {
            struct jarayon *y = &jadval[i];
            y->pid = i;
            snprintf(y->nom, sizeof(y->nom), "%s", (const char *)j->b);
            y->kirish = (void (*)(void))j->a;
            y->stek = malloc(STEK_HAJM);
            getcontext(&y->ctx);                /* hozirgi kontekstni nusxalab, so'ng o'zgartiramiz */
            y->ctx.uc_stack.ss_sp = y->stek;
            y->ctx.uc_stack.ss_size = STEK_HAJM;
            y->ctx.uc_link = &yadro_ctx;        /* kirish funksiyasi qaytsa, yadroga qaytamiz */
            makecontext(&y->ctx, y->kirish, 0);
            y->holat = TAYYOR;
            return i;
        }
    return -1;
}
```

### 3) `trap` — jarayondan yadroga o'tish

Eng muhim ikki qator. `swapcontext(&saqlash, &tiklash)` — joriy registrlarni `saqlash` ga yozadi va `tiklash` dagilarni **yuklaydi**. Jarayon `trap()` chaqirganda **to'xtab qoladi**; yadro bizni qayta ishga tushirganda **aynan shu `swapcontext` dan keyingi qatordan davom etamiz**:

```c
static long trap(int nr, long a, long b)
{
    struct jarayon *j = &jadval[joriy];
    j->nr = nr;
    j->a = a;
    j->b = b;
    swapcontext(&j->ctx, &yadro_ctx);           /* ENG MUHIM QATOR: registrlarni saqlab, yadro kontekstiga o'tamiz */
    return j->natija;                           /* yadro bizni qayta ishga tushirganda shu yerdan davom etamiz */
}
```

### 4) Yadroning rejalashtiruvchi sikli

Yadro doim: (1) uxlayotganlarni **uyg'otadi** (vaqti kelgan bo'lsa), (2) **aylanma navbat** (round-robin) bilan keyingi tayyor jarayonni tanlaydi, (3) unga **o'tadi**, (4) u `trap` qilganda qaytib kelib, so'ralgan syscall ni **bajaradi**:

```c
static void yadro_kirish(void)
{
    while (tirik_bormi()) {
        uyg_otish();
        int k = keyingi_tayyor();
        if (k < 0) {
            tik++;                              /* hamma uxlayapti: vaqt o'tadi (CPU "bo'sh turadi", idle) */
            continue;
        }
        joriy = k;
        tik++;                                  /* har rejalashtirish qarori bitta tik (taymer uzilishi) */
        swapcontext(&yadro_ctx, &jadval[k].ctx);        /* jarayonga o'tamiz; u trap() qilganda shu yerga qaytamiz */

        struct jarayon *j = &jadval[k];
        if (j->nr >= 0 && j->nr < SYS_SONI)
            j->natija = jadval_syscall[j->nr](j);       /* so'ralgan syscall ni jadval orqali bajaramiz */
        if (j->holat == TUGADI) {                       /* sys_exit: stekni qaytaramiz, jarayon boshqa ishlamaydi */
            free(j->stek);
            j->stek = NULL;
        }
    }
}
```

Har rejalashtirish qarori **bitta tik** (taymer uzilishi: haqiqiy yadroda har ~1–4 ms). Hamma jarayon uxlayotgan bo'lsa — vaqt baribir o'tadi (`idle`: CPU "bo'sh turadi").

### 5) "Dasturlar" va ishga tushirish

Jarayonlar — oddiy funksiyalar, faqat `sys_*` chaqiradi. `init` (birinchi jarayon, PID 1) uch bola yaratadi (A, B, C) va hammasi tugashini kutadi:

```c
static void init_dastur(void)
{
    sys_write("init ishga tushdi, bolalar yaratilyapti");
    int a = sys_spawn("A", dastur_a);
    int b = sys_spawn("B", dastur_b);
    int c = sys_spawn("C", dastur_c);
    int ka = sys_join(a), kb = sys_join(b), kc = sys_join(c);
    char s[64];
    snprintf(s, sizeof(s), "hammasi tugadi: chiqish kodlari A=%d B=%d C=%d", ka, kb, kc);
    sys_write(s);
    sys_exit(0);
}
```

```console
$ cd katta_loyiha/tizim/30_ucontext_yadro
$ gcc -Wall -Wextra -g mini_yadro.c -o mini_yadro
$ ./mini_yadro
[tik  1] init : init ishga tushdi, bolalar yaratilyapti
[tik  3] A    : qadam 1, 4 tik uxlayman
[tik  6] B    : hisoblayapman 1/4 (yield)
[tik 11] A    : qadam 2, 4 tik uxlayman
[tik 12] B    : hisoblayapman 2/4 (yield)
[tik 15] B    : hisoblayapman 3/4 (yield)
[tik 17] B    : hisoblayapman 4/4 (yield)
[tik 18] A    : qadam 3, 4 tik uxlayman
[tik 20] C    : uyg'ondim, ish qildim
[tik 29] init : hammasi tugadi: chiqish kodlari A=10 B=20 C=30
yadro to'xtadi: jami 30 tik
```

**Nima ko'rdik (qator bo'yicha):**

| Tik | Voqea | Tushuntirish |
|---|---|---|
| 1 | `init ishga tushdi` | birinchi jarayon rejalashtirildi |
| 3 | `A: qadam 1, 4 tik uxlayman` | `init` A ni `spawn` qildi; A birinchi marta ishga tushdi |
| 6 | `B: hisoblayapman 1/4` | B ham spawn bo'ldi. B har safar `yield` qiladi: CPU ni boshqalarga beradi |
| 11 | `A: qadam 2` | A `sleep(4)` qilgandi (tik 5 da, uyg'onish — 9 da), lekin tik 9–10 da navbat boshqalarda edi: **uxlash — "kamida" shuncha**, aniq emas |
| 12, 15, 17 | B ning keyingi qadamlari | B boshqalar bilan **navbatlashib** ishlayapti (aylanma navbat) |
| 20 | `C: uyg'ondim` | C `sleep(9)` qilgan edi (tik 9 da): 18-tikda uyg'onishi kerak edi, 20-tikda navbat yetdi |
| 29 | `init: hammasi tugadi: A=10 B=20 C=30` | `init` `sys_join` bilan hammasini kutdi va **chiqish kodlarini** oldi |
| — | `jami 30 tik` | yadro hamma jarayon tugagach to'xtadi |

**Muhim kuzatuvlar:**

- Jarayonlar **bir-biriga ko'rinmaydi**: har birining o'z steki, o'z `ctx`. `A` ning mahalliy o'zgaruvchisi (`i`) `swapcontext` lar orasida **saqlanib qoladi** — chunki butun stek saqlanadi.
- **`join` — bloklanuvchi chaqiruv:** init `KUTADI` holatiga o'tadi va **CPU sarflamaydi**; yadro uni faqat A tugagach `TAYYOR` qiladi (`uyg_otish` ichida).
- Jarayonlar **`yield` / `sleep` / `join` / `exit`** da yadroga o'tadi. Bu — **kooperativ** rejalashtirish: jarayon CPU ni o'zi qaytarmaguncha (`yield`/`sleep`/`join`/`exit`) yadro uni **majburan to'xtata olmaydi**. Haqiqiy yadro **taymer uzilishi** bilan majburan to'xtatadi (**preemptiv**, 23-bob).

> **Eslab qoling:** jarayon = **kontekst** (registrlar + stek) + **holat** + **resurslar**. **Kontekst almashish** = bittasini saqlash + ikkinchisini tiklash. Syscall = **jarayon → yadro** o'tish, yadro ishni **jadval** orqali bajaradi. Yadroning 4 asosiy qismi: **jarayonlar jadvali**, **rejalashtiruvchi**, **syscall jadvali**, **bloklanish/uyg'otish** mexanizmi. Bu bosqichdagi dastur — shularning **butun, ishlaydigan** modeli.

**O'zingiz qo'shing (yechimsiz):**

1. **Yangi syscall** qo'shing: `SYS_GETPID` — chaqiruvchi jarayonning `pid` ini qaytaradi. Qaysi **uch joyga** o'zgartirish kiritish kerak? (enum, jadval, `sys_*` o'rami.)
2. `dastur_d` yozing: `sys_sleep(2)` + `sys_write` ni **5 marta** takrorlasin va `init` uni ham `spawn` va `join` qilsin. Natijadagi tik raqamlari qanday o'zgardi?
3. **Ustuvorlik** qo'shing (23-bob): `struct jarayon` ga `ustuvor` maydonini qo'shing va `keyingi_tayyor` ni eng muhim tayyor jarayonni tanlaydigan qilib o'zgartiring.
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. Yadro beshta ish qiladi: **CPU**, **xotira**, **qurilmalar**, **fayllar**, **aloqa** — hammasi **himoya chegarasi** (user ↔ yadro) orqali; xizmat so'rash = **syscall raqami** → jadval → ishlovchi; xato = manfiy son (`-ENOSYS`).
2. **Monolit** (Linux, MyOS) — tez, lekin drayver xatosi hammani o'ldiradi; **mikroyadro** (seL4, QNX) — ishonchli, lekin ko'p IPC (sekinroq); gibrid — Windows, macOS.
3. **Modul** — yadroga ish vaqtida qo'shiladigan kod (`init`/`exit`); `init` xato qaytarsa yuklanmaydi; yuklangach yadro ichida ishlaydi.
4. Linux xaritasi: `arch/` (apparat), `kernel/` (jarayonlar), `mm/`, `fs/`, `drivers/` (~70%), `include/linux/`, `lib/`. O'qish usuli: "MyOS'dagi X ning Linux versiyasi qayerda?"; sayt — elixir.bootlin.com.
5. Hissa qo'shish: `git commit -s`, `get_maintainer.pl`, `git send-email`, sharhlarga `v2`; boshlash joyi — `drivers/staging`, `checkpatch.pl`.

## Savol-javob

**Savol:** Nega Linux mikroyadro emas, axir ishonchliroq-ku?
**Javob:** Tezlik. Monolitda qatlamlar orasida xabar almashish yo'q (30.2 da 2 ga 8 almashish). Linux ishonchlilikni boshqa yo'llar bilan oshiradi: kod tekshiruvi, sanitizer'lar (KASAN), modullar, xavfsizlik qatlamlari.

**Savol:** Modulni yuklash xavflimi?
**Javob:** Ha: modul yadro ichida ishlaydi, uning xatosi butun tizimni qulatadi (monolit!). Shuning uchun birinchi tajribalarni virtual mashinada qiling.

**Savol:** Nega Linux'da patch GitHub orqali emas, pochta orqali yuboriladi?
**Javob:** Tarixiy va amaliy: minglab dasturchi bitta pochta ro'yxatida ochiq muhokama qiladi, har bir qator ko'p ko'zdan o'tadi; maintainer'lar o'z ish tartibiga moslashgan.

## O'zingizni tekshiring

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

## Mashq

- Linux manbasini yuklab olib, 30.4-dagi "solishtirib o'qish" juftliklaridan bittasini tanlang: MyOS funksiyasini va Linux funksiyasini yonma-yon o'qing, farqlarini daftarga yozing.
- Virtual mashinada 30.6-dagi modulni yig'ib yuklang.

<!-- loyiha:boshi -->
## Loyiha: kooperativ mini yadro

**Maqsad:** yadroning eng kichik **yuragi**ni yasash: vazifalar jadvali (task table), rejalashtiruvchi (scheduler) va "vazifa bitta qadam bajaradi va
boshqaruvni qaytaradi" tamoyili. Haqiqiy yadroda kontekst almashish apparat qo'llab-quvvatlaydi; bu yerda uni funksiya chaqiruvi bilan modellaymiz (30.1).
**Bobdan ishlatiladi:** vazifa (task/process) tuzilmasi, holatlar, round-robin rejalashtirish, funksiya ko'rsatkichlari.

**Talab:** 3 ta vazifa. Yadro sikli har "tik"da navbatdagi tayyor vazifaga **bitta qadam** bajartiradi. Vazifa xabar qaytaradi yoki tugaganini bildiradi.
**Ma'lumotlar:** `struct vazifa { const char *nom; enum holat holat; int qadam; const char *(*ish)(struct vazifa *); }`.
**Qoida:** vazifa `NULL` qaytarsa — u tugadi. Navbat aylanma (round-robin): oxirgi ishlagan vazifadan **keyingisi**dan qidiriladi.

```c
/* yadro.c - kooperativ mini yadro */
#include <stdio.h>

enum holat { TAYYOR, TUGADI };

struct vazifa {
    const char *nom;
    enum holat holat;
    int qadam;
    const char *(*ish)(struct vazifa *);        /* bitta qadam; NULL - tugadi */
};

static const char *sanoq(struct vazifa *v)
{
    static const char *xabar[] = { "bir", "ikki", "uch" };
    return v->qadam < 3 ? xabar[v->qadam++] : NULL;
}

static const char *salom(struct vazifa *v)
{
    static const char *xabar[] = { "salom, yadro!", "xayr!" };
    return v->qadam < 2 ? xabar[v->qadam++] : NULL;
}

static const char *tez(struct vazifa *v)
{
    return v->qadam++ < 1 ? "men tezman" : NULL;
}

int main(void)
{
    struct vazifa jadval[] = {
        { "sanoq", TAYYOR, 0, sanoq },
        { "salom", TAYYOR, 0, salom },
        { "tez", TAYYOR, 0, tez },
    };
    int n = sizeof(jadval) / sizeof(jadval[0]);
    int qolgan = n, oxirgi = n - 1;

    for (int tik = 0; qolgan > 0; tik++) {
        int tanlov = -1;
        for (int k = 1; k <= n; k++) {          /* oxirgi ishlagandan KEYINGI tayyor vazifa */
            int i = (oxirgi + k) % n;
            if (jadval[i].holat == TAYYOR) {
                tanlov = i;
                break;
            }
        }
        struct vazifa *v = &jadval[tanlov];
        const char *xabar = v->ish(v);          /* "kontekst almashish": vazifaga o'tdik */
        if (xabar) {
            printf("[tik %d] %-6s: %s\n", tik, v->nom, xabar);
        } else {
            v->holat = TUGADI;
            qolgan--;
            printf("[tik %d] %-6s tugadi\n", tik, v->nom);
        }
        oxirgi = tanlov;
    }
    printf("Hamma vazifa tugadi.\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined yadro.c -o yadro
$ ./yadro
[tik 0] sanoq : bir
[tik 1] salom : salom, yadro!
[tik 2] tez   : men tezman
[tik 3] sanoq : ikki
[tik 4] salom : xayr!
[tik 5] tez    tugadi
[tik 6] sanoq : uch
[tik 7] salom  tugadi
[tik 8] sanoq  tugadi
Hamma vazifa tugadi.
```

Vazifalar navbat bilan ishlaydi (`sanoq`, `salom`, `tez`, `sanoq`, ...) — bitta CPUda ham "bir vaqtda" ishlayotgandek. Kamchilik: vazifa qadam ichida qotib qolsa
(cheksiz sikl), butun yadro qotadi — shuning uchun real yadro **taymer uzilishi** bilan vazifani majburan to'xtatadi (**preemptive** rejalashtirish, 23-bob).

**Kengaytiring:** yana bitta vazifa qo'shing. Bitta vazifaning `ish` funksiyasida cheksiz sikl yozib, yadro qotishini ko'ring (Ctrl+C).

## Mustaqil loyiha: taymer uzilishi va uxlash navbati ★★★

**Vazifa:** yuqoridagi yadroga **vaqt** tushunchasini qo'shing: har tik — taymer uzilishi. Vazifa `SLEEP n` bilan `n` tikka **uxlashi** mumkin; yadro
uni uyg'otadi. Hech kim tayyor bo'lmasa — yadro **bo'sh turadi** (idle). Fayl: `taymer.c`.

**Vazifalar** — "dastur" (buyruqlar ro'yxati). Buyruq turlari: `PRINT "matn"`, `SLEEP n`, `END`:

| Vazifa | Buyruqlar |
|---|---|
| A | `PRINT "A1"`, `SLEEP 3`, `PRINT "A2"`, `END` |
| B | `PRINT "B1"`, `PRINT "B2"`, `SLEEP 1`, `PRINT "B3"`, `END` |
| C | `SLEEP 2`, `PRINT "C1"`, `END` |

**Yadro qoidalari** (har tik `t = 0, 1, 2, ...` uchun, shu tartibda):
1. **Uyg'otish:** uxlayotgan vazifalardan `uyg'onish_vaqti <= t` bo'lganlar `TAYYOR` bo'ladi.
2. **Tanlash:** tayyorlar orasidan oxirgi ishlagan vazifadan **keyingisi** (aylanma, dastlab `A` dan boshlanadi; ya'ni "oxirgi" = oxirgisi, `C`).
3. **Bajarish:** tanlangan vazifaning **bitta buyrug'i**:
   - `PRINT` → chiqaradi, vazifa tayyor qoladi;
   - `SLEEP n` → `uyg'onish_vaqti = t + n`, vazifa uxlaydi (buyruq shu tikni oladi);
   - `END` → vazifa tugaydi.
4. Hech kim tayyor bo'lmasa, lekin uxlayotgan vazifa bor → `idle`. Hech kim qolmasa → tugatish.

**Chiqish shakli:** har tik uchun bitta qator (aniq shakl kutilgan natijada) va oxirida `Hamma vazifa tugadi (t = ...)`.

**Kutilgan natija** (`darslik/loyihalar/30_taymer_yadro/kutilgan.txt`):

```text
t= 0: A yozdi "A1"
t= 1: B yozdi "B1"
t= 2: C uxlaydi (t=4 gacha)
t= 3: A uxlaydi (t=6 gacha)
t= 4: B yozdi "B2"
t= 5: C yozdi "C1"
t= 6: A yozdi "A2"
t= 7: B uxlaydi (t=8 gacha)
t= 8: C tugadi
t= 9: A tugadi
t=10: B yozdi "B3"
t=11: B tugadi
Hamma vazifa tugadi (t = 12)
```

**Maslahat** (yechim emas):
- Vazifa tuzilmasi: `holat` (`TAYYOR`, `UXLAYDI`, `TUGADI`), `pc` (keyingi buyruq indeksi), `uyg` (uyg'onish vaqti), buyruqlar massivi.
  Buyruq — `struct { enum {PRINT, SLEEP, END} tur; const char *matn; int n; }`.
- Tik boshida faqat **uyg'otish** — uxlagan vazifa ayni `t` da `TAYYOR` ga o'tadi va o'sha tikning o'zida tanlanishi mumkin.
- "Oxirgi ishlagan" vazifa indeksini saqlang. `idle` tikda o'zgarmaydi.
- Qo'lda 6–7 tikni izlab chiqing: A1 (t=0), B1, C uxlaydi, A uxlaydi... Shunda qoidalarni to'g'ri tushunganingizni bilasiz.
- Bu — Linux `schedule()`, `msleep()` va taymer uzilishi (`tick`) ning oddiy modeli (23-bob, 30.1).

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined taymer.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/30_taymer_yadro/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [31-bob. Lug'at: ingliz texnik atamalari](31-lugat.md)
