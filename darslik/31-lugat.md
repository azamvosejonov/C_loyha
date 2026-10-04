# 31-bob. Lug'at: ingliz texnik atamalari va xabarlar

> **Bu bobda nima o'rganasiz:** kod, kompilyator xabarlari, Linux va hujjatlar — hammasi **ingliz tilida**. Bu bob ingliz tilini bilmasdan ham ular bilan ishlash uchun: **koddagi nomlarni o'qish usuli**, **atamalar**, **qisqartmalar**,
> **kompilyator va dastur xabarlarining haqiqiy namunalari tarjimasi** va git xabarlari. Kerak bo'lganda qidiring (`Ctrl-F`). Ingliz tilini asta-sekin o'rganish uchun ham eng foydali 300 so'z — shu yerda.
> **Oldindan nima kerak:** hech narsa (boshqa boblar bilan birga ishlatiladi).   **Vaqt:** 3–4 soat o'qish + kundalik takrorlash.

## Bu bob nima haqida?

Dasturlash tili — C — ingliz so'zlaridan tuzilgan (`if`, `while`, `return`), funksiya va o'zgaruvchi nomlari ham asosan inglizcha (`alloc_pages`, `pipe_read`), xato xabarlari ham inglizcha. Ingliz tilini bilmasangiz, xato xabarini o'qiy olmaysiz va
koddagi nomni tushunmaysiz. Lekin yaxshi xabar: texnik inglizchada **bir necha yuz so'z** takrorlanadi va ular juda oddiy. Bu bob shu so'zlarning **to'liq xaritasi**.

Bobdan quyidagicha foydalaning:

| Vaziyat | Qayerga qarang |
|---|---|
| koddagi nom tushunarsiz (`vma_alloc_buf`) | 31.2 — nomni qismlarga bo'lib o'qish; qisqartmalar jadvali |
| atama notanish (`starvation`, `preemption`) | 31.3 — alifbo bo'yicha atamalar |
| kompilyator xato berdi | 31.4 — xabarlar tarjimasi (haqiqiy namunalar bilan) |
| dastur qulab tushdi (`Segmentation fault`, `Aborted`) | 31.5 |
| git g'alati narsa dedi | 31.6 |
| izohda `TODO`, `FIXME`, `must` ... | 31.7 |

## 31.1. So'zlarning asl ma'nosi

**Oddiy qilib aytganda:** inglizcha atamalarning ko'pi oddiy, kundalik so'zlardan olingan. Asl ma'nosini bilsangiz, atama **yodda o'z-o'zidan qoladi** — "nega bunday atalgan?" degan savolning javobi esdagi tayanchga aylanadi.

