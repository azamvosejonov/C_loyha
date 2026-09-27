# 31-bob. Lug'at: ingliz texnik atamalari va xabarlar

> Kod, kompilyator xabarlari, Linux va hujjatlar — hammasi ingliz tilida. Bu bob ingliz tilini bilmasdan
> ham ular bilan ishlash uchun: **atamalar**, **koddagi qisqartmalar**, **kompilyator va dastur xabarlari**
> tarjimasi. Kerak bo'lganda qidiring (`Ctrl-F`). Ingliz tilini asta-sekin o'rganish uchun ham eng
> foydali 300 so'z — shu yerda.

## 31.1. Koddagi qisqartmalar (nomlarda tez-tez uchraydi)

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
| `fd` | file descriptor | fayl deskriptori |
| `pid` | process ID | jarayon raqami |
| `uid` / `gid` | user / group ID | foydalanuvchi / guruh raqami |
| `mm` | memory management | xotira boshqaruvi |
| `vm`, `vma` | virtual memory (area) | virtual xotira (hududi) |
| `pte` | page table entry | sahifa jadvali yozuvi |
| `phys`, `virt` | physical, virtual | fizik, virtual |
| `addr` | address | manzil |
| `off`, `offset` | offset | siljish |
| `irq` | interrupt request | uzilish so'rovi |
| `isr` | interrupt service routine | uzilish ishlovchisi |
| `dev` | device | qurilma |
| `drv` | driver | drayver |
| `blk` | block | blok |
| `sb` | superblock | superblok |
| `ino` | inode | inode |
| `dir` | directory | papka |
| `ops`, `fops` | operations (file operations) | amallar jadvali |
| `priv` | private | shaxsiy (drayverning o'z ma'lumoti) |
| `ref`, `refcount` | reference (count) | havola (sanog'i) |
| `lock` / `unlock` | lock | qulflash / ochish |
| `sem` | semaphore | semafor |
| `mtx`, `mutex` | mutual exclusion | o'zaro istisno qulfi |
| `cb` | callback | qayta chaqiriladigan funksiya |
| `cfg`, `conf` | configuration | sozlama |
| `ver` | version | versiya |
| `hdr` | header | sarlavha |
| `msg` | message | xabar |
| `req` / `resp` | request / response | so'rov / javob |
| `rx` / `tx` | receive / transmit | qabul qilish / yuborish |
| `wr` / `rd` | write / read | yozish / o'qish |
| `en`, `enable` / `dis`, `disable` | enable / disable | yoqish / o'chirish |
| `sched` | scheduler | rejalashtiruvchi |
| `proc` | process | jarayon |
| `thr`, `thread` | thread | oqim |
| `sig` | signal | signal |
| `fs` | file system | fayl tizimi |
| `ctl` | control | boshqaruv |
| `stat` | status / statistics | holat / statistika |
| `util` | utility | yordamchi |
| `impl` | implementation | amalga oshirish |
| `dbg` | debug | debug, xato izlash |

## 31.2. Asosiy atamalar (alifbo tartibida)

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
| bug | xato | |
| build | yig'ish | kompilyatsiya + bog'lash |
| cache | kesh | (21) |
| cache line | kesh qatori | 64 bayt (21) |
| call stack | chaqiruvlar steki | (5, 17) |
| callee / caller | chaqiriluvchi / chaqiruvchi | |
| cast | tur o'zgartirish | `(int)x` (2) |
| clone | klonlash; nusxa | git clone |
| commit | commit (saqlash) | git; tranzaksiyani tasdiqlash |
| compiler | kompilyator | (1) |
| concurrency | parallellik | (15, 26) |
| condition variable | shart o'zgaruvchisi | (26) |
| context switch | kontekst almashish | (23) |
| copy-on-write (COW) | yozishda nusxalash | (24) |
| core | yadro (CPU); core dump | |
| critical section | kritik seksiya | (15) |
| deadlock | o'zaro qotish | (15, 26) |
| declaration / definition | e'lon / ta'rif | (1, 5) |
| dereference | ko'rsatkich orqali murojaat | `*p` (7) |
| descriptor | deskriptor | fd; GDT yozuvi |
| device | qurilma | |
| directory | papka, katalog | |
| dirty | "iflos" (o'zgartirilgan) | diskka yozilmagan (24) |
| driver | drayver | (27) |
| dynamic linking | dinamik bog'lash | (22) |
| endianness | bayt tartibi | (16, 20) |
| entry point | kirish nuqtasi | `main`, `_start` |
| exception | istisno | CPU: #PF, #GP (17, 24) |
| executable | bajariladigan fayl | |
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
| implementation | amalga oshirish | |
| inode | inode | fayl metama'lumoti (27) |
| instruction | buyruq (CPU) | |
| interrupt | uzilish | (16, 23) |
| kernel | yadro | |
| kernel mode / user mode | yadro rejimi / user rejimi | 0-halqa / 3-halqa |
| latency | kechikish | |
| leak | sizib chiqish | xotira/fd (8) |
| library | kutubxona | (11) |
| linker | bog'lovchi | (1, 22) |
| lock | qulf | (15) |
| lock-free | qulfsiz | (26) |
| macro | makro | (10) |
| memory-mapped I/O | xotiraga xaritalangan kiritish-chiqarish | (16) |
| mount | ulash (fayl tizimini) | |
| mutex | mutex (o'zaro istisno) | (15) |
| namespace | nomlar maydoni | |
| null pointer | nol ko'rsatkich | (7) |
| object file | obyekt fayl | `.o` (1, 22) |
| offset | siljish | |
| overflow | toshish | (2, 20) |
| page | sahifa | 4 KB (24) |
| page fault | sahifa xatosi | (24) |
| page table | sahifa jadvali | (24, 31-mashq) |
| panic | panika (halokatli xato) | yadro to'xtashi |
| parameter | parametr | |
| parser / parsing | tahlilchi / tahlil | (24-mashq) |
| patch | tuzatma (o'zgarishlar fayli) | (30) |
| permission | ruxsat | |
| pipe | quvur | (14) |
| pointer | ko'rsatkich | (7) |
| polling | so'rab turish | (27) |
| preemption | majburiy to'xtatish | (23) |
| preprocessor | preprotsessor | (10) |
| priority | ustuvorlik | |
| process | jarayon | (14, 23) |
| race condition | poyga holati | (15) |
| register | registr | (17) |
| relocation | relokatsiya | (22) |
| repository | repozitoriy (ombor) | git (19) |
| return value | qaytish qiymati | |
| root | ildiz; administrator | `/`; root foydalanuvchi |
| runtime | ish vaqti | |
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
| throughput | o'tkazuvchanlik | |
| timer | taymer | |
| TLB | manzil tarjimasi keshi | (21) |
| trap | tuzoq; ushlash (istisno) | |
| two's complement | ikkiga to'ldirish | (20) |
| undefined behavior | aniqlanmagan xatti-harakat | (13) |
| union | birlashma | (9) |
| user space | foydalanuvchi maydoni | |
| virtual memory | virtual xotira | (24) |
| volatile | o'zgaruvchan (keshlanmaydigan) | (16) |
| wait queue | kutish navbati | (26, 38-mashq) |
| warning | ogohlantirish | |
| zombie | zombi jarayon | (14) |

## 31.3. Kompilyator (gcc) xabarlari tarjimasi

| Xabar | Tarjima | Odatda nima qilish kerak |
|---|---|---|
| `expected ';' before 'X'` | X dan oldin `;` kutilgan edi | bir qator yuqorida `;` qo'ying |
| `expected ')' before ...` | `)` kutilgan | qavslarni sanang |
| `expected declaration or statement at end of input` | fayl oxirida e'lon yoki buyruq kutilgan | `}` yetishmayapti |
| `'x' undeclared (first use in this function)` | `x` e'lon qilinmagan | nom xato yozilgan yoki e'lon yo'q |
| `implicit declaration of function 'f'` | `f` funksiyasi e'lonsiz ishlatilgan | `#include` qo'shing |
| `undefined reference to 'f'` | `f` ga havola aniqlanmagan (linker) | `.c`/kutubxona ulanmagan |
| `multiple definition of 'x'` | `x` bir necha marta ta'riflangan | `.h` da ta'rif bor — `extern` qiling |
| `conflicting types for 'f'` | `f` uchun turlar zid | e'lon va ta'rif mos emas |
| `incompatible pointer type` | mos kelmaydigan ko'rsatkich turi | tur yoki `&` ni tekshiring |
| `makes integer from pointer without a cast` | ko'rsatkichdan son yasaldi | `*` yoki `&` unutilgan |
| `assignment to 'x' from incompatible pointer type` | | |
| `passing argument 1 of 'f' makes pointer from integer` | 1-argument: sondan ko'rsatkich yasaldi | `&x` kerak bo'lishi mumkin |
| `too few / too many arguments to function` | argument kam / ko'p | |
| `control reaches end of non-void function` | qiymat qaytaruvchi funksiya oxiriga `return` siz yetib keldi | `return` qo'shing |
| `unused variable 'x'` / `unused parameter 'x'` | `x` ishlatilmagan | o'chiring yoki `(void)x;` |
| `'x' is used uninitialized` | `x` boshlang'ich qiymatsiz ishlatilgan | qiymat bering |
| `comparison of integer expressions of different signedness` | ishorali va ishorasiz taqqoslanmoqda | turlarni moslang (2-bob) |
| `suggest parentheses around assignment used as truth value` | shartda `=` ishlatilgan | `==` kerakmi? |
| `format '%d' expects argument of type 'int', but argument 2 has type 'long'` | format va tur mos emas | `%ld` |
| `array subscript is above array bounds` | indeks massiv chegarasidan katta | |
| `this 'if' clause does not guard...` (misleading indentation) | chekinish aldamchi | `{ }` qo'ying (4-bob) |
| `function returns address of local variable` | lokal o'zgaruvchi manzili qaytarilmoqda | heap yoki chaqiruvchi buferi (5, 8) |
| `dereferencing pointer to incomplete type` | to'liq ta'riflanmagan turga murojaat | struct ta'rifi ko'rinmaydi (`#include`) |
| `storage size of 'x' isn't known` | `x` hajmi noma'lum | xuddi shu |
| `"/*" within comment` | izoh ichida `/*` | (10-bob) |
| `missing separator` (make) | ajratgich yo'q | Makefile'da TAB o'rniga bo'shliq |
| `No such file or directory` | bunday fayl yoki papka yo'q | yo'lni tekshiring |
| `Permission denied` | ruxsat berilmadi | `chmod +x` yoki `sudo` |
| `command not found` | buyruq topilmadi | o'rnatilmagan yoki `./` unutilgan |

## 31.4. Dastur ishlaganda chiqadigan xabarlar

| Xabar | Tarjima | Sabab |
|---|---|---|
| `Segmentation fault (core dumped)` | segmentatsiya xatosi | ruxsatsiz xotiraga murojaat (7, 8) |
| `Bus error` | shina xatosi | tekislanmagan murojaat yoki `mmap` fayl chegarasi |
| `Floating point exception` | kasr son istisnosi | aslida ko'pincha butun **nolga bo'lish** |
| `Aborted` | to'xtatildi | `abort()`, `assert` buzildi |
| `Killed` | o'ldirildi | SIGKILL (ko'pincha xotira tugadi — OOM) |
| `double free or corruption` | ikki marta free yoki buzilish | (8) |
| `free(): invalid pointer` | noto'g'ri ko'rsatkich free qilindi | |
| `malloc(): corrupted top size` | heap buzilgan | oldingi chegaradan chiqish |
| `stack smashing detected` | stek buzilishi aniqlandi | stekdagi bufer to'ldi |
| `Broken pipe` | uzilgan quvur | o'quvchisi yo'q pipe'ga yozish (SIGPIPE) |
| `Resource temporarily unavailable` | resurs vaqtincha mavjud emas | EAGAIN |
| `Interrupted system call` | tizim chaqiruvi uzildi | EINTR (signal) |
| `Too many open files` | ochiq fayllar juda ko'p | fd sizib chiqishi |
| `Bad file descriptor` | noto'g'ri fayl deskriptori | yopilgan fd ishlatildi |
| `Address already in use` | manzil band | port boshqa dasturda |
| `Device or resource busy` | qurilma band | ulangan diskni ajratish |

AddressSanitizer xabarlari — `tools/mashq.py` ularni o'zi tushuntiradi (8-bob, 18-mashq).

## 31.5. Git xabarlari

| Xabar | Tarjima / ma'nosi |
|---|---|
| `nothing to commit, working tree clean` | commit qiladigan narsa yo'q, hammasi saqlangan |
| `Changes not staged for commit` | o'zgarishlar bor, lekin `git add` qilinmagan |
| `Untracked files` | git kuzatmaydigan yangi fayllar |
| `Your branch is ahead of 'origin/main' by 2 commits` | sizda GitHub'da yo'q 2 ta commit bor — `git push` |
| `Your branch is behind ... can be fast-forwarded` | GitHub'da yangiliklar bor — `git pull` |
| `CONFLICT (content): Merge conflict in fayl.c` | ikki o'zgarish to'qnashdi — faylda `<<<<<<<` belgilarni topib, qo'lda hal qiling |
| `detached HEAD` | tarmoqsiz eski commitdasiz — `git checkout main` bilan qayting |
| `rejected ... (fetch first)` | push rad etildi — avval `git pull` |

## 31.6. Kodda ko'p uchraydigan so'zlar (izohlar va nomlar)

**Fe'llar:** get (olish), set (o'rnatish), add (qo'shish), remove/del (o'chirish), create (yaratish),
destroy (yo'q qilish), open/close (ochish/yopish), read/write (o'qish/yozish), send/recv (yuborish/qabul),
find/lookup/search (topish/qidirish), insert (qo'yish), update (yangilash), check/validate (tekshirish),
handle (ishlov berish), map/unmap (xaritalash/olib tashlash), load/store (yuklash/saqlash),
flush (tozalab yozish), reset (qayta boshlash), wait (kutish), wake (uyg'otish), sleep (uxlash),
yield (navbatni berish), spawn (yangi jarayon yaratish), exit (chiqish), return (qaytish),
copy (nusxalash), move (ko'chirish), fill (to'ldirish), clear (tozalash), parse (tahlil qilish),
dump (to'kib chiqarish, ko'rsatish), trace (kuzatish), probe (tekshirib topish — drayverda), register (ro'yxatdan o'tkazish).

**Sifatlar/holatlar:** valid/invalid (yaroqli/yaroqsiz), empty/full (bo'sh/to'la), busy/idle (band/bo'sh),
ready (tayyor), pending (kutilayotgan), active (faol), enabled/disabled (yoqilgan/o'chirilgan),
present (mavjud), missing (yetishmayotgan), dirty/clean (o'zgargan/toza), shared/private (umumiy/shaxsiy),
readonly (faqat o'qish), writable (yozish mumkin), locked (qulflangan), aligned (tekislangan),
unused (ishlatilmagan), deprecated (eskirgan), legacy (eski, meros), generic (umumiy), default (sukut bo'yicha).

**Izohlardagi iboralar:** `TODO` (qilish kerak), `FIXME` (tuzatish kerak), `XXX` (diqqat, shubhali joy),
`HACK` (vaqtinchalik yechim), `NOTE` (eslatma), `must` (shart), `should` (kerak), `may` (mumkin),
`never` (hech qachon), `always` (doim), `assume` (faraz qilamiz), `caller` (chaqiruvchi),
`on success / on failure` (muvaffaqiyatda / xatoda), `returns` (qaytaradi), `if any` (agar bo'lsa).

## 31.7. Ingliz tilini asta-sekin o'rganish maslahati

Bu darslik bilan ingliz tili shart emas. Lekin kelajakda (Linux, ish, spetsifikatsiyalar) kerak bo'ladi.
Eng samarali yo'l — **texnik ingliz tili**, umumiy emas:
1. Shu bobdagi so'zlarni har kuni 10 tadan takrorlang — ular kodda doim uchraydi.
2. Kod o'qiganda nomlarni ovoz chiqarib tarjima qiling: `alloc_pages` → "sahifalarni ajrat".
3. Kompilyator xabarlarini tarjima jadvalisiz tushunishga harakat qiling.
4. Keyinchalik: Linux `Documentation/` dan qisqa hujjatlarni lug'at bilan o'qish.

Kuniga 20–30 daqiqa — REJA.md dagi kundalik tartibda shunga joy qoldirilgan.