| Atama | So'zma-so'z | Nega shunday atalgan |
|---|---|---|
| **kernel** | yong'oq mag'zi | Tizimning ichki, eng muhim qismi. Uni o'rab turgan qism — **shell** (po'choq) |
| **shell** | po'choq | Yadroni o'rab turgan va foydalanuvchi bilan gaplashadigan qatlam |
| **boot** | etik | "Pull oneself up by one's bootstraps" — o'zini etigining ipidan tortib ko'tarish. Kompyuter o'zini o'zi ishga tushiradi |
| **fork** | ayri, vilka; yo'l ayrilishi | Bitta jarayon ikkiga ayriladi — xuddi yo'l ayrilgandek |
| **pipe** | quvur | Bir tomondan kiradi, boshqa tomondan chiqadi |
| **thread** | ip | Bitta dastur ichidagi bajarilishning alohida "ipi" |
| **lock** | qulf | Eshikni qulflash kabi — bir vaqtda bitta kishi |
| **stack** | taxlam | Likopchalar taxlami |
| **heap** | uyum | Tartibsiz uyum — bo'sh joy istalgan joydan olinadi |
| **cache** | yashirin zaxira | Fransuzcha "yashirmoq": tez olish uchun yaqinda yashirib qo'yilgan narsa |
| **page** | sahifa | Xotira kitob kabi teng sahifalarga bo'lingan |
| **bus** | avtobus (shina) | Ma'lumotlarni hamma "bekatlarga" tashiydigan umumiy yo'l |
| **bug** | qo'ng'iz | 1947-yilda Harvard Mark II kompyuterining relesiga haqiqiy kuya qamalib qolgan va u jurnalga yopishtirib qo'yilgan |
| **patch** | yamoq | Kiyimdagi teshikni yamash kabi — kodning kichik tuzatmasi |
| **daemon** | ruh, jin (yunoncha: yordamchi ruh) | Ko'rinmasdan, fonda ishlab yuradigan yordamchi dastur |
| **zombie** | tirik o'lik | Jarayon tugagan (o'lgan), lekin hali ro'yxatda "yuribdi" |
| **orphan** | yetim | Otasi tugagan jarayon — uni `init` "asrab oladi" |
| **spin** | aylanmoq | Qulf bo'shashini aylanib-aylanib kutish |
| **dump** | to'kib tashlamoq | Xotiraning hammasini faylga "to'kib" tashlash (core dump) |
| **flush** | yuvib yubormoq | Bufer ichidagini bir yo'la chiqarib yuborish |
| **hook** | ilgak | Boshqa kod "ilinishi" mumkin bo'lgan joy |
| **handler** | ishlovchi, boshqaruvchi | Hodisa kelganda uni "qo'lga oladigan" funksiya |
| **mount** | minmoq, o'rnatmoq | Diskni katalog daraxtiga "mindirish" |
| **sandbox** | qumdon | Bolalar faqat qumdon ichida o'ynaydi — tashqariga zarar yo'q |
| **firmware** | "qattiq" dastur | Apparatga yozib qo'yilgan, kam o'zgaradigan dastur (hard + soft oralig'i) |

**Sinab ko'ring:** 31.3-bo'limdagi atamalardan 10 tasini tanlab, har biri uchun o'zingizning hayotiy misolingizni o'ylab toping va daftaringizga yozing. O'zingiz o'ylab topgan o'xshatish boshqasinikidan yaxshiroq eslab qolinadi.

## 31.2. Koddagi nomlarni o'qish

### Usul: nomni qismlarga bo'lish

**Oddiy qilib aytganda:** koddagi uzun nomlar — **bir nechta oddiy so'zlarning** birikmasi (`_` bilan ajratilgan yoki qisqartirilgan). Tarjima qilish uchun uchta qadam:

1. Nomni `_` bo'yicha **so'zlarga bo'ling**: `pipe_read` → `pipe` + `read`.
2. Har bir so'zni **tarjima qiling** (jadvaldan): quvur + o'qish.
3. **Tartibini** o'ylang: odatda "nima + nima qiladi" (`pipe_read` = "quvurdan o'qish"), yoki "nima qiladi + nimani" (`alloc_pages` = "sahifalarni ajratish").

### Haqiqiy misol: MyOS'dan bitta qator

MyOS yadrosining `kernel/fs/pipe.c` faylidan olingan haqiqiy qator:

```text
static int64_t pipe_read(struct file *f, void *dst, size_t len, uint64_t off)
```

| Qism | Inglizcha | Ma'nosi |
|---|---|---|
| `static` | static | shu fayldan tashqarida ko'rinmaydi (5-bob) |
| `int64_t` | integer, 64-bit | 64 bitli butun son — qaytariladigan tur (2-bob) |
| `pipe_read` | pipe + read | "quvurdan o'qish" — funksiya nomi |
| `struct file *f` | file, `f` | fayl tuzilmasiga ko'rsatkich; `f` — file'ning qisqartmasi |
| `void *dst` | destination | nishon: ma'lumot **qayerga** yoziladi |
| `size_t len` | length | uzunlik: **nechta** bayt o'qiladi |
| `uint64_t off` | offset | siljish: faylning **qaysi joyidan** boshlab |

Butun qator ma'nosi: **"quvurdan `len` bayt o'qib, `dst` manziliga yoz"** — `read()` syscall'ining quvur uchun ishlovchisi (14-bob).

Yana bir misol: `alloc_pages(order, gfp)` = allocate + pages = "sahifalarni ajrat"; `order` — tartib (2ᵏ sahifa, 25-bob), `gfp` — get free pages: qanday ajratish bayroqlari. `free_pages` — "sahifalarni bo'shat". Ko'ryapsizmi: bir marta
so'zlarni bilsangiz, yuzlab funksiyani **o'qiy olasiz**.

### Koddagi qisqartmalar (kategoriyalar bo'yicha)

**Umumiy:**

| Qisqartma | To'liq | Ma'nosi |
|---|---|---|
| `alloc` | allocate | ajratish (xotira) |
| `free` | free | ozod qilish, bo'shatish |
| `init` | initialize | boshlash, boshlang'ich holatga keltirish |
| `buf` | buffer | bufer, vaqtinchalik xotira |
| `len` | length | uzunlik |
| `sz`, `size` | size | hajm |
| `cnt`, `count` | count | soni |
| `idx` | index | indeks, tartib raqami |
| `ptr` | pointer | ko'rsatkich |
| `src` / `dst` | source / destination | manba / nishon |
| `tmp` | temporary | vaqtincha |
| `ret` | return (value) | qaytish qiymati |
| `err` | error | xato |
| `ctx` | context | kontekst, holat |
| `prev` / `next` | previous / next | oldingi / keyingi |
| `cur`, `curr` | current | joriy |
| `max` / `min` | maximum / minimum | eng katta / eng kichik |
| `num` | number | son |
| `str` | string | satr |
| `arg`, `argv`, `argc` | argument (vector, count) | argument (massivi, soni) |
| `off`, `offset` | offset | siljish |
| `addr` | address | manzil |
| `cb` | callback | qayta chaqiriladigan funksiya |
| `cfg`, `conf` | configuration | sozlama |
| `ver` | version | versiya |
| `hdr` | header | sarlavha |
| `msg` | message | xabar |
| `req` / `resp` | request / response | so'rov / javob |
| `en`, `enable` / `dis`, `disable` | enable / disable | yoqish / o'chirish |
| `stat` | status / statistics | holat / statistika |
| `ctl` | control | boshqaruv |
| `util` | utility | yordamchi |
| `impl` | implementation | amalga oshirish |
| `dbg` | debug | debug, xato izlash |
| `priv` | private | shaxsiy (drayverning o'z ma'lumoti) |
| `ref`, `refcount` | reference (count) | havola (sanog'i): nechta joy ishlatyapti |
| `ops`, `fops` | operations (file operations) | amallar jadvali (funksiya ko'rsatkichlari to'plami, 7-bob) |

**Xotira:**

| Qisqartma | To'liq | Ma'nosi |
|---|---|---|
| `mm` | memory management | xotira boshqaruvi |
| `vm`, `vma` | virtual memory (area) | virtual xotira (hududi) |
| `pte` | page table entry | sahifa jadvali yozuvi |
| `phys`, `virt` | physical, virtual | fizik, virtual |
| `blk` | block | blok |

**Jarayonlar va sinxronizatsiya:**

| Qisqartma | To'liq | Ma'nosi |
|---|---|---|
| `pid` | process ID | jarayon raqami |
| `uid` / `gid` | user / group ID | foydalanuvchi / guruh raqami |
| `proc` | process | jarayon |
| `thr`, `thread` | thread | oqim |
| `sched` | scheduler | rejalashtiruvchi |
| `sig` | signal | signal |
| `lock` / `unlock` | lock | qulflash / ochish |
| `sem` | semaphore | semafor |
| `mtx`, `mutex` | mutual exclusion | o'zaro istisno qulfi |
| `irq` | interrupt request | uzilish so'rovi |
| `isr` | interrupt service routine | uzilish ishlovchisi |

**Fayl va qurilmalar:**

| Qisqartma | To'liq | Ma'nosi |
|---|---|---|
| `fd` | file descriptor | fayl deskriptori |
| `fs` | file system | fayl tizimi |
| `dev` | device | qurilma |
| `drv` | driver | drayver |
| `sb` | superblock | superblok |
| `ino` | inode | inode |
| `dir` | directory | papka |
| `wr` / `rd` | write / read | yozish / o'qish |
| `rx` / `tx` | receive / transmit | qabul qilish / yuborish |

> **Eslab qoling:** noma'lum nomni uch qadamda ochasiz: `_` bo'yicha bo'l → har so'zni tarjima qil → tartibini o'yla. 40–50 ta qisqartma yodlansa, yadro kodining ko'p qismi tushunarli bo'ladi.

## 31.3. Asosiy atamalar (alifbo tartibida)

Qavs ichidagi raqam — shu atama qaysi bobda tushuntirilgan.

| Inglizcha | O'zbekcha | Izoh / bob |
|---|---|---|
| abstraction | abstraksiya | murakkablikni yashiruvchi oddiy interfeys |
| address space | manzil maydoni | jarayon ko'radigan virtual xotira (24) |
| alignment | tekislash | manzil N ga karrali (9, 16) |
| allocator | ajratuvchi | xotira beruvchi kod (25) |
| argument | argument | funksiyaga beriladigan qiymat |
| array | massiv | (6) |
| assembly | assembler tili | (17) |
| assertion | tasdiq | "bu rost bo'lishi shart" tekshiruvi (12) |
| atomic | atomik, bo'linmas | (15, 26) |
| backtrace | chaqiruvlar zanjiri | gdb `bt` (29) |
| barrier | to'siq | xotira tartibini saqlash (15) |
| binary | ikkilik; bajariladigan fayl | (20) |
| bit / byte | bit / bayt | 8 bit = 1 bayt |
| bitmap | bitlar xaritasi | (16, 21-mashq) |
| block | blok | disk yoki xotira bo'lagi |
| boot / bootloader | yuklash / yuklovchi | GRUB (18) |
| branch | tarmoq; sakrash | git / CPU |
| breakpoint | to'xtash nuqtasi | gdb (1, 29) |
| buffer overflow | bufer to'lishi | (6, 13) |
| bug | xato | dasturdagi xato (so'zma-so'z: qo'ng'iz) |
| build | yig'ish | kompilyatsiya + bog'lash |
| cache | kesh | (21) |
| cache line | kesh qatori | 64 bayt (21) |
| call stack | chaqiruvlar steki | (5, 17) |
| callee / caller | chaqiriluvchi / chaqiruvchi | funksiyani chaqirgan (caller) va chaqirilgan (callee) tomon |
| cast | tur o'zgartirish | `(int)x` (2) |
| clone | klonlash; nusxa | git clone |
| commit | commit (saqlash) | git; tranzaksiyani tasdiqlash |
| compiler | kompilyator | (1) |
| concurrency | parallellik | (15, 26) |
| condition variable | shart o'zgaruvchisi | (26) |
| context switch | kontekst almashish | (23) |
| copy-on-write (COW) | yozishda nusxalash | (24) |
| core | yadro (CPU); core dump | protsessorning bitta hisoblash bloki; `core dump` — qulagan dastur xotirasining surati (29) |
| critical section | kritik seksiya | (15) |
| deadlock | o'zaro qotish | (15, 26) |
| declaration / definition | e'lon / ta'rif | (1, 5) |
| dereference | ko'rsatkich orqali murojaat | `*p` (7) |
| descriptor | deskriptor | fd; GDT yozuvi |
| device | qurilma | qurilma: disk, klaviatura, tarmoq kartasi... (16, 27) |
| directory | papka, katalog | ichida fayllar turadigan papka (27) |
| dirty | "iflos" (o'zgartirilgan) | diskka yozilmagan (24) |
| driver | drayver | (27) |
| dynamic linking | dinamik bog'lash | (22) |
| endianness | bayt tartibi | (16, 20) |
| entry point | kirish nuqtasi | `main`, `_start` |
| exception | istisno | CPU: #PF, #GP (17, 24) |
| executable | bajariladigan fayl | ishga tushirsa bo'ladigan fayl (22) |
| fault | xato; istisno | page fault |
| file system | fayl tizimi | (27) |
| flag | bayroq | bitli sozlama |
| fork | ayirish | jarayon nusxasi (14) |
| fragmentation | fragmentatsiya | (25) |
| frame | kadr; freym | stek kadri; fizik sahifa |
| freestanding | mustaqil (OS'siz) | (18) |
| handler | ishlovchi | uzilish/signal ishlovchisi |
| hash table | xesh jadval | (28) |
| header | sarlavha | `.h` fayl; paket sarlavhasi |
| heap | heap (dinamik xotira); uyum | (8, 28) |
| hosted | OS ustida | (18) |
| implementation | amalga oshirish | g'oyaning kodda yozilgan ko'rinishi |
| inode | inode | fayl metama'lumoti (27) |
| instruction | buyruq (CPU) | protsessorga beriladigan bitta oddiy buyruq (17) |
| interrupt | uzilish | (16, 23) |
| kernel | yadro | OS ning ichki, eng muhim qismi (30) |
| kernel mode / user mode | yadro rejimi / user rejimi | 0-halqa / 3-halqa |
| latency | kechikish | so'rov berilgandan javob kelgunga qadar o'tgan vaqt |
| leak | sizib chiqish | xotira/fd (8) |
| library | kutubxona | (11) |
| linker | bog'lovchi | (1, 22) |
| lock | qulf | (15) |
| lock-free | qulfsiz | (26) |
| macro | makro | (10) |
| memory-mapped I/O | xotiraga xaritalangan kiritish-chiqarish | (16) |
| mount | ulash (fayl tizimini) | diskni katalog daraxtiga "ulash" (27) |
| mutex | mutex (o'zaro istisno) | (15) |
| namespace | nomlar maydoni | nomlar bir-biriga to'qnashmasligi uchun alohida "guruh" |
| null pointer | nol ko'rsatkich | (7) |
| object file | obyekt fayl | `.o` (1, 22) |
| offset | siljish | boshidan necha bayt/element narida (27, 14) |
| overflow | toshish | (2, 20) |
| page | sahifa | 4 KB (24) |
| page fault | sahifa xatosi | (24) |
| page table | sahifa jadvali | (24, 31-mashq) |
| panic | panika (halokatli xato) | yadro to'xtashi |
| parameter | parametr | funksiya e'lonidagi o'zgaruvchi nomi (argument — chaqiruvdagi qiymat) |
| parser / parsing | tahlilchi / tahlil | (24-mashq) |
| patch | tuzatma (o'zgarishlar fayli) | (30) |
| permission | ruxsat | kim o'qishi/yozishi/ishga tushirishi mumkin (19, 27) |
| pipe | quvur | (14) |
| pointer | ko'rsatkich | (7) |
| polling | so'rab turish | (27) |
| preemption | majburiy to'xtatish | (23) |
| preprocessor | preprotsessor | (10) |
| priority | ustuvorlik | qaysi jarayon oldin ishlashi muhimligi (23) |
| process | jarayon | (14, 23) |
| race condition | poyga holati | (15) |
| register | registr | (17) |
| relocation | relokatsiya | (22) |
| repository | repozitoriy (ombor) | git (19) |
| return value | qaytish qiymati | funksiya qaytargan natija (5) |
| root | ildiz; administrator | `/`; root foydalanuvchi |
| runtime | ish vaqti | dastur ishlayotgan vaqt (kompilyatsiya vaqtiga qarshi) |
| scheduler | rejalashtiruvchi | (23) |
| scope | ko'rinish sohasi | (2, 4) |
| section / segment | bo'lim / segment | ELF (22) |
| semaphore | semafor | (26) |
| shell | shell (buyruq qobig'i) | (14, 19) |
| signal | signal | (14) |
| signed / unsigned | ishorali / ishorasiz | (2, 20) |
| spinlock | aylanuvchi qulf | (15) |
| stack | stek | (5, 17) |
| starvation | och qolish | (23, 26) |
| statement | buyruq (C'da) | `;` bilan tugaydi |
| static | statik | (5, 8, 11) |
| struct | struktura | (9) |
| symbol | belgi (nom) | (22) |
| syscall (system call) | tizim chaqiruvi | (14) |
| thread | oqim | (15) |
| throughput | o'tkazuvchanlik | vaqt birligida bajarilgan ish miqdori (soniyasiga nechta so'rov) |
| timer | taymer | belgilangan vaqtda uzilish beruvchi qurilma (23) |
| TLB | manzil tarjimasi keshi | (21) |
| trap | tuzoq; ushlash (istisno) | dasturni to'xtatib, boshqaruvni yadroga beradigan istisno/syscall (14) |
| two's complement | ikkiga to'ldirish | (20) |
| undefined behavior | aniqlanmagan xatti-harakat | (13) |
| union | birlashma | (9) |
| user space | foydalanuvchi maydoni | oddiy dasturlar ishlaydigan, cheklangan soha (24) |
| virtual memory | virtual xotira | (24) |
| volatile | o'zgaruvchan (keshlanmaydigan) | (16) |
| wait queue | kutish navbati | (26, 38-mashq) |
| warning | ogohlantirish | kompilyator "bu shubhali" deydi, lekin yig'adi (31.4) |
| zombie | zombi jarayon | (14) |

## 31.4. Kompilyator (gcc) xabarlari tarjimasi

### Xabar qanday tuzilgan

**Oddiy qilib aytganda:** kompilyator xato topsa, bir xil shaklda xabar beradi:

```text
fayl.c:7:5: error: expected ',' or ';' before 'printf'
```

| Qism | Ma'nosi |
|---|---|
| `fayl.c:7:5` | **fayl : qator : ustun** — xato qayerda (7-qator, 5-belgi) |
| `error` / `warning` | **xato** (yig'ib bo'lmaydi) yoki **ogohlantirish** (yig'iladi, lekin shubhali) |
| `expected ',' or ';' before 'printf'` | **matn**: "`printf` dan oldin `,` yoki `;` kutilgan edi" |
| `[-Wunused-variable]` (oxirida) | qaysi **ogohlantirish bayrog'i** chiqargani — qidirish uchun yaxshi kalit |

**Eng muhim qoida:** xabarlarni **yuqoridan pastga** o'qing va **birinchi** xatoni tuzating — keyingilari ko'pincha shunchaki uning oqibati. Quyida eng ko'p uchraydigan xatolarning **haqiqiy** chiqishi bor (7 ta kichik dastur). Sizdagi
gcc versiyasida tirnoq belgilari (`'x'` yoki `‘x’`) va matn biroz farq qilishi mumkin.

### 1) Nuqtali vergul unutilgan

```c
/* xato1.c - nuqtali vergul unutilgan */
#include <stdio.h>

int main(void)
{
    int son = 5
    printf("%d\n", son);
    return 0;
}
```

```console
$ gcc -Wall -Wextra xato1.c -o xato1   # xato kutiladi
xato1.c: In function ‘main’:
xato1.c:7:5: error: expected ‘,’ or ‘;’ before ‘printf’
    7 |     printf("%d\n", son);
      |     ^~~~~~
xato1.c:6:9: warning: unused variable ‘son’ [-Wunused-variable]
    6 |     int son = 5
      |         ^~~
```

**Tarjima:** `error: expected ',' or ';' before 'printf'` = "`printf` dan **oldin** `,` yoki `;` kutilgan edi". Kompilyator xatoni **keyingi** qatorda ko'radi (6-qatordagi `;` yo'qligini 7-qatorda sezadi), shuning uchun **bir qator yuqoriga** qarang.
Ikkinchi xabar (`unused variable 'son'`) — oqibat: xato tufayli `son` "ishlatilmagan" bo'lib qoldi. **Tuzatish:** `int son = 5;`.

### 2) E'lon qilinmagan nom (imlo xatosi)

```c
/* xato2.c - e'lon qilinmagan nom */
#include <stdio.h>

int main(void)
{
    int narx = 100;
    printf("%d\n", nrax);
    return 0;
}
```

```console
$ gcc -Wall -Wextra xato2.c -o xato2   # xato kutiladi
xato2.c: In function ‘main’:
xato2.c:7:20: error: ‘nrax’ undeclared (first use in this function); did you mean ‘narx’?
    7 |     printf("%d\n", nrax);
      |                    ^~~~
      |                    narx
xato2.c:7:20: note: each undeclared identifier is reported only once for each function it appears in
xato2.c:6:9: warning: unused variable ‘narx’ [-Wunused-variable]
    6 |     int narx = 100;
      |         ^~~~
```

**Tarjima:** `'nrax' undeclared (first use in this function)` = "`nrax` e'lon qilinmagan (shu funksiyada birinchi ishlatilishi)". gcc yana **`did you mean 'narx'?`** ("`narx` demoqchi emasmidingiz?") deb taklif ham qiladi. **Tuzatish:** imloni to'g'rilash.

### 3) Funksiya e'lonsiz ishlatilgan va ta'rifi yo'q

```c
/* xato3.c - funksiya e'lonsiz ishlatilgan; ta'rifi esa yo'q */
int main(void)
{
    return hisobla(5);
}
```

```console
$ gcc -Wall -Wextra xato3.c -o xato3 2>&1 | sed 's#/tmp/cc[A-Za-z0-9]*\.o#/tmp/ccXXXX.o#'   # xato kutiladi
xato3.c: In function ‘main’:
xato3.c:4:12: warning: implicit declaration of function ‘hisobla’ [-Wimplicit-function-declaration]
    4 |     return hisobla(5);
      |            ^~~~~~~
/usr/bin/ld: /tmp/ccXXXX.o: in function `main':
xato3.c:(.text+0x13): undefined reference to `hisobla'
collect2: error: ld returned 1 exit status
```

**Tarjima:** bu yerda **ikki** bosqichning xabari:

| Xabar | Tarjima | Qaysi bosqich |
|---|---|---|
| `warning: implicit declaration of function 'hisobla'` | `hisobla` funksiyasining **yashirin e'loni** (e'lon yo'q, kompilyator o'zicha taxmin qildi) | **kompilyator** (1-bob) |
| `undefined reference to 'hisobla'` | `hisobla` ga **havola aniqlanmagan** (ta'rifi topilmadi) | **linker** (bog'lovchi) |
| `ld returned 1 exit status` | `ld` (linker) 1 chiqish kodi bilan tugadi — ya'ni **xato** | linker |

**Tuzatish:** funksiyani yozing (yoki uning `.c` faylini yig'ishga qo'shing/kutubxonani ulang) va `#include` bilan e'lonini bering.

### 4) Format va tur mos emas

```c
/* xato4.c - format va tur mos emas */
#include <stdio.h>

int main(void)
{
    long katta = 5000000000L;
    printf("%d\n", katta);
    return 0;
}
```

```console
$ gcc -Wall -Wextra xato4.c -o xato4   # xato kutiladi
xato4.c: In function ‘main’:
xato4.c:7:14: warning: format ‘%d’ expects argument of type ‘int’, but argument 2 has type ‘long int’ [-Wformat=]
    7 |     printf("%d\n", katta);
      |             ~^     ~~~~~
      |              |     |
      |              int   long int
      |             %ld
```

**Tarjima:** `format '%d' expects argument of type 'int', but argument 2 has type 'long int'` = "`%d` formati `int` turini kutadi, lekin 2-argument `long int`". gcc to'g'ri variantni ham chizib ko'rsatdi (`%ld`).
**Tuzatish:** `%ld`. (Aks holda katta son noto'g'ri chiqadi.)

### 5) Ishlatilmagan o'zgaruvchi, boshlanmagan qiymat, `return` yo'q

```c
/* xato5.c - ishlatilmagan o'zgaruvchi, boshlanmagan qiymat, return yo'q */
int yig(int a, int b)
{
    int natija;
    int ortiqcha = 0;
    if (a > 0)
        natija = a + b;
    return natija;
}

static int bosh(void)
{
    yig(1, 2);
}

int main(void)
{
    return bosh();
}
```

```console
$ gcc -Wall -Wextra -O2 xato5.c -o xato5   # xato kutiladi
xato5.c: In function ‘yig’:
xato5.c:5:9: warning: unused variable ‘ortiqcha’ [-Wunused-variable]
    5 |     int ortiqcha = 0;
      |         ^~~~~~~~
xato5.c: In function ‘bosh’:
xato5.c:14:1: warning: no return statement in function returning non-void [-Wreturn-type]
   14 | }
      | ^
xato5.c: In function ‘yig’:
xato5.c:8:12: warning: ‘natija’ may be used uninitialized [-Wmaybe-uninitialized]
    8 |     return natija;
      |            ^~~~~~
xato5.c:4:9: note: ‘natija’ was declared here
    4 |     int natija;
      |         ^~~~~~
```

**Tarjima:**

| Xabar | Tarjima | Nima qilish kerak |
|---|---|---|
| `unused variable 'ortiqcha'` | `ortiqcha` ishlatilmagan | o'chirish (yoki `(void)ortiqcha;`) |
| `'natija' may be used uninitialized` | `natija` boshlang'ich qiymatsiz ishlatilishi **mumkin** (`a > 0` bo'lmasa qiymat berilmaydi) | boshlang'ich qiymat bering |
| `no return statement in function returning non-void` | qiymat qaytarishi kerak funksiyada `return` yo'q | `return` qo'shing |

### 6) Ko'rsatkich va son aralashgan

```c
/* xato6.c - ko'rsatkich va son aralashib ketgan */
#include <stdio.h>

int main(void)
{
    int son = 7;
    int *p = son;
    printf("%d\n", *p);
    return 0;
}
```

```console
$ gcc -Wall -Wextra xato6.c -o xato6   # xato kutiladi
xato6.c: In function ‘main’:
xato6.c:7:14: warning: initialization of ‘int *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
    7 |     int *p = son;
      |              ^~~
```

**Tarjima:** `initialization of 'int *' from 'int' makes pointer from integer without a cast` = "`int *` ni `int` dan boshlash: songa **tur o'zgartirmasdan** ko'rsatkich yasaydi". Ya'ni ko'rsatkichga oddiy son berildi. **Tuzatish:** `int *p = &son;` (7-bob).

### 7) Lokal o'zgaruvchining manzili qaytarilmoqda

```c
/* xato7.c - lokal o'zgaruvchining manzili qaytarilmoqda */
static int *yaratish(void)
{
    int qiymat = 42;
    return &qiymat;
}

int main(void)
{
    return *yaratish();
}
```

```console
$ gcc -Wall -Wextra xato7.c -o xato7   # xato kutiladi
xato7.c: In function ‘yaratish’:
xato7.c:5:12: warning: function returns address of local variable [-Wreturn-local-addr]
    5 |     return &qiymat;
      |            ^~~~~~~
```

**Tarjima:** `function returns address of local variable` = "funksiya **lokal o'zgaruvchi manzilini** qaytaradi". Funksiya tugagach lokal o'zgaruvchi yo'qoladi (stek, 5-bob), qaytarilgan manzil esa "osilib qolgan". **Tuzatish:** heap (`malloc`) yoki chaqiruvchi bergan bufer.

### Boshqa xabarlar tarjimasi

| Xabar | Tarjima | Odatda nima qilish kerak |
|---|---|---|
| `expected ')' before ...` | `)` kutilgan | qavslarni sanang |
| `expected declaration or statement at end of input` | fayl oxirida e'lon yoki buyruq kutilgan | `}` yetishmayapti |
| `multiple definition of 'x'` | `x` bir necha marta ta'riflangan | `.h` da ta'rif bor — `extern` qiling (11-bob) |
| `conflicting types for 'f'` | `f` uchun turlar zid | e'lon va ta'rif mos emas |
| `incompatible pointer type` | mos kelmaydigan ko'rsatkich turi | tur yoki `&` ni tekshiring |
| `passing argument 1 of 'f' makes pointer from integer` | 1-argument: sondan ko'rsatkich yasaldi | `&x` kerak bo'lishi mumkin |
| `too few / too many arguments to function` | argument kam / ko'p | chaqiruvni e'lon bilan solishtiring |
| `comparison of integer expressions of different signedness` | ishorali va ishorasiz taqqoslanmoqda | turlarni moslang (2-bob) |
| `suggest parentheses around assignment used as truth value` | shartda `=` ishlatilgan | `==` kerakmi? |
| `array subscript is above array bounds` | indeks massiv chegarasidan katta | indeksni tekshiring |
| `this 'if' clause does not guard...` (misleading indentation) | chekinish aldamchi | `{ }` qo'ying (4-bob) |
| `dereferencing pointer to incomplete type` | to'liq ta'riflanmagan turga murojaat | struct ta'rifi ko'rinmaydi (`#include`) |
| `storage size of 'x' isn't known` | `x` hajmi noma'lum | xuddi shu |
| `"/*" within comment` | izoh ichida `/*` | (10-bob) |
| `missing separator` (make) | ajratgich yo'q | Makefile'da TAB o'rniga bo'shliq (11-bob) |
| `No such file or directory` | bunday fayl yoki papka yo'q | yo'lni tekshiring |
| `Permission denied` | ruxsat berilmadi | `chmod +x` yoki `sudo` |
| `command not found` | buyruq topilmadi | o'rnatilmagan yoki `./` unutilgan |

> **Eslab qoling:** `error` — tuzating (yig'ilmaydi); `warning` — ham tuzating (ko'pincha haqiqiy xato). Avval **birinchi** xabarni o'qing. `fayl:qator:ustun` — xatoning manzili; oldingi qatorga ham qarang.

## 31.5. Dastur ishlaganda chiqadigan xabarlar

**Oddiy qilib aytganda:** kompilyator xabarlari — dastur **yig'ilmasdan oldin**; bu xabarlar esa dastur **ishlayotganda** (yoki qulaganda) chiqadi. Ko'pchiligi OS yoki libc'dan keladi. Pastda ularning **haqiqiy** chiqishi (bobning "to'liq dastur" qismida) ko'rsatilgan.

| Xabar | Tarjima | Sabab |
|---|---|---|
| `Segmentation fault (core dumped)` | segmentatsiya xatosi | ruxsatsiz xotiraga murojaat (7, 8) |
| `Bus error` | shina xatosi | tekislanmagan murojaat yoki `mmap` fayl chegarasi |
| `Floating point exception` | kasr son istisnosi | aslida ko'pincha butun **nolga bo'lish** |
| `Aborted` | to'xtatildi | `abort()`, `assert` buzildi |
| `Killed` | o'ldirildi | SIGKILL (ko'pincha xotira tugadi — OOM) |
| `double free or corruption` / `free(): double free detected` | ikki marta free yoki buzilish | (8) |
| `free(): invalid pointer` | noto'g'ri ko'rsatkich free qilindi | (8) |
| `malloc(): corrupted top size` | heap buzilgan | oldingi chegaradan chiqish |
| `stack smashing detected` | stek buzilishi aniqlandi | stekdagi bufer to'ldi |
| `Broken pipe` | uzilgan quvur | o'quvchisi yo'q pipe'ga yozish (SIGPIPE) |
| `Resource temporarily unavailable` | resurs vaqtincha mavjud emas | EAGAIN |
| `Interrupted system call` | tizim chaqiruvi uzildi | EINTR (signal) |
| `Too many open files` | ochiq fayllar juda ko'p | fd sizib chiqishi |
| `Bad file descriptor` | noto'g'ri fayl deskriptori | yopilgan fd ishlatildi |
| `Address already in use` | manzil band | port boshqa dasturda |
| `Device or resource busy` | qurilma band | ulangan diskni ajratish |

AddressSanitizer xabarlari — 29-bobda tushuntirilgan; `tools/mashq.py` ham ularni o'zi tushuntiradi (8-bob, 18-mashq).

## 31.6. Git xabarlari

**Oddiy qilib aytganda:** git holat haqida **inglizcha gaplar** bilan xabar beradi. Quyida eng ko'p uchraydiganlarining **haqiqiy** chiqishi (bitta kichik repozitoriyda ketma-ket bajarilgan). Git xabarlari tilga qarab o'zgaradi; aniqlik uchun `LC_ALL=C` ishlatildi.

```console
$ rm -rf gdemo gdemo_origin.git; git init -q --bare --initial-branch=main gdemo_origin.git; git clone -q gdemo_origin.git gdemo 2>/dev/null; cd gdemo && git config user.name Ali && git config user.email a@b.c && echo 'int x;' > fayl.c && LC_ALL=C git status | grep -E 'Untracked|nothing added'
Untracked files:
nothing added to commit but untracked files present (use "git add" to track)
$ cd gdemo && git add fayl.c && git commit -q -m birinchi && git push -q -u origin main 2>/dev/null; LC_ALL=C git status | grep -E 'nothing to commit|up to date'
Your branch is up to date with 'origin/main'.
nothing to commit, working tree clean
$ cd gdemo && echo 'int y;' >> fayl.c && LC_ALL=C git status | grep -E 'not staged|modified'
Changes not staged for commit:
	modified:   fayl.c
$ cd gdemo && git commit -q -a -m ikkinchi && LC_ALL=C git status | grep -E 'ahead'
Your branch is ahead of 'origin/main' by 1 commit.
$ cd gdemo && git push -q origin main 2>/dev/null; git checkout -q -b boshqa && echo 'int a;' > fayl.c && git commit -q -a -m boshqa && git checkout -q main && echo 'int b;' > fayl.c && git commit -q -a -m main-da && LC_ALL=C git merge boshqa | grep CONFLICT; git merge --abort
CONFLICT (content): Merge conflict in fayl.c
$ cd gdemo && git checkout -q HEAD~1 2>/dev/null; LC_ALL=C git status | head -1 | sed 's/at [0-9a-f]*/at XESH/'; git checkout -q main
HEAD detached at XESH
```

**Qadamma-qadam tarjima:**

| Qadam | Chiqqan xabar | Tarjima / ma'nosi |
|---|---|---|
| 1. yangi fayl yaratildi | `Untracked files:` ... `nothing added to commit but untracked files present` | git **kuzatmaydigan** yangi fayllar bor, lekin saqlashga hech narsa qo'shilmagan — `git add` kerak |
| 2. `add` + `commit` + `push` | `nothing to commit, working tree clean` / `Your branch is up to date with 'origin/main'` | saqlaydigan narsa yo'q, hammasi saqlangan / sizning shoxingiz GitHub'dagi bilan bir xil |
| 3. fayl o'zgartirildi | `Changes not staged for commit:` ... `modified: fayl.c` | o'zgarishlar bor, lekin hali `git add` (yoki `commit -a`) qilinmagan |
| 4. `commit` (push'siz) | `Your branch is ahead of 'origin/main' by 1 commit.` | sizda GitHub'da yo'q 1 ta commit bor — `git push` |
| 5. ikki shoxda bir qatorni o'zgartirib `merge` | `CONFLICT (content): Merge conflict in fayl.c` | **to'qnashuv**: ikki o'zgarish bir joyni o'zgartirgan — faylda `<<<<<<<` belgilarni topib, qo'lda hal qiling (`merge --abort` — bekor qilish) |
| 6. `checkout HEAD~1` | `HEAD detached at ...` | "HEAD uzilgan": shoxsiz eski commitdasiz — `git checkout main` bilan qayting |

**Boshqa xabarlar:**

| Xabar | Tarjima / ma'nosi |
|---|---|
| `Your branch is behind ... can be fast-forwarded` | GitHub'da yangiliklar bor — `git pull` |
| `rejected ... (fetch first)` | push rad etildi (GitHub'da sizda yo'q commit bor) — avval `git pull` |

> **Eslab qoling:** git xabari odatda **keyingi qadamni o'zi aytadi** (`use "git add"`, `git push`...). Qavs ichidagi maslahatni o'qing.

## 31.7. Kodda ko'p uchraydigan so'zlar (izohlar va nomlar)

**Fe'llar** (funksiya nomlarining boshida):

| Guruh | So'zlar |
|---|---|
| olish/berish | get (olish), set (o'rnatish), send/recv (yuborish/qabul), read/write (o'qish/yozish), load/store (yuklash/saqlash) |
| yaratish/yo'q qilish | add (qo'shish), remove/del (o'chirish), create (yaratish), destroy (yo'q qilish), insert (qo'yish), open/close (ochish/yopish), register (ro'yxatdan o'tkazish) |
| qidirish/tekshirish | find/lookup/search (topish/qidirish), check/validate (tekshirish), probe (tekshirib topish — drayverda), parse (tahlil qilish), trace (kuzatish) |
| holat o'zgarishi | update (yangilash), reset (qayta boshlash), clear (tozalash), fill (to'ldirish), flush (tozalab yozish), copy (nusxalash), move (ko'chirish), map/unmap (xaritalash/olib tashlash) |
| kutish/jarayon | wait (kutish), wake (uyg'otish), sleep (uxlash), yield (navbatni berish), spawn (yangi jarayon yaratish), exit (chiqish), return (qaytish), handle (ishlov berish), dump (to'kib chiqarish, ko'rsatish) |

**Sifatlar/holatlar:**

| Juft | Ma'nosi |
|---|---|
| valid / invalid | yaroqli / yaroqsiz |
| empty / full | bo'sh / to'la |
| busy / idle | band / bo'sh turibdi |
| enabled / disabled | yoqilgan / o'chirilgan |
| present / missing | mavjud / yetishmayotgan |
| dirty / clean | o'zgargan (diskka yozilmagan) / toza |
| shared / private | umumiy / shaxsiy |
| readonly / writable | faqat o'qish / yozish mumkin |
| ready, pending, active | tayyor, kutilayotgan, faol |
| locked, aligned, unused | qulflangan, tekislangan, ishlatilmagan |
| deprecated, legacy, generic, default | eskirgan, eski/meros, umumiy, sukut bo'yicha |

**Izohlardagi iboralar:**

| Ibora | Ma'nosi |
|---|---|
| `TODO` | qilish kerak |
| `FIXME` | tuzatish kerak |
| `XXX` | diqqat, shubhali joy |
| `HACK` | vaqtinchalik yechim |
| `NOTE` | eslatma |
| `must` / `should` / `may` | shart / kerak / mumkin |
| `never` / `always` | hech qachon / doim |
| `assume` | faraz qilamiz |
| `caller` | chaqiruvchi |
| `on success` / `on failure` | muvaffaqiyatda / xatoda |
| `returns` | qaytaradi |
| `if any` | agar bo'lsa |

## 31.8. Ingliz tilini asta-sekin o'rganish maslahati

Bu darslik bilan ingliz tili shart emas. Lekin kelajakda (Linux, ish, spetsifikatsiyalar) kerak bo'ladi. Eng samarali yo'l — **texnik ingliz tili**, umumiy emas:

1. Shu bobdagi so'zlarni har kuni 10 tadan takrorlang — ular kodda doim uchraydi.
2. Kod o'qiganda nomlarni ovoz chiqarib tarjima qiling: `alloc_pages` → "sahifalarni ajrat" (31.2 dagi 3 qadam).
3. Kompilyator xabarlarini tarjima jadvalisiz tushunishga harakat qiling.
4. Keyinchalik: Linux `Documentation/` dan qisqa hujjatlarni lug'at bilan o'qish.

Kuniga 20–30 daqiqa — REJA.md dagi kundalik tartibda shunga joy qoldirilgan.

## Hayotdan misol va to'liq dastur

**Dastur nega qulaydi.** 31.5 dagi xabarlar jadvalini **jonli** ko'ramiz: bitta dasturni 5 xil usulda ataylab "buzamiz" va har safar OS/libc qanday xabar berishini ko'ramiz.

**Bu dastur nima qiladi (umumiy):** buyruq satridagi raqamga qarab 5 ta xatoning birini hosil qiladi: (1) `NULL` ga murojaat, (2) butun songa nolga bo'lish, (3) buzilgan `assert`, (4) ikki marta `free`, (5) stekdagi kichik buferga ko'p bayt yozish.
`volatile int nol` — kompilyator xatoni oldindan "ko'rib" olib tashlamasligi uchun (16-bob).

```c
/* xabarlar.c - dastur ishlayotganda chiqadigan xabarlarni ataylab hosil qiladi */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int variant = argc > 1 ? atoi(argv[1]) : 0;
    volatile int nol = 0;                       /* kompilyator oldindan hisoblab qo'ymasin */

    switch (variant) {
    case 1: {
        int *p = NULL;
        printf("%d\n", *p);                     /* NULL ga murojaat */
        break;
    }
    case 2:
        printf("%d\n", 10 / nol);               /* butun songa nolga bo'lish */
        break;
    case 3:
        assert(nol == 1);                       /* tasdiq buzildi */
        break;
    case 4: {
        char *p = malloc(16);
        free(p);
        free(p);                                /* ikki marta free */
        break;
    }
    case 5: {
        char kichik[8];
        for (int i = 0; i < 64 + nol; i++)      /* 8 baytlik buferga 64 bayt yozamiz */
            kichik[i] = 'A';
        printf("%c\n", kichik[0]);
        break;
    }
    default:
        printf("variant: 1..5\n");
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O0 xabarlar.c -o xabarlar   # xato kutiladi
xabarlar.c: In function ‘main’:
xabarlar.c:26:9: warning: pointer ‘p’ used after ‘free’ [-Wuse-after-free]
   26 |         free(p);                                /* ikki marta free */
      |         ^~~~~~~
xabarlar.c:25:9: note: call to ‘free’ here
   25 |         free(p);
      |         ^~~~~~~
$ for v in 1 2 3 4 5; do echo "== variant $v"; bash -c "./xabarlar $v; echo chiqish kodi: \$?" 2>&1 | sed 's#^.*line [0-9]*: *[0-9]* ##; s#^\([A-Za-z ]*[a-z]\) *\./xabarlar [0-9]#\1#'; done
== variant 1
Segmentation fault
chiqish kodi: 139
== variant 2
Floating point exception
chiqish kodi: 136
== variant 3
xabarlar: xabarlar.c:21: main: Assertion `nol == 1' failed.
Aborted
chiqish kodi: 134
== variant 4
free(): double free detected in tcache 2
Aborted
chiqish kodi: 134
== variant 5
*** stack smashing detected ***: terminated
Aborted
chiqish kodi: 134
```

**Nima ko'rdik:**

| Variant | Xabar | Chiqish kodi | Ma'nosi |
|---|---|---|---|
| 1 (`*NULL`) | `Segmentation fault` | 139 | ruxsatsiz xotiraga murojaat; 139 = 128 + 11 (SIGSEGV) |
| 2 (`10 / 0`) | `Floating point exception` | 136 | nomi chalg'itadi: aslida **butun** songa bo'lish; 136 = 128 + 8 (SIGFPE) |
| 3 (`assert`) | `Assertion 'nol == 1' failed` + `Aborted` | 134 | shart buzildi; `abort()` → 134 = 128 + 6 (SIGABRT) |
| 4 (`free` ikki marta) | `free(): double free detected in tcache 2` + `Aborted` | 134 | glibc xotira buzilishini aniqladi va dasturni to'xtatdi |
| 5 (stek buferi) | `*** stack smashing detected ***: terminated` + `Aborted` | 134 | kompilyatorning stek himoyasi ("canary") buzilganini sezdi |

**Qoida:** dastur **signal** bilan o'lsa, chiqish kodi = **128 + signal raqami** (`echo $?` bilan ko'riladi). Sizda `Segmentation fault` yoniga `(core dumped)` ham qo'shilishi mumkin (core fayllar yoqilgan bo'lsa, 29-bob).

## Bob xulosasi (yodlash uchun)

1. Ko'pgina inglizcha atamalar oddiy so'zlardan olingan (kernel — yong'oq mag'zi, bug — qo'ng'iz, fork — ayri). **Asl ma'nosini bilsangiz atama yodda qoladi.**
2. Koddagi nomni **3 qadamda** o'qing: `_` bo'yicha bo'l → har so'zni tarjima qil → tartibni o'yla (`pipe_read` = "quvurdan o'qish"). 40–50 qisqartma (`buf`, `len`, `ptr`, `src/dst`, `ctx`...) kodning ko'pini ochadi.
3. Kompilyator xabari: **`fayl:qator:ustun: error/warning: matn`**. Birinchi xabardan boshlang; xato ko'pincha **oldingi** qatorda (unutilgan `;`). `error` — yig'ilmaydi, `warning` — shubhali (tuzating).
4. Dastur signal bilan o'lsa chiqish kodi **128 + signal**: `Segmentation fault` = 139, `Floating point exception` (butun songa nolga bo'lish ham) = 136, `Aborted` = 134.
5. Git xabarlari keyingi qadamni o'zi aytadi; eng ko'p: `Untracked`, `Changes not staged`, `ahead`, `CONFLICT`, `detached HEAD`.

## Savol-javob

**Savol:** Ingliz tilini bilmasam, yadro dasturchisi bo'la olamanmi?
**Javob:** Ha, boshlash mumkin: texnik inglizcha ~300 so'z va ko'p takrorlanadi. Lekin uzoq muddatda ingliz tili kerak bo'ladi (Linux, hujjatlar, jamoa) — shuning uchun har kuni 20–30 daqiqa ajrating (31.8).

**Savol:** Nega xato xabari yig'ilmagan qatorni emas, keyingisini ko'rsatadi?
**Javob:** Kompilyator xatoni **keyingi** token (so'z/belgi) o'qilganda sezadi (`int son = 5` dan keyin `printf` kelganda "bu yerda `;` kerak edi" deydi). Shuning uchun xabardagi qatordan **bir yuqoriga** ham qarang.

**Savol:** `warning` ni e'tiborsiz qoldirsam bo'ladimi?
**Javob:** Yo'q: ko'p haqiqiy xatolar (`uninitialized`, format mos kelmasligi, lokal manzil qaytarish) aynan ogohlantirish sifatida chiqadi. Darslik `-Wall -Wextra` bilan yig'adi va ogohlantirishlarni xato deb hisoblaydi.

## O'zingizni tekshiring

1. `alloc_pages` va `free_pages` nomlarini tarjima qiling.
2. `fayl.c:12:9: error: expected ';' before 'return'` xabarida xatoni qayerdan qidirasiz?
3. Dastur `Floating point exception` bilan o'ldi — kasr sonlar bilan ishlamasa-yu? Nega shunday?
4. Chiqish kodi 139 nimani bildiradi?
5. `git status`: `Your branch is ahead of 'origin/main' by 2 commits` — nima qilish kerak?

<details><summary>Javoblar</summary>

1. Allocate pages — "sahifalarni ajrat"; free pages — "sahifalarni bo'shat".
2. 12-qatordan **bir qator yuqorida** (11-qatorda `;` yetishmayapti): kompilyator xatoni keyingi so'z (`return`) o'qilganda sezdi.
3. Bu nom tarixiy: SIGFPE signali butun songa **nolga bo'lishda** ham keladi (protsessor "arifmetik istisno" beradi).
4. 139 = 128 + 11: dastur 11-signal (SIGSEGV, segmentatsiya xatosi) bilan o'ldirildi.
5. `git push` — sizdagi 2 ta commitni GitHub'ga yuborish.
</details>

## Mashq

- 31.4 dagi xatolardan 3 tasini **o'zingiz** hosil qiling (kodni o'zgartirib) va xabarni tarjima qilib daftarga yozing.
- Haqiqiy yadro kodidan (`~/C_loyha/kernel/mm/pmm.c`) 5 ta funksiya nomini tanlab, 31.2 dagi usulda tarjima qiling.
- 31.3 dagi 10 ta atama uchun o'z hayotiy o'xshatishingizni yozing.

<!-- loyiha:boshi -->
## Loyiha: atamalar qidiruvchisi

**Maqsad:** saralangan ma'lumotdan **ikkilik qidiruv** bilan tez topish, va prefiks bo'yicha qidirish. Bu lug'at (31-bob) uchun ham, yadro ichida
saralangan jadvallar (`bsearch`, belgilar jadvali, `extable`) uchun ham ishlaydi (28.4).
**Bobdan ishlatiladi:** `struct` massivi, `bsearch`, `strcasecmp` (katta-kichik harfga sezgir emas solishtirish), prefiks tekshirish.

**Talab:** alifbo bo'yicha **saralangan** atamalar massivi. `qidir(nom)` — katta-kichik harfni farqlamasdan aniq nom bo'yicha; `prefiks(p)` — `p`
bilan boshlanuvchi hamma atamalar.
**Muhim:** `bsearch` faqat **saralangan** massivda to'g'ri ishlaydi — massiv tartibi `strcasecmp` bilan mos bo'lishi shart.

```c
/* lugat.c - atamalar qidiruvchisi */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

struct atama {
    const char *nom;
    const char *izoh;
};

static const struct atama lugat[] = {           /* alifbo tartibida! */
    { "bug", "dasturdagi xato" },
    { "cache", "tez yashirin xotira" },
    { "daemon", "fonda ishlovchi dastur" },
    { "fork", "jarayon nusxasini yaratish" },
    { "heap", "dinamik xotira sohasi" },
    { "kernel", "operatsion tizim yadrosi" },
    { "mutex", "o'zaro istisno qulfi" },
    { "pipe", "jarayonlar orasidagi quvur" },
    { "shell", "buyruq qobig'i" },
    { "stack", "chaqiruvlar steki" },
    { "thread", "bajarilish ipi" },
    { "zombie", "tugagan, lekin kutilmagan jarayon" },
};
#define SONI (sizeof(lugat) / sizeof(lugat[0]))

static int taqqosla(const void *kalit, const void *elem)
{
    return strcasecmp((const char *)kalit, ((const struct atama *)elem)->nom);
}

static void qidir(const char *nom)
{
    const struct atama *a = bsearch(nom, lugat, SONI, sizeof(lugat[0]), taqqosla);
    if (a)
        printf("%-8s -> %s\n", a->nom, a->izoh);
    else
        printf("%-8s -> topilmadi\n", nom);
}

static void prefiks(const char *p)
{
    printf("'%s' bilan boshlanuvchilar:", p);
    int topildi = 0;
    for (size_t i = 0; i < SONI; i++)
        if (strncasecmp(lugat[i].nom, p, strlen(p)) == 0) {
            printf(" %s", lugat[i].nom);
            topildi++;
        }
    printf("%s\n", topildi ? "" : " (yo'q)");
}

int main(void)
{
    qidir("kernel");
    qidir("MUTEX");                             /* katta harf ham topiladi */
    qidir("zombie");
    qidir("linux");
    prefiks("s");
    prefiks("TH");
    prefiks("x");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined lugat.c -o lugat
$ ./lugat
kernel   -> operatsion tizim yadrosi
mutex    -> o'zaro istisno qulfi
zombie   -> tugagan, lekin kutilmagan jarayon
linux    -> topilmadi
's' bilan boshlanuvchilar: shell stack
'TH' bilan boshlanuvchilar: thread
'x' bilan boshlanuvchilar: (yo'q)
```

12 atamada ko'pi bilan 4 solishtirish (2⁴ = 16 ≥ 12) kifoya; million atamada ham 20 ta. Ketma-ket qidirish esa o'rtacha yarmini ko'rar edi.
Massivni saralamasdan `bsearch` chaqirilsa — natija **noto'g'ri** (yoki topmaydi), lekin xato ham bermaydi: bunday xatolar eng ayyor.

**Kengaytiring:** `"stack"` va `"shell"` o'rnini almashtirib (saralanmagan qilib) `qidir("thread")` ni chaqiring. Prefiks qidiruvini `bsearch` kabi tezlashtiring
(birinchi mosini ikkilik qidirish bilan toping, keyin oldinga yuring).

## Mustaqil loyiha: `hexdump` ★★★

**Vazifa:** yadro dasturchisi har kuni **baytlarni ko'zi bilan ko'radi**: disk tasviri, tarmoq paketi, ELF fayl, xotira dumpi. Buning uchun `hexdump -C`
(yoki `xxd`) kerak. Siz shunday vositani yozing: standart kirishdan baytlarni o'qib, `hexdump -C -v` formatida chiqaring. Fayl: `hexdump.c`.

**Format (aniq).** Har qator — 16 bayt:

```text
00000000  53 61 6c 6f 6d 2c 20 79  61 64 72 6f 21 0a 00 01  |Salom, yadro!...|
```

1. **Offset:** 8 xonali kichik harfli o'n oltilik (`%08x`), keyin **ikki probel**.
2. **Baytlar:** har bayt `%02x` va bitta probel. **8-baytdan keyin qo'shimcha bitta probel** (ya'ni ikkita guruh orasida ikki probel).
3. Oxirgi qator to'liq bo'lmasa, yetishmagan baytlar o'rniga **3 ta probel** (`"   "`) qo'yiladi (8-baytdan keyingi qo'shimcha probel ham saqlanadi).
4. So'ng **bitta probel**, `|`, ASCII ko'rinishi, `|`. ASCII: kodi 32..126 — belgining o'zi, qolganlari — `.`. Oxirgi qatorda faqat mavjud baytlar.
5. Hamma qatordan keyin **oxirgi qator**: jami baytlar soni `%08x` ko'rinishida (bo'sh kirish uchun `00000000`).
6. Takrorlanuvchi qatorlar `*` bilan **qisqartirilmaydi** (bu `hexdump -v` kabi).

**Kirish fayli** (`darslik/loyihalar/31_hexdump/kirish.bin`, 56 bayt: matn + 0x00–0x13 baytlar + matn + `ff fe 7f 80`).

**1-sinov:** `./dastur < kirish.bin`

```text
00000000  53 61 6c 6f 6d 2c 20 79  61 64 72 6f 21 0a 00 01  |Salom, yadro!...|
00000010  02 03 04 05 06 07 08 09  0a 0b 0c 0d 0e 0f 10 11  |................|
00000020  12 13 48 65 78 20 64 75  6d 70 20 74 65 73 74 20  |..Hex dump test |
00000030  30 31 32 33 ff fe 7f 80                           |0123....|
00000038
```

**2-sinov:** bo'sh kirish — `./dastur < /dev/null`

```text
00000000
```

**3-sinov:** aniq 16 bayt — `printf 'ABCDEFGHIJKLMNOP' | ./dastur`

```text
00000000  41 42 43 44 45 46 47 48  49 4a 4b 4c 4d 4e 4f 50  |ABCDEFGHIJKLMNOP|
00000010
```

**Maslahat** (yechim emas):
- Kirishni `fread(bufer, 1, 16, stdin)` bilan 16 baytdan o'qing — qaytgan son `n` (oxirida 16 dan kam bo'lishi mumkin, 0 bo'lsa tugadi).
- Bo'lakni chiqarish: `for i in 0..15`: `i < n` bo'lsa `"%02x "`, aks holda `"   "`; `i == 7` dan keyin qo'shimcha `" "`. Keyin `" |"`...
- Baytni `unsigned char` sifatida qayta ishlang: `isprint((unsigned char)c)` yoki `c >= 32 && c <= 126`.
- Jami baytlarni yig'ing va oxirida `%08x` bilan chiqaring.
- Sinab ko'rish: sizda `hexdump -C -v fayl` yoki `xxd fayl` bo'lsa, natijani solishtiring; yo'q bo'lsa `od -A x -t x1z -v fayl` ham o'xshash (format biroz boshqacha).
- Bu dastur bilan MyOS disk tasviri, ELF sarlavhasi (22-bob) va o'z `.o` fayllaringizga qarang: `./dastur < namuna.elf | head`.

**Tekshirish:**

```bash
D=~/C_loyha/darslik/loyihalar/31_hexdump
gcc -Wall -Wextra -g -fsanitize=address,undefined hexdump.c -o dastur
./dastur < $D/kirish.bin | diff - $D/kutilgan.txt && echo "1: TO'G'RI"
./dastur < /dev/null | diff - $D/kutilgan_2.txt && echo "2: TO'G'RI"
printf 'ABCDEFGHIJKLMNOP' | ./dastur | diff - $D/kutilgan_3.txt && echo "3: TO'G'RI"
```
<!-- loyiha:oxiri -->

